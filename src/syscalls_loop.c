
#include <sys/ptrace.h> /*ptrace*/
#include <sys/resource.h> /*rusage*/
#include <sys/reg.h> /*ORIG_RAX*/
#include <sys/user.h> /*strust user*/
#include <sys/wait.h> /*waitpid*/
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int
syscalls_loop(
	pid_t const pid
) {
	int							wstatus = 0;
	struct rusage				rusage;
	struct user_regs_struct		regs;
	long						syscall = 0;
	long						retsyscall = 0;

	ptrace(PTRACE_SEIZE, pid, NULL, PTRACE_O_TRACESYSGOOD|PTRACE_O_TRACEEXEC|PTRACE_O_TRACEEXIT);
	ptrace(PTRACE_INTERRUPT, pid, NULL, NULL);
	// ptrace(PTRACE_LISTEN, pid, NULL, NULL); // ??
	ptrace(PTRACE_SYSCALL, pid, NULL, NULL); // ??
	while (wait4(pid, &wstatus, 0, &rusage) && !WIFEXITED(wstatus)) {
		// if (WIFSTOPPED(wstatus) && (WSTOPSIG(wstatus) == 0x80)) {
		syscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*ORIG_RAX, NULL);
		ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		retsyscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RAX, NULL);

		ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
	}
	return 0;
}
