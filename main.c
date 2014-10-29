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

#define OUTPUT "/dev/stdout"
#define CONFIG "config"

#include "socket.h"
#include "irc.h"
#include <string.h>

char *getln(FILE *conf);

int main(int argc, char **argv) {
  FILE * conf;
  conf = fopen(CONFIG, "r");

  irc_t irc;
  irc_set_output(&irc, OUTPUT);

  char *servname = NULL, *servport = NULL, *line = NULL;
  line = getln(conf);

  servname = strtok(line, " :");
  servport = strtok(NULL, " :");

  if ( irc_connect(&irc, servname, servport) < 0 ) {
    fprintf(stderr, "Connection failed.\n");
    goto exit_err;
  }
  else {
    if ( irc_login(&irc, "bajrbot") < 0 ) {
      fprintf(stderr, "Couldn't log in.\n");
      goto exit_err;
    }

    if ( irc_join_channel(&irc, "#tabletop") < 0 ) {
      fprintf(stderr, "Couldn't join channel.\n");
      goto exit_err;
    }

    while ( irc_handle_data(&irc) >= 0 );

    irc_close(&irc);
    fclose(conf);
    return 0;
  }
exit_err:
  if ( irc.s >= 0) {
    irc_close(&irc);
  }
  fclose(conf);
  fprintf(stderr,"That's rough, buddy.\n");
  exit(1);
}

// This method is kind of cludgy for making dynamic strings
// Does not cut whitespace
char *getln(FILE *file) {
  char *line = NULL, *tmp = NULL;
  size_t size = 0, index = 0;
  int ch = EOF;
  while (ch) {
    ch = getc(file);
    /* Check if we need to stop. */
    if (ch == EOF || ch == '\n')
      ch = 0;
    /* Check if we need to expand. */
    if (size <= index) {
      size += sizeof(char);
      tmp = realloc(line, size);
      if (!tmp) {
        free(line);
        line = NULL;
        break;
      }
      line = tmp;
    }
    line[index++] = ch;
  }
  return line;
}
