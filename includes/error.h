#ifndef ERROR_H
#define ERROR_H

# define GETENV_FAILED "secure_getenv() failed."
# define ASPRINTF_FAILED "asprintf() failed."
# define CANT_STAT "Can't stat"
# define CANT_OPEN "Can't open"
# define INVALID_OPTION "invalid option --"
# define MALLOC_FAILED "malloc failed"
# define ARGUMENT_MISSING "missing argument."
# define FORK_FAILED "fork() failed."
# define SIGACTION_FAILED "sigaction failed."

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
