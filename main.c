#include "socket.h"
#include "irc.h"

int main(int argc, char **argv)
{
   parse_args(argc, argv);

   irc_t *irc;

   if ( irc_connect(&irc, server, port) < 0 )
   {
      fprintf(stderr, "Connection failed.\n");
      goto exit_err;
   }

   if ( irc_login(irc, name, channel) < 0 )
   {
      fprintf(stderr, "Couldn't log in.\n");
      goto exit_err;
   }

   while ( irc_handle_data(irc) >= 0 );

   irc_close(irc);
   return 0;

exit_err:
   irc_close(irc);
   exit(1);
}
