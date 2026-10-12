/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   edf.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:32 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/17 18:11:33 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

long	compute_priority_edf(t_data *data, t_coder *coder)
{
	long	seq;
	long	deadline;

	pthread_mutex_lock(&data->seq_mutex);
	data->request_seq++;
	seq = data->request_seq;
	pthread_mutex_unlock(&data->seq_mutex);
	deadline = coder->last_compile_start + data->params.burnout;
	return (deadline * 1000000 + seq);
}
