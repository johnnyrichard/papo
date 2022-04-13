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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITAL_CAPACITY 16

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL


static uint64_t    hash_key(const char *key);
static const char *hash_table_set_entry(hash_table_entry_t *entries,
                                        size_t              capacity,
                                        const char         *key,
                                        void               *value,
                                        size_t             *plength);
static bool        hash_table_expand(hash_table_t *ht);

hash_table_t*
hash_table_new(void)
{
  hash_table_t* ht = malloc(sizeof(hash_table_t));
  if (ht == NULL) {
    fprintf(stderr, "could not create hash_table: out of memory\n");
    return NULL;
  }
  ht->capacity = INITAL_CAPACITY;
  ht->length = 0;

  ht->entries = calloc(ht->capacity, sizeof(hash_table_entry_t));
  if (ht->entries == NULL) {
    fprintf(stderr, "could not create hash_entry: out of memory\n");
    free(ht);
    return NULL;
  }

  return ht;
}

void
hash_table_destroy(hash_table_t *ht)
{
  for (size_t i = 0; i < ht->capacity; ++i) {
    if (ht->entries[i].key != NULL) {
      free((void*) ht->entries[i].key);
    }
  }
  free(ht->entries);
  free(ht);
}

void
hash_table_insert(hash_table_t *ht, const char *key, void *value)
{
  assert(value != NULL);
  
  if (ht->length >= ht->capacity / 2) {
    if (!hash_table_expand(ht)) {
      return;
    }
  }

  hash_table_set_entry(ht->entries, ht->capacity, key, value, &ht->length);
}


void*
hash_table_get(hash_table_t *ht,
               const char   *key)
{
  uint64_t hash = hash_key(key);
  size_t index = (size_t)(hash & (uint64_t)(ht->capacity - 1));

  while (ht->entries[index].key != NULL) {
    if (strcmp(key, ht->entries[index].key) == 0) {
      return ht->entries[index].value;
    }
    index++;
    if (index >= ht->capacity) {
      index = 0;
    }
  }
  return NULL;
}

bool
hash_table_remove(hash_table_t *ht,
                  const char   *key)
{
  uint64_t hash = hash_key(key);
  size_t index = (size_t)(hash & (uint64_t)(ht->capacity - 1));

  while (ht->entries[index].key != NULL) {
    if (strcmp(key, ht->entries[index].key) == 0) {
      free(ht->entries[index].key);
      ht->entries[index] = (hash_table_entry_t) { NULL, NULL };
      ht->length--;
      return true;
    }
    index++;
    if (index >= ht->capacity) {
      index = 0;
    }
  }
  return false;
}

size_t
hash_table_length(hash_table_t *ht)
{
  return ht->length;
}

static uint64_t
hash_key(const char *key)
{
  uint64_t hash = FNV_OFFSET;
  for (const char* p = key; *p; ++p) {
    hash ^= (uint64_t) (unsigned char) (*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

static const char*
hash_table_set_entry(hash_table_entry_t *entries,
                     size_t              capacity,
                     const char         *key,
                     void               *value,
                     size_t             *plength)
{
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key != NULL) {
        if (strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }
        index++;
        if (index >= capacity) {
            index = 0;
        }
    }

    if (plength != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        (*plength)++;
    }
    entries[index].key = (char*)key;
    entries[index].value = value;
    return key;
}

static bool
hash_table_expand(hash_table_t *ht)
{
    size_t new_capacity = ht->capacity * 2;
    if (new_capacity < ht->capacity) {
        return false;
    }
    hash_table_entry_t* new_entries = calloc(new_capacity, sizeof(hash_table_entry_t));
    if (new_entries == NULL) {
        return false;
    }

    for (size_t i = 0; i < ht->capacity; ++i) {
        hash_table_entry_t entry = ht->entries[i];
        if (entry.key != NULL) {
            hash_table_set_entry(
                new_entries,
                new_capacity,
                entry.key,
                entry.value,
                NULL
            );
        }
    }

    free(ht->entries);
    ht->entries = new_entries;
    ht->capacity = new_capacity;
    return true;
}

hash_table_iterator_t hash_table_get_iterator(hash_table_t *ht) {
    hash_table_iterator_t it;
    it._table = ht;
    it._index = 0;
    return it;
}

bool hash_table_iterator_next(hash_table_iterator_t *it) {
    hash_table_t* ht = it->_table;
    while (it->_index < ht->capacity) {
        size_t i = it->_index;
        it->_index++;
        if (ht->entries[i].key != NULL) {
            hash_table_entry_t entry = ht->entries[i];
            it->key = entry.key;
            it->value = entry.value;
            return true;
        }
    }
    return false;
}

