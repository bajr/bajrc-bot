#ifndef __IRC_H
#define __IRC_H

#include <stdio.h>

#define BOTNAME "bajrbot"
#define OWNER "bajr"

typedef struct chan {
  char * name;
  struct chan * next;
} chan;

typedef struct {
   int s;
   FILE * file;
   chan * chanlist;
   char * nick;
   char servbuf[MSG_LEN];
   int bufptr;
} irc_t;

int irc_connect(irc_t *irc, const char* server, const char* port);
int irc_login(irc_t *irc, const char* nick);
int irc_handle_data(irc_t *irc);
int irc_set_output(irc_t *irc, const char* file);
void irc_close(irc_t *irc);

int irc_join(irc_t *irc, char *data);
int irc_part(irc_t *irc, char *data);
int irc_msg(int s, char *channel, char *data);
int irc_quit(int s, char *data);

#endif
