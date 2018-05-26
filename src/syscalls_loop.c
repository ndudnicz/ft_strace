#define _GNU_SOURCE
#include <stdio.h> /*dprintf*/
#include <sys/ptrace.h> /*ptrace*/
#include <sys/reg.h> /*ORIG_RAX*/
#include <sys/user.h> /*strust user*/
#include <sys/wait.h> /*waitpid*/
#include <stdlib.h> /*calloc*/
#include <errno.h> /*ENOSYS*/
#include <linux/unistd.h> /*__NR_execve*/
#include <string.h> /*strlen*/
#include <unistd.h>/*write*/

#include "context.h"
#include "syscalls_table.h"
#include "signal_handler.h"
#include "error.h"

/*
** Print str like strace :)
*/

static int
print_escaped_str(
	t_context ctx,
	char const *const str
) {
	size_t const	len = strlen(str);
	int						printed = 0;
	size_t				i = 0;

	printed += write(ctx.output_fd, "\"", 1);
	for (i = 0; i < len && i < 32; ++i) {
		switch (str[i]) {
			case 0x07:
			printed += write(ctx.output_fd, "\\a", 2);
			break;
			case 0x08:
			printed += write(ctx.output_fd, "\\b", 2);
			break;
			case 0x09:
			printed += write(ctx.output_fd, "\\t", 2);
			break;
			case 0x0a:
			printed += write(ctx.output_fd, "\\n", 2);
			break;
			case 0x0b:
			printed += write(ctx.output_fd, "\\v", 2);
			break;
			case 0x0c:
			printed += write(ctx.output_fd, "\\f", 2);
			break;
			case 0x0d:
			printed += write(ctx.output_fd, "\\r", 2);
			break;
			default:
			printed += write(ctx.output_fd, str + i, 1);
			break;
		}
	}
	printed += write(ctx.output_fd, "\"", 1);
	return printed;
}

/*
** Loop to get peek the string from the child memory land
*/

static int
peek_string(
	pid_t const pid,
	char **const str,
	struct user_regs_struct *regs,
	int const reg_index
) {
	unsigned long	tmp = 0;
	size_t				n = 0;

	*str = (char*)calloc(1, 33);
	if (!*str) {
		ft_exit_perror(MALLOC_FAILED, NULL);
	}
	do {
		tmp = (unsigned long)ptrace(PTRACE_PEEKDATA, pid, ((long*)regs)[reg_index] + n, 0L);
		memcpy(*str + n, &tmp, sizeof(long));
		n += sizeof(long);
	} while (n < 32 && !memchr((void*)&tmp, 0, sizeof(long)));
	return 0;
}

/*
** Print one reg, depends on its type
*/

static int
print_param(
	t_context const ctx,
	pid_t const pid,
	enum e_param const param_type,
	struct user_regs_struct *const regs,
	int const reg_index
) {
	char	*str = NULL;
	int		printed = 0;

	switch (param_type) {
		case E_NONE:
		return 0;
		break;

		case E_INT:
		return dprintf(ctx.output_fd, "%d", (int)((long*)regs)[reg_index]);

		case E_UINT:
		return dprintf(ctx.output_fd, "%u", (unsigned int)((unsigned long*)regs)[reg_index]);

		case E_STR:
		peek_string(pid, &str, regs, reg_index);
		printed = print_escaped_str(ctx, str);
		if (strlen(str) == 32) {
			printed += write(ctx.output_fd, "...", 3);
		}
		free(str);
		str = NULL;
		return printed;

		case E_STRUCT:
		if (((unsigned long*)regs)[reg_index]) {
			return dprintf(ctx.output_fd, "{ 0x%lx }", ((unsigned long*)regs)[reg_index]);
		} else {
			return dprintf(ctx.output_fd, "{ NULL }");
		}

		default:
		if (((unsigned long*)regs)[reg_index]) {
			return dprintf(ctx.output_fd, "0x%lx", ((unsigned long*)regs)[reg_index]);
		} else {
			return dprintf(ctx.output_fd, "NULL");
		}
	}
	return 0;
}

/*
** Loop to print all regs
*/

static int
print_params(
	t_context const ctx,
	pid_t const pid,
	t_syscall const syscall,
	struct user_regs_struct *const regs
) {
	static const int	regs_indexes[6] = { RDI, RSI, RDX, R10, R8, R9 };
	int								printed = 0;
	int								i;

	if (syscall.n_param > 0) {
		for (i = 0; i < syscall.n_param; ++i) {
			printed += print_param(ctx, pid, syscall.params[ i ], regs, regs_indexes[ i ]);
			if (i < syscall.n_param - 1) {
				printed += dprintf(ctx.output_fd, ", ");
			} else {
				printed += dprintf(ctx.output_fd, ")");
			}
		}
	} else {
		printed += dprintf(ctx.output_fd, ")");
	}
	return printed;
}

/*
** Peek regs values from child memory land
*/

static int
get_syscall_number_and_registers(
	pid_t const pid,
	long *syscall,
	long *retsyscall,
	struct user_regs_struct *regs
) {
	ptrace(PTRACE_GETREGS, pid, 0L, regs);
	*syscall = regs->orig_rax;
	*retsyscall = regs->rax;
	return 0;
}

/*
** Wait for a syscall, return 0 or display a sig msg and/or exit
*/

static int
wait_syscall(
	t_context ctx,
	pid_t pid,
	int i
) {
	int		wstatus;

	while (1) {
		ptrace(PTRACE_SYSCALL, pid, 0L, 0L);

		sig_empty();
		(void)waitpid(pid, &wstatus, 0);
		sig_block();

		if (i && WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) & (SIGTRAP|0x80)) {
			return 0;
		} else if (!i && WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) & 0x80) {
			return 0;
		} else {
			if (WIFEXITED(wstatus)) {
				dprintf(ctx.output_fd, " = ?\n+++ exited with %d +++\n", WEXITSTATUS(wstatus));
				exit(0);
			} else {
				signal_handler(ctx, pid, wstatus);
			}
			signal_killer(ctx, pid, wstatus);
		}
	}
	return 0;
}

/*
** Handle error msg
*/

static int
display_error_or_not(
	t_context ctx,
	long retsyscall
) {
	if (-retsyscall != ENOSYS) {
		if (-retsyscall != EUNKNOWN) {
			if (retsyscall < 0) {
				dprintf(ctx.output_fd, " = %d (%s)\n", -1, strerror(-retsyscall));
			} else {
				dprintf(ctx.output_fd, " = %ld (0x%lx)\n", retsyscall, retsyscall);
			}
		} else {
			dprintf(ctx.output_fd, " = ?\n");
		}
	}
	return 0;
}

int
syscalls_loop(
	t_context ctx,
	pid_t pid
) {
	int												wstatus = 0;
	struct user_regs_struct		regs;
	long											syscall = 0;
	long											retsyscall = 0;
	int												i = 0;
	int												printed = 0;

	if (ptrace(PTRACE_SEIZE, pid, 0L, PTRACE_O_TRACESYSGOOD) < 0  || ptrace(PTRACE_INTERRUPT, pid, 0L, 0L) < 0) {
		ft_exit_perror(PTRACE_FAILED, NULL);
	}

	sig_empty();
	(void)waitpid(pid, &wstatus, 0);
	sig_block();

	while (1) {
		if (wait_syscall(ctx, pid, 1)) {
			return 1;
		}
		get_syscall_number_and_registers(pid, &syscall, &retsyscall, &regs);
		if (i == 0 && syscall == __NR_execve) {
			i = 1;
		}
		if (i) {
			display_error_or_not(ctx, retsyscall);
		}
		ptrace(PTRACE_SEIZE, pid, 0L, PTRACE_O_TRACESYSGOOD);
		if (wait_syscall(ctx, pid, 0)) {
			return 2;
		}
		get_syscall_number_and_registers(pid, &syscall, &retsyscall, &regs);
		if ((i == 0 && syscall == __NR_execve) || i) {
			i = 1;
			printed += dprintf(ctx.output_fd, "%s(", syscalls_table[syscall].name);
			printed += print_params(ctx, pid, syscalls_table[syscall], &regs);
			while (printed < 39) {
				dprintf(ctx.output_fd, " ");
				printed++;
			}
			printed = 0;
			display_error_or_not(ctx, retsyscall);
		}
	}
	return 3;
}
