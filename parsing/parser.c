/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:34 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/17 18:14:10 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"
#include <limits.h>

static int	positive(const char *s)
{
	int	i;

	i = 0;
	if (!s || !s[0])
		return (0);
	while (s[i] != '\0')
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

long	parse_long(const char *s, int *ok)
{
	long	res;
	int		i;
	int		digit;

	if (!positive(s))
	{
		*ok = 0;
		return (0);
	}
	res = 0;
	i = 0;
	while (s[i])
	{
		digit = s[i] - '0';
		if (res > (LONG_MAX - digit) / 10)
		{
			*ok = 0;
			return (0);
		}
		res = res * 10 + digit;
		i++;
	}
	*ok = 1;
	return (res);
}

int	parser(int ac, char **av, t_params *params)
{
	if (ac == 9)
	{
		if (!parse_av1_2(av, params))
			return (0);
		if (!parse_av3_4(av, params))
			return (0);
		if (!parse_av5_6(av, params))
			return (0);
		if (!parse_av7_8(av, params))
			return (0);
		return (1);
	}
	else
	{
		fprintf(stderr, "Error: expected 8 arguments\n");
		return (0);
	}
}
