/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: olprymas <olprymas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 18:11:25 by olprymas          #+#    #+#             */
/*   Updated: 2026/09/17 18:11:25 by olprymas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>

typedef struct s_data	t_data;

typedef struct s_params
{
	int					num_coders;
	long				burnout;
	long				compile;
	long				debug;
	long				refactor;
	int					required;
	long				cooldown;
	char				*scheduler;
}						t_params;

typedef struct s_request
{
	int					coder_id;
	long				priority;
}						t_request;

typedef struct s_queue
{
	t_request			*items;
	int					size;
}						t_queue;

typedef struct s_dongle
{
	int					id;
	int					in_use;
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	long				cooldown;
	t_queue				waiting;
}						t_dongle;

typedef struct s_coder
{
	int					id;
	pthread_t			thread;
	t_dongle			*left;
	t_dongle			*right;
	long				last_compile_start;
	int					compiles_done;
	t_data				*thread_data;
}						t_coder;

typedef struct s_data
{
	t_params			params;
	t_coder				*coders;
	t_dongle			*dongles;
	struct timeval		start_time;
	pthread_mutex_t		log_mutex;
	int					stop;
	pthread_mutex_t		stop_mutex;
	long				request_seq;
	int					finished;
	pthread_mutex_t		seq_mutex;
	pthread_mutex_t		progress_mutex;
}						t_data;

int						parser(int ac, char **av, t_params *params);
long					parse_long(const char *s, int *ok);
int						parse_av1_2(char **av, t_params *p);
int						parse_av3_4(char **av, t_params *p);
int						parse_av5_6(char **av, t_params *p);
int						parse_av7_8(char **av, t_params *p);

int						create_data(t_data *data, t_params *params);
void					destroy_dongles(t_dongle *dongles, int count);

void					log_msg(t_data *data, int coder_id, char *message);
void					log_burnout(t_data *data, int coder_id);
void					sim_sleep(t_data *data, long ms);
long					diff_ms(struct timeval *start);

int						take_dongles(t_coder *coder);
int						dongle_acquire(t_dongle *dongle, t_coder *coder);
void					dongle_release(t_dongle *dongle, t_data *data);
long					compute_priority_fifo(t_data *data, t_coder *coder);
long					compute_priority_edf(t_data *data, t_coder *coder);

void					*monitor_thread(void *arg);
int						stopped(t_data *data);
void					request_stop(t_data *data);

void					*coder_thread(void *arg);
int						all_required_met(t_data *data);

void					queue_push(t_queue *queue, int coder_id, long priority);
void					queue_remove(t_queue *queue, int coder_id);

#endif