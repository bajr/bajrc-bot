//#include <openssl/ssl.h> Implement this later
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "socket.h"

int get_socket(const char* host, const char* port) {
   int rc;
   int s;
   struct addrinfo hints, *res;

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;

   if ((rc = getaddrinfo(host, port, &hints, &res) ) < 0 ) {
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
      return -1;
   }
   s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if ( s < 0 ) {
      fprintf(stderr, "Couldn't get socket.\n");
      goto error;
   }
   if ( connect(s, res->ai_addr, res->ai_addrlen) < 0 ) {
      fprintf(stderr, "Couldn't connect.\n");
      goto error;
   }
   freeaddrinfo(res);
   return s;

error:
   freeaddrinfo(res);
   return -1;
}

// Sends data to socket with file descriptor s
// Returns the number of characters sent, or -1 if there is an error.
int sck_send(int s, const char* data, size_t size) {
  size_t written = 0;
  int rc;

  fprintf(stdout, "%s\n", data);
  while ( written < size ) {
    rc = send(s, data + written, size - written, 0); // Pointer arithmetic
    //fprintf(stderr, "Sending -%s-", (data + written));
    if ( rc <= 0 )
      return -1;
    written += rc;
  }
  return written;
}

// Prepares a message to send to the socket.
// Returns the number of characters sent, 0 if the message is empty, or -1 if there is an error.
int sck_sendf(int s, const char *fmt, ...) {
   char send_buf[MSG_LEN];
   int send_len;
   va_list args;

   if (strlen(fmt) != 0 ) {
      // Format the data
      va_start(args, fmt);
      send_len = vsnprintf(send_buf, sizeof (send_buf), fmt, args);
      va_end(args);

      // Clamp the chunk
      if (send_len > MSG_LEN)
         send_len = MSG_LEN;
      if (sck_send( s, send_buf, send_len ) <= 0)
         return -1;
      return send_len;
   }
   return 0;
}

int sck_recv(int s, char* buffer, size_t size) {
  int rc;

  rc = recv(s, buffer, size, 0);
  int errsv = errno;

  if ( rc <= 0 ) {
    switch (errsv) {
      case EAGAIN: fprintf(stderr, "EAGAIN"); break;
      case EBADF: fprintf(stderr, "EBADF"); break;
      case ECONNREFUSED: fprintf(stderr, "ECONNREFUSED"); break;
      case EFAULT: fprintf(stderr, "EFAULT"); break;
      case EINTR: fprintf(stderr, "EINTR"); break;
      case EINVAL: fprintf(stderr, "EINVAL"); break;
      case ENOMEM: fprintf(stderr, "ENOMEM"); break;
      case ENOTCONN: fprintf(stderr, "ENOTCONN"); break;
      case ENOTSOCK: fprintf(stderr,"ENOTSOCK"); break;
      default: fprintf(stderr, "Unknown Error: %d", errsv);
    }
  }
  return rc;
}

