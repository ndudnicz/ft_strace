#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct	s_context {
	char			*output_filename; /* malloc */
	int				output_fd; /* open */
	unsigned char	options;
}				t_context;

#endif /* end of include guard: CONTEXT_H */
