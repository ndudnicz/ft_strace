#define _GNU_SOURCE
#include <stdio.h>

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
#include <string.h> /*strlen*/
#include <unistd.h>/*write*/

#include "context.h"
#include "syscalls_table.h"
#include "syscalls_loop.h"
#include "signal_handler.h"

static int
is_null_terminated(
	long n
) {
	printf("\n-----------> %016lx\n", n);
	return (!((n>>56)&0xff) || !((n>>48)&0xff) || !((n>>40)&0xff) ||
			!((n>>32)&0xff) || !((n>>24)&0xff) || !((n>>16)&0xff) ||
			!((n>>8)&0xff) || !(n&0xff));
}

static int
print_escaped_str(
	t_context ctx,
	char const *const str
) {
	write(ctx.output_fd, "\"", 1);
	for (size_t i = 0; i < strlen(str); ++i) {
		switch (str[i]) {
			case 0x07:
			write(ctx.output_fd, "\\a", 2);
			break;
			case 0x08:
			write(ctx.output_fd, "\\b", 2);
			break;
			case 0x09:
			write(ctx.output_fd, "\\t", 2);
			break;
			case 0x0a:
			write(ctx.output_fd, "\\n", 2);
			break;
			case 0x0b:
			write(ctx.output_fd, "\\v", 2);
			break;
			case 0x0c:
			write(ctx.output_fd, "\\f", 2);
			break;
			case 0x0d:
			write(ctx.output_fd, "\\r", 2);
			break;
			default:
			write(ctx.output_fd, str + i, 1);
			break;
		}
	}
	write(ctx.output_fd, "\"", 1);
	return 0;
}

static int
peek_string(
	pid_t const pid,
	char **const str,
	struct user_regs_struct *regs,
	int const reg_index
) {
	long		tmp = 0;
	size_t		n = 0;

	*str = (char*)calloc(1, 33);
	do {
		tmp = ptrace(PTRACE_PEEKDATA, pid, ((long*)regs)[reg_index] + n, NULL);
	// 	if (tmp == -1) {
			// printf("%ld\n", tmp);
	// 		kill(pid, SIGKILL);
	// 		exit(EXIT_FAILURE);
	// 	};
		printf("%016lx\n", tmp);
		memcpy(*str + n, &tmp, sizeof(long));
		tmp = 0;
		n += sizeof(long);
	} while (n < (32 * sizeof(long)) && !is_null_terminated(tmp));
	(*str)[n] = 0;
	// char *s = &(char*)tmp;
	// printf("%8s", tmp);
	return 0;
}

static int
print_param(
	t_context const ctx,
	pid_t const pid,
	t_syscall const syscall,
	enum e_param const param_type,
	struct user_regs_struct *const regs,
	int const reg_index
) {
	char *str = NULL;

	switch (param_type) {
		case E_NONE:
		break;

		case E_INT:
		dprintf(ctx.output_fd, "%ld", ((long*)regs)[reg_index]);
		break;

		case E_UINT:
		dprintf(ctx.output_fd, "%lu", ((unsigned long*)regs)[reg_index]);
		break;

		case E_PTR:
		dprintf(ctx.output_fd, "%016lx", ((unsigned long*)regs)[reg_index]);
		break;

		case E_STR:
		// size_t const		str_len = strlen(str);
		// str = (char*)ptrace(PTRACE_PEEKDATA, pid, reg_index * sizeof(long), NULL);
		peek_string(pid, &str, regs, reg_index);
		// printf("--->%d %d\n", reg_index, RSI);
		// write(1, str[0], 1);
		// perror(NULL);
		print_escaped_str(ctx, str);
		if (strlen(str) == 32) {
			write(ctx.output_fd, "...", 2);
		}
		memset(str, 0, strlen(str));
		free(str);
		str = NULL;
		break;

		case E_STRUCT:
		dprintf(ctx.output_fd, "%016lx", ((unsigned long*)regs)[reg_index]);
		break;

		default:
		break;
	}
	return 0;
}

static int
print_params(
	t_context const ctx,
	pid_t const pid,
	t_syscall const syscall,
	struct user_regs_struct *const regs,
	int const start,
	int const end
) {
	static const int	regs_indexes[6] = { RDI, RSI, RDX, R10, R8, R9 };

	for (int i = start; i < end; ++i) {
		print_param(ctx, pid, syscall, syscall.params[ i ], regs, regs_indexes[ i ]);
		if (i < syscall.n_param - 1) {
			dprintf(2, ", ");
		} else {
			dprintf(2, ")");
		}
	}
	return 0;
}

static int
get_syscall_number_and_registers(
	pid_t const pid,
	long *syscall,
	long *retsyscall,
	struct user_regs_struct *regs
) {
	*syscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*ORIG_RAX, NULL);
	ptrace(PTRACE_GETREGS, pid, NULL, regs);
	*retsyscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RAX, NULL);
	return 0;
}

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

		if (WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) != SIGTRAP && WSTOPSIG(wstatus) != (SIGTRAP|0x80)) {
			dprintf(2, "WIFSTOPPED1\n");
			(void)signal_handler(ctx, wstatus);
			ptrace(PTRACE_DETACH, pid, NULL, NULL);
			break;
		} else {
			/* Syscall has been called by didn't returned yet */
			get_syscall_number_and_registers(pid, &syscall, &retsyscall, &regs);

			if (syscall == __NR_exit || syscall == __NR_exit_group) {
				dprintf(2, "exited: %s\n", syscalls_table[syscall].name);
				ptrace(PTRACE_GETREGS, pid, NULL, &regs);
				printf("%s(%d) = ?\n", syscalls_table[syscall].name, (int)regs.rdi);
				printf("+++ exited with %hhu +++\n", (unsigned char)regs.rdi);
				break;
			}

			/* Print the first part of the line and wait for the syscall return */
			dprintf(ctx.output_fd, "%s(", syscalls_table[syscall].name);
			print_params(ctx, pid, syscalls_table[syscall], &regs, 0, syscalls_table[syscall].n_param_p1);
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			wait4(pid, &wstatus, 0, &rusage);
			if (WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) != SIGTRAP && WSTOPSIG(wstatus) != (SIGTRAP|0x80)) {
				dprintf(2, "WIFSTOPPED2\n");
				(void)signal_handler(ctx, wstatus);
				ptrace(PTRACE_DETACH, pid, NULL, NULL);
				// return 0;
				break;
			} else {
				/* Syscall returned, finish to print the line */
				get_syscall_number_and_registers(pid, &syscall, &retsyscall, &regs);

				/*
				** retsyscall < 0 ? -1 : retsyscall, errno is given as negative int
				** so print -1, take the abs retsyscall value and get the errno str
				** through strerror()
				*/
				print_params(ctx, pid, syscalls_table[syscall], &regs, syscalls_table[syscall].n_param_p1, syscalls_table[syscall].n_param);
				dprintf(ctx.output_fd, " = %ld (%ld)(%p)", retsyscall < 0 ? -1 : retsyscall, retsyscall,retsyscall);
				if ((unsigned long)(-retsyscall) != ENOSYS && retsyscall < 0) {
					error = strerror(retsyscall * -1);
					dprintf(ctx.output_fd, " (%s)", error);
				}
				dprintf(ctx.output_fd, "\n");

			}


			/* See you next syscall */
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

		}

		// /* Syscall has been called by didn't returned yet */
		// syscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*ORIG_RAX, NULL);
		// ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		// retsyscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RAX, NULL);
        //
		// if (syscall == __NR_exit || syscall == __NR_exit_group) {
		// 	dprintf(2, "exited: %s\n", syscalls_table[syscall].name);
		// 	ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		// 	printf("%s(%d) = ?\n", syscalls_table[syscall].name, (int)regs.rdi);
		// 	printf("+++ exited with %hhu +++\n", (unsigned char)regs.rdi);
		// 	break;
		// }
        //
		// /* Print the first part of the line and wait for the syscall return */
		// dprintf(ctx.output_fd, "%s", syscalls_table[syscall].name);
		// ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
		// wait4(pid, &wstatus, 0, &rusage);
		// if (WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) != SIGTRAP && WSTOPSIG(wstatus) != (SIGTRAP|0x80)) {
		// 	dprintf(2, "WIFSTOPPED2\n");
		// 	(void)signal_handler(ctx, wstatus);
		// 	ptrace(PTRACE_DETACH, pid, NULL, NULL);
		// 	// return 0;
		// 	break;
		// } else {
		// 	/* Syscall returned, finish to print the line */
		// 	syscall = ptrace(PTRACE_PEEKUSER, pid, O_RAX, NULL);
		// 	ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		// 	retsyscall = ptrace(PTRACE_PEEKUSER, pid, R_RAX, NULL);
        //
		// 	/*
		// 	** retsyscall < 0 ? -1 : retsyscall, errno is given as negative int
		// 	** so print -1, take the abs retsyscall value and get the errno str
		// 	** through strerror()
		// 	*/
		// 	dprintf(ctx.output_fd, "(...) = %ld (%ld)(%p)", retsyscall < 0 ? -1 : retsyscall, retsyscall,retsyscall);
		// 	if ((unsigned long)(-retsyscall) != ENOSYS && retsyscall < 0) {
		// 		error = strerror(retsyscall * -1);
		// 		// dprintf(ctx.output_fd, " (%s)", error);
		// 	}
		// 	dprintf(ctx.output_fd, "\n");
        //
		// }
        //
        //
		// /* See you next syscall */
		// ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
	}
	return 0;
}
