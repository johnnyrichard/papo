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

#include <assert.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 4096
#define EXIT_COMMAND "exit"
#define EMPTY_CCLIENT_SPOT -1

typedef struct client {
  int fd;
  char nick[128];
  char buf[BUFFER_SIZE];
  char msg_buf[BUFFER_SIZE];
  int msg_buf_i;
} client_t;

static void server_handle_client_data(server_t *server, struct epoll_event *event);
static void server_handle_server_data(server_t *server, struct epoll_event *event);
static void server_on_client_msg(server_t *server, client_t *client);

void
server_init(server_t *server, uint32_t port)
{
  assert(server != NULL);
  memset(server, 0, sizeof(server));

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    log_fatal("unable to create a socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  server->fd = server_fd;

  struct sockaddr_in server_addr;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    log_fatal("could not bind: %s", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, SOMAXCONN) == -1) {
    log_fatal("could not listen: %s", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  server->client_table = hash_table_new();
  if (server->client_table == NULL) {
    log_fatal("could not create server->hash_table");
    exit(EXIT_FAILURE);
  }

  server->epoll_fd = epoll_create1(0);
  if (server->epoll_fd == -1) {
    log_fatal("faild to create epoll: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = server->fd;

  if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->fd, &event) == -1) {
    log_fatal("could not add server to epoll: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  log_debug("server listening on port (%d)", port);
}

void
server_run(server_t *server)
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
      struct epoll_event event = server->events[i];
      if (event.data.fd == server->fd) {
        server_handle_server_data(server, &event);
      } else {
        server_handle_client_data(server, &event);
      }
    }
  }
  close(server->epoll_fd);
  close(server->fd);
  exit(EXIT_SUCCESS);
}

void
server_destroy(server_t *server)
{
  hash_table_destroy(server->client_table);
}

static void
server_handle_server_data(server_t *server, struct epoll_event *event)
{
  struct sockaddr_in clientaddr;
  socklen_t client_len = sizeof(clientaddr);

  int client_fd = accept(server->fd, (struct sockaddr *) &clientaddr, &client_len);
  if (client_fd == -1) {
    log_error("could not accept connection: %s", strerror(errno));
    close(server->fd);
    exit(EXIT_FAILURE);
  }

  struct epoll_event client_event;
  client_event.events = EPOLLIN;

  client_t* client = (client_t *) malloc(sizeof(client_t));
  if (client == NULL) {
    log_error("could not create client_t", strerror(errno));
    close(client_fd);
    return;
  }

  client->fd = client_fd;
  sprintf(client->nick, "nick%d", client->fd);
  client_event.data.ptr = (void *) client;

  if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
    log_error("could not add server to epoll: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  hash_table_insert(server->client_table, client->nick, client);
}

static void
server_handle_client_data(server_t *server, struct epoll_event *event)
{
  client_t* client = event->data.ptr;
  int client_fd = client->fd;
  if (event->events & EPOLLHUP) {
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) == -1) {
      log_error("could not del fd : %s", strerror(errno));
    }
    hash_table_remove(server->client_table, client->nick);
    free(client);
    return;
  }
  memset(client->buf, 0, BUFFER_SIZE);

  int readn = recv(client_fd, client->buf, BUFFER_SIZE, 0);
  if (readn == -1) {
    log_error("could not read data from client: %s", strerror(errno));
    return;
  }


  size_t buf_i = 0;
  while (buf_i < readn) {
    for (; client->buf[buf_i] != '\n' && buf_i < readn; ++client->msg_buf_i, ++buf_i) {
      client->msg_buf[client->msg_buf_i] = client->buf[buf_i];
    }
    if (buf_i < readn) {
      client->msg_buf[client->msg_buf_i] = '\0';
      client->msg_buf_i = 0;

      log_debug("Message received from client (%d): %s", client->fd, client->msg_buf);

      server_on_client_msg(server, client);

      buf_i++;
    }
  }

  if (!strncasecmp(client->buf, EXIT_COMMAND, strlen(EXIT_COMMAND) - 1)) {
    log_debug("exiting program. bye bye!");
    server->running = false;
  }
}

static void
server_on_client_msg(server_t *server, client_t *client)
{
  char reply[128];
  memset(reply, 0, 128);
  char* msg = strdup(client->msg_buf);
  char* token = strtok(msg, " ");

  if (token == NULL) {
    goto exit;

  } else if (strcmp(token, "USER") == 0) {
    token = strtok(NULL, " ");

    sprintf(reply, "001 %s :Welcome!\n", client->nick);
    if (send(client->fd, reply, strlen(reply), 0) == -1) {
      log_error("could not send data to client: %s", strerror(errno));
      goto exit;
    }

  } else if (strcmp(token, "PING") == 0) {
    token = strtok(NULL, " ");
    sprintf(reply, "PONG %s\n", token);
    if (send(client->fd, reply, strlen(reply), 0) == -1) {
      log_error("could not send data to client: %s", strerror(errno));
      goto exit;
    }

  } else if (strcmp(token, "PRIVMSG") == 0) {
    token = strtok(NULL, " ");

    client_t *client_recv = hash_table_get(server->client_table, token);

    if (client_recv == NULL) {
      sprintf(reply, "could not send message to nick: %s. nick not found\n", token);
      if (send(client->fd, reply, strlen(reply), 0) == -1) {
        log_error("could not send data to client: %s", strerror(errno));
        goto exit;
      }
    }
    token = strtok(NULL, ":");
    sprintf(reply, ":%s PRIVMSG %s :%s\n", client->nick, client_recv->nick, token);
    if (send(client_recv->fd, reply, strlen(reply), 0) == -1) {
      log_error("could not send data to client: %s", strerror(errno));
      goto exit;
    }
  }
exit:
  free(msg);
}
