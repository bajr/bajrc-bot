#ifndef IRC_SOCKET_H
#define IRC_SOCKET_H

#include <stdlib.h>

#define IRC_PORT "6667"
#define IRC_SSL_PORT "6697"

int get_socket(const char* host, const char* port);
int sck_send(int socket, const char* data, size_t size);
int sck_sendf(int socket, const char* fmt, ...);
int sck_recv(int socket, char* buffer, size_t size);

#endif
