#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /*execve*/
#include <sys/types.h> /*fork*/
#include <sys/wait.h> /*wait*/
#include <sys/resource.h>

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
	char *av[],
	char *env[]
) {
	t_context	ctx;

	memset(&ctx, 0, sizeof(t_context));
	get_options(&ctx, &ac, av);
	if (ac < 2) {

		dprintf(2, "ft_strace: must have PROG [ARGS]\n");
		usage(EXIT_FAILURE);

	} else if ((ctx.options & OPT_OUTPUT_FILE) && open_output_file(&ctx) <= 0) {

		ft_exit_perror(CANT_OPEN, ctx.output_filename);

	} else if ((ctx.bin_fullpath = get_bin_path(av[1])) == NULL) {

		ft_exit_perror(CANT_STAT, av[1]);

	} else {

		pid_t			pid = 0;
		int				wstatus;

		if ((pid = fork()) > 0) {

			while (1) {
				waitpid(pid, &wstatus, 0);
				printf("%x\n", WEXITSTATUS(wstatus));
				break;
			}

		} else if (pid == 0) {

			execve(ctx.bin_fullpath, av + 1, env);

		} else {

			ft_exit_perror(FORK_FAILED, NULL);

		}
	}
	free_context(&ctx);
	return 0;
}
