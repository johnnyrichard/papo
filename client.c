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
#include "client.h"
#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void
client_send_msg(client_t *client, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  if (vdprintf(client->fd, fmt, args) < 0) {
    log_error("could not send data to client: %s", client->nick);
  }
  va_end(args);
}
