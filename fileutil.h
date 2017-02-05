/* Copyright 2017 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef PW_FILEUTIL_H
#define PW_FILEUTIL_H

#include "queue.h"

struct file {
	char * path;
	unsigned char type;
	SLIST_ENTRY(file) files;
};

SLIST_HEAD(filelist, file);

void file_destroy (struct file * f);
void filter_numeric_dirs (struct filelist * filelist);
int get_dir_contents (const char * dirpath, struct filelist * filelist);
int get_dir_dirs (const char * dirpath, struct filelist * filelist);

#endif /* PW_FILEUTIL_H */