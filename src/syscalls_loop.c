#define _GNU_SOURCE
#include <stdio.h> // ??

#include <sys/ptrace.h> /*ptrace*/
#include <sys/resource.h> /*rusage*/
#include <sys/reg.h> /*ORIG_RAX*/
#include <sys/user.h> /*strust user*/
#include <sys/wait.h> /*waitpid*/
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h> /*strerror*/
#include <errno.h> /*ENOSYS*/
#include <linux/unistd.h> /*exit defines*/

#include "context.h"
#include "syscalls_table.h"
#include "syscalls_loop.h"

int
syscalls_loop(
	t_context ctx,
	pid_t const pid
) {
	int							wstatus = 0;
	struct rusage				rusage;
	struct user_regs_struct		regs;
	long						syscall = 0;
	long						retsyscall = 0;
	char						*error = NULL;

	ptrace(PTRACE_SEIZE, pid, NULL, PTRACE_O_TRACESYSGOOD|PTRACE_O_TRACEEXEC|PTRACE_O_TRACEEXIT);
	while (wait4(pid, &wstatus, 0, &rusage) && !WIFEXITED(wstatus)) {

			// printf("WIFSTOPPED:%d\n", WIFSTOPPED(wstatus));
			// printf("SIG:%d\n", WSTOPSIG(wstatus));
			if (WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) == SIGINT) {
				ptrace(PTRACE_DETACH, pid, NULL, NULL);
				break;
			}
		// if (WIFEXITED(wstatus)) {
        //
		// 	dprintf(ctx.output_fd, " exited\n");
		// 	break;
		// }

		syscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*ORIG_RAX, NULL);
		ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		retsyscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RAX, NULL);

		if (syscall == __NR_exit || syscall == __NR_exit_group) {
			ptrace(PTRACE_GETREGS, pid, NULL, &regs);
			printf("%s(%d) = ?\n", syscalls_table[syscall].name, (int)regs.rdi);
			printf("+++ exited with %hhu +++\n", (unsigned char)regs.rdi);
			break;
		}

		/* Syscall has been called by didn't returned yet */
		// if ((unsigned long)retsyscall == ENOSYS) {

			/* Print the first part of the line and wait for the syscall return */
			dprintf(ctx.output_fd, "%s", syscalls_table[syscall].name);
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			wait4(pid, &wstatus, 0, &rusage);
			// printf("WIFSTOPPED:%d\n", WIFSTOPPED(wstatus));
			// printf("SIG:%d\n", WSTOPSIG(wstatus));
			if (WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) == SIGINT) {
				ptrace(PTRACE_DETACH, pid, NULL, NULL);
				break;
			}

			/* Syscall returned, finish to print the line */
			syscall = ptrace(PTRACE_PEEKUSER, pid, O_RAX, NULL);
			ptrace(PTRACE_GETREGS, pid, NULL, &regs);
			retsyscall = ptrace(PTRACE_PEEKUSER, pid, R_RAX, NULL);

			/*
			** retsyscall < 0 ? -1 : retsyscall, errno is given as negative int
			** so print -1, take the abs retsyscall value and get the errno str
			** through strerror()
			*/
			dprintf(ctx.output_fd, "(...) = %ld (%ld)(%p)", retsyscall < 0 ? -1 : retsyscall, retsyscall,retsyscall);
			if ((unsigned long)(-retsyscall) != ENOSYS && retsyscall < 0) {
				error = strerror(retsyscall * -1);
				dprintf(ctx.output_fd, " (%s)", error);
			}
			dprintf(ctx.output_fd, "\n");

		/* See you next syscall */
		ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
	}
	return 0;
}
