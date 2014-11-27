#ifndef __CMD_H
#define __CMD_H

#include <stdio.h>

#define HELP ": help, ping, roll"
#define ABOUT "I'm a simple IRC-bot written in C by bajr"
#define SOURCE "http://github.com/bajr"

int cmd_help(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_ping(int s, char *chan, int chanl, char *nick, int nickl);
int cmd_roll(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_join(irc_t *irc, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_part(irc_t *irc, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_quit(irc_t *irc, char *chan, int chanl, char *nick, int nickl);

#endif
