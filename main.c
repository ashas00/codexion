/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:22 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/17 18:11:23 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_sync(t_data *data)
{
	gettimeofday(&data->start_time, NULL);
	data->stop = 0;
	data->request_seq = 0;
	if (pthread_mutex_init(&data->log_mutex, NULL) != 0
		|| pthread_mutex_init(&data->stop_mutex, NULL) != 0
		|| pthread_mutex_init(&data->seq_mutex, NULL) != 0
		|| pthread_mutex_init(&data->progress_mutex, NULL) != 0)
		return (0);
	return (1);
}

static void	cleanup(t_data *data)
{
	destroy_dongles(data->dongles, data->params.num_coders);
	pthread_mutex_destroy(&data->log_mutex);
	pthread_mutex_destroy(&data->stop_mutex);
	pthread_mutex_destroy(&data->seq_mutex);
	pthread_mutex_destroy(&data->progress_mutex);
	free(data->coders);
	free(data->dongles);
}

static int	run_coders(t_data *data)
{
	int	i;
	int	created;

	created = 0;
	while (created < data->params.num_coders)
	{
		if (pthread_create(&data->coders[created].thread, NULL, coder_thread,
				&data->coders[created]) != 0)
			break ;
		created++;
	}
	if (created < data->params.num_coders)
	{
		fprintf(stderr, "Error: cannot create coder thread\n");
		request_stop(data);
	}
	i = 0;
	while (i < created)
	{
		pthread_join(data->coders[i].thread, NULL);
		i++;
	}
	return (created == data->params.num_coders);
}

static int	run_simulation(t_data *data)
{
	pthread_t	monitor;
	int			ok;

	if (!init_sync(data))
		return (fprintf(stderr, "Error: cannot init mutexes\n"), 0);
	if (pthread_create(&monitor, NULL, monitor_thread, data) != 0)
		return (fprintf(stderr, "Error: cannot create monitor thread\n"), 0);
	ok = run_coders(data);
	request_stop(data);
	pthread_join(monitor, NULL);
	return (ok);
}

int	main(int argc, char **argv)
{
	t_params	params;
	t_data		data;
	int			ok;

	if (!parser(argc, argv, &params))
		return (1);
	if (params.required == 0)
		return (0);
	if (!create_data(&data, &params))
		return (fprintf(stderr, "Error: allocation failed\n"), 1);
	ok = run_simulation(&data);
	cleanup(&data);
	if (!ok)
		return (1);
	return (0);
}
