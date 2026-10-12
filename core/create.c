/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:42 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/17 18:11:43 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

void	destroy_dongles(t_dongle *dongles, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&dongles[i].mutex);
		pthread_cond_destroy(&dongles[i].cond);
		free(dongles[i].waiting.items);
		i++;
	}
}

static int	init_dongles(t_dongle *dongles, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		dongles[i].id = i + 1;
		dongles[i].waiting.items = malloc(sizeof(t_request) * 2);
		if (!dongles[i].waiting.items)
			return (destroy_dongles(dongles, i), 0);
		pthread_mutex_init(&dongles[i].mutex, NULL);
		pthread_cond_init(&dongles[i].cond, NULL);
		dongles[i].in_use = 0;
		dongles[i].cooldown = -1000000;
		dongles[i].waiting.size = 0;
		i++;
	}
	return (1);
}

static void	init_coders(t_data *data)
{
	int	i;
	int	n;

	n = data->params.num_coders;
	i = 0;
	while (i < n)
	{
		data->coders[i].id = i + 1;
		data->coders[i].right = &data->dongles[i];
		data->coders[i].left = &data->dongles[(i - 1 + n) % n];
		data->coders[i].last_compile_start = 0;
		data->coders[i].compiles_done = 0;
		data->coders[i].thread_data = data;
		i++;
	}
}

int	create_data(t_data *data, t_params *params)
{
	data->params = *params;
	data->finished = 0;
	data->dongles = malloc(sizeof(t_dongle) * params->num_coders);
	if (!data->dongles)
		return (0);
	data->coders = malloc(sizeof(t_coder) * params->num_coders);
	if (!data->coders)
		return (free(data->dongles), 0);
	if (!init_dongles(data->dongles, params->num_coders))
		return (free(data->coders), free(data->dongles), 0);
	init_coders(data);
	return (1);
}
