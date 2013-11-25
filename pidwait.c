/* Copyright 2013 Tuomo Hartikainen <tuomo.hartikainen@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef DEBUG
	#define DEBUG 0
#endif

#define debug_print(fmt, ...) \
	do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
				__LINE__, __func__, __VA_ARGS__); } while (0)

#define FILENAME_BUF_LEN 32
#define STAT_COL_LEN 32


struct options {
	unsigned pid;
	unsigned sleep;
	int verbose;
};

/* contents of the parsed file /proc/PID/stat */
struct stat {
	unsigned pid;
	char pname[STAT_COL_LEN];
	unsigned t0;
};

/* Field indexes for file /proc/PID/stat */
enum {
	STAT_PID,
	STAT_PNAME,
	STAT_T0 = 21
};

enum Error {
	E_SUCCESS,
	E_FAIL,
	E_INVAL
};

static int handle_field (unsigned field, const char *field_buf, struct stat *s);
static void load_default_opts (struct options *opt);
static int parse_options (int argc, char **argv, struct options *opt);
static int parse_stat_file (unsigned pid, struct stat *s);
static inline int stat_eq (const struct stat const *a, const struct stat const *b);
static int validate_stat_file (const struct stat *s);


int main (int argc, char **argv)
{
	struct options opt;
	struct stat proc = { 0, "", 0 };
	int retval;

	debug_print("%s", "Loading default options\n");
	load_default_opts(&opt);
	debug_print("%s", "Loading default options done!\n");

	debug_print("%s", "Parsing options\n");
	retval = parse_options(argc, argv, &opt);
	debug_print("%s", "Parsing options done\n");

	if (retval != E_SUCCESS)
		return retval;

	if (!parse_stat_file(opt.pid, &proc)) {
		printf("Process %u not running\n", opt.pid);
		return 0;
	}
	debug_print("PID is %u after parsing stat\n", proc.pid);

	if (opt.verbose)
		printf("Waiting for PID %u to terminate\n", proc.pid);

	while (1) {
		struct stat tmp = { 0, "", 0 };
		parse_stat_file(proc.pid, &tmp);

		if (stat_eq(&proc, &tmp)) {
			debug_print("Process running, sleeping %u seconds\n", opt.sleep);
			sleep(opt.sleep);
		} else {
			break;
		}
	}

	return 0;
}


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


static void load_default_opts (struct options *opt)
{
	opt->sleep = 1;
	opt->verbose = 0;

#if DEBUG
	opt->verbose = 1;
#endif
}


/* returns 0 on success, error code for errors */
static int parse_options (int argc, char **argv, struct options *opt)
{
	int option;
	int retval = E_SUCCESS;

	/* temp values for argv validation */
	char *tmpstr;
	unsigned long tmpul;

	opterr = 0; /* Don't print getopt errors automatically */

	while ((option = getopt(argc, argv, "s:v")) != -1 && !retval) {
		switch (option) {
		case 's':
			/* validate ctmp to sleep_int */
			tmpul = strtoul(optarg, &tmpstr, 10);
			if (*tmpstr == '\0') {
				opt->sleep = (unsigned) tmpul;
			} else {
				fprintf(stderr,
					"Error: Invalid sleep value '%s'\n",
					optarg);
				retval = E_INVAL;
			}
			break;
		case 'v':
			opt->verbose = 1;
			break;
		default:
			fprintf(stderr, "Error: unknown option '%c'\n", option);
			retval = E_INVAL;
		}
	}

	/* check if PID is supplied */
	if (optind == argc) {
		fprintf(stderr, "Error: PID missing\n");
		retval = E_INVAL;
	} else {
		tmpul = strtoul(argv[optind], &tmpstr, 10);
		if (*tmpstr == '\0') {
			opt->pid = (unsigned) tmpul;
			debug_print("PID to follow is %lu\n", tmpul);
		} else {
			fprintf(stderr, "Error: Invalid PID '%s'\n", optarg);
			retval = E_INVAL;
		}
	}

	return retval;
}


static int parse_stat_file (unsigned pid, struct stat *s)
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
	if (file == NULL)
		goto pst_err;

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


static inline int stat_eq (const struct stat const *a, const struct stat const *b)
{
	if (a->pid == b->pid && a->t0 == b->t0)
		return 1;
	else
		return 0;
}


int validate_stat_file (const struct stat *s)
{
	int valid = 1;

	if (s->pid == 0 || s->t0 == 0)
		valid = 0;

	return valid;
}

