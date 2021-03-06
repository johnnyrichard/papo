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
#include "channel.h"
#include "client.h"
#include "log.h"
#include "server.h"
#include "string_view.h"

#include <assert.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define EXIT_COMMAND "exit"

static void server_handle_client_data(server_t *server, struct epoll_event *event);
static void server_handle_server_data(server_t *server, struct epoll_event *event);
static void server_client_msg_dipatcher(server_t *server, client_t *client);
static void server_on_nick_msg(server_t *server, client_t *client, string_view_t msg);
static void server_on_user_msg(server_t *server, client_t *client, string_view_t msg);
static void server_on_ping_msg(server_t *server, client_t *client, string_view_t msg);
static void server_on_privmsg_msg(server_t *server, client_t *client, string_view_t msg);
static void server_on_join_msg(server_t *server, client_t *client, string_view_t msg);

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
    log_fatal("could not create server->client_table");
    exit(EXIT_FAILURE);
  }

  server->channel_table = hash_table_new();
  if (server->client_table == NULL) {
    log_fatal("could not create server->channel_table");
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

      server_client_msg_dipatcher(server, client);

      buf_i++;
    }
  }

  if (!strncasecmp(client->buf, EXIT_COMMAND, strlen(EXIT_COMMAND) - 1)) {
    log_debug("exiting program. bye bye!");
    server->running = false;
  }
}

static void
server_client_msg_dipatcher(server_t *server, client_t *client)
{
  string_view_t msg;
  msg = string_view_from_cstr(client->msg_buf);
  msg = string_view_chop_by_delim(&msg, '\r');

  string_view_t msg_type = string_view_chop_by_delim(&msg, ' ');

  if (msg_type.size == 0) {
    return;
  } 
  if (string_view_eq(msg_type, string_view_from_cstr("NICK"))) {
    return server_on_nick_msg(server, client, msg);
  } 
  if (string_view_eq(msg_type, string_view_from_cstr("USER"))) {
    return server_on_user_msg(server, client, msg);
  } 
  if (string_view_eq(msg_type, string_view_from_cstr("PING"))) {
    return server_on_ping_msg(server, client, msg);
  }
  if (string_view_eq(msg_type, string_view_from_cstr("PRIVMSG"))) {
    return server_on_privmsg_msg(server, client, msg);
  }
  if (string_view_eq(msg_type, string_view_from_cstr("JOIN"))) {
    return server_on_join_msg(server, client, msg);
  }
}

static void
server_on_user_msg(server_t     *server,
                   client_t     *client,
                   string_view_t msg)
{
  // FIXME: Fix server host on reply messages
  client_send_msg(
      client, 
      ":localhost 001 %s :Welcome to the Internet Relay Network\n", 
      client->nick
  );
}

static void
server_on_nick_msg(server_t     *server,
                   client_t     *client,
                   string_view_t msg)
{
  hash_table_remove(server->client_table, client->nick);

  sprintf(client->nick, SVFMT, SVARG(&msg));

  hash_table_insert(server->client_table, client->nick, client);
}

static void
server_on_ping_msg(server_t     *server, 
                   client_t     *client, 
                   string_view_t msg)
{
  // TODO: Create custom functions for each client message
  client_send_msg(client, "PONG "SVFMT"\n", SVARG(&msg));
}

static void
server_on_privmsg_msg(server_t     *server,
                      client_t     *client,
                      string_view_t msg)
{
  string_view_t sv_nick = string_view_chop_by_delim(&msg, ' ');

  char nick[sv_nick.size + 1];
  string_view_to_cstr(&sv_nick, nick);
  client_t *client_receiver = hash_table_get(server->client_table, nick);

  if (client_receiver == NULL) {
    client_send_msg(client, ":localhost 401 %s %s :No such nick/channel\n", client->nick, nick);
    return;
  }

  string_view_chop_by_delim(&msg, ':');
  client_send_msg(client_receiver, ":%s PRIVMSG %s :"SVFMT"\n", client->nick, client_receiver->nick, SVARG(&msg));
}

static void
server_on_join_msg(server_t     *server,
                   client_t     *client,
                   string_view_t msg)
{
  channel_t* channel = NULL;

  channel = (channel_t*) hash_table_get(server->channel_table, msg.data);
  if (channel == NULL) {
    string_view_chop_by_delim(&msg, '#');

    char channel_name[msg.size + 1];
    string_view_to_cstr(&msg, channel_name);

    // FIXME: Destroy channel when everyone leave channel or server get destroyed
    channel = channel_new(channel_name);
    if (channel == NULL) {
      fprintf(stderr, "server_on_join_msg: could not create new channel\n");
      return;
    }
    channel_add_client(channel, client);
    hash_table_insert(server->channel_table, channel->name, channel);
  }

  client_send_msg(
      client,
      ":%s!~%s@localhost JOIN "SVFMT"\n",
      client->nick,
      client->nick,
      SVARG(&msg)
  );
}
