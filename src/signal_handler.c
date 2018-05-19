#define _GNU_SOURCE
#include <stdio.h> // ??

#include <sys/wait.h> /*wait macros*/

#include "context.h"

int
signal_handler(
	t_context ctx,
	int const wstatus
) {
	switch (WSTOPSIG(wstatus)) {
		case 2:
		dprintf(ctx.output_fd, "sig 2\n");
		break;
		default:
		dprintf(ctx.output_fd, "other sig\n");
		break;
	}
	return 0;
}
