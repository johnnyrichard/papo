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

  return MUNIT_OK;
}

static MunitTest tests[] = {
  { "/test_create_new", test_create_new, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
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
