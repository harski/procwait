/* Copyright 2014-2015 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "go.h"
#include "strutil.h"

#define WHITESPACE "\t\n "

bool is_whitespace (const char c)
{
    int i = 0;
    while (WHITESPACE[i++] != '\0') {
        if (c == WHITESPACE[i])
            return true;
    }

    return false;
}


int strtou (const char * const str, unsigned * restrict u)
{
	int succ = E_SUCCESS;
	unsigned long ul;
	char *endptr;

	ul = strtoul(str, &endptr, 10);

	/* if result overflows ulong ||
	 *    result overflows uint ||
	 *    the str was a valid ul */
	if ((ul == ULONG_MAX && errno == ERANGE) ||
	    (ul > UINT_MAX) ||
	    !(*str != '\0' && *endptr == '\0')) {
		succ = E_FAIL;
	} else {
		*u = (unsigned) ul;
	}

	return succ;
}
