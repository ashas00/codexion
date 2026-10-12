/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:47 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/20 19:22:01 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

int	all_required_met(t_data *data)
{
	int	done;

	pthread_mutex_lock(&data->progress_mutex);
	done = (data->finished >= data->params.num_coders);
	pthread_mutex_unlock(&data->progress_mutex);
	return (done);
}

static void	finish_compile(t_coder *coder, t_data *data)
{
	pthread_mutex_lock(&data->progress_mutex);
	coder->compiles_done++;
	if (coder->compiles_done == data->params.required)
		data->finished++;
	pthread_mutex_unlock(&data->progress_mutex);
}

static void	run_cycle(t_coder *coder, t_data *data)
{
	pthread_mutex_lock(&data->progress_mutex);
	coder->last_compile_start = diff_ms(&data->start_time);
	pthread_mutex_unlock(&data->progress_mutex);
	log_msg(data, coder->id, "is compiling");
	sim_sleep(data, data->params.compile);
	dongle_release(coder->left, data);
	dongle_release(coder->right, data);
	finish_compile(coder, data);
	log_msg(data, coder->id, "is debugging");
	sim_sleep(data, data->params.debug);
	log_msg(data, coder->id, "is refactoring");
	sim_sleep(data, data->params.refactor);
}

void	*coder_thread(void *arg)
{
	t_coder	*coder;
	t_data	*data;

	coder = (t_coder *)arg;
	data = coder->thread_data;
	while (!stopped(data))
	{
		if (!take_dongles(coder))
			break ;
		run_cycle(coder, data);
	}
	return (NULL);
}
