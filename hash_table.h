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
#ifndef HASH_TABLE_H
#define HASH_TABLE

#include <stdio.h>

typedef struct hash_entry {
  const char* key;
  void* value;
} hash_entry_t;

typedef struct hash_table {
  hash_entry_t* entries;
  size_t capacity;
  size_t length;
} hash_table_t;

hash_table_t* hash_table_new();

#endif /* HASH_TABLE_H */
