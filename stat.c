/* Copyright 2013 Tuomo Hartikainen <tuomo.hartikainen@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "pidwait.h"
#include "stat.h"

#define FILENAME_BUF_LEN 32

/* Field indexes for file /proc/PID/stat */
enum {
	STAT_PID,
	STAT_PNAME,
	STAT_T0 = 21
};


static int handle_field (unsigned field, const char *field_buf, struct stat *s);


static int handle_field (unsigned field, const char *field_buf, struct stat *s)
{
	int success = E_SUCCESS;
	char *tmp;

	switch (field) {
	/* default case is we are not interested on the field in this index,
	 * so return E_SUCCESS */
	case STAT_PID:
		s->pid = strtoul(field_buf, &tmp, 10);
		if (*tmp != '\0') {
			fprintf(stderr, "Error: failed to parse PID field from"
					" /proc/PID/stat\n");
			s->pid = 0;
			success = E_FAIL;
			debug_print("field_buf = '%s'\n", field_buf);
			debug_print("field_buf = '%s'\n", tmp);
		}
		debug_print("set pid to %u\n", s->pid);
		break;

	case STAT_PNAME:
		/* both fields are of STAT_COL_LEN */
		strcpy(s->pname, field_buf);
		break;

	case STAT_T0:
		s->t0 = strtoul(field_buf, &tmp, 10);
		if (*tmp != '\0') {
			fprintf(stderr, "Error: failed to parse process start"
					" time from /proc/PID/stat\n");
			s->t0 = 0;
			success = E_FAIL;
		}
		debug_print("set t0 to %u\n", s->t0);
		break;
	}

	return success;
}


int parse_stat_file (unsigned pid, struct stat *s)
{
	FILE *file;
	char filename[FILENAME_BUF_LEN];
	char field_buf[STAT_COL_LEN];
	unsigned field = 0;
	int i, done;
	int retval = E_SUCCESS;

	snprintf(filename, FILENAME_BUF_LEN, "/proc/%u/stat", pid);
	file = fopen(filename, "r");

	/* file could not be read: does it exist? */
	if (file == NULL) {
		int err = errno;
		if (err != ENOENT) {
			error(0, err, "parse_stat_file()");
		}
		goto pst_err;
	}

	/* loop the fields */
	for (i = 0, done = 0; !done; ++i) {
		int ch = fgetc(file);

		if (ch == EOF)
			done = 1;

		if (ch == ' ' || ch == EOF) {
			field_buf[i] = '\0';

			/* if handling a required field fails, bail out */
			retval = handle_field(field, field_buf, s);
			if (retval !=  E_SUCCESS)
				goto pst_close_err;

			/* now start buffering the next field */
			i = 0;
			++field;
		}
		field_buf[i] = (char) ch;
	}

	fclose(file);

	return validate_stat_file(s);

pst_close_err:
	fclose(file);

pst_err:
	return 0;
}


int stat_eq (const struct stat * const a, const struct stat * const b)
{
	if (a->pid == b->pid && a->t0 == b->t0)
		return 1;
	else
		return 0;
}


int validate_stat_file (const struct stat * const s)
{
	int valid = 1;

	if (s->pid == 0 || s->t0 == 0)
		valid = 0;

	return valid;
}

