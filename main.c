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

#include "./server.h"

#define SERVER_PORT 6667
#define BUFFER_SIZE 1024
#define EXIT_COMMAND "exit"

int main() {
  server_t server = server_create(SERVER_PORT);

  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  int client_fd = accept(server.fd, (struct sockaddr *) &client, &client_len);
  if (client_fd == -1) {
    perror("[ERROR] could not accept connection");
    close(server.fd);
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
      close(server.fd);
      exit(EXIT_SUCCESS);
    }

    if (send(client_fd, client_buf, BUFFER_SIZE, 0) == -1) {
      perror("[ERROR] could not send data to client");
      continue;
    }
  }

  close(client_fd);
  close(server.fd);
  return EXIT_SUCCESS;
}
