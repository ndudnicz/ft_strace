#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "options.h"
#include "error.h"

/*
** Moves NULLs argv to the end of the array. Options are set in config
** and doesn't exist anymore in argv.
*/

static void
del_null_params(
	int *ac,
	char **av,
	int offset
) {
	int		i;
	int		n;
	char	*tmp;

	n = 1;
	while (n < *ac)
	{
		i = 1;
		while (i < *ac - 1)
		{
			if (av[i] == NULL)
			{
				tmp = av[i];
				av[i] = av[i + 1];
				av[i + 1] = tmp;
			}
			i++;
		}
		n++;
	}
	*ac -= offset;
}

static int
set_filename(
	char const *param,
	t_context *ctx
) {
	ctx->options |= OPT_OUTPUT_FILE;
	if (param == NULL) {
		ft_exit_error(ARGUMENT_MISSING, NULL);
	} else {
		if ((ctx->output_filename = strdup(param)) == NULL) {
			ft_exit_perror(MALLOC_FAILED, NULL);
		}
	}
	return 2;
}

static int
switch_set_options(
	char const *arg,
	char const *param,
	t_context *ctx
) {
	char	s[2] = {0};

	if (!arg) {
		return (0);
	}
	else {
		arg++;
	}
	while (arg && *arg)
	{
		if (*arg && strchr(PARAMS_STR, (int)(*arg))) {
			switch (*arg) {
				case OPT_OUTPUT_FILE_CHAR:
					return set_filename(param, ctx);
			}
		}
		else {
			s[0] = *arg;
			ft_exit_error(INVALID_OPTION, s);
		}
	}
	return 0;
}

/*
** Parse argv and set argv[i] & argv[i + 1] at NULL if an options was found.
*/

int
get_options(
	t_context *ctx,
	int *ac,
	char **av
) {
	int		i = 1, n = 0, jump = 0;

	while (i < *ac)
	{
		if (av[i][0] == '-') {
			if ((jump = switch_set_options(av[i], av[i + 1], ctx)) < 0)
				return 1;
			if (jump == 1) {
				av[i] = NULL; n++; i++;
			} else {
				av[i] = NULL; av[i + 1] = NULL; n += 2; i += 2;
			}
		}
		else {
			del_null_params(ac, av, n);
			return 0;
		}
	}
	del_null_params(ac, av, n);
	return 0;
}
