/* Copyright 2013-2014 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "error.h"
#include "go.h"
#include "proc.h"
#include "queue.h"
#include "strutil.h"

#define PROGNAME "procwait"

#ifndef VERSION
#define VERSION "unknown (" __DATE__ ")"
#endif

#define LICENSE_STR "Copyright 2013-2014 Tuomo Hartikainen "\
			"<tth@harski.org>.\n"\
			"Licensed under the 2-clause BSD license."


struct options {
	int action;	/* selected action */
	struct timespec sleep;
};

/* available actions */
enum {
	A_PROCWAIT,
	A_VERSION,
	A_HELP
};

static int do_action (const struct options * const opt,
		      struct proclist * restrict proclist);
static void load_default_opts (struct options * restrict opt);
static int parse_options (int argc, char **argv, struct options * restrict opt,
			  struct proclist * restrict proclist);
static int parse_sleep_time (const char * const timestr,
			     struct timespec * restrict ts);
static void print_help ();
static int procwait (const struct options * const opt,
		     struct proclist * restrict proclist);


int main (int argc, char **argv)
{
	struct options opt;
	int retval;
	struct proclist proclist;

	/* init proclist */
	SLIST_INIT(&proclist);

	/* set up runtime options */
	load_default_opts(&opt);
	retval = parse_options(argc, argv, &opt, &proclist);

	/* do the specified action */
	if (retval == E_SUCCESS)
		retval = do_action(&opt, &proclist);

	return retval;
}


static int do_action (const struct options * const opt,
		      struct proclist * restrict proclist)
{
	int retval = E_SUCCESS;

	switch (opt->action) {
	case A_PROCWAIT:
		retval = procwait(opt, proclist);
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


static void load_default_opts (struct options * restrict opt)
{
	opt->action = A_PROCWAIT;
	opt->sleep.tv_sec = 1;
	opt->sleep.tv_nsec = 0;
	go_set_lvl(GO_NORMAL);

#if DEBUG
	go_set_lvl(GO_VERBOSE);
#endif
}


/* returns E_SUCCESS on success, error code for errors */
static int parse_options (int argc, char **argv, struct options * restrict opt,
			  struct proclist * restrict proclist)
{
	int retval = E_SUCCESS;

	/* temp values for argv validation */
	unsigned tmpu;

	while (!retval) {
		int option;
		int option_index = 0;
		static struct option long_options[] = {
			{"help",	no_argument,		0, 'h'},
			{"quiet",	no_argument,		0, 'q'},
			{"sleep",	required_argument,	0, 's'},
			{"verbose",	no_argument,		0, 'v'},
			{"version",	no_argument,		0, 'V'},
			{0,		0,			0,  0 }
		};

		option = getopt_long(argc, argv, "hqs:v", long_options,
				     &option_index);
		if (option == -1)
			break;

		switch (option) {
		case 'h':
			opt->action = A_HELP;
			break;
		case 'q':
			go_set_lvl(GO_QUIET);
			break;
		case 's':
			if (parse_sleep_time(optarg, &opt->sleep) == E_INVAL) {
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
	while (optind != argc) {
		if (strtou(argv[optind], &tmpu) == E_SUCCESS) {
			/* add PID to the list */
			struct proc * proc = malloc(sizeof(struct proc));
			if (proc==NULL) {
				retval = E_FAIL;
				go(GO_ERR, "Could not allocate memory"
					   "for struct proc\n");
				break;
			}
			proc->pid = tmpu;
			SLIST_INSERT_HEAD(proclist, proc, procs);
		} else {
			go(GO_ERR, "Invalid PID '%s'\n", argv[optind]);
			retval = E_INVAL;
		}
		++optind;
	}

	return retval;
}


static int parse_sleep_time (const char * const str,
			     struct timespec * restrict ts)
{
	unsigned long tmpul;
	char *tmpstr;

	tmpul = strtoul(str, &tmpstr, 10);

	/* if valid ul (sleep time in seconds) */
	if (*tmpstr == '\0' || !strcmp(tmpstr, "s")) {
		ts->tv_sec = (time_t) tmpul;
	} else if (!strcmp(tmpstr, "ms")) {
		/* get full seconds */
		ts->tv_sec = tmpul/1000;
		/* get milliseconds */
		ts->tv_nsec = 1000000 * (tmpul % 1000);

	} else {
		return E_INVAL;
	}

	return E_SUCCESS;
}


static void print_help ()
{
	go(GO_ESS, "Usage: %s [OPTIONS] PID...\n\n", PROGNAME);
	go(GO_ESS, "Options:\n");

	go(GO_ESS, "-h, --help\n"
		   "\tPrint this help.\n");

	go(GO_ESS, "-q, --quiet\n"
		   "\tOnly print essential output and errors.\n");

	go(GO_ESS, "-s NUM[ms], --sleep NUM[ms]\n"
		   "\tSleep NUM seconds (milliseconds) between"
		   "process checks.\n");

	go(GO_ESS, "-v, --verbose\n"
		   "\tBe verbose.\n");

	go(GO_ESS, "-V, --version\n"
		   "\tPrint version information.\n");
}


static int procwait (const struct options * const opt,
		     struct proclist * restrict proclist)
{
	struct proc * proc, * tmp_proc;

	/* if list is empty, print help and error out */
	if (SLIST_EMPTY(proclist)) {
		print_help();
		return E_FAIL;
	}

	/* check that processes are running and get initial info on then */
	SLIST_FOREACH_SAFE(proc, proclist, procs, tmp_proc) {
		if (parse_stat_file(proc->pid, proc) == E_SUCCESS) {
			go(GO_MESS,
			   "Waiting for PID %u %s to terminate\n",
			   proc->pid, proc->name);
		} else {
			go(GO_MESS, "Process %u not running\n", proc->pid);
			SLIST_REMOVE(proclist, proc, proc, procs);
			free(proc);
		}
	}

	/* main wait loop */
	while (!SLIST_EMPTY(proclist)) {
		go(GO_INFO, "Sleeping for %u.%03.3u seconds\n",
		   (unsigned) opt->sleep.tv_sec,
		   (unsigned) opt->sleep.tv_nsec / 1000000);
		nanosleep(&opt->sleep, NULL);

		SLIST_FOREACH_SAFE(proc, proclist, procs, tmp_proc) {
			/* read current stat file of PID */
			struct proc tmp = { 0, "", 0, {NULL}};

			/* check that stat could be read and the  process is
			 * still the same */
			if (parse_stat_file(proc->pid, &tmp) ||
			    !proc_eq(proc, &tmp)) {
				SLIST_REMOVE(proclist,
					     proc, proc, procs);
				go(GO_MESS, "Process %u %s terminated\n",
				   proc->pid, proc->name);
				free(proc);
			}
		}
	}

	return E_SUCCESS;
}
