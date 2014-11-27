#include "socket.h"
#include "irc.h"
#include "cmd.h"
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <regex.h>
#include <limits.h>
#include <ctype.h>

static int parse_roll (char *arg, unsigned long num[], unsigned long sides[], char oper[], char *modstr[], int * numsum);
static void roll_bajr (int s, char *chan, char *nick, int num);
static void roll_fudge (int s, char *chan, char *nick, int num);
static int roll_dice (int s, char *chan, char *nick, unsigned long num[], unsigned long sides[], char oper[], char *modstr[], int numsum);
static char* find_rts ( char *chan, char *nick);
static int make_msg ( char **msg, char tmpmsg[], const char *format, ...);
static char * trimStr (char *str, int strl);
static int find_highsum(long unsigned rolls[], int rolls_len, int num);
static int find_lowsum(long unsigned rolls[], int rolls_len, int num);

/******************************************************************************/

int cmd_help(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char * msg = NULL;
  int ret = 0;
  msg = malloc(nickl + strlen(HELP) + 1);
  strncpy(msg, nick, nickl + 1);
  msg[nickl] = 0;
  strncat(msg, HELP, strlen(HELP));

  if ( irc_msg(s, find_rts(chan, nick), msg) < 0)
    ret = -1;
  else
    ret = 0;

  if (msg != NULL)
    free(msg);

  return ret;
}


/******************************************************************************/

int cmd_ping(int s, char *chan, int chanl, char *nick, int nickl) {
  char * msg = NULL;
  int ret = 0;
  msg = malloc(nickl + strlen(": pong") + 1);
  strncpy(msg, nick, nickl + 1);
  msg[nickl] = 0;
  strncat(msg, ": pong\0", strlen(": pong\0"));

  if ( irc_msg(s, find_rts(chan, nick), msg) < 0)
    ret = -1;
  else
    ret = 0;

  if (msg != NULL)
    free(msg);

  return ret;
}

/******************************************************************************/

int cmd_roll(int s, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char tmpmsg[MSG_LEN] = {0};
  char oper[64] = {0}, *modstr[64] = {0};
  char *msg = NULL;
  int ret = 0, i = 0, n = 0, numsum = 0;
  unsigned long num[64] = {0}, sides[64] = {0};

  // Parse arg and extract meaningful output
  if ( arg != NULL)
    n = parse_roll (arg, num, sides, oper, modstr, &numsum);
  else
    return 0; // Nothing to roll

  for ( i = 0; oper[i] != 0 && i < 64; ++i ) {
    fprintf(stderr, "\tRoll: _%lud%lu_%c_%s_\n", num[i], sides[i], oper[i], modstr[i]);
  }

  // INVALID ROLL
  switch (n) {
    case 0:
      roll_dice(s, chan, nick, num, sides, oper, modstr, numsum);
      break;
    case 1:
      roll_bajr(s, chan, nick, num[0]);
      break;
    case 2:
      roll_fudge(s, chan, nick, num[0]);
      break;
    case -1: default:
      make_msg(&msg, tmpmsg, "%s: %s is not a valid roll.", nick, arg);
      ret = irc_msg(s, find_rts(chan, nick), msg);
  }

  if (msg != NULL) {
    free(msg);
    msg = NULL;
  }

  return ret;
}

/******************************************************************************/

int cmd_join(irc_t *irc, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char *ptr = NULL, * newchan = NULL, * newchankey = NULL;
  int ret = 0;

  if ( strcmp(nick, OWNER) != 0) {
    return irc_msg(irc->s, find_rts(chan, nick), "No.");
  }

  if ( arg == NULL) {
    ret = irc_msg(irc->s, chan, "Please specify a channel.");
  }
  else {
    ptr = strtok(arg, " ");
    newchan = malloc(strlen(ptr)+1);
    strcpy(newchan, ptr);

    ptr = strtok(NULL, " ");
    if ( ptr != NULL) {
      newchankey = malloc(strlen(ptr)+1);
      strcpy(newchankey, ptr);
    }

    if ( newchan[0] != '#' ) {
      return irc_msg(irc->s, chan, "That's not a channel");
    }

    if ( newchan != NULL) {
      ret = irc_join(irc, newchan);
    }
    else
      ret = 0;

    if (newchankey != NULL)
      free(newchankey);
  }
  return ret;
}

/******************************************************************************/

int cmd_part(irc_t *irc, char *chan, int chanl, char *nick, int nickl, char *arg, int argl) {
  char *ptr = NULL, * partchan = NULL;
  int len = 0, ret = 0;

  if ( strcmp(nick, OWNER) != 0) {
    return irc_msg(irc->s, find_rts(chan, nick), "No.");
  }

  if ( arg == NULL) {
    partchan = malloc (chanl+1);
    strncpy(partchan, chan, chanl);
    partchan[chanl] = '\0';
  }
  else {
    ptr = strtok(arg, " ");
    if (ptr == NULL) {
      partchan = malloc (chanl+1);
      strncpy(partchan, chan, chanl);
      partchan[chanl] = '\0';
    }
    else {
      len = strlen(ptr);
      partchan = malloc(len+1);
      strncpy(partchan, ptr, len);
      partchan[len] = '\0';
    }

    if ( partchan[0] != '#' ) {
      ret = irc_msg(irc->s, chan, "That's not a channel");

      if ( partchan != NULL)
        free(partchan);
      return ret;
    }
  }

  if ( irc_part(irc, partchan) < 0)
    ret = -1;
  else
    ret = 0;

  if ( partchan != NULL)
    free(partchan);

  return ret;
}

/******************************************************************************/

int cmd_quit(irc_t *irc, char *chan, int chanl, char *nick, int nickl) {
  if ( strcmp(nick, OWNER) != 0) {
    return irc_msg(irc->s, find_rts(chan, nick), "No.");
  }

  while (irc->chanlist != NULL)
    irc_part(irc, irc->chanlist->name);

  irc_quit(irc->s, "Keep rolling");

  return -1;
}

/******************************************************************************/

// Return to sender
char * find_rts (char *chan, char *nick) {
  char * dest;

  if ( strncmp(chan, BOTNAME, strlen(BOTNAME)) == 0) {
    dest = nick;
  }
  else
    dest = chan;

  return dest;
}

/******************************************************************************/

// Prepare an IRC message. Contains a malloc to be free'd in calling function
static int make_msg ( char **msg, char tmpmsg[], const char *format, ...) {
  int write_len = 0;
  va_list args;

  if ( format != NULL && strlen(format) != 0) {
    va_start(args, format);
    write_len = vsnprintf(tmpmsg, MSG_LEN, format, args);
    va_end(args);
  }

  *msg = malloc(strlen(tmpmsg)+1);
  strcpy(*msg, tmpmsg);

  return write_len;
}

/******************************************************************************/

// Parse Roll: Return value is number of tokens made
static int parse_roll (char *arg, unsigned long num[], unsigned long sides[], char oper[], char *modstr[], int * numsum) {
  int i = 0, j = 0, ioper = 0, ret = 0;
  char opers[] = "+-,";
  char *tmp = strdup(arg); // tmp will be divided up into substrings and stored in toks, remember to free this memory later.
  char *toks[64] = {0};
  char *ptr = NULL, *endptr = NULL;

  ptr = strtok(tmp, opers);

  while ( ptr != NULL && i < 64) {
    toks[i] = ptr;
    ioper += (strlen(ptr));
    oper[i] = arg[ioper];
    ++ioper;
    ++i;
    ptr = strtok(NULL, opers);
  }

  i = 0;

  while ( toks[i] != NULL && i < 64 ) {
    // Look for d in token. Perfectly ok if string is NULL
    ptr = strstr(toks[i], "d");

    if ( ptr == NULL ) { // d not found, use as mod of previous roll
        if ( j != 0)
          --j;
      if ( num[j] != 0 && sides[j] != 0 && modstr[j] == NULL) {
        ptr = strtok(toks[i], opers);
        ptr = trimStr(ptr, strlen(ptr));
        modstr[j] = malloc(strlen(ptr) + 1);
        strncpy(modstr[j], ptr, strlen(ptr)+1);
      }
      else {
        ret = -1;
        break;
      }
    }
    // d found, parse as dice roll
    else {
      ptr = strtok(toks[i], "d");

      if ( ptr == NULL || ptr > toks[i] ) {
        num[j] = 1;
        ++(*numsum);
      }
      else {
        ptr = trimStr(ptr, strlen(ptr));
        num[j] = strtoul(ptr, &endptr, 10);
        *numsum += num[j];

        // Error: Invalid number of rolls
        if ( num[j] == 0 || *numsum >= USHRT_MAX ) {
          ret = -1;
          break;
        }
      }

      if ( ptr == toks[i] )
        ptr = strtok(NULL, "");
      if ( ptr == NULL )
        sides[j] = 6;
      else {
        ptr = trimStr(ptr, strlen(ptr));

        if ( strcmp(ptr, "%") == 0 ) {
          sides[j] = 100;
        }
        // bajrs roll alone
        else if ( strcmp(ptr, "bajr") == 0 ) {
          if ( i != 0 ) {
            ret = -1;
            break;
          }
          else {
            ret = 1;
            break;
          }
        }
        // Have not yet decided if Fudge dice should be chainable
        else if ( strcmp(ptr, "F") == 0 || strcmp(ptr, "f") == 0 ) {
          if ( i != 0 ) {
            ret = -1;
            break;
          }
          else {
            ret = 2;
            break;
          }
        }
        else {
          sides[j] = strtoul(ptr, &endptr, 10);

          // Error: Invalid number of sides
          if ( sides[j] == 0 || sides[j] >= USHRT_MAX ) {
            ret = -1;
            break;
          }
        }
      }
    }

    ++i;
    ++j;
  }

  if (tmp != NULL) {
    free(tmp);
    tmp = NULL;
  }

  return ret;
}

/******************************************************************************/

// Trim leading and trailing whitespace from a string
static char * trimStr (char *str, int strl) {
  char * end = str + strl - 1;

  // Trim whitespace
  while ( str < end && isspace(*str) ) ++str;
  if ( *str == 0 ) // All whitespace
    return str;

  while ( end > str && isspace(*end) ) --end;
  *(end+1) = 0;

  return str;
}

/******************************************************************************/

// Roll bajrs
static void roll_bajr (int s, char *chan, char *nick, int num) {
  char tmpmsg[MSG_LEN] = {0}, tmpbajr[257] = {0};
  char *msg = 0;
  int i;

  if ( num > 64) {
    make_msg(&msg, tmpmsg, "%s: I only have 64 bajrs...", nick);
    irc_msg(s, find_rts(chan, nick), msg);

    if (msg != NULL) {
      free(msg);
      msg = NULL;
    }

    for ( i = 0; tmpmsg[i] != '\0' && i < MSG_LEN; ++i )
      tmpmsg[i] = '\0';
    num = 64;
  }

  i = 0;
  while (i < num) {
    strcat(tmpbajr, "bajr");
    ++i;
  }

  make_msg(&msg, tmpmsg, "%s: %ddbajr = %s", nick, num, tmpbajr);
  irc_msg(s, find_rts(chan, nick), msg);

  if (msg != NULL) {
    free(msg);
    msg = NULL;
  }
}

/******************************************************************************/

// Fudge dice.
static void roll_fudge (int s, char *chan, char *nick, int num) {
  int i = num, sum = 0, rnd = 0, sides = 3;
  char tmpmsg[MSG_LEN] = {0}, *msg = 0;

  while (i > 0) {
    rnd = rand() % sides - 1;
    sum += rnd;
    --i;
  }

  make_msg(&msg, tmpmsg, "%s: %ddF = %d", nick, num, sum);
  irc_msg(s, find_rts(chan, nick), msg);

  if (msg != NULL) {
    free(msg);
    msg = NULL;
  }
}

/******************************************************************************/

// Roll dice.
static int roll_dice (int s, char *chan, char *nick, unsigned long num[], unsigned long sides[], char oper[], char *modstr[], int numsum) {
  int i = 0, j = 0, k = 0, ioper = 0, len = 0, sum = 0, mod = 0, modsum = 0, tmp = 0, ret = 0;
  char tmpmsg[MSG_LEN] = {0}, *msg = 0, *endptr = NULL;
  unsigned long rolls[numsum];

  srand(time(NULL));

  if ( numsum < 16 ) {
    len += strlen(nick) + 2;
    strncpy(tmpmsg, nick, len);
    strncat(tmpmsg, ": ", 3);
  }

  for ( i = 0; num[i] != 0 && i < 64; ++i ) {
    // Next Operator applies to next roll...
    if ( i != 0 && numsum < 16 ) {
      switch (oper[ioper]) {
       case '+':
         len += snprintf(&tmpmsg[len], MSG_LEN - len, " + ");
         break;
       case '-':
         len += snprintf(&tmpmsg[len], MSG_LEN - len, " - ");
         break;
       case ',':
         len += snprintf(&tmpmsg[len], MSG_LEN - len, "; ");
         break;
       default:
         break;
      }
      ++ioper;
    }

    for ( j = 0; j < num[i] && j < numsum; ++j ) {
      rolls[j] = rand() % sides[i] + 1;
      sum += rolls[j];
    }

    // Find mod (1d6 == 1d6+0)
    if ( modstr[i] != NULL ) {
      mod = strtoul(modstr[i], &endptr, 10);

      if ( *endptr != 0 && mod > 0 && mod < num[i] ) {
        if ( toupper(*endptr) == 'H' ) {
          modsum = find_highsum(rolls, j, mod);
        }
        else if ( toupper(*endptr) == 'L' ) {
          modsum = find_lowsum(rolls, j, mod);
        }
      }
      else if ( mod > 0 && mod < USHRT_MAX ) {
        modsum = mod;
      }
      else {
        // bad mod
      }
    }
    else {
      // use operator on next roll
    }

    // Compose message to display dice rolls
    if ( numsum < 16 ) {
      ++len;
      strncat(tmpmsg, "(", 2);
      for ( k = 0; k < j; ++k ) {
        if ( k != 0 ) {
          ++len;
          strncat(tmpmsg, ",", 2);
        }
        len += snprintf(&tmpmsg[len], MSG_LEN - len, "%lu", rolls[k]);
      }
      ++len;
      strncat(tmpmsg, ")", 2);

      if ( modstr[i] != NULL ) {
        // Apply mod to previous roll
        switch (oper[ioper]) {
          case '+':
            if ( toupper(*endptr) == 'H' || toupper(*endptr) == 'L' )
              len += snprintf(&tmpmsg[len], MSG_LEN - len, "+%d%c ", mod, *endptr);
            else
              len += snprintf(&tmpmsg[len], MSG_LEN - len, "+%d ", mod);
            break;
          case '-':
            if ( toupper(*endptr) == 'H' || toupper(*endptr) == 'L' )
              len += snprintf(&tmpmsg[len], MSG_LEN - len, "-%d%c ", mod, *endptr);
            else
              len += snprintf(&tmpmsg[len], MSG_LEN - len, "-%d ", mod);
            break;
          case ',':
            len += snprintf(&tmpmsg[len], MSG_LEN - len, "; ");
            break;
          default:
            break;
        }
        ++ioper;
      }
    }

    switch (oper[ioper]) {
      case '+':
        sum += modsum;
        break;
      case '-':
        sum -= modsum;
        break;
      case ',': default:
        tmp = sum;
        sum = 0;
    }
  }

  // Display roll
  if ( numsum < 16 ) {
    make_msg(&msg, tmpmsg, NULL, NULL);
    ret = irc_msg(s, find_rts(chan, nick), msg);
  }
  else {
    //make_msg(&msg, tmpmsg, , );
    //ret = irc_msg(s, find_rta(chan, nick), msg);
  }

  for ( i = 0; i < 64; ++i ) {
    if ( modstr[i] != NULL )
      free(modstr[i]);
    modstr[i] = 0;
  }

  if (msg != NULL) {
    free(msg);
    msg = NULL;
  }

  return ret;
}

/******************************************************************************/

static int find_highsum(long unsigned rolls[], int rolls_len, int num) {
  int i = 0, k = 0, index = 0, n = 0, sum = 0;
  int high[num];

  for ( i = 0; i < rolls_len; ++i ) {
    n = rolls[i];
    for ( k = 0; k < num; ++k ) {
      if ( high[k] == 0 || n > high[k] ) {
        index = k;
        break;
      }
    }
    if ( index >= 0 && index < num ) {
      high[index] = n;
      index = -1;
    }
  }

  for ( i = 0; i < num; ++i ) {
    sum += high[i];
  }

  return sum;
}

/******************************************************************************/

static int find_lowsum(long unsigned rolls[], int rolls_len, int num) {
  int i = 0, k = 0, index = 0, n = 0, sum = 0;
  int low[num];

  for ( i = 0; i < rolls_len; ++i ) {
    n = rolls[i];
    for ( k = 0; k < num; ++k ) {
      if ( low[k] == 0 || n < low[k] ) {
        index = k;
        break;
      }
    }
    if ( index >= 0 && index < num ) {
      low[index] = n;
      index = -1;
    }
  }

  for ( i = 0; i < num; ++i ) {
    sum += low[i];
  }

  return sum;
}

/******************************************************************************/
