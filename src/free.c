#include <stdlib.h>
#include <unistd.h>

#include "context.h"

int
free_context(
	t_context *ctx
) {
	if (ctx->output_filename) {
		(void)free(ctx->output_filename);
		ctx->output_filename = NULL;
	}
	if (ctx->bin_fullpath) {
		(void)free(ctx->bin_fullpath);
		ctx->bin_fullpath = NULL;
	}
	(void)close(ctx->output_fd);
	return 0;
}
