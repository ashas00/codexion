/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fifo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:29 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/22 14:10:05 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

long	compute_priority_fifo(t_data *data, t_coder *coder)
{
	long	seq;

	(void)coder;
	pthread_mutex_lock(&data->seq_mutex);
	data->request_seq++;
	seq = data->request_seq;
	pthread_mutex_unlock(&data->seq_mutex);
	return (seq);
}
