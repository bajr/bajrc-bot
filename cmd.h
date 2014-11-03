#ifndef __CMD_H
#define __CMD_H

#include <stdio.h>

#define HELP ": help, ping, roll, join, part, quit"

int cmd_help(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_ping(int s, char *chan, int chanl, char *nick, int nickl);
int cmd_roll(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_join(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_part(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl);
int cmd_quit(int s, char *chan, int chanl, char *nick, int nickl);

char* find_rts (char *chan, char *nick);
int make_msg ( char tmpmsg[], const char *format, ...);
#endif
