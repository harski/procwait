/* Copyright 2013 Tuomo Hartikainen <tuomo.hartikainen@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
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

static unsigned long long get_process_start_time (unsigned pid);
static void load_default_opts (struct options *opt);
static int parse_options (int argc, char **argv, struct options *opt, unsigned *pid);


int main (int argc, char **argv)
{
	struct options opt;
	struct stat proc;
	int retval;

	debug_print("%s", "Loading default options\n");
	load_default_opts(&opt);
	debug_print("%s", "Loading default options done!\n");

	debug_print("%s", "Parsing options\n");
	retval = parse_options(argc, argv, &opt, &(proc.pid));
	debug_print("%s", "Parsing options done\n");

	if (retval) {
		fprintf(stderr, "An error '%d' occured during option parsing, "
				"quitting...\n", retval);
		return retval > 0 ? retval : 1;
	}

	proc.t0 = get_process_start_time(proc.pid);

	if (opt.verbose)
		printf("Waiting for PID %u to terminate\n", proc.pid);

	while (1) {
		struct stat tmp;
		tmp.t0 = get_process_start_time(proc.pid);
		if ( proc.t0 == tmp.t0) {
			debug_print("Process running, sleeping %u seconds\n", opt.sleep);
			sleep(opt.sleep);
		} else {
			break;
		}
	}

	return 0;
}


static long long unsigned get_process_start_time (unsigned pid)
{
	FILE *file;
	char filename[FILENAME_BUF_LEN];
	char time_str[STAT_COL_LEN];
	unsigned long long start_time;
	int len; /* start time_str length */
	int ws; /* Counter for whitespace */
	int tmp; /* int-width tmp var for characters in /proc/#/stat */
	char cbuf; /* char tmp var for characters in /proc/#/stat */

	snprintf(filename, FILENAME_BUF_LEN, "/proc/%u/stat", pid);
	file = fopen(filename, "r");

	/* file could not be read: does it exist? */
	if (file == NULL)
		goto pst_err;

	/* find start index */
	for (ws = 0; ws < STAT_T0; ) {
		tmp = fgetc(file);
		if (tmp == EOF)
			goto pst_close_err;

		cbuf = (char) tmp;
		if (cbuf == ' ')
			++ws;
	}

	/* get start time as string */
	len = 0;
	while (1) {
		tmp = (char) fgetc(file);
		if (cbuf == EOF)
			goto pst_close_err;

		cbuf = (char) tmp;
		if (cbuf == ' ') {
			time_str[len] = cbuf;
			break;
		}
		time_str[len++] = cbuf;
	}

	if (len == 0)
		goto pst_close_err;

	/* We don't need the file anymore */
	fclose(file);

	start_time = strtoull(time_str, NULL, 10);

	return start_time;

pst_close_err:
	fclose(file);

pst_err:
	return 0;
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
static int parse_options (int argc, char **argv, struct options *opt, unsigned *pid)
{
	int option;
	int retval = 0;

	/* temp values for argv validation */
	char *tmpstr;
	unsigned long tmpul;

	opterr = 0; /* Don't print getopt errors automatically */

	while ((option = getopt (argc, argv, "s:v")) != -1 && !retval) {
		switch (option) {
		case 's':
			debug_print("Found option '%c'\n", (char) option);
			/* validate ctmp to sleep_int */
			tmpul = strtoul(optarg, &tmpstr, 10);
			if (*tmpstr == '\0') {
				opt->sleep = (unsigned) tmpul;
			} else {
				fprintf(stderr,
					"Error: Invalid sleep value '%s'\n",
					optarg);
				retval = 1;
			}
			break;
		case 'v':
			debug_print("Found option '%c'\n", (char) option);
			opt->verbose = 1;
			break;
		default:
			fprintf(stderr, "Error: unknown option '%c'\n", option);
			retval = 2;
		}
	}

	/* check if PID is supplied */
	if (optind == argc) {
		fprintf(stderr, "Error: PID missing\n");
		retval = 3;
	} else {
		tmpul = strtoul(argv[optind], &tmpstr, 10);
		if (*tmpstr == '\0') {
			*pid = (unsigned) tmpul;
		} else {
			fprintf(stderr, "Error: Invalid PID '%s'\n", optarg);
			retval = 4;
		}
	}

	return retval;
}

