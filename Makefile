CC=gcc
CFLAGS=-c -g -Wall
SOURCE=main.c irc.c socket.c
OBJECTS=$(SOURCE:.c=.o)
EXEC=bajrcbot

all: $(SOURCE) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@



