#ifndef __IRC_H
#define __IRC_H

#include <stdio.h>

#define NICK_LEN 32
#define CHAN_LEN 256
#define MSG_LEN 512
#define BOTNAME "bajrbot"

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
int irc_join_channel(irc_t *irc, const char* channel);
int irc_handle_data(irc_t *irc);
int irc_set_output(irc_t *irc, const char* file);
void irc_close(irc_t *irc);

int irc_msg(int s, char *channel, char *data);

#endif
