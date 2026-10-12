/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:28 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/22 13:25:30 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../codexion.h"

void	queue_push(t_queue *queue, int coder_id, long priority)
{
	t_request	temporary;

	if (queue->size >= 2)
		return ;
	queue->items[queue->size].coder_id = coder_id;
	queue->items[queue->size].priority = priority;
	queue->size++;
	if (queue->size == 2 && queue->items[1].priority < queue->items[0].priority)
	{
		temporary = queue->items[0];
		queue->items[0] = queue->items[1];
		queue->items[1] = temporary;
	}
}

void	queue_remove(t_queue *queue, int coder_id)
{
	int	position;

	if (!queue || queue->size <= 0)
		return ;
	position = 0;
	while (position < queue->size
		&& queue->items[position].coder_id != coder_id)
		position++;
	if (position == queue->size)
		return ;
	if (position + 1 < queue->size)
		queue->items[position] = queue->items[position + 1];
	queue->size--;
}
