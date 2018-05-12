#define _GNU_SOURCE /* dprintf */
#include <stdio.h>
#include <string.h>

#include "context.h"
#include "error.h"
#include "get_bin_path.h"
#include "options.h"
#include "output_file.h"

static int
usage(
	int ret
) {
	dprintf(2, "usage: ./ft_strace PROG [ARGS]\n");
	return ret;
}

int		main(
	int ac,
	char **av,
	char **env
) {
	t_context	ctx;
	char		*fullpath = NULL;

	memset(&ctx, 0, sizeof(t_context));
	get_options(&ctx, &ac, av);
	if (ac < 2) {
		dprintf(2, "ft_strace: must have PROG [ARGS]\n");
		return usage(1);
	}
	if (ctx.options & OPT_OUTPUT_FILE) {
		(void)open_output_file(&ctx);
	}
	if ((fullpath = get_bin_path(av[1])) == NULL) {
		ft_exit_perror(CANT_STAT, av[1]);
	} else {
		printf("%s\n", fullpath);
	}
	return 0;
}
