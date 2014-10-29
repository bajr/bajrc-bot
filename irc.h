#ifndef __IRC_H
#define __IRC_H

#include <stdio.h>

typedef struct
{
   int s;
   FILE *file;
   char channel[256]; // Why 256? Check RFC
   char *nick;
   char servbuf[512]; // Why 512? Check RFC
   int bufptr;
} irc_t; 

int irc_connect(irc_t *irc, const char* server, const char* port); //
int irc_login(irc_t *irc, const char* nick); // 
int irc_join_channel(irc_t *irc, const char* channel); //
int irc_handle_data(irc_t *irc); //
int irc_set_output(irc_t *irc, const char* file); //
void irc_close(irc_t *irc); //

#endif
