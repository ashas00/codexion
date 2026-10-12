/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logs.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:38 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/22 13:17:18 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

long	diff_ms(struct timeval *start)
{
	struct timeval	now;
	long			sec_diff;
	long			usec_diff;

	gettimeofday(&now, NULL);
	sec_diff = now.tv_sec - start->tv_sec;
	usec_diff = now.tv_usec - start->tv_usec;
	return (sec_diff * 1000 + usec_diff / 1000);
}

void	log_msg(t_data *data, int coder_id, char *mes)
{
	pthread_mutex_lock(&data->log_mutex);
	if (!stopped(data))
		printf("%ld %d %s\n", diff_ms(&data->start_time), coder_id, mes);
	pthread_mutex_unlock(&data->log_mutex);
}

void	log_burnout(t_data *data, int coder_id)
{
	pthread_mutex_lock(&data->log_mutex);
	if (!stopped(data))
	{
		printf("%ld %d burned out\n", diff_ms(&data->start_time), coder_id);
		request_stop(data);
	}
	pthread_mutex_unlock(&data->log_mutex);
}

void	sim_sleep(t_data *data, long ms)
{
	long	start;
	long	remaining;

	start = diff_ms(&data->start_time);
	while (!stopped(data))
	{
		remaining = ms - (diff_ms(&data->start_time) - start);
		if (remaining <= 0)
			return ;
		if (remaining > 1)
			remaining = 1;
		usleep(remaining * 1000);
	}
}
