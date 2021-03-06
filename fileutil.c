/* Copyright 2017 Tuomo Hartikainen <tth@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "fileutil.h"

#define STAT_PATH_LEN 32


static bool is_numeric (const char *str)
{
	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] < 48 || str[i] > 57)
			return 0;
	}

	return 1;
}


void file_destroy (struct file * f)
{
	free(f->path);
	free(f);
}


void filter_numeric_dirs (struct filelist * filelist)
{
	struct file *fp, *fp_tmp;
	char *str;

	SLIST_FOREACH_SAFE(fp, filelist, files, fp_tmp) {
		/* skip "/proc/" */
		str = fp->path + 6;
		if (!is_numeric(str)) {
			SLIST_REMOVE(filelist, fp, file, files);
			file_destroy(fp);
		}
	}
}


/* search for a process called pname from the filelist, and if matches are
 * found put them on proclist */
int find_pid (struct filelist * filelist, const char * pname,
	      struct proclist * proclist)
{
	struct proc proc, *proc_tmp;
	struct file *fp;
	char stat_path[STAT_PATH_LEN];
	int cnt = 0;

	SLIST_FOREACH(fp, filelist, files) {
		snprintf(stat_path, STAT_PATH_LEN, "%s/stat", fp->path);
		parse_stat_file(stat_path, &proc);
		if (!strcmp(pname, proc.name)) {
			proc_tmp = malloc(sizeof(struct proc));
			*proc_tmp = proc;
			SLIST_INSERT_HEAD(proclist, proc_tmp, procs);
			++cnt;
		}
	}

	return cnt;
}


int get_dir_contents (const char * dirpath, struct filelist * filelist)
{
	size_t dplen = strlen(dirpath);
	DIR * dir = opendir(dirpath);
	struct dirent *de;

	if (dir == NULL) {
		return E_FAIL;
	}

	while ((de = readdir(dir)) != NULL) {
		size_t pathlen = dplen + 1 + strlen(de->d_name) + 1;
		struct file *f = malloc(sizeof(struct file));
		f->path = malloc(sizeof(char) * pathlen);
		snprintf(f->path, pathlen, "%s/%s", dirpath, de->d_name);
		f->type = de->d_type;

		SLIST_INSERT_HEAD(filelist, f, files);
	}

	closedir(dir);
	return E_SUCCESS;
}


int get_dir_dirs (const char * dirpath, struct filelist * filelist)
{
	struct filelist tmp_list;
	SLIST_INIT(&tmp_list);
	get_dir_contents(dirpath, &tmp_list);

	struct file *f;
	while ((f = SLIST_FIRST(&tmp_list)) != NULL) {
		SLIST_REMOVE_HEAD(&tmp_list, files);

		if (f->type == DT_DIR) {
			SLIST_INSERT_HEAD(filelist, f, files);
		} else {
			file_destroy(f);
		}
	}

	return E_SUCCESS;
}


int get_proc_dirs (struct filelist * fl)
{
	get_dir_dirs("/proc", fl);
	filter_numeric_dirs(fl);
	return E_SUCCESS;
}
