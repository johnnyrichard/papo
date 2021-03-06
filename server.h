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
#ifndef SERVER_H
#define SERVER_H

#include "hash_table.h"

#include <stdint.h>
#include <stdbool.h>
#include <sys/epoll.h>

#define MAXEVENTS 64

typedef struct server {
  int fd;
  int epoll_fd;
  struct epoll_event events[MAXEVENTS];
  hash_table_t *client_table;
  hash_table_t *channel_table;
  bool running;
} server_t;

void server_init(server_t *server, uint32_t port);
void server_run(server_t *server);
void server_destroy(server_t *server);

#endif /* SERVER_H */
