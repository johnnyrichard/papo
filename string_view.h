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
#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#define SVFMT "%.*s"
#define SVARG(sv) (sv)->size, (sv)->data

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct string_view {
    size_t size;
    const char *data;
} string_view_t;

string_view_t string_view_new(const char *cstr, size_t size);

string_view_t string_view_from_cstr(const char *cstr);

bool string_view_to_cstr(string_view_t *sv, char *ret);

bool string_view_eq(string_view_t a, string_view_t b);

string_view_t string_view_chop_by_delim(string_view_t *sv, char delim);

#endif /* STRING_VIEW_H */
