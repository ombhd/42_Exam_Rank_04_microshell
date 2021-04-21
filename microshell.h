/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obouykou <obouykou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 12:48:50 by obouykou          #+#    #+#             */
/*   Updated: 2021/04/21 14:45:08 by obouykou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MICROSHELL_H
# define MICROSHELL_H 

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define STDIN		0
# define STDOUT		1
# define STDERR		2

# define END		0
# define S_COLON	1
# define PIPE		2

typedef struct		s_cmds
{
	char 			**args;
	char 			type;
	int 			len;
	int 			fds[2];
	struct s_cmds 	*next;
	struct s_cmds 	*prev;
}					t_cmds;

#endif // !MICROSHELL_H
