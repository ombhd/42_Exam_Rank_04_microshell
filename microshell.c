/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obouykou <obouykou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 08:43:26 by obouykou          #+#    #+#             */
/*   Updated: 2021/04/19 15:17:25 by obouykou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define ERRFATAL "error: fatal\n"

#define END 0
#define S_COLON 1
#define PIPE 2

typedef struct s_cmds
{
	char **args;
	char type;
	int len;
	int pipes[2];
	struct s_cmds *next;
	struct s_cmds *prev;
} t_cmds;

size_t ft_strlen(char const *str)
{
	size_t l;

	l = 0;
	if (str)
		while (*str++)
			l++;
	return (l);
}

char *ft_strdup(char const *str)
{
	char *res;
	int len;

	len = ft_strlen(str);
	res = (char *)malloc(len + 1);
	for (size_t i = 0; i < len; i++)
	{
		res[i] = str[i];
	}
	res[len] = '\0';
	return (res);
}

void ft_putchar_fd(char c, int fd)
{
	write(fd, &c, 1);
}

void ft_putstr_fd(char const *str, int fd)
{
	while (*str)
		ft_putchar_fd(*str++, fd);
}

void *fatal_exit(void)
{
	ft_putstr_fd(ERRFATAL, STDERR);
	exit(EXIT_FAILURE);
	return (NULL);
}

t_cmds *get_list_head(t_cmds *cmds)
{
	while (cmds)
	{
		if (!cmds->prev)
			break;
		cmds = cmds->prev;
	}
	return (cmds);
}

int push_arg(t_cmds *cmds, char const *arg)
{
	char **new_args;
	int i;

	if (!(new_args = (char **)malloc(sizeof(*new_args) * (cmds->len + 1))))
		return (EXIT_FAILURE);
	i = -1;
	while (++i < cmds->len)
	{
		new_args[i] = cmds->args[i];
	}
	if (cmds->len > 0)
		free(cmds->args);
	cmds->args = new_args;
	cmds->args[i++] = ft_strdup(arg);
	cmds->len++;
	return (EXIT_SUCCESS);
}

t_cmds *clear_list(t_cmds *cmds)
{
	int i;
	void *tmp;

	cmds = get_list_head(cmds);
	while (cmds)
	{
		i = -1;
		while (++i < cmds->len)
		{
			free(cmds->args[i]);
		}
		free(cmds->args);
		cmds->args = NULL;
		tmp = cmds->next;
		free(cmds);
		cmds = tmp;
	}
	// puts("DONE");
	return (NULL);
}

t_cmds *new_cmd_node(t_cmds **lst)
{
	t_cmds *new;

	if (!(new = (t_cmds *)malloc(sizeof(t_cmds))))
		return ((t_cmds *)fatal_exit());
	new->next = NULL;
	new->prev = NULL;
	new->args = NULL;
	new->len = 0;
	new->type = END;
	return (new);
}

int push_cmd(t_cmds **cmds_list, char const *arg)
{
	t_cmds *new;

	new = new_cmd_node(cmds_list);
	push_arg(new, arg);
	if (!*cmds_list)
	{
		*cmds_list = new;
	}
	else
	{
		(*cmds_list)->next = new;
		new->prev = *cmds_list;
		*cmds_list = new;
	}
	return (EXIT_SUCCESS);
}

int parse_args(t_cmds **cmds, int ac, char const *av[])
{
	size_t i;
	char b;

	i = 0;
	while (++i < ac)
	{
		b = (strcmp(av[i], ";") == 0);
		if (b && !*cmds)
			continue;
		if (!b && (!*cmds || (*cmds)->type != END))
		{
			push_cmd(cmds, av[i]);
		}
		else if (b)
		{
			(*cmds)->type = S_COLON;
		}
		else if (!strcmp(av[i], "|"))
		{
			(*cmds)->type = PIPE;
		}
		else
		{
			push_arg(*cmds, av[i]);
		}
	}
	return (EXIT_SUCCESS);
}

void print_data(t_cmds *cmds)
{
	int i;

	printf("\n=================== DATA ====================\n");
	cmds = get_list_head(cmds);
	printf("-------------------------\n\n");
	while (cmds)
	{
		i = -1;
		printf("- ARGS:\n");
		while (++i < cmds->len)
		{
			printf("arg[%d]=[%s]\n", i, cmds->args[i]);
		}
		if (cmds->type == END)
			printf("type: [END]\n");
		else if (cmds->type == S_COLON)
			printf("type: [S_COLON]\n");
		else if (cmds->type == PIPE)
			printf("type: [PIPE]\n");
		printf("len: %d\n", cmds->len);
		if (cmds->type == PIPE)
			printf("pipes[0]: %d \t pipe[1]: %d\n", cmds->pipes[0], cmds->pipes[1]);
		cmds = cmds->next;
		printf("\n-------------------------\n\n");
	}
	printf("=============================================\n");
}

int cd(t_cmds *cmds)
{
	int ret;

	ret = EXIT_FAILURE;
	if (cmds->len != 2)
		ft_putstr_fd("error: cd: bad arguments\n", STDERR);
	if (cmds->len == 2)
	{
		if ((ret = chdir(cmds->args[1])))
		{
			ft_putstr_fd("error: cd: cannot change directory to ", STDERR);
			ft_putstr_fd(cmds->args[1], STDERR);
			ft_putstr_fd("\n", STDERR);
		}
	}
	return (ret);
}

int run_child(t_cmds *cmd, t_cmds *prev, int *ret, char *const *env)
{
	if (cmd->type == PIPE)
	{
		if (dup2(cmd->pipes[1], STDOUT) == -1)
			return ((int)fatal_exit());
	}
	if (prev && prev->type == PIPE)
	{
		if (dup2(prev->pipes[0], STDIN) == -1)
			return ((int)fatal_exit());
	}
	if ((*ret = execve(cmd->args[0], cmd->args, env)) == -1)
	{
		ft_putstr_fd("error: cannot execute ", STDERR);
		ft_putstr_fd(cmd->args[0], STDERR);
		ft_putstr_fd("\n", STDERR);
	}
	exit(*ret);
	return (*ret);
}

int single_cmd_exec(t_cmds *cmd, char *const *env)
{
	int ret;
	pid_t pid;
	int status;
	char is_pipe_open;
	t_cmds *prev;

	ret = EXIT_FAILURE;
	is_pipe_open = 0;
	prev = cmd->prev;
	if (cmd->type == PIPE || (prev && prev->type == PIPE))
	{
		is_pipe_open = 1;
		if (pipe(cmd->pipes))
			return ((int)fatal_exit());
	}
	if ((pid = fork()) == -1)
		return ((int)fatal_exit());
	if (pid == 0)
	{
		run_child(cmd, prev, &ret, env);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (is_pipe_open)
		{
			close(cmd->pipes[1]);
			if (cmd->type != PIPE)
				close(cmd->pipes[0]);
		}
		if (prev && prev->type == PIPE)
			close(prev->pipes[0]);
		// if (WIFEXITED(status))
		// 	ret = WEXITSTATUS(status);
	}
	return (ret);
}

int cmds_exec(t_cmds *cmds, char *const *env)
{
	int ret;
	ret = EXIT_SUCCESS;

	cmds = get_list_head(cmds);
	while (cmds)
	{
		if (!strcmp(cmds->args[0], "cd"))
		{
			ret = cd(cmds);
		}
		else
			ret = single_cmd_exec(cmds, env);
		if (!cmds->next)
			break;
		cmds = cmds->next;
	}
	return (ret);
}

int main(int ac, char const *av[], char *const *env)
{
	t_cmds *cmds;
	int ret;

	cmds = NULL;
	ret = EXIT_SUCCESS;
	parse_args(&cmds, ac, av);
	puts("\n======================== EXECUTION ======================");
	if (cmds)
		ret = cmds_exec(cmds, env);
	puts("======================== EXECUTION ======================");
	print_data(cmds);
	cmds = clear_list(cmds);
	// getchar();
	return (ret);
}
