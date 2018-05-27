#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "context.h"
#include "error.h"

static char*
check_path (
	char const *const path,
	char *bin_name
) {
	char		*fullpath = NULL;

	if (strstr(bin_name, "./") == bin_name && asprintf(&fullpath, "%s", bin_name) < 0) {
		ft_exit_perror(ASPRINTF_FAILED, NULL);
	} else if (fullpath == NULL && strstr(bin_name, "/") == bin_name && asprintf(&fullpath, "%s", bin_name) < 0) {
		ft_exit_perror(ASPRINTF_FAILED, NULL);
	} else if (fullpath == NULL && asprintf(&fullpath, "%s/%s", path, bin_name) < 0) {
		ft_exit_perror(ASPRINTF_FAILED, NULL);
	} else {
		if (access(fullpath, F_OK) < 0) {
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
	char *bin_name
) {
	char	*env_path = secure_getenv("PATH");
	char	*path = NULL;
	char	*fullpath = NULL;

	if (env_path == NULL) {
		ft_exit_perror(GETENV_FAILED, NULL);
	} else {
		if (strstr(bin_name, "./") == bin_name || strstr(bin_name, "/") == bin_name) {
			return check_path(path, bin_name);
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
