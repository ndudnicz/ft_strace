#ifndef OPTIONS_H
#define OPTIONS_H

# define OPT_OUTPUT_FILE		0x01
# define OPT_OUTPUT_FILE_CHAR	'o'
# define PARAMS_STR "o"

int			get_options(
	t_context *ctx,
	int *ac,
	char **av
);

#endif /* end of include guard: OPTIONS_H */
