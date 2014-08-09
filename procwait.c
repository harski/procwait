/* Copyright 2013-2014 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "go.h"
#include "stat.h"

#define PROGNAME "procwait"

#define LICENSE_STR "Copyright 2013-2014 Tuomo Hartikainen "\
			"<tth@harski.org>.\n"\
			"Licensed under the 2-clause BSD license."


struct options {
	int action;	/* selected action */
	unsigned pid;	/* pid to wait for termimation */
	unsigned sleep; /* seconds to sleep between termination polls */
};

/* available actions */
enum {
	A_PROCWAIT,
	A_VERSION,
	A_HELP
};


static int do_action (const struct options * const opt);
static void load_default_opts (struct options *opt);
static int parse_options (int argc, char **argv, struct options *opt);
static void print_help ();
static int procwait (const struct options * const opt);


int main (int argc, char **argv)
{
	struct options opt;
	int retval;

	/* set up runtime options */
	load_default_opts(&opt);
	retval = parse_options(argc, argv, &opt);

	/* do the specified action */
	if (retval == E_SUCCESS)
		retval = do_action(&opt);

	return retval;
}


static int do_action (const struct options * const opt)
{
	int retval = E_SUCCESS;

	switch (opt->action) {
	case A_PROCWAIT:
		retval = procwait(opt);
		break;

	case A_VERSION:
		go(GO_ESS, "%s version %s\n", PROGNAME, VERSION);
		go(GO_ESS, "%s\n", LICENSE_STR);
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
	opt->action = A_PROCWAIT;
	opt->pid = 0;
	opt->sleep = 1;
	go_set_lvl(GO_NORMAL);

#if DEBUG
	go_set_lvl(GO_VERBOSE);
#endif
}


/* returns E_SUCCESS on success, error code for errors */
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
				go(GO_ERR,
					"Invalid sleep value '%s'\n",
					optarg);
				retval = E_INVAL;
			}
			break;
		case 'V':
			opt->action = A_VERSION;
			break;
		case 'v':
			go_set_lvl(GO_VERBOSE);
			break;
		default:
			/* unknown option: quit */
			retval = E_INVAL;
		}
	}

	/* if argv parsing has already failed or a secondary action has been
	 * selected PID parsing is not necessary */
	if (retval == E_INVAL || opt->action != A_PROCWAIT)
		return retval;

	/* check if PID is supplied */
	if (optind != argc) {
		tmpul = strtoul(argv[optind], &tmpstr, 10);
		if (*tmpstr == '\0' && tmpul != 0) {
			opt->pid = (unsigned) tmpul;
		} else {
			go(GO_ERR, "Invalid PID '%s'\n", argv[optind]);
			retval = E_INVAL;
		}
	}

	return retval;
}


static void print_help ()
{
	go(GO_ESS, "Usage: %s [OPTIONS] PID\n\n", PROGNAME);
	go(GO_ESS, "Options:\n");

	go(GO_ESS, "-h, --help\n"
		"\tPrint this help.\n");

	go(GO_ESS, "-s NUM, --sleep NUM\n"
		"\tSleep NUM seconds between process checks.\n");

	go(GO_ESS, "-v, --verbose\n"
		"\tBe verbose.\n");

	go(GO_ESS, "-V, --version\n"
		"\tPrint version information.\n");
}


static int procwait (const struct options * const opt)
{
	bool wait = true;

	/* initial stat struct that is compared against subsequent reads to
	 * determine if the current process with specified PID is still the
	 * same we are waiting to terminate. */
	struct stat proc = { 0, "", 0 };

	/* check that process is running to begin with */
	if (!parse_stat_file(opt->pid, &proc)) {
		go(GO_MESS, "Process %u not running\n", opt->pid);
		return !E_SUCCESS;
	}

	go(GO_INFO, "Waiting for PID %u to terminate\n", proc.pid);

	do {
		sleep(opt->sleep);

		/* read current stat file of PID */
		struct stat tmp = { 0, "", 0 };
		parse_stat_file(proc.pid, &tmp);

		/* check that the process is still the same */
		if (!stat_eq(&proc, &tmp))
			wait = false;

	} while (wait);

	return E_SUCCESS;
}
