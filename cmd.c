#include "socket.h"
#include "irc.h"
#include "cmd.h"
#include <string.h>
#include <time.h>
#include <unistd.h>


int cmd_help(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char * msg = NULL;
  msg = malloc(nickl + strlen(HELP) + 1);
  strncpy(msg, nick, nickl + 1);
  msg[nickl] = 0;
  strncat(msg, HELP, strlen(HELP));

  if ( irc_msg(s, find_rts(chan, nick), msg) < 0)
    return -1;
  else
    return 0;
}


int cmd_ping(int s, char *chan, int chanl, char *nick, int nickl) {
  char * msg = NULL;
  msg = malloc(nickl + strlen(": pong") + 1);
  strncpy(msg, nick, nickl + 1);
  msg[nickl] = 0;
  strncat(msg, ": pong\0", strlen(": pong\0"));

  if ( irc_msg(s, find_rts(chan, nick), msg) < 0)
    return -1;
  else
    return 0;
}

int cmd_bajr(int s, char *chan, int chanl, char *nick, int nickl) {
  char * msg = NULL;
  msg = malloc(nickl + strlen(": bajrbajrbajr") + 2);
  strncpy(msg, nick, nickl + 1);
  msg[nickl] = 0;
  strncat(msg, ": bajrbajrbajr\0", strlen(": bajrbajrbajr\0"));

  if ( irc_msg(s, find_rts(chan, nick), msg) < 0 )
    return -1;
  else
    return 0;
}

int cmd_roll(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char tmpmsg[MSG_LEN], tmpstr[NICK_LEN], * msg = NULL;
  int num = -1, sides = -1, mod = 0, sum = 0;

  if ( arg != NULL) {
    sscanf(arg, "%id%s + %i", &num, tmpstr, &mod);
  }

  if (sscanf(tmpstr, "%d", &sides) == 0) {
    if (strcmp(tmpstr, "bajr") != 0) {
      strncpy(tmpmsg, nick, nickl);
      strcat(tmpmsg, ": ");
      if (arg != NULL)
        strncat(tmpmsg, arg, argl);
      strcat(tmpmsg, " is not a valid roll.");
    }
    else {
      strncpy(tmpmsg, nick, nickl);
      strcat(tmpmsg, ": ");
      strcat(tmpmsg, arg);
      strcat(tmpmsg, " = ");

      while (num > 0) {
        if (strlen(tmpmsg) < MSG_LEN / 2)
          strcat(tmpmsg, "bajr");
        --num;
      }
    }
  }
  else {
    if (num <= 0 || sides <= 0) {
      strncpy(tmpmsg, nick, nickl);
      strcat(tmpmsg, ": ");
      if (arg != NULL)
        strncat(tmpmsg, arg, argl);
      strcat(tmpmsg, " is not a valid roll.");
    }
    else {
      srand(time(NULL));
      strcpy(tmpmsg, nick);
      strcat(tmpmsg, ": ");
      strcat(tmpmsg, arg);
      strcat(tmpmsg, " = ");
    
      while (num > 0) {
        sum += rand() % sides + 1;
        --num;
      }
    
      sum += mod;
    
      sprintf(tmpstr, "%d", sum);
      strcat(tmpmsg, tmpstr);
    }
  }

  msg = malloc(strlen(tmpmsg));
  strcpy(msg, tmpmsg);

  if ( arg != NULL ); {
    free(arg);
  }

  if ( irc_msg(s, find_rts(chan, nick), msg) < 0 )
    return -1;
  else
    return 0;
}

// Return to sender
char * find_rts (char *chan, char *nick) {
  char * dest;

  if ( strncmp(chan, BOTNAME, strlen(BOTNAME)) == 0) {
    dest = nick;
    if ( nick != NULL)
      free(nick);
  }
  else
    dest = chan;

  return dest;
}
