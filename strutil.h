/* Copyright 2014-2015 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef PW_STRUTIL
#define PW_STRUTIL

#include <stdbool.h>

#define STRUTIL_EXIT_SUCCESS 1
#define STRUTIL_EXIT_EOF 2
#define STRUTIL_EXIT_TRUNCATED 3

/* get next whitespace separated field from fd */
int get_next_field (FILE * fd, char * str, const size_t len);

/* check if char c is \t, \n or ' ' */
bool is_whitespace (const char c);

/* skip fd until whitespace is found */
int skip_field (FILE * fd);

/* parse str to unsigned int */
int strtou (const char * const str, unsigned * restrict u);

#endif
