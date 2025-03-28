CC = gcc
CFLAGS = -g
RM = rm -rf

SOURCE = main.c initenv.c cmd/startcmd.c cmd/cmdsplit.c \
utils/utils.c utils/cmdutils_0.c utils/cmdutils_1.c lib/libft/libft.a -lreadline \
lib/ftprint/libftprintf.a execute/execute.c execute/first_ex.c \
execute/heredoc.c cmd/control.c cmd/startcmd2.c buildin/ft_echo.c

all:
	make -C lib/libft
	make -C lib/ftprint
	$(CC) $(CFLAGS) $(SOURCE) -o minishell

clean:
	make clean -C lib/libft -s
	make clean -C lib/ftprint -s


fclean:
	make fclean -C lib/libft -s
	make fclean -C lib/ftprint -s
	$(RM) minishell

re:	fclean all

.PHONY: all re fclean clean