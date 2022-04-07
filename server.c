#include "server.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

server_t
server_create(uint32_t port)
{
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("[ERROR] unable to create a socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  if (bind(server_fd, (struct sockaddr *) &server, sizeof(server)) == -1) {
    perror("[ERROR] could not bind");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, SOMAXCONN) == -1) {
    perror("[ERROR] could not listen");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("[INFO] server listening at port (%d)\n", port);

  return (server_t) { .fd = server_fd };
}
