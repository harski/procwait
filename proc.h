/* Copyright 2013-2017 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef PW_PROC_H
#define PW_PROC_H

#include <stdbool.h>

#include "queue.h"

#define STAT_COL_LEN 32

/* represents the process PID. Content is parsed from file /proc/PID/stat */
struct proc {
	unsigned pid;
	char name[STAT_COL_LEN];
	unsigned t0;
	SLIST_ENTRY(proc) procs;
};

SLIST_HEAD(proclist, proc);

int parse_stat_file (const char * path, struct proc * p);
int parse_stat_pid (const unsigned pid, struct proc * restrict p);
bool proc_eq (const struct proc * const p1, const struct proc * const p2);
bool validate_proc (const struct proc * const p);

#endif
