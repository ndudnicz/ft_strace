#ifndef ERROR_H
#define ERROR_H

# define EUNKNOWN	512

# define GETENV_FAILED "secure_getenv() failed."
# define ASPRINTF_FAILED "asprintf() failed."
# define CANT_STAT "Can't stat"
# define MALLOC_FAILED "malloc failed"
# define FORK_FAILED "fork() failed."
# define PTRACE_FAILED "ptrace() failed."
# define SIG_BLOCK_FAILED "sig_block() failed."
# define SIG_EMPTY_FAILED "sig_empty() failed."

__attribute__ ((noreturn)) void
ft_exit_error(
	char const *const error,
	char const *const bin_name
);

__attribute__ ((noreturn)) void
ft_exit_perror(
	char const *const error,
	char const *const bin_name
);

#endif /* end of include guard: ERROR_H */
