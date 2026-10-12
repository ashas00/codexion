/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:44 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/20 19:50:41 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

int	stopped(t_data *data)
{
	int	stopped;

	pthread_mutex_lock(&data->stop_mutex);
	stopped = data->stop;
	pthread_mutex_unlock(&data->stop_mutex);
	return (stopped);
}

void	request_stop(t_data *data)
{
	pthread_mutex_lock(&data->stop_mutex);
	data->stop = 1;
	pthread_mutex_unlock(&data->stop_mutex);
}

static void	wake_all_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->params.num_coders)
	{
		pthread_mutex_lock(&data->dongles[i].mutex);
		pthread_cond_broadcast(&data->dongles[i].cond);
		pthread_mutex_unlock(&data->dongles[i].mutex);
		i++;
	}
}

static int	find_burned(t_data *data)
{
	long	now;
	int		i;

	pthread_mutex_lock(&data->progress_mutex);
	now = diff_ms(&data->start_time);
	i = 0;
	while (i < data->params.num_coders)
	{
		if (data->coders[i].compiles_done < data->params.required
			&& now - data->coders[i].last_compile_start
			>= data->params.burnout)
		{
			pthread_mutex_unlock(&data->progress_mutex);
			return (data->coders[i].id);
		}
		i++;
	}
	pthread_mutex_unlock(&data->progress_mutex);
	return (0);
}

void	*monitor_thread(void *arg)
{
	t_data	*data;
	int		victim;

	data = (t_data *)arg;
	while (!stopped(data))
	{
		victim = find_burned(data);
		if (victim != 0)
			log_burnout(data, victim);
		if (victim != 0 || all_required_met(data))
			request_stop(data);
		else
			usleep(5000);
	}
	wake_all_dongles(data);
	return (NULL);
}
