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
#include "channel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

channel_t*
channel_new(char *name)
{
  if (name == NULL) {
    fprintf(stderr, "Could not create channel: name is required\n");
    return NULL;
  }

  channel_t* channel = (channel_t*) malloc(sizeof(channel_t));
  if (channel == NULL) {
    fprintf(stderr, "Could not create channel: Out or memory\n");
    return NULL;
  }

  strcpy(channel->name, name);
  channel->client_table = hash_table_new();
  if (channel->client_table == NULL) {
    fprintf(stderr, "Could not create channel->client_table\n");
    return NULL;
  }

  return channel;
}

void
channel_add_client(channel_t* channel, client_t* client)
{
  if (client == NULL) {
    fprintf(stderr, "client_add_client: client is required\n");
    return;
  }
  hash_table_insert(channel->client_table, client->nick, client);
}

void
channel_destroy(channel_t *channel)
{
  hash_table_destroy(channel->client_table);
  free(channel);
}

