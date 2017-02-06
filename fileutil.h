/* Copyright 2017 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef PW_FILEUTIL_H
#define PW_FILEUTIL_H

#include "proc.h"
#include "queue.h"

struct file {
	char * path;
	unsigned char type;
	SLIST_ENTRY(file) files;
};

SLIST_HEAD(filelist, file);

/* deep free struct file */
void file_destroy (struct file * f);

/* filter out and free all non-numeric /proc -dirs */
void filter_numeric_dirs (struct filelist * filelist);

/* find PIDs for process pname from filelist, and put the matching processes in
 * proclist */
int find_pid (struct filelist * filelist, const char * pname,
	      struct proclist * proclist);

/* get files in a directory */
int get_dir_contents (const char * dirpath, struct filelist * filelist);

/* get directies in a directory */
int get_dir_dirs (const char * dirpath, struct filelist * filelist);

/* get only numeric directories inside /proc */
int get_proc_dirs (struct filelist * fl);

#endif /* PW_FILEUTIL_H */
