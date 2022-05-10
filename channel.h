/*
 * Papo IRC Server
 * Copyright (C) 2022 Johnny Richard
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
#ifndef CHANNEL_H
#define CHANNEL_H

#include "client.h"
#include "hash_table.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct channel {
  char name[128];
  hash_table_t *client_table;
} channel_t;

// TODO: Broadcast message to connected clients
channel_t* channel_new(char *name);
void channel_add_client(channel_t* channel, client_t* client);
void channel_destroy(channel_t *channel);

#endif /* CHANNEL_H */
