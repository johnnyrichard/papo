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
#define MUNIT_ENABLE_ASSERT_ALIASES
#include "munit/munit.c"
#include "../hash_table.h"

#include <stdlib.h>

static MunitResult
test_create_new(const  MunitParameter params[],
                void  *user_data_or_fixture)
{
  hash_table_t* table = hash_table_new();

  assert_not_null(table);
  assert_int(table->capacity, >, 0);
  assert_int(table->length, ==, 0);

  hash_table_destroy(table);
  return MUNIT_OK;
}

static MunitResult
test_insert_and_get(const  MunitParameter params[],
                    void  *user_data_or_fixture)
{
  hash_table_t* table = hash_table_new();

  hash_table_insert(table, "key1", "value1");
  hash_table_insert(table, "key2", "value2");

  char* value = hash_table_get(table, "key1");
  assert_string_equal("value1", value);

  value = hash_table_get(table, "key2");
  assert_string_equal("value2", value);

  value = hash_table_get(table, "invalid_key");
  assert_null(value);

  hash_table_destroy(table);
  return MUNIT_OK;
}

static MunitResult
test_remove(const  MunitParameter params[],
            void  *user_data_or_fixture)
{
  hash_table_t* table = hash_table_new();

  hash_table_insert(table, "key", "value");

  char* value = hash_table_get(table, "key");
  assert_string_equal("value", value);

  bool removed = hash_table_remove(table, "key");

  assert_int(hash_table_length(table), ==, 0);
  assert_null(hash_table_get(table, "key"));
  assert_true(removed);

  hash_table_destroy(table);
  return MUNIT_OK;
}

static MunitResult
test_length(const  MunitParameter params[],
            void  *user_data_or_fixture)
{
  hash_table_t* table = hash_table_new();

  hash_table_insert(table, "key1", "value1");
  hash_table_insert(table, "key2", "value2");

  assert_int(2, ==, hash_table_length(table));

  hash_table_destroy(table);
  return MUNIT_OK;
}

static MunitResult
test_iterator(const  MunitParameter params[],
              void  *user_data_or_fixture)
{
  hash_table_t* table = hash_table_new();

  hash_table_insert(table, "key1", "value1");
  hash_table_insert(table, "key2", "value2");

  hash_table_iterator_t it = hash_table_get_iterator(table);

  size_t count;
  for (count = 0; hash_table_iterator_next(&it); ++count) {
    if (strcmp(it.key, "key1") != 0 &&
        strcmp(it.key, "key2") != 0) {
      fprintf(stderr, "key not found: %s\n", it.key);
      hash_table_destroy(table);
      return MUNIT_FAIL;
    }

    if (strcmp(it.value, "value1") != 0 &&
        strcmp(it.value, "value2") != 0) {
      fprintf(stderr, "value not found: %s\n", it.value);
      hash_table_destroy(table);
      return MUNIT_FAIL;
    }
  }

  assert_int(hash_table_length(table), ==, count);

  hash_table_destroy(table);
  return MUNIT_OK;
}

static MunitTest tests[] = {
  { "/test_create_new", test_create_new, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_insert_and_get", test_insert_and_get, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_remove", test_remove, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_length", test_length, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_iterator", test_iterator, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
  "/hash_table", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int
main(int argc, char *argv[])
{
  return munit_suite_main(&suite, NULL, argc, argv);
  return EXIT_SUCCESS;
}
