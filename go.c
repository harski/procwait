/* Copyright 2014 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "go.h"

static enum GO_PRINT_LVL go_lvl = GO_NORMAL;


void go (enum GO_LVL lvl, const char *fmt, ...)
{
	bool print = false;
	FILE *os = stdout;

	switch (lvl) {
	case GO_INFO:
		if (go_lvl == GO_VERBOSE) {
			print = true;
		}
		break;

	case GO_MESS:
		if (go_lvl <= GO_NORMAL) {
			print = true;
		}
		break;

	case GO_ESS:
		/* always print essential messages */
		print = true;
		break;

	case GO_WARN:
		/* don't print warnings if quiet */
		if (go_lvl > GO_QUIET) {
			print = true;
			os = stderr;

			/* print a prefix */
			fprintf(stderr, "Warning: ");
		}
		break;

	case GO_ERR:
		/* always print errors */
		print = true;
		os = stderr;

		/* print a prefix */
		fprintf(stderr, "Error: ");
		break;

	default:
		/* this should NEVER happen */
		assert(0);
	}

	if (print) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(os, fmt, ap);
		va_end(ap);
	}
}


void go_set_lvl (enum GO_PRINT_LVL lvl)
{
	go_lvl = lvl;
}
