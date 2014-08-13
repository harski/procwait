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
	STAT_T0 = 21
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
			go(GO_ERR, "Error: failed to parse process start"
					" time from /proc/PID/stat\n");
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
	char field_buf[STAT_COL_LEN];
	unsigned field;
	bool done;
	int i;
	int retval = E_SUCCESS;

	snprintf(filename, FILENAME_BUF_LEN, "/proc/%u/stat", pid);
	file = fopen(filename, "r");

	if (file == NULL) {
		/* check if error is file not existing (which is ok, the
		 * process has terminated) or if some other error happened */
		if (errno != ENOENT) {
			error(0, errno, "parse_proc()");
		}
		goto pst_err;
	}

	/* loop the fields */
	for (i = 0, done = false, field = 0; !done; ++i) {
		int ch = fgetc(file);

		if (ch == EOF)
			done = true;

		if (ch == ' ' || ch == EOF) {
			field_buf[i] = '\0';

			/* if handling a required field fails, bail out */
			retval = handle_field(field, field_buf, p);
			if (retval !=  E_SUCCESS)
				goto pst_close_err;

			/* now start buffering the next field */
			i = 0;
			++field;
		}
		field_buf[i] = (char) ch;
	}

	fclose(file);

	return validate_proc(p);

pst_close_err:
	fclose(file);

pst_err:
	return 0;
}


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
