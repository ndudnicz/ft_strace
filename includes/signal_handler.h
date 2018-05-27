#ifndef SIGNAL_HANDLER_H
# define SIGNAL_HANDLER_H

int
signal_handler(
	pid_t const pid,
	int const wstatus
);

int
signal_killer(
	pid_t const pid,
	int const wstatus
);

void
sig_block(void);

void
sig_empty(void);

#endif
