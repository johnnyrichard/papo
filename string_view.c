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
#include "string_view.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

string_view_t
string_view_new(const char *cstr, size_t size)
{
  assert(cstr);

  return (string_view_t) {
    .size = size,
    .data = cstr
  };
}

string_view_t
string_view_from_cstr(const char *cstr)
{
  return string_view_new(cstr, strlen(cstr));
}

bool
string_view_to_cstr(string_view_t *sv, char *ret)
{
  return sprintf(ret, SVFMT, SVARG(sv)) < 0;
}

bool
string_view_eq(string_view_t a, string_view_t b)
{
  return a.size == b.size && memcmp(a.data, b.data, a.size) == 0;
}

string_view_t
string_view_chop_by_delim(string_view_t *sv, char delim)
{
  size_t i = 0;
  while (sv->data[i] != delim && i < sv->size) {
    ++i;
  }

  string_view_t chopped_sv = string_view_new(sv->data, i);

  if (i < sv->size) {
    sv->size -= i + 1;
    sv->data += i + 1;
  } else {
    sv->size -= i;
    sv->data += i;
  }

  return chopped_sv;
}
