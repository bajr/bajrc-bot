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
  char * msg = NULL;
  int num = 0, sides = 0;
  sscanf(arg, "%id%i", &num, &sides);

  if (num <= 0 || sides <= 0) {
    int errlen = strlen(":  is not a valid roll.");
    msg = malloc(nickl + argl + errlen + 1);
    strncpy(msg, nick, nickl);
    msg[nickl] = 0;
    strcat(msg, ": ");
    strcat(msg, arg);
    strncat(msg, " is not a valid roll.", errlen);
  }
  else {
    msg = malloc(nickl + strlen(": Rolling ") + argl + 1);
    strncpy(msg, nick, nickl);
    msg[nickl] = 0;
    strncat(msg, ": Rolling ", strlen(": Rolling "));
    strncat(msg, arg, argl);
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
  }
  else
    dest = chan;

  return dest;
}
