#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /*execve*/
#include <sys/types.h> /*fork*/
#include <sys/wait.h> /*waitpid*/
#include <sys/ptrace.h> /*ptrace*/
#include <sys/user.h> /*strust user*/
#include <sys/reg.h> /*ORIG_RAX*/

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

		pid_t						pid = 0;
		int							wstatus;
		struct user_regs_struct		regs;
		int							syscall;

		if ((pid = fork()) > 0) {

			ptrace(PTRACE_SEIZE, pid, NULL, NULL);
/*			ptrace(PTRACE_INTERRUPT, pid, NULL, NULL);*/
			ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD);
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

			while (waitpid(pid, &wstatus, 0) && !WIFEXITED(wstatus)) {

				ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
				if (WIFSTOPPED(wstatus) && (WSTOPSIG(wstatus) == 0x80)) {
					syscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*ORIG_RAX, NULL);
					ptrace(PTRACE_GETREGS, pid, NULL, &regs);
					dprintf(ctx.output_fd, "syscall(%d)\n", syscall);
				}

			}

		} else if (pid == 0) {
			kill(getpid(), SIGSTOP);
			execve(ctx.bin_fullpath, av + 1, env);

		} else {

			ft_exit_perror(FORK_FAILED, NULL);

		}
	}
	free_context(&ctx);
	return 0;
}
