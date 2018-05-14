#include <fcntl.h>

#include "context.h"
#include "error.h"

int
open_output_file(
	t_context *ctx
) {
	return ctx->output_fd = open(ctx->output_filename, O_WRONLY|O_CREAT|O_TRUNC);
}
