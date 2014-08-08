/* Copyright 2013 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef PW_STAT_H
#define PW_STAT_H

#include <stdbool.h>

#define STAT_COL_LEN 32

/* contents of the parsed file /proc/PID/stat */
struct stat {
	unsigned pid;
	char pname[STAT_COL_LEN];
	unsigned t0;
};

int parse_stat_file (unsigned pid, struct stat *s);
bool stat_eq (const struct stat * const a, const struct stat * const b);
bool validate_stat_file (const struct stat * const s);

#endif

