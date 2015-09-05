/* Copyright 2014-2015 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef PW_STRUTIL
#define PW_STRUTIL

#include <stdbool.h>

/* check if char c is \t, \n or ' ' */
bool is_whitespace (const char c);

/* parse str to unsigned int */
int strtou (const char * const str, unsigned * restrict u);

#endif
