#include "socket.h"
#include "irc.h"
#include "cmd.h"
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

static int parse_roll ( char *arg, char numstr[], char sidestr[], char *oper, char modstr[]);
static void roll_bajr (int s, char *chan, char *nick, int num);
static void roll_fudge (int s, char *chan, char *nick, int num);
static int roll_dice (int s, char *chan, char *nick, int num, int sides, char oper, char *modstr);
static char* find_rts ( char *chan, char *nick);
static int make_msg ( char **msg, char tmpmsg[], const char *format, ...);

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
  char tmpmsg[MSG_LEN] = {0}, numstr[NICK_LEN] = {0}, sidestr[NICK_LEN] = {0}, modstr[NICK_LEN] = {0};
  char *msg = NULL;
  char oper = 0;
  unsigned int num = 0, sides = 0;
  int ret = 0;


  // Parse Roll: ptr1, ptr2, arg, numstr, sidestr, oper, modstr, i
  if ( arg != NULL) {
    parse_roll (arg, numstr, sidestr, &oper, modstr);
  }
  else {
    return 0;
  }

  // Find num (d6 == 1d6)
  if ( strcmp(numstr, "") == 0)
    num = 1;
  else if ( strlen(numstr) > 8) {
    make_msg(&msg, tmpmsg, "I don't have %s dice!", numstr);
    num = 0;
  }
  else
    sscanf(numstr, "%d", &num);

  // Find sides (1d == 1d6)
  if ( strcmp(sidestr, "") == 0)
    sides = 6;
  else if ( strcmp(sidestr, "%") == 0)
    sides = 100;
  else if ( strcmp(sidestr, "bajr") == 0)
    sides = 1;
  else if ( strcmp(sidestr, "F") == 0 || strcmp(sidestr, "f") == 0)
    sides = 3;
  else if ( strlen(sidestr) > 8) {
    make_msg(&msg, tmpmsg, "I don't have that kind of die.");
    irc_msg(s, find_rts(chan, nick), msg);
    sides = 0;
  }
  else
    sscanf(sidestr, "%d", &sides);

  // Are we rolling bad dice?
  if (num == 0 || sides == 0) {
    make_msg(&msg, tmpmsg, "%s: %s is not a valid roll.", nick, arg);
    irc_msg(s, find_rts(chan, nick), msg);
  }
  // Are we rolling bajrs? : tmpmsg?
  else if (strcmp(sidestr, "bajr") == 0) {
    roll_bajr (s, chan, nick, num);
  }
  // Are these Fudge dice? : s, num, chan, nick
  else if ( strcmp(sidestr, "F") == 0 || strcmp(sidestr, "f") == 0) {
    roll_fudge(s, chan, nick, num);
  }
  // Everything checks out. Let's roll
  else if (num > 0 && sides > 0){
    ret = roll_dice(s, chan, nick, num, sides, oper, modstr);
  }
  else {
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

  if (strlen(format) != 0) {
    va_start(args, format);
    write_len = vsnprintf(tmpmsg, MSG_LEN, format, args);
    va_end(args);
  }

  *msg = malloc(strlen(tmpmsg)+1);
  strcpy(*msg, tmpmsg);

  return write_len;
}

/******************************************************************************/

// Parse Roll: arg, numstr, sidestr, oper, modstr
static int parse_roll (char *arg, char numstr[], char sidestr[], char *oper, char modstr[]) {
  char *ptr1 = 0, *ptr2 = 0;
  int i = 1, ret = 0;

  ptr1 = strpbrk(arg, "d");
  ptr2 = strpbrk(arg, "-+*x/");

  if (ptr1 == NULL) { // No d, invalid roll
    strcpy(numstr, arg);
    ret = 0;
  }
  else { // Contains d, potentially valid
    strncpy(numstr, arg, ptr1 - arg);
    if (ptr2 == NULL) { // Does not have a modifier, valid
      strcpy(sidestr, ptr1 + 1);
      ret = 1;
    }
    else if (ptr2 < ptr1) { // Operator found before d, invalid
      strcpy(numstr, arg);
      ret = 0;
    }
    else { // Operator found after d, valid
      i = 1;
      while ( (ptr2-i) != ptr1 && *(ptr2 - i) == ' ' ) // Cut trailing whitespace
        ++i;
      strncpy(sidestr, ptr1+1, ptr2 - i - ptr1);
      *oper = *ptr2;
      i = 1;
      while ( *(ptr2 + i) == ' ') // Cut leading whitespace
        ++i;
      strcpy(modstr, ptr2 + i);
      ret = 1;
    }
  }
  return ret;
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
static int roll_dice (int s, char *chan, char *nick, int num, int sides, char oper, char *modstr) {
  int i = num, low = sides, high = 0, rnd = 0, sum = 0, mod = 0, ret = 0;
  char tmpmsg[MSG_LEN] = {0}, *msg = 0;

  srand(time(NULL));

  while (i > 0) {
    rnd = rand() % sides + 1;
    if (rnd > high)
      high = rnd;
    if (rnd < low)
      low = rnd;
    sum += rnd;
    --i;
  }

  // Find mod (1d6 == 1d6+0)
  if ( strcmp(modstr, "") == 0) {
    oper = 0;
    mod = 0;
  }
  else if ( strcmp(modstr, "L") == 0) {
    mod = low;
  }
  else if ( strcmp(modstr, "H") == 0) {
    mod = high;
  }
  else {
    sscanf(modstr, "%d", &mod);
  }

  switch (oper) {
    case '+': sum += mod; break;
    case '-': sum -= mod; break;
    case '*': case 'x': sum *= mod; break;
    case '/': sum /= mod; break;
    case 0: default: break;
  }


  if ( oper ) {
    make_msg(&msg, tmpmsg, "%s: %dd%d%c%d = %d", nick, num, sides, oper, mod, sum);
    ret = irc_msg(s, find_rts(chan, nick), msg);
  }
  else {
    make_msg(&msg, tmpmsg, "%s: %dd%d = %d", nick, num, sides, sum);
    ret = irc_msg(s, find_rts(chan, nick), msg);
  }

  if (msg != NULL) {
    free(msg);
    msg = NULL;
  }

  return ret;
}

