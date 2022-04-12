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
#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>

#define INITAL_CAPACITY 16

hash_table_t*
hash_table_new(void)
{
  hash_table_t* ht = malloc(sizeof(hash_table_t));
  if (ht == NULL) {
    fprintf(stderr, "could not create hash_table: out of memory\n");
    return NULL;
  }
  return ht;
}
