/* Copyright 2013 Tuomo Hartikainen <tuomo.hartikainen@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#define _XOPEN_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "pidwait.h"
#include "stat.h"


struct options {
	unsigned pid;
	unsigned sleep;
	int verbose;
};

static void load_default_opts (struct options *opt);
static int parse_options (int argc, char **argv, struct options *opt);


int main (int argc, char **argv)
{
	struct options opt;
	struct stat proc = { 0, "", 0 };
	int retval;
	int wait = 1;

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

	while (wait) {
		struct stat tmp = { 0, "", 0 };
		parse_stat_file(proc.pid, &tmp);

		if (stat_eq(&proc, &tmp)) {
			debug_print("Process running, sleeping %u seconds\n", opt.sleep);
			sleep(opt.sleep);
		} else {
			wait = 0;
		}
	}

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
static int parse_options (int argc, char **argv, struct options *opt)
{
	int option;
	int retval = E_SUCCESS;

	/* temp values for argv validation */
	char *tmpstr;
	unsigned long tmpul;

	opterr = 0; /* Don't print getopt errors automatically */

	while (!retval) {
		int option_index = 0;
		static struct option long_options[] = {
			{"sleep",	required_argument,	0, 's'},
			{"verbose",	no_argument,		0, 'v'},
			{0,		0,			0,  0 }
		};

		option = getopt_long(argc, argv, "s:v", long_options,
					&option_index);
		if (option == -1)
			break;

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


