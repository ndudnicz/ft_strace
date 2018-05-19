#ifndef SYSCALLS_TABLE_H
# define SYSCALLS_TABLE_H

# define SYSCALL_NUMBER		329

# define MODE_SPECIAL		0x01

enum e_param {
	E_NONE = 0,
	E_INT = 1,
	E_UINT = 2,
	E_PTR = 3,
	E_STR = 4,
	E_STRUCT = 5
};

typedef struct		s_syscall {
	char const *const		name;
	int const				n_param;
	enum e_param const		params[6];
	unsigned char const		mode;
}					t_syscall;

extern t_syscall const syscalls_table[SYSCALL_NUMBER];

#endif
