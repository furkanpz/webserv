#include "inc/minishell.h"

t_shell	*g_globals;

void	errorm(char *str)
{
	ft_putstr_fd(str, 2);
	exit(EXIT_FAILURE);
}

void	startglobal(void)
{
	g_globals = malloc(sizeof(t_shell));
	g_globals->env = NULL;
	g_globals->local = NULL;
}

int	main(int ac, char **av, char **env)
{
	(void)av;
	if (ac != 1)
		errorm("Error: too many arguments!\n");
	startglobal();
	init_env(env);
	start_cmd();
}
