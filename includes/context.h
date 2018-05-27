#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
	char			*bin_fullpath;
	char			*output_filename;
	int				output_fd;
	unsigned char	options;
}				t_context;

#endif /* end of include guard: CONTEXT_H */
