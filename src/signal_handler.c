#define _GNU_SOURCE
#include <stdio.h> // ??
#include <sys/wait.h> /*wait macros*/
#include <sys/ptrace.h>
#include <stdlib.h> /*exit*/
#include <linux/unistd.h> /*exit defines*/
 // #include <sys/syscall.h>
 #include <unistd.h>

#include "context.h"

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

	ptrace(PTRACE_GETSIGINFO, pid, 0L, &siginfo);
	if (WSTOPSIG(wstatus) < 1 || WSTOPSIG(wstatus) > 64) {
		return 1;
	} else if (siginfo.si_signo == SIGINT) {
		dprintf(ctx.output_fd, "%s: Process %d detached\n<detached ...>\n", __progname, pid);
		pid_t p = 0;
		int s = 0;
		p = wait(&s);
		printf("p:%d\n", p);
		// while (p > 0)
		// {
		// 	kill(p, SIGKILL);
		// 	p = waitpid(-1, &s, P_ALL);
		// 	printf("p:%d\n", p);
		// }
		exit(0);
	} else {
		if (WSTOPSIG(wstatus) < 32) {
			dprintf(ctx.output_fd, "--- %s {si_signo=%d, si_code=%d, si_pid=%d, si_uid=%d}\n",
				g_sig_table[WSTOPSIG(wstatus)],
				siginfo.si_signo,
				siginfo.si_code,
				siginfo.si_pid,
				siginfo.si_uid
			);
		} else {
			dprintf(ctx.output_fd, "--- SIGRT_%d {si_signo=%d, si_code=%d, si_pid=%d, si_uid=%d}\n",
				WSTOPSIG(wstatus) - 32,
				siginfo.si_signo,
				siginfo.si_code,
				siginfo.si_pid,
				siginfo.si_uid
			);
		}
		// if (siginfo.si_signo == SIGINT) {
		// 	dprintf(ctx.output_fd, "%s: Process %d detached\n<detached ...>\n", __progname, pid);
		// 	// ptrace(PTRACE_DETACH, pid);
		// 	exit(0);
		// } else {
			return 0;
		// }
	}
}
