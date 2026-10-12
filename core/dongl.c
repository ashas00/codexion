/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongl.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:40 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/22 14:13:50 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

static int	wait_for_dongle(t_dongle *dongle, t_data *data, int coder_id)
{
	long			now;
	long			time_ms;
	struct timespec	deadline;

	while (!stopped(data))
	{
		now = diff_ms(&data->start_time);
		if (dongle->in_use || dongle->waiting.size == 0
			|| dongle->waiting.items[0].coder_id != coder_id)
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
		else if (now - dongle->cooldown < data->params.cooldown)
		{
			time_ms = data->start_time.tv_sec * 1000;
			time_ms += data->start_time.tv_usec / 1000;
			time_ms += dongle->cooldown + data->params.cooldown;
			deadline.tv_sec = time_ms / 1000;
			deadline.tv_nsec = (time_ms % 1000) * 1000000;
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &deadline);
		}
		else
			return (1);
	}
	return (0);
}

static void	single_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	while (!stopped(coder->thread_data))
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	pthread_mutex_unlock(&dongle->mutex);
	dongle_release(dongle, coder->thread_data);
}

int	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->left;
	second = coder->right;
	if (first->id > second->id)
	{
		first = coder->right;
		second = coder->left;
	}
	if (!dongle_acquire(first, coder))
		return (0);
	log_msg(coder->thread_data, coder->id, "has taken a dongle");
	if (first == second)
	{
		single_dongle(coder, first);
		return (0);
	}
	if (!dongle_acquire(second, coder))
	{
		dongle_release(first, coder->thread_data);
		return (0);
	}
	log_msg(coder->thread_data, coder->id, "has taken a dongle");
	return (1);
}

int	dongle_acquire(t_dongle *dongle, t_coder *coder)
{
	t_data	*data;
	long	priority;

	data = coder->thread_data;
	pthread_mutex_lock(&dongle->mutex);
	if (strcmp(data->params.scheduler, "edf") == 0)
		priority = compute_priority_edf(data, coder);
	else
		priority = compute_priority_fifo(data, coder);
	queue_push(&dongle->waiting, coder->id, priority);
	if (!wait_for_dongle(dongle, data, coder->id))
	{
		queue_remove(&dongle->waiting, coder->id);
		pthread_cond_broadcast(&dongle->cond);
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	if (dongle->waiting.size == 2)
		dongle->waiting.items[0] = dongle->waiting.items[1];
	dongle->waiting.size--;
	dongle->in_use = 1;
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}

void	dongle_release(t_dongle *dongle, t_data *data)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->cooldown = diff_ms(&data->start_time);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
