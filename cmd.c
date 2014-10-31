#include "socket.h"
#include "irc.h"
#include "cmd.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

int cmd_ping (int s, const char *chan, const char *nick) { // check chan length first
  char * msg = NULL;
  msg = malloc(strlen(nick) + strlen(": pong") + 2);
  strncpy(msg, nick, strlen(nick));
  msg[strlen(nick)] = 0;
  strncat(msg, ": pong\0", strlen(": pong\0"));
  if ( strncmp(chan, BOTNAME, strlen(BOTNAME)) == 0) {
    chan = nick;
  }

  if ( irc_msg(s, chan, msg) < 0)
    return -1;
  else
    return 0;
}
