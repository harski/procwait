/* Copyright 2013-2014 Tuomo Hartikainen <tuomo.hartikainen@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#define _XOPEN_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "procwait.h"
#include "stat.h"

#define PROGNAME "procwait"
#define VERSION_STR "0.1"

#define LICENSE_STR "Copyright 2013-2014 Tuomo Hartikainen "\
			"<tuomo.hartikainen@harski.org>.\n"\
			"Licensed under the 2-clause BSD license."


struct options {
	int action;
	unsigned pid;
	unsigned sleep;
	int verbose;
};

enum {
	A_NONE,
	A_VERSION,
	A_HELP
};


static int do_secondary_action (const struct options * const opt);
static void load_default_opts (struct options *opt);
static int parse_options (int argc, char **argv, struct options *opt);
static void print_help ();


int main (int argc, char **argv)
{
	struct options opt;
	int retval;

	load_default_opts(&opt);
	retval = parse_options(argc, argv, &opt);

	if (retval != E_SUCCESS)
		goto exit_error;

	if (opt.pid && opt.action == A_NONE) {
		int wait = 1;
		struct stat proc = { 0, "", 0 };

		if (!parse_stat_file(opt.pid, &proc)) {
			printf("Process %u not running\n", opt.pid);
			retval = 0; /* not a "real" error */
			goto exit_error;
		}

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
	} else {
		do_secondary_action (&opt);
	}

	retval = 0;

exit_error:
	return retval;
}


static int do_secondary_action (const struct options * const opt)
{
	int retval = 0;

	switch (opt->action) {
	case A_VERSION:
		printf("%s version %s\n", PROGNAME, VERSION_STR);
		printf("%s\n", LICENSE_STR);
		break;

	case A_HELP:
		print_help();
		break;

	default:
		retval = E_INVAL;
	}

	return retval;
}


static void load_default_opts (struct options *opt)
{
	opt->action = A_NONE;
	opt->pid = 0;
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
			{"help",	no_argument,		0, 'H'},
			{"sleep",	required_argument,	0, 's'},
			{"verbose",	no_argument,		0, 'v'},
			{"version",	no_argument,		0, 'V'},
			{"usage",	no_argument,		0, 'H'},
			{0,		0,			0,  0 }
		};

		option = getopt_long(argc, argv, "s:v", long_options,
					&option_index);
		if (option == -1)
			break;

		switch (option) {
		case 'H':
			opt->action = A_HELP;
			break;
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
		case 'V':
			opt->action = A_VERSION;
			break;
		case 'v':
			opt->verbose = 1;
			break;
		default:
			/* TODO: %c is always '?' */
			fprintf(stderr, "Error: unknown option '%c'\n", option);
			retval = E_INVAL;
		}
	}

	/* check if PID is supplied */
	if (optind != argc) {
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


static void print_help ()
{
	printf("Usage: %s [OPTIONS] PID\n\n", PROGNAME);
	printf("Options:\n");

	printf("--help | --usage\n");
	printf("\tPrint this help.\n\n");

	printf("-s | --sleep\n");
	printf("\tSleep time (in seconds) between process checks.\n\n");

	printf("-v | --verbose\n");
	printf("\tBe verbose.\n\n");

	printf("-V | --version\n");
	printf("\tPrint version information.\n");
}

