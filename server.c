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
#include "server.h"

#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define EXIT_COMMAND "exit"

server_t
server_create(uint32_t port)
{
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("[ERROR] unable to create a socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    perror("[ERROR] could not bind");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, SOMAXCONN) == -1) {
    perror("[ERROR] could not listen");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  server_t server;
  memset(&server, 0, sizeof(server));
  server.fd = server_fd;

  server.epoll_fd = epoll_create1(0);
  if (server.epoll_fd == -1) {
    perror("[ERROR] faild to create epoll");
    exit(EXIT_FAILURE);
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = server.fd;

  if (epoll_ctl(server.epoll_fd, EPOLL_CTL_ADD, server.fd, &event) == -1) {
    perror("[ERROR] could not add server to epoll");
    exit(EXIT_FAILURE);
  }

  printf("[INFO] server listening at port (%d)\n", port);

  return server;
}

void
server_start(server_t *server)
{
  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  server->running = true;

  while (server->running) {
    int event_count = epoll_wait(server->epoll_fd, server->events, MAXEVENTS, 10000);
    if (event_count == -1) {
      perror("[ERROR] could not wait for epoll events");
      continue;
    }

    if (event_count == 0) {
      puts("[TRACE] epoll_wait timeout");
      continue;
    }

    for (int i = 0; i < event_count; ++i) {
      int sockfd = server->events[i].data.fd;
      if (sockfd == server->fd) {
        int client_fd = accept(server->fd, (struct sockaddr *) &client, &client_len);
        if (client_fd == -1) {
          perror("[ERROR] could not accept connection");
          close(server->fd);
          exit(EXIT_FAILURE);
        }

        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = client_fd;

        if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
          perror("[ERROR] could not add server to epoll");
          exit(EXIT_FAILURE);
        }

        server->connected_clients[server->connected_clients_index++] = client_fd;
      } else {
        int client_fd = sockfd;
        char client_buf[BUFFER_SIZE];
        memset(client_buf, 0, BUFFER_SIZE);

        if (recv(client_fd, client_buf, BUFFER_SIZE, 0) == -1) {
          perror("[ERROR] could not read data from client");
          continue;
        }

        if (!strncasecmp(client_buf, EXIT_COMMAND, strlen(EXIT_COMMAND) - 1)) {
          puts("[INFO] exiting program. bye bye!");
          close(client_fd);
          close(server->fd);
          exit(EXIT_SUCCESS);
        }

        for (int i = 0; i < server->connected_clients_index; ++i) {
          char message[BUFFER_SIZE];
          sprintf(message, "client_%d :%s", client_fd, client_buf);
          if (send(server->connected_clients[i], message, BUFFER_SIZE, 0) == -1) {
            perror("[ERROR] could not send data to client");
            continue;
          }
        }
      }
    }
  }
  close(server->epoll_fd);
  close(server->fd);
  exit(EXIT_SUCCESS);
}
