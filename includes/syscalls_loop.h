#ifndef SYSCALLS_LOOP_H
# define SYSCALLS_LOOP_H

# ifdef __x86_64__
#  define O_RAX (sizeof(long)*ORIG_RAX)
#  define R_RAX (sizeof(long)*RAX)
# else
#  error ARCH NOT SUPPORTED
# endif

int
syscalls_loop(
	t_context ctx,
	pid_t const pid
);

#endif
