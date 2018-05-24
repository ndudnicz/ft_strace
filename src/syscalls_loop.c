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
#include "signal_killer.h"

static int
print_escaped_str(
	t_context ctx,
	char const *const str
) {
	size_t const	len_max = strlen(str) < 32 ? strlen(str) : 32;

	write(ctx.output_fd, "\"", 1);
	for (size_t i = 0; i < len_max; ++i) {
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
	unsigned long	tmp = 0;
	size_t				n = 0;

	*str = (char*)calloc(1, 33);
	// printf("%llx\n", ((long*)regs)[reg_index]);
	do {
		tmp = (unsigned long)ptrace(PTRACE_PEEKDATA, pid, ((long*)regs)[reg_index] + n, 0L);
		memcpy(*str + n, &tmp, sizeof(long));
		n += sizeof(long);
	} while (n < 32 && !memchr((void*)&tmp, 0, sizeof(long)));
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
		dprintf(ctx.output_fd, "%d", (int)((long*)regs)[reg_index]);
		break;

		case E_UINT:
		dprintf(ctx.output_fd, "%u", (unsigned int)((unsigned long*)regs)[reg_index]);
		break;

		case E_PTR:
		dprintf(ctx.output_fd, "0x%lx", ((unsigned long*)regs)[reg_index]);
		break;

		case E_STR:
		peek_string(pid, &str, regs, reg_index);
		print_escaped_str(ctx, str);
		if (strlen(str) == 32) {
			write(ctx.output_fd, "...", 3);
		}
		memset(str, 0, strlen(str));
		free(str);
		str = NULL;
		break;

		case E_STRUCT:
		dprintf(ctx.output_fd, "0x%lx", ((unsigned long*)regs)[reg_index]);
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
			dprintf(ctx.output_fd, ", ");
		} else {
			dprintf(ctx.output_fd, ")");
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
	ptrace(PTRACE_GETREGS, pid, 0L, regs);
	*syscall = regs->orig_rax;
	*retsyscall = regs->rax;
	return 0;
}

static int
wait_syscall(
	t_context ctx,
	pid_t pid
) {
	int		wstatus;

	while (1) {
		ptrace(PTRACE_SYSCALL, pid, 0L, 0L);
		waitpid(pid, &wstatus, 0);
		if (WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) & 0x80) {
			return 0;
		} else {
			signal_handler(ctx, pid, wstatus);
		}
		if (WIFEXITED(wstatus)) {
			printf("WIFEXITED:%d WIFSIGNALED:%d\n", WIFEXITED(wstatus), WIFSIGNALED(wstatus));
			return 1;
		}
	}
	return 0;
}

static void	block_sig(sigset_t *sig_block)
{
	sigemptyset(sig_block);
	sigaddset(sig_block, SIGHUP);
	sigaddset(sig_block, SIGINT);
	sigaddset(sig_block, SIGQUIT);
	sigaddset(sig_block, SIGPIPE);
	sigaddset(sig_block, SIGTERM);
	sigprocmask(SIG_BLOCK, sig_block, NULL);
}

int
syscalls_loop(
	t_context ctx,
	pid_t pid
) {
	int												wstatus = 0;
	struct rusage							rusage;
	struct user_regs_struct		regs;
	long											syscall = 0;
	long											retsyscall = 0;
	char											*error = NULL;
	int												i = 0;

	sigset_t		set;
	sigset_t		sig_block;

	sigemptyset(&set);
	ptrace(PTRACE_SEIZE, pid, 0L, PTRACE_O_TRACESYSGOOD);
	ptrace(PTRACE_INTERRUPT, pid, 0L, 0L);
	sigprocmask(SIG_SETMASK, &set, NULL);
	ptrace(PTRACE_SYSCALL, pid, 0L, 0L);
	// while (1) {
	// 	printf("pid: %d  %d\n", waitpid(-1, &wstatus, P_ALL), wstatus>>8 == (SIGTRAP | (PTRACE_EVENT_EXEC<<8)));
	// 	ptrace(PTRACE_SYSCALL, pid, 0L, 0L);
	// }
	// printf("new_pid:%d\n",waitpid(-1, &wstatus, P_ALL));
	waitpid(pid, &wstatus, 0);
	block_sig(&sig_block);
	// ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD|PTRACE_O_EXITKILL|PTRACE_O_TRACECLONE);

	// signal_killer();
	while (1) {
		if (wait_syscall(ctx, pid)) {
			return 0;
		}
		// if (WIFEXITED(wstatus)) {
			// printf("WIFEXITED:%d WIFSIGNALED:%d\n", WIFEXITED(wstatus), WIFSIGNALED(wstatus));
			// return 1;
		// }
		get_syscall_number_and_registers(pid, &syscall, &retsyscall, &regs);
		// printf("%lld\n", regs.orig_rax);
		dprintf(ctx.output_fd, "%s(", syscalls_table[syscall].name);
		print_params(ctx, pid, syscalls_table[syscall], &regs, 0, syscalls_table[syscall].n_param_p1);

		ptrace(PTRACE_SEIZE, pid, 0L, PTRACE_O_TRACESYSGOOD);
		if (wait_syscall(ctx, pid)) {
			return 0;
		}
		// if (WIFEXITED(wstatus)) {
			// printf("WIFEXITED:%d WIFSIGNALED:%d\n", WIFEXITED(wstatus), WIFSIGNALED(wstatus));
			// return 1;
		// }
		get_syscall_number_and_registers(pid, &syscall, &retsyscall, &regs);
		print_params(ctx, pid, syscalls_table[syscall], &regs, syscalls_table[syscall].n_param_p1, syscalls_table[syscall].n_param);
		dprintf(ctx.output_fd, " = %ld (%p)", retsyscall, retsyscall);
		dprintf(ctx.output_fd, "\n");
		i++;
	}
	return 0;
}
