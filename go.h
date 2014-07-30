/* Copyright 2014 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

enum GO_LVL {
	GO_INFO,
	GO_MESS,
	GO_ESS,
	GO_WARN,
	GO_ERR
};

enum GO_PRINT_LVL {
	GO_QUIET,
	GO_NORMAL,
	GO_VERBOSE
};


/* output messages if GO_LVL is high enough compared to current GO_PRINT_LVL */
void go (enum GO_LVL lvl, const char *fmt, ...);

/* set GO_PRINT_LVL */
void go_set_lvl (enum GO_PRINT_LVL lvl);