#ifndef SIGNAL_HANDLER_H
# define SIGNAL_HANDLER_H

int
signal_handler(
	t_context ctx,
	pid_t pid,
	int const wstatus
);

int
signal_killer(
	t_context ctx,
	pid_t pid,
	int const wstatus
);

void
sig_block(void);

void
sig_empty(void);

#endif
