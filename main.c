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
#include "cmd.h"
#include <string.h>

#define OUTPUT "/dev/stdout"
#define CONFIG "config"

char *getln(FILE *conf);
void init_conf(irc_t *irc, FILE * conf);

int main(int argc, char **argv) {
  FILE * conf;
  conf = fopen(CONFIG, "r");

  irc_t irc;
  irc_set_output(&irc, OUTPUT);
  irc.chanlist = NULL;

  init_conf(&irc, conf);

  while ( irc_handle_data(&irc) >= 0 );

  irc_close(&irc);
  fclose(conf);
  return 0;
}



void init_conf (irc_t * irc, FILE * conf) {
  char *servname = NULL, *servport = NULL, *line = NULL, *tok = NULL, ch = 0;
  int state = 0;

  do {
    line = getln(conf);
    ch = 0;
    tok = NULL;

    if (line != NULL)
      tok = strtok(line, " :");
    if (tok != NULL) {
      if (strcmp(tok, "") == 0)
        tok = NULL;
      if (tok != NULL)
        ch = tok[0];
    }

    if (ch == '{') {
      state = 1;
    }
    else if (ch == '}') {
      state = 0;
    }
    else if (state == 0 && tok != NULL) {
      servname = tok;
      servport = strtok(NULL, " :");
      if ( irc_connect(irc, servname, servport) < 0 ) {
        fprintf(stderr, "Connection to %s:%sfailed.\n", servname, servport);
      }
      else {
        if ( irc_login(irc, BOTNAME) < 0 ) {
          fprintf(stderr, "Couldn't log in.\n");
        }
      }
    }
    else if (state == 1 && ch == '#') {
      char *chan = malloc(strlen(tok)+1);
      strcpy(chan, tok);
      irc_join(irc, chan);
    }

    if (line != NULL) {
      free(line);
    }
  } while (line != NULL);
}

// This method is kind of cludgy for making dynamic strings
// Does not cut whitespace
char* getln(FILE *file) {
  char *line = NULL, tmp[MSG_LEN];
  size_t index = 0;
  int ch = EOF;

  while (ch) {
    ch = getc(file);
    // Check if we need to stop.
    if (ch == EOF)
      return NULL;
    if (ch == '\n')
      ch = 0;

    tmp[index++] = ch;
  }

  line = malloc(strlen(tmp)+1);
  strcpy(line, tmp);
  return line;
}
