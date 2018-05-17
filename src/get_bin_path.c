#define _GNU_SOURCE /* asprintf */
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "error.h"

static char*
check_path (
	char const *const path,
	char const *const bin_name
) {
	struct stat		s;
	char			*fullpath = NULL;

	if (asprintf(&fullpath, "%s/%s", path, bin_name) < 0) {
		ft_exit_perror(ASPRINTF_FAILED, NULL);
	} else {
		if (stat(fullpath, &s) < 0) {
			free(fullpath);
			fullpath = NULL;
			return NULL;
		} else {
			return fullpath;
		}
	}
}

/*
** Get PATH env and check each one of these to find the bin_name fullpath.
** Return a malloced fullpath string.
*/

char*
get_bin_path (
	char const *const bin_name
) {
	char	*env_path = secure_getenv("PATH");
	char	*path = NULL;
	char	*fullpath;

	if (env_path == NULL) {
		ft_exit_perror(GETENV_FAILED, NULL);
	} else {
		if (strstr(bin_name, "./") == bin_name || strstr(bin_name, "/") == bin_name) {
			return strdup(bin_name);
		} else {
			while ((path = strsep(&env_path, ":")) != NULL) {
				if ((fullpath = check_path(path, bin_name))) {
					return fullpath;
				}
			}
			return NULL;
		}
	}
return NULL;
}
