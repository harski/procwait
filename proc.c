/* Copyright 2013-2017 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "go.h"
#include "proc.h"
#include "strutil.h"

#define FILENAME_BUF_LEN 32

/* Field indexes for file /proc/PID/stat */
enum {
	STAT_PID = 0,
	STAT_PNAME = 1,
	STAT_T0 = 21,
	STAT_FIELDS
};


static int handle_field (const unsigned field, const char * const field_buf,
			 struct proc * restrict p);


static int handle_field (const unsigned field, const char * const field_buf,
			 struct proc * restrict p)
{
	int success = E_SUCCESS;

	switch (field) {
	case STAT_PID:
		success = strtou(field_buf, &(p->pid));
		break;

	case STAT_PNAME:
		/* Just copy the process name, both field's width is
		 * STAT_COL_LEN */
		strcpy(p->name, field_buf);
		break;

	case STAT_T0:
		success = strtou(field_buf, &(p->t0));
		break;

	default:
		/* default case is we are not interested on the field in this
		 * index, so return E_SUCCESS */
		break;
	}

	return success;
}


int parse_stat_file (const char * path, struct proc * p)
{
	int retval = E_SUCCESS;
	FILE *file = fopen(path, "r");

	if (file == NULL) {
		/* check if error is 'file does not exist' (which is ok, the
		 * process has terminated) or if some other error happened */
		if (errno != ENOENT)
			error(0, errno, "parse_proc()");
		return E_FAIL;
	}

	/* loop the fields */
	for (unsigned field = 0; field < STAT_FIELDS; ++field) {
		char field_buf[STAT_COL_LEN];

		/* get next field */
		int field_succ = get_next_field(file, field_buf, STAT_COL_LEN);

		if (field_succ == STRUTIL_EXIT_TRUNCATED) {
			go(GO_WARN, "Parsing field number %u in %s failed: "
				    "too long record\n", field, path);
		}

		/* if handling a required field fails, bail out */
		if (handle_field(field, field_buf, p) !=  E_SUCCESS) {
			retval = E_FAIL;
			break;
		}
	}

	fclose(file);
	return retval;
}


int parse_stat_pid (const unsigned pid, struct proc * restrict p)
{
	char filename[FILENAME_BUF_LEN];

	/* TODO: check if FILENAME_BUF_LEN is sufficient */
	snprintf(filename, FILENAME_BUF_LEN, "/proc/%u/stat", pid);
	parse_stat_file (filename, p);

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
