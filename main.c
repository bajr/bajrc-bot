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

exit_err:
  if ( irc.s >= 0) {
    irc_close(&irc);
  }
  fclose(conf);
  fprintf(stderr,"Fatal error occurred.\n");
  exit(1);
}



void init_conf (irc_t * irc, FILE * conf) {
  char *servname = NULL, *servport = NULL, *line = NULL, *tok = NULL, ch = 0;
  int state = 0;
  chan * link = NULL;

  do {
    line = getln(conf);

    if (line != NULL)
      tok = strtok(line, " :");
    if (tok != NULL)
      sscanf(tok, "%c", &ch);

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
    }
    else if (state == 1 && tok != NULL && ch == '#') {
      link = irc->chanlist;
      irc->chanlist = malloc(sizeof(chan));
      irc->chanlist->name = malloc(strlen(tok)+1);
      strcpy(irc->chanlist->name, tok);
      irc->chanlist->next = link;
    }
    if (line != NULL)
      free(line);
  } while (line != NULL);

  if ( irc_login(irc, BOTNAME) < 0 ) {
    fprintf(stderr, "Couldn't log in.\n");
  }

  link = irc->chanlist;
  while (link != NULL) {
    if ( irc_join_channel(irc, link->name) < 0 ) {
      fprintf(stderr, "Couldn't join channel.\n");
    }
    link = link->next;
  }
}

// This method is kind of cludgy for making dynamic strings
// Does not cut whitespace
char* getln(FILE *file) {
  char *line = NULL, tmp[MSG_LEN];
  size_t size = 0, index = 0;
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
