#ifndef SYSCALLS_LOOP_H
# define SYSCALLS_LOOP_H

# ifndef __x86_64__
#  error ARCH NOT SUPPORTED
# endif

int
syscalls_loop(
	pid_t const pid
);

#endif /* end of include guard: SYSCALLS_LOOP_H */
