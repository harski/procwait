/* Copyright 2013 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef PW_PROC_H
#define PW_PROC_H

#include <stdbool.h>

#define STAT_COL_LEN 32

/* represents the process PID. Content is parsed from file /proc/PID/stat */
struct proc {
	unsigned pid;
	char pname[STAT_COL_LEN];
	unsigned t0;
};

int parse_stat_file (unsigned pid, struct proc *p);
bool proc_eq (const struct proc * const p1, const struct proc * const p2);
bool validate_proc (const struct proc * const p);

#endif

