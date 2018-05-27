#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /*execve*/
#include <sys/types.h> /*fork*/
#include <signal.h>

#include "context.h"
#include "error.h"
#include "get_bin_path.h"
#include "syscalls_loop.h"

pid_t							g_pid = 0;
pid_t							g_output_fd = 2;
extern const char	*__progname;

void
sigint_handler(int sig) {
	(void)sig;
	(void)dprintf(g_output_fd, "%s: Process %d detached\n", __progname, g_pid);
	exit(0);
}

__attribute__ ((noreturn)) static void
usage(
	int ret
) {
	(void)dprintf(2, "usage: ./ft_strace PROG [ARGS]\n");
	exit(ret);
}

int		main(
	int ac,
	char *av[],
	char *env[]
) {
	t_context	ctx;

	(void)memset(&ctx, 0, sizeof(t_context));
	if (ac < 2) {

		(void)dprintf(2, "ft_strace: must have PROG [ARGS]\n");
		usage(EXIT_FAILURE);

	} else if ((ctx.bin_fullpath = get_bin_path(av[1])) == NULL) {

		free(ctx.output_filename);
		ft_exit_perror(CANT_STAT, av[1]);

	} else {
		g_pid = fork();
		g_output_fd = ctx.output_fd;
		switch (g_pid) {
			case -1:
			ft_exit_perror(FORK_FAILED, NULL);
			case 0:
			(void)kill(getpid(), SIGSTOP);
			if (execve(ctx.bin_fullpath, av + 1, env) < 0) {
				free(ctx.bin_fullpath);
				free(ctx.output_filename);
				ft_exit_perror("exec", NULL);
			}
			break;
			default:
			free(ctx.bin_fullpath);
			free(ctx.output_filename);
			(void)signal(SIGINT, &sigint_handler);
			(void)syscalls_loop(ctx, g_pid);
			break;
		}
	}
	return 0;
}
