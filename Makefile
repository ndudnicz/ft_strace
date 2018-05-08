NAME = ft_strace

CC = gcc

FLAGS = -Wextra -Wall -std=c89 -g -fsanitize=address# -Werror

PATH_SRC = src

PATH_OBJ = obj

PATH_INCLUDES = includes/

SRC =	\
main.c

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