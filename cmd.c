#include "socket.h"
#include "irc.h"
#include "cmd.h"
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>


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

int cmd_roll(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char tmpmsg[MSG_LEN] = {0}, tmpstr[NICK_LEN] = {0}, numstr[NICK_LEN] = {0}, \
       sidestr[NICK_LEN] = {0}, modstr[NICK_LEN] = {0};
  char *msg = NULL, *ptr1 = NULL, *ptr2 = NULL;
  char oper = 0;
  int i = 1, rnd = 0, num = -1, sides = -1, mod = 0, low = 0, high = 0, sum = 0;

  if ( arg != NULL) {
    ptr1 = strpbrk(arg, "d");
    ptr2 = strpbrk(arg, "-+*x/");

    if (ptr1 == NULL) { // No d, invalid roll
      strcpy(numstr, arg);
    }
    else { // Contains d, potentially valid
      strncpy(numstr, arg, ptr1 - arg);
      if (ptr2 == NULL) { // Does not have a modifier, valid
        strcpy(sidestr, ptr1 + 1);
      }
      else if (ptr2 < ptr1) { // Operator found before d, invalid
        strcpy(numstr, arg);
      }
      else { // Operator found after d, valid
        i = 1;
        while ( (ptr2-i) != ptr1 && *(ptr2 - i) == ' ' ) // Cut trailing whitespace
          ++i;
        strncpy(sidestr, ptr1+1, ptr2 - i - ptr1);
        oper = *ptr2;
        i = 1;
        while ( *(ptr2 + i) == ' ') // Cut leading whitespace
          ++i;
        strcpy(modstr, ptr2 + i);
      }
    }
  }
  else {
    return 0;
  }

  fprintf(stderr, "ARG:_%s_\n_%sd%s.%c.%s.\n", arg, numstr, sidestr, oper, modstr);

  // Find num (d6 == 1d6)
  if ( strcmp(numstr, "") == 0)
    num = 1;
  else
    sscanf(numstr, "%d", &num);

  // Find sides (1d == 1d6)
  if ( strcmp(sidestr, "") == 0)
    sides = 6;
  else
    sscanf(sidestr, "%d", &sides);

  // Are we rolling a bajr?
  if (strcmp(sidestr, "bajr") == 0) {
    make_msg(tmpmsg, "%s: %s = ", nick, arg);

    while (num > 0) {
      if (strlen(tmpmsg) < MSG_LEN / 2)
        strcat(tmpmsg, "bajr");
      --num;
    }
  }
  // Are we rolling bad dice?
  else if (num <= 0 || sides <= 0) {
    make_msg(tmpmsg, "%s: %s is not a valid roll.", nick, arg);
  }
  // Everything checks out. Let's roll
  else if (num > 0 && sides > 0){
    srand(time(NULL));
    i = num;
    low = sides + 1;
    while (i > 0) {
      rnd = rand() % sides + 1;
      if (rnd > high)
        high = rnd;
      if (rnd < low)
        low = rnd;
      sum += rnd;
      --i;
    }

    // Find mod (1d6 == 1d6+0)
    if ( strcmp(modstr, "") == 0) {
      oper = 0;
      mod = 0;
    }
    else if ( strcmp(modstr, "L") == 0) {
      mod = low;
    }
    else if ( strcmp(modstr, "H") == 0) {
      mod = high;
    }
    else {
      sscanf(modstr, "%d", &mod);
    }

    switch (oper) {
      case '+': sum += mod; break;
      case '-': sum -= mod; break;
      case '*': case 'x': sum *= mod; break;
      case '/': sum /= mod; break;
      case 0: default: break;
    }


    if ( oper )
      make_msg(tmpmsg, "%s: %dd%d%c%d = %d", nick, num, sides, oper, mod, sum);
    else
      make_msg(tmpmsg, "%s: %dd%d = %d", nick, num, sides, sum);
  }
  else
    make_msg(tmpmsg, "%s: %s is not a valid roll.", nick, arg);

  msg = malloc(strlen(tmpmsg));
  strcpy(msg, tmpmsg);

  if ( irc_msg(s, find_rts(chan, nick), msg) < 0 )
    return -1;
  else
    return 0;
}

int cmd_join(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char *ptr = NULL, * newchan = NULL, * newchankey = NULL;
  
  if ( strcmp(nick, OWNER) != 0) {
    return irc_msg(s, find_rts(chan, nick), "You're not the boss of me!");
  }

  if ( arg == NULL) {
    return irc_msg(s, chan, "Please specify a channel.");
  }
  else {
    ptr = strtok(arg, " ");
    newchan = malloc(strlen(ptr)+1);
    strcpy(newchan, ptr);

    ptr = strtok(NULL, " ");
    if ( ptr != NULL) {
      newchankey = malloc(strlen(ptr)+1);
      strcpy(newchankey, ptr);
    }

    if ( newchan[0] != '#' ) {
      return irc_msg(s, chan, "That's not a channel");
    }

    if ( irc_join(s, newchan) < 0)
      return -1;
    else
      return 0;
  }
}

int cmd_part(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {

}

int cmd_quit(int s, char *chan, int chanl, char *nick, int nickl) {

}

// Return to sender
char * find_rts (char *chan, char *nick) {
  char * dest;

  if ( strncmp(chan, BOTNAME, strlen(BOTNAME)) == 0) {
    dest = nick;
  }
  else
    dest = chan;

  return dest;
}

int make_msg ( char tmpmsg[], const char *format, ...) {
  va_list args;
  int write_len = 0;

  if (strlen(format) != 0) {
    va_start(args, format);
    write_len = vsnprintf(tmpmsg, MSG_LEN, format, args);
    va_end(args);
  }

  return write_len;
}

