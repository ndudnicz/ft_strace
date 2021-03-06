NAME = ft_strace

CC = gcc

FLAGS = -Wextra -Wall -Werror -std=c89 -flto -O1

PATH_SRC = src

PATH_OBJ = obj

PATH_INCLUDES = includes/

SRC =	\
main.c \
get_bin_path.c \
syscalls_table.c \
syscalls_loop.c \
signal_handler.c \
error.c

OBJ = $(SRC:%.c=obj/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) -o $@ $(OBJ)

$(PATH_OBJ)/%.o: $(PATH_SRC)/%.c
	$(CC) $(FLAGS) -MMD -o $@ -c $< -I $(PATH_INCLUDES)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

-include $(OBJ:.o=.d)
