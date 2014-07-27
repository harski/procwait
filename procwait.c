/* Copyright 2013-2014 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "procwait.h"
#include "stat.h"

#define PROGNAME "procwait"
#define VERSION_STR "0.2"

#define LICENSE_STR "Copyright 2013-2014 Tuomo Hartikainen "\
			"<tth@harski.org>.\n"\
			"Licensed under the 2-clause BSD license."


struct options {
	int action;
	unsigned pid;
	unsigned sleep;
	int verbose;
};

enum {
	A_DEFAULT,
	A_VERSION,
	A_HELP
};


static int do_secondary_action (const struct options * const opt);
static void load_default_opts (struct options *opt);
static int parse_options (int argc, char **argv, struct options *opt);
static void print_help ();
static int procwait (const struct options * const opt);


int main (int argc, char **argv)
{
	struct options opt;
	int retval;

	load_default_opts(&opt);
	if ((retval = parse_options(argc, argv, &opt) != E_SUCCESS))
		goto exit_error;

	if (opt.action == A_DEFAULT)
		retval = procwait(&opt);
	else
		retval = do_secondary_action(&opt);

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
	opt->action = A_DEFAULT;
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
	int retval = E_SUCCESS;

	/* temp values for argv validation */
	char *tmpstr;
	unsigned long tmpul;

	while (!retval) {
		int option;
		int option_index = 0;
		static struct option long_options[] = {
			{"help",	no_argument,		0, 'h'},
			{"sleep",	required_argument,	0, 's'},
			{"verbose",	no_argument,		0, 'v'},
			{"version",	no_argument,		0, 'V'},
			{0,		0,			0,  0 }
		};

		option = getopt_long(argc, argv, "hs:v", long_options,
					&option_index);
		if (option == -1)
			break;

		switch (option) {
		case 'h':
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
			/* unknown option: quit */
			retval = E_INVAL;
		}
	}

	if (retval == E_INVAL || opt->action != A_DEFAULT)
		goto opt_exit;

	/* check if PID is supplied */
	if (optind != argc) {
		tmpul = strtoul(argv[optind], &tmpstr, 10);
		if (*tmpstr == '\0' && tmpul != 0) {
			opt->pid = (unsigned) tmpul;
			debug_print("PID to follow is %lu\n", tmpul);
		} else {
			fprintf(stderr, "Error: Invalid PID '%s'\n", argv[optind]);
			retval = E_INVAL;
		}
	}

opt_exit:
	return retval;
}


static void print_help ()
{
	printf("Usage: %s [OPTIONS] PID\n\n", PROGNAME);
	printf("Options:\n");

	printf("-h, --help\n");
	printf("\tPrint this help.\n");

	printf("-s NUM, --sleep NUM\n");
	printf("\tSleep NUM seconds between process checks.\n");

	printf("-v, --verbose\n");
	printf("\tBe verbose.\n");

	printf("-V, --version\n");
	printf("\tPrint version information.\n");
}


static int procwait (const struct options * const opt)
{
	int retval = E_SUCCESS;
	int wait = 1;
	struct stat proc = { 0, "", 0 };

	if (!parse_stat_file(opt->pid, &proc)) {
		printf("Process %u not running\n", opt->pid);
		return !E_SUCCESS;
	}

	if (opt->verbose)
		printf("Waiting for PID %u to terminate\n", proc.pid);

	while (wait) {
		struct stat tmp = { 0, "", 0 };
		parse_stat_file(proc.pid, &tmp);

		if (stat_eq(&proc, &tmp)) {
			debug_print("Process running, sleeping %u seconds\n", opt->sleep);
			sleep(opt->sleep);
		} else {
			wait = 0;
		}
	}

	return retval;
}
