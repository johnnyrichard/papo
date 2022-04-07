/*
 * Papo IRC Server
 * Copyright (C) 2021 Johnny Richard
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 6667
#define BUFFER_SIZE 1024
#define EXIT_COMMAND "exit"

int main() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("[ERROR] unable to create a socket");
    return EXIT_FAILURE;
  }

  struct sockaddr_in server;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  if (bind(server_fd, (struct sockaddr *) &server, sizeof(server)) == -1) {
    perror("[ERROR] could not bind");
    close(server_fd);
    return EXIT_FAILURE;
  }

  if (listen(server_fd, SOMAXCONN) == -1) {
    perror("[ERROR] could not listen");
    close(server_fd);
    return EXIT_FAILURE;
  }

  printf("[INFO] server listening at port (%d)\n", SERVER_PORT);

  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  int client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
  if (client_fd == -1) {
    perror("[ERROR] could not accept connection");
    close(server_fd);
    return EXIT_FAILURE;
  }

  bool running = true;

  while (running) {
    char client_buf[BUFFER_SIZE];
    memset(client_buf, 0, BUFFER_SIZE);

    if (recv(client_fd, client_buf, BUFFER_SIZE, 0) == -1) {
      perror("[ERROR] could not read data from client");
      continue;
    }

    if (!strncasecmp(client_buf, EXIT_COMMAND, strlen(EXIT_COMMAND) - 1)) {
      puts("[INFO] exiting program. bye bye!");
      close(client_fd);
      close(server_fd);
      exit(EXIT_SUCCESS);
    }

    if (send(client_fd, client_buf, BUFFER_SIZE, 0) == -1) {
      perror("[ERROR] could not send data to client");
      continue;
    }
  }

  close(client_fd);
  close(server_fd);
  return EXIT_SUCCESS;
}
