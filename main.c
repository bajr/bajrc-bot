/* Bradley Rasmussen
 *
 * forked from maister for educational purposes
 * 
 * TO DO:
 * Learn backend magic
 * See what it would take to make the bot modular. Multiple servers & channels
 * Add functionality
 * Make code more "secure"
 */

#include "socket.h"
#include "irc.h"

int main(int argc, char **argv) {
  irc_t irc;

  if ( irc_connect(&irc, "irc.cat.pdx.edu", "6697") < 0 ) {
    fprintf(stderr, "Connection failed.\n");
    goto exit_err;
  }
  else {
    irc_set_output(&irc, "/dev/stdout");

    if ( irc_login(&irc, "1bajrbot") < 0 ) {
      fprintf(stderr, "Couldn't log in.\n");
      goto exit_err;
    }

    if ( irc_join_channel(&irc, "#tabletop") < 0 ) {
      fprintf(stderr, "Couldn't join channel.\n");
      goto exit_err;
    }

    while ( irc_handle_data(&irc) >= 0 );

    irc_close(&irc);
    return 0;
  }
exit_err:
  if ( irc.s >= 0) {
    irc_close(&irc);
  }
  fprintf(stderr,"That's rough, buddy.\n");
  exit(1);
}

