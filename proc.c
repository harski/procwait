/* Copyright 2013-2014 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "go.h"
#include "proc.h"

#define FILENAME_BUF_LEN 32

/* Field indexes for file /proc/PID/stat */
enum {
	STAT_PID = 0,
	STAT_PNAME = 1,
	STAT_T0 = 21,
	STAT_FIELDS
};


static int handle_field (unsigned field, const char *field_buf, struct proc *p);


static int handle_field (unsigned field, const char *field_buf, struct proc *p)
{
	int success = E_SUCCESS;
	char *endptr;

	switch (field) {
	case STAT_PID:
		p->pid = strtoul(field_buf, &endptr, 10);
		/* if field wasn't completely parsed something is fundamentally
		 * wrong with the stat file */
		if (*endptr != '\0') {
			go(GO_ERR, "Failed to parse PID field from"
				   " /proc/PID/stat\n");
			p->pid = 0;
			success = E_FAIL;
		}
		break;

	case STAT_PNAME:
		/* just copy the process name. both fields are STAT_COL_LEN */
		strcpy(p->pname, field_buf);
		break;

	case STAT_T0:
		p->t0 = strtoul(field_buf, &endptr, 10);
		/* if field wasn't completely parsed something is fundamentally
		 * wrong with the stat file */
		if (*endptr != '\0') {
			go(GO_ERR, "Error: failed to parse process start "
				   "time from /proc/PID/stat\n");
			p->t0 = 0;
			success = E_FAIL;
		}
		break;

	default:
		/* default case is we are not interested on the field in this
		 * index, so return E_SUCCESS */
		break;
	}

	return success;
}


int parse_stat_file (unsigned pid, struct proc *p)
{
	FILE *file;
	char filename[FILENAME_BUF_LEN];

	snprintf(filename, FILENAME_BUF_LEN, "/proc/%u/stat", pid);
	file = fopen(filename, "r");

	if (file == NULL) {
		/* check if error is file not existing (which is ok, the
		 * process has terminated) or if some other error happened */
		if (errno != ENOENT)
			error(0, errno, "parse_proc()");
		return E_FAIL;
	}

	/* loop the fields */
	for (unsigned field = 0; field < STAT_FIELDS; ++field) {
		char field_buf[STAT_COL_LEN];

		/* get next field */
		for (int i = 0; ; ++i) {
			int ch = fgetc(file);

			/* if ch is field separator terminate str and stop */
			if (ch == ' ' ||  ch == EOF) {
				field_buf[i] = '\0';
				break;
			} else {
				field_buf[i] = (char) ch;
			}
		}

		/* if handling a required field fails, bail out */
		if (handle_field(field, field_buf, p) !=  E_SUCCESS)
			break;
	}

	fclose(file);

	return validate_proc(p) ? E_SUCCESS : E_FAIL;
}


/* if PID or start time are left uninialized return false */
bool proc_eq (const struct proc * const p1, const struct proc * const p2)
{
	if (p1->pid == p2->pid && p1->t0 == p2->t0)
		return true;
	else
		return false;
}


bool validate_proc (const struct proc * const p)
{
	bool valid = true;

	if (p->pid == 0 || p->t0 == 0)
		valid = false;

	return valid;
}
