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


int get_next_field (FILE * fd, char * str, const size_t len)
{
	int success;
	int i;

	for (i = 0; ; ++i) {
		int tmp_char = fgetc(fd);

		if (tmp_char == EOF) {
			str[i] = '\0';
			success = STRUTIL_EXIT_EOF;
			break;
		} else if (is_whitespace((char) tmp_char)) {
			str[i] = '\0';
			success = STRUTIL_EXIT_SUCCESS;
			break;
		} else if (i == (int) len -1) {
			/* No more room for characters, and tmp_char is
			 * non-whitespace. terminate string and
			 * fast-forward until next whitespace. */
			str[i] = '\0';
			success = STRUTIL_EXIT_TRUNCATED;
			skip_field(fd);
			break;
		} else {
			str[i] = (char) tmp_char;
		}
	}

	return success;
}


bool is_whitespace (const char c)
{
    int i = 0;
    while (WHITESPACE[i++] != '\0') {
        if (c == WHITESPACE[i])
            return true;
    }

    return false;
}


int skip_field (FILE * fd)
{
	int tmp_char;
	do {
		tmp_char = fgetc(fd);
	} while (tmp_char != EOF && !is_whitespace((char) tmp_char));

	return tmp_char;
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
