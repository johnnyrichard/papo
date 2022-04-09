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
#include "log.h"
#include "server.h"

#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define EXIT_COMMAND "exit"

static void server_handle_client_data(server_t *server, struct epoll_event *event);
static void server_handle_server_data(server_t *server, struct epoll_event *event);

server_t
server_create(uint32_t port)
{
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    log_error("unable to create a socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    log_error("could not bind: %s", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, SOMAXCONN) == -1) {
    log_error("could not listen: %s", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  server_t server;
  memset(&server, 0, sizeof(server));
  server.fd = server_fd;
  memset(server.connected_clients, -1, sizeof(int) * MAXEVENTS);

  server.epoll_fd = epoll_create1(0);
  if (server.epoll_fd == -1) {
    log_error("faild to create epoll: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = server.fd;

  if (epoll_ctl(server.epoll_fd, EPOLL_CTL_ADD, server.fd, &event) == -1) {
    log_error("could not add server to epoll: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  log_info("server listening at port (%d)", port);

  return server;
}

void
server_start(server_t *server)
{
  server->running = true;

  while (server->running) {
    int event_count = epoll_wait(server->epoll_fd, server->events, MAXEVENTS, 10000);
    if (event_count == -1) {
      log_error("could not wait for epoll events: %s", strerror(errno));
      continue;
    }

    if (event_count == 0) {
      log_trace("epoll_wait timeout");
      continue;
    }

    for (int i = 0; i < event_count; ++i) {
      if (server->events[i].data.fd == server->fd) {
        server_handle_server_data(server, &server->events[i]);
      } else {
        server_handle_client_data(server, &server->events[i]);
      }
    }
  }
  close(server->epoll_fd);
  close(server->fd);
  exit(EXIT_SUCCESS);
}

static void
server_handle_server_data(server_t *server, struct epoll_event *event)
{       
  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  int client_fd = accept(server->fd, (struct sockaddr *) &client, &client_len);
  if (client_fd == -1) {
    log_error("could not accept connection: %s", strerror(errno));
    close(server->fd);
    exit(EXIT_FAILURE);
  }

  struct epoll_event client_event;
  client_event.events = EPOLLIN;
  client_event.data.fd = client_fd;

  if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
    log_error("could not add server to epoll: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  int j = 0;
  while (server->connected_clients[j] != -1 && j < MAXEVENTS) {
    j++; 
  }

  if (j < MAXEVENTS) {
    server->connected_clients[j] = client_fd;
  } else {
    log_warn("max number of connection has been reached");
  }

}

static void
server_handle_client_data(server_t *server, struct epoll_event *event)
{ 
  int client_fd = event->data.fd;
  if (event->events & EPOLLHUP) {
    for (int j = 0; j < MAXEVENTS; ++j) {
      if (server->connected_clients[j] == client_fd) {
        server->connected_clients[j] = -1;
        break;
      }
    }
    return;
  }
  char client_buf[BUFFER_SIZE];
  memset(client_buf, 0, BUFFER_SIZE);

  if (recv(client_fd, client_buf, BUFFER_SIZE, 0) == -1) {
    log_error("could not read data from client: %s", strerror(errno));
    return;
  }

  if (!strncasecmp(client_buf, EXIT_COMMAND, strlen(EXIT_COMMAND) - 1)) {
    log_info("exiting program. bye bye!");
    server->running = false;
  }

  for (int j = 0; j < MAXEVENTS; ++j) {
    if (server->connected_clients[j] == -1) {
      continue;
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(char) * BUFFER_SIZE);

    sprintf(message, "client_%d :%s", client_fd, client_buf);
    if (send(server->connected_clients[j], message, BUFFER_SIZE, 0) == -1) {
      log_error("could not send data to client: %s", strerror(errno));
      continue;
    }
  }
}
