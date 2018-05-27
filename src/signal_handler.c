#define _GNU_SOURCE
#include <stdio.h>
#include <sys/wait.h> /*wait macros*/
#include <sys/ptrace.h>
#include <stdlib.h> /*exit*/
#include <string.h> /*strsignal*/

#include "context.h"
#include "error.h"

extern const char *__progname;
static char const *const	g_sig_table[64] = {
	"","SIGHUP","SIGINT","SIGQUIT","SIGILL",
	"SIGTRAP","SIGABRT","SIGBUS","SIGFPE",
	"SIGKILL","SIGUSR1","SIGSEGV","SIGUSR2",
	"SIGPIPE","SIGALRM","SIGTERM","SIGSTKFLT",
	"SIGCHLD","SIGCONT","SIGSTOP","SIGTSTP",
	"SIGTTIN","SIGTTOU","SIGURG","SIGXCPU",
	"SIGXFSZ","SIGVTALRM","SIGPROF","SIGWINCH",
	"SIGIO","SIGPWR","SIGSYS"
};

int
signal_handler(
	t_context ctx,
	pid_t pid,
	int const wstatus
) {
	siginfo_t		siginfo;

	(void)ptrace(PTRACE_GETSIGINFO, pid, 0L, &siginfo);
	if (WSTOPSIG(wstatus) < 1 || WSTOPSIG(wstatus) > 64) {
		return 1;
	} else {
		if (WSTOPSIG(wstatus) < 32) {
			/* SIGNALS 1-32 */
			(void)dprintf(ctx.output_fd, "--- %s {si_signo=%s, si_code=%d, si_pid=%d, si_uid=%d} ---\n",
				g_sig_table[WSTOPSIG(wstatus)],
				g_sig_table[WSTOPSIG(wstatus)],
				siginfo.si_code,
				siginfo.si_pid,
				siginfo.si_uid
			);
		} else {
			/* REAL TIME SIGNALS 33-64 */
			(void)dprintf(ctx.output_fd, "--- SIGRT_%d {si_signo=SIGRT_%d, si_code=%d, si_pid=%d, si_uid=%d}\n",
				WSTOPSIG(wstatus) - 32,
				WSTOPSIG(wstatus) - 32,
				siginfo.si_code,
				siginfo.si_pid,
				siginfo.si_uid
			);
		}
		return 0;
	}
}

/*
** Display a killed msg and exit or not, depends on sig type
*/

int
signal_killer(
	t_context ctx,
	pid_t pid,
	int const wstatus
) {
	if (
		WSTOPSIG(wstatus) != SIGCONT &&
		WSTOPSIG(wstatus) != SIGCHLD &&
		WSTOPSIG(wstatus) != SIGURG &&
		WSTOPSIG(wstatus) != SIGWINCH &&
		WSTOPSIG(wstatus) != SIGSTOP &&
		WSTOPSIG(wstatus) != SIGTSTP &&
		WSTOPSIG(wstatus) != SIGTTIN &&
		WSTOPSIG(wstatus) != SIGTRAP &&
		WSTOPSIG(wstatus) != SIGTTOU
	) {
		(void)kill(pid, WSTOPSIG(wstatus));
		if (WSTOPSIG(wstatus) < 32) {
			/* SIGNALS 1-32 */
			(void)dprintf(ctx.output_fd, "+++ killed by %s +++\n", g_sig_table[WEXITSTATUS(wstatus)]);
		} else {
			/* REAL TIME SIGNALS 33-64 */
			(void)dprintf(ctx.output_fd, "+++ killed by SIGRT_%d +++\n", WEXITSTATUS(wstatus) - 32);
		}
		(void)dprintf(ctx.output_fd, "%s\n", strsignal(WSTOPSIG(wstatus)));
		exit(0);
	} else {
		return 0;
	}
}

/*
** Signals got from `strace strace ls` :)
*/

void
sig_block(void) {
	sigset_t	block;

	if (
		sigemptyset(&block) ||
		sigaddset(&block, SIGHUP) ||
		sigaddset(&block, SIGINT) ||
		sigaddset(&block, SIGQUIT) ||
		sigaddset(&block, SIGPIPE) ||
		sigaddset(&block, SIGTERM) ||
		sigprocmask(SIG_BLOCK, &block, NULL)
	) {
		ft_exit_perror(SIG_BLOCK_FAILED, NULL);
	}
}

void
sig_empty(void) {
	sigset_t	set;

	if (
		sigemptyset(&set) ||
		sigprocmask(SIG_SETMASK, &set, NULL)
	) {
		ft_exit_perror(SIG_EMPTY_FAILED, NULL);
	}
}
