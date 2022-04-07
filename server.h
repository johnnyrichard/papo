#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

typedef struct server {
  int fd;
} server_t;

server_t server_create(uint32_t port);

#endif /* SERVER_H */
