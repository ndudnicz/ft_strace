#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
	char			*bin_fullpath; /* malloc */
	char			*output_filename; /* malloc */
	int				output_fd; /* open */
	unsigned char	options;
}				t_context;

#endif /* end of include guard: CONTEXT_H */
