#include <fcntl.h>

#include "context.h"
#include "error.h"

int
open_output_file(
	t_context *ctx
) {
	if ((ctx->output_fd = open(ctx->output_filename, O_WRONLY|O_CREAT|O_TRUNC)) < 0) {
		ft_exit_perror(CANT_OPEN, ctx->output_filename);
	} else {
		return 0;
	}
}
