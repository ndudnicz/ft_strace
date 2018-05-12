#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "context.h"
#include "error.h"
#include "get_bin_path.h"
#include "options.h"
#include "output_file.h"
#include "free.h"

__attribute__ ((noreturn)) static void
usage(
	int ret
) {
	dprintf(2, "usage: ./ft_strace PROG [ARGS]\n");
	exit(ret);
}

int		main(
	int ac,
	char **av,
	char **env
) {
	t_context	ctx;

	memset(&ctx, 0, sizeof(t_context));
	get_options(&ctx, &ac, av);
	if (ac < 2) {

		dprintf(2, "ft_strace: must have PROG [ARGS]\n");
		usage(EXIT_FAILURE);

	} else if (ctx.options & OPT_OUTPUT_FILE) {

		(void)open_output_file(&ctx);

	} else if ((ctx.bin_fullpath = get_bin_path(av[1])) == NULL) {

		ft_exit_perror(CANT_STAT, av[1]);

	} else {

		printf("%s\n", ctx.bin_fullpath);

	}
	free_context(&ctx);
	return 0;
}
