#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* C global */

extern const char *__progname;

/*
** Display an error message and exit(EXIT_FAILURE)
*/

void
ft_exit_error(
	char const *const error,
	char const *const bin_name
) {
	if (bin_name == NULL) {
		dprintf(2, "%s: %s\n", __progname, error);
	} else {
		dprintf(2, "%s: %s '%s'\n", __progname, error, bin_name);
	}
	exit(EXIT_FAILURE);
}

/*
** Format an error string, launch perror, free the error string and
** exit(EXIT_FAILURE)
*/

void
ft_exit_perror(
	char const *const error,
	char const *const bin_name
) {
	char	*error_str;

	if (bin_name == NULL) {
		asprintf(&error_str, "%s: %s", __progname, error);
	} else {
		asprintf(&error_str, "%s: %s '%s'", __progname, error, bin_name);
	}
	perror(error_str);
	free(error_str);
	exit(EXIT_FAILURE);
}
