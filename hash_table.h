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
#define HASH_TABLE_H

#include <stdio.h>
#include <stdbool.h>

typedef struct hash_table hash_table_t;
typedef struct hash_table_entry hash_table_entry_t;
typedef struct hash_table_iterator hash_table_iterator_t;

struct hash_table {
  hash_table_entry_t* entries;
  size_t capacity;
  size_t length;
};

struct hash_table_entry {
  const char* key;
  void* value;
};

struct hash_table_iterator {
  const char* key;
  void* value;
  hash_table_t *_table;
  size_t _index;
};

hash_table_t* hash_table_new     ();
void          hash_table_destroy (hash_table_t *ht);
void          hash_table_insert  (hash_table_t *ht,
                                  const char   *key,
                                  void         *value);
bool          hash_table_remove  (hash_table_t *ht,
                                  const char   *key);
void*         hash_table_get     (hash_table_t *ht,
                                  const char   *key);
size_t        hash_table_length  (hash_table_t *ht);

hash_table_iterator_t hash_table_get_iterator(hash_table_t *ht);
bool hash_table_iterator_next(hash_table_iterator_t *it);

#endif /* HASH_TABLE_H */
