#include "socket.h"
#include "irc.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

static int irc_parse_action(irc_t *irc);
static int irc_leave_channel(irc_t *irc, const char* channel); 
static int irc_log_message(irc_t *irc, const char* channel, const char *nick, const char* msg);
static int irc_reply_message(irc_t *irc, const char* channel, char *nick, char* msg);
static int irc_pong(int s, const char *data);
static int irc_reg(int s, const char *nick, const char *username, const char *fullname);
static int irc_join(int s, const char *data);
static int irc_part(int s, const char *data);
static int irc_nick(int s, const char *data);
static int irc_quit(int s, const char *data);
static int irc_topic(int s, const char *channel, const char *data);
static int irc_action(int s, const char *channel, const char *data);
static int irc_msg(int s, const char *channel, const char *data);

int irc_connect(irc_t *irc, const char* server, const char* port) {
   if ( (irc->s =  get_socket(server, port)) < 0 ) {
      return -1;
   }
   irc->bufptr = 0;
   return 0;
}

int irc_login(irc_t *irc, const char* nick) {
   return irc_reg(irc->s, nick, "bajr", "bajrbajr");
}

int irc_join_channel(irc_t *irc, const char* channel) {
   return irc_join(irc->s, channel);
}

int irc_handle_data(irc_t *irc) {
   char tempbuffer[MSG_LEN];
   int rc, i;

   if ( (rc = sck_recv(irc->s, tempbuffer, sizeof(tempbuffer) - 2 ) ) <= 0 ) {
      fprintf(stderr, ":v\n");
      return -1;
   }
   tempbuffer[rc] = '\0';
   for ( i = 0; i < rc; ++i ) {
      switch (tempbuffer[i]) {
         case '\r':
         case '\n': {
            irc->servbuf[irc->bufptr] = '\0';
            irc->bufptr = 0;
            if ( irc_parse_action(irc) < 0 )
               return -1;
            break;
         }
         default: {
            irc->servbuf[irc->bufptr] = tempbuffer[i];
            if ( irc->bufptr >= (sizeof ( irc->servbuf ) -1 ) ); // Overflow!
            else
               irc->bufptr++;
         }
      }
   }
   return 0;
}

int irc_set_output(irc_t *irc, const char* file) {
   irc->file = fopen(file, "w");
   if ( irc->file == NULL )
      return -1;
   return 0;
}

void irc_close(irc_t *irc) {
   close(irc->s);
   fclose(irc->file);
}

static int irc_leave_channel(irc_t *irc, const char* channel) {
   return irc_part(irc->s, channel);
}

static int irc_parse_action(irc_t *irc) {
  //fprintf(stdout, "%s\n", irc->servbuf); // For debugging
  if ( strncmp(irc->servbuf, "PING :", 6) == 0 ) {
    return irc_pong(irc->s, &irc->servbuf[6]);
  } else if ( strncmp(irc->servbuf, "NOTICE AUTH :", 13) == 0 ) {
    // Don't care
    return 0;
  } else if ( strncmp(irc->servbuf, "ERROR :", 7) == 0 ) {
    // Still don't care
    return 0;
  } else {
  // Parses IRC message that pulls out nick and message. 
    char *ptr;
    int privmsg = 0;
    char irc_nick[NICK_LEN];
    char irc_msg[MSG_LEN];
    char irc_chan[CHAN_LEN];
    *irc_nick = '\0';
    *irc_msg = '\0';

    // Check for non-message string
    if ( strchr(irc->servbuf, 1) != NULL )
      return 0;

    if ( irc->servbuf[0] == ':' ) {
      ptr = strtok(irc->servbuf, "!");
      if ( ptr == NULL ) {
        printf("ptr == NULL\n");
        return 0;
      } else {
        strncpy(irc_nick, &ptr[1], 127);
        irc_nick[127] = '\0';
      }

      while ( (ptr = strtok(NULL, " ")) != NULL ) {
        if ( strcmp(ptr, "PRIVMSG") == 0 ) {
          privmsg = 1;
          break;
        }
      }
      if ( privmsg ) {
        if ( (ptr = strtok(NULL, ":")) != NULL && (ptr = strtok(NULL, "")) != NULL ) {
          strncpy(irc_msg, ptr, 511);
          irc_msg[511] = '\0';
        }
      }
      if ( privmsg == 1 && strlen(irc_nick) > 0 && strlen(irc_msg) > 0 ) {
        irc_log_message(irc, irc_chan, irc_nick, irc_msg);
        if ( irc_reply_message(irc, irc_chan, irc_nick, irc_msg) < 0 )
          return -1;
      }
    }
  }
  return 0;
}

static int irc_reply_message(irc_t *irc, const char* channel, char *irc_nick, char *msg) {
   // Checks if someone calls on the bot.
   if ( *msg != '!' )
      return 0;

   char *command;
   char *arg;
   // Gets command
   command = strtok(&msg[1], " ");
   arg = strtok(NULL, "");
  if ( arg != NULL )
    while ( *arg == ' ' )
      arg++;
  if ( command == NULL )
    return 0;

  if ( strcmp(command, "ping") == 0) {
    //return cmd_ping;
    if ( irc_msg(irc->s, channel, strcat(irc_nick, ": pong")) < 0)
      return -1;
  } 
  else if ( strcmp(command, "bajr") == 0 ) {
    if ( irc_msg(irc->s, channel, strcat(irc_nick, ": bajrbajrbajr")) < 0 )
      return -1;
  } 
  return 0;
}

static int irc_log_message(irc_t *irc, const char* channel, const char* nick, const char* message) {
  char timestring[128];
  time_t curtime;
  time(&curtime);
  strftime(timestring, 127, "%F - %H:%M:%S", localtime(&curtime));
  timestring[127] = '\0';

  fprintf(irc->file, "%s - [%s] <%s> %s\n", channel, timestring, nick, message);
  fflush(irc->file);
  return 0;
}

// irc_pong: For answering pong requests...
static int irc_pong(int s, const char *data) {
   return sck_sendf(s, "PONG :%s\r\n", data);
}

// irc_reg: For registering upon login
static int irc_reg(int s, const char *nick, const char *username, const char *fullname) {
   return sck_sendf(s, "NICK %s\r\nUSER %s localhost 0 :%s\r\n", nick, username, fullname);
}

// irc_join: For joining a channel
static int irc_join(int s, const char *data) {
   return sck_sendf(s, "JOIN %s\r\n", data);

}

// irc_part: For leaving a channel
static int irc_part(int s, const char *data) {
   return sck_sendf(s, "PART %s\r\n", data);

}

// irc_nick: For changing your nick
static int irc_nick(int s, const char *data) {
   return sck_sendf(s, "NICK %s\r\n", data);

}

// irc_quit: For quitting IRC
static int irc_quit(int s, const char *data) {
   return sck_sendf(s, "QUIT :%s\r\n", data);
}

// irc_topic: For setting or removing a topic
static int irc_topic(int s, const char *channel, const char *data) {
   return sck_sendf(s, "TOPIC %s :%s\r\n", channel, data);
}

// irc_action: For executing an action (.e.g /me is hungry)
static int irc_action(int s, const char *channel, const char *data) {
   return sck_sendf(s, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
}

// irc_msg: For sending a channel message or a query
static int irc_msg(int s, const char *channel, const char *data) {
   return sck_sendf(s, "PRIVMSG %s :%s\r\n", channel, data);
}

