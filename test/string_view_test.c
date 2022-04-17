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
#include "../string_view.h"

#include <stdlib.h>

static MunitResult
test_create_new(const  MunitParameter params[],
                void  *user_data_or_fixture)
{
  char *data = "hello world";
  string_view_t sv = string_view_new(data, strlen(data));

  assert_string_equal("hello world", (char *) sv.data);
  assert_int(sv.size, ==, strlen(data));

  return MUNIT_OK;
}

static MunitResult
test_from_cstr(const  MunitParameter params[],
                void  *user_data_or_fixture)
{
  char *cstr = "hello world";

  string_view_t sv = string_view_from_cstr(cstr);

  assert_string_equal(cstr, (char *) sv.data);
  assert_int(sv.size, ==, strlen(cstr));

  return MUNIT_OK;
}

static MunitResult
test_eq(const  MunitParameter params[],
        void  *user_data_or_fixture)
{
  string_view_t a = string_view_from_cstr("hello");
  string_view_t b = string_view_from_cstr("world");

  assert_false(string_view_eq(a, b));

  b = string_view_from_cstr("hello");

  assert_true(string_view_eq(a, b));

  return MUNIT_OK;
}

static MunitResult
test_chop_by_delim_when_has_delim(const  MunitParameter params[],
                                  void  *user_data_or_fixture)
{
  string_view_t sv = string_view_from_cstr("hello world");
  string_view_t result = string_view_chop_by_delim(&sv, ' ');

  char actual[10];
  sprintf(actual, "%.*s", sv.size, sv.data);
  assert_string_equal(actual, "world");

  sprintf(actual, "%.*s", result.size, result.data);
  assert_string_equal(actual, "hello");

  return MUNIT_OK;
}

static MunitResult
test_chop_by_delim_when_has_no_delim(const  MunitParameter params[],
                                     void  *user_data_or_fixture)
{
  string_view_t sv = string_view_from_cstr("helloworld");
  string_view_t result = string_view_chop_by_delim(&sv, ' ');

  char actual[10];
  sprintf(actual, "%.*s", sv.size, sv.data);
  assert_string_equal(actual, "");

  sprintf(actual, "%.*s", result.size, result.data);
  assert_string_equal(actual, "helloworld");

  return MUNIT_OK;
}

static MunitTest tests[] = {
  { "/test_create_new", test_create_new, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_from_cstr", test_from_cstr, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_eq", test_eq, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_chop_by_delim_when_has_delim", test_chop_by_delim_when_has_delim, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_chop_by_delim_when_has_no_delim", test_chop_by_delim_when_has_no_delim, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
  "/string_view", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int
main(int argc, char *argv[])
{
  return munit_suite_main(&suite, NULL, argc, argv);
  return EXIT_SUCCESS;
}
