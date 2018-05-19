#include <signal.h> // rt_sigaction
#include <stdlib.h>

int
signal_killer(void) {
	const struct sigaction		sigact = {
		.sa_handler = SIG_IGN,
		.sa_mask = {},
		.sa_flags = 0,
		.sa_restorer = NULL
	};
	struct sigaction			old_sigact;

	/* from `strace strace ls` :) */
	return sigaction(SIGTTOU, &sigact, &old_sigact) ||
	sigaction(SIGTTIN, &sigact, &old_sigact) ||
	sigaction(SIGHUP, &sigact, &old_sigact) ||
	sigaction(SIGINT, &sigact, &old_sigact) ||
	sigaction(SIGQUIT, &sigact, &old_sigact) ||
	sigaction(SIGPIPE, &sigact, &old_sigact) ||
	sigaction(SIGTERM, &sigact, &old_sigact);
}
