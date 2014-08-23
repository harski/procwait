/* Copyright 2014 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "go.h"
#include "strutil.h"

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

