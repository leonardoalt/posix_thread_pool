/*
------------------------------------------------------------------------
This file is part of POSIX Thread Pool.
For the latest info, see http://gitorious.org/posix_thread_pool

Copyright (C) 2010 Leonardo Alt <leonardoaltt@gmail.com>

This software is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------
*/

#include <errno.h>
#include <stdlib.h>

#include "ptp.h"

void
init_thread_pool(thread_pool *tp, unsigned int min_threads, unsigned int max_threads, unsigned int timeout, create_new_thread_func cntf)
{
	int i;
	tp->head_worker = NULL;
	tp->head_task = NULL;
	tp->tail_task = NULL;
	tp->n_of_tasks = 0;
	tp->min_threads = min_threads;
	tp->max_threads = max_threads;
	tp->n_of_threads = min_threads;
	tp->d_flag = 0;
	tp->create_thread = cntf;
	pthread_mutex_init(&tp->queue_mutex, NULL);
	pthread_cond_init(&tp->tasks_cond_var, NULL);
	tp->timeout.tv_sec = timeout / 1000;
	tp->timeout.tv_nsec = (timeout % 1000) * 1000000;

	tp->head_worker = (worker*)malloc(sizeof(worker));
	worker **aux_w = &tp->head_worker;
	worker *new_w;
	(*aux_w)->prev = NULL;
	pthread_create(&tp->head_worker->id, NULL, execute, (void*)tp);
	for(i = 1; i < min_threads; ++i)
	{
		new_w = (worker*)malloc(sizeof(worker));
		(*aux_w)->next = new_w;
		new_w->prev = *aux_w;
		pthread_create(&new_w->id, NULL, execute, (void*)tp);
		aux_w = &new_w;
	}
	(*aux_w)->next = NULL;
}

void
wait_and_destroy(thread_pool *tp)
{
	tp->d_flag = 1;
	pthread_cond_broadcast(&tp->tasks_cond_var);
	worker *aux = tp->head_worker;
	while(aux != NULL)
	{
		pthread_join(aux->id, NULL);
		free(aux);
		aux = aux->next;
	}
}

void
add_task(thread_pool *tp, task *t)
{
	pthread_mutex_lock(&tp->queue_mutex);
	t->next = NULL;
	
	if(tp->n_of_tasks == 0)
	{
		tp->head_task = t;
		tp->tail_task = t;
	}
	else
	{
		tp->tail_task->next = t;
		tp->tail_task = t;
	}

	++tp->n_of_tasks;
	if(tp->n_of_threads < tp->max_threads && tp->create_thread(tp->min_threads, tp->n_of_tasks))
	{
		pthread_t *t = malloc(sizeof(pthread_t));
		pthread_create(t, NULL, execute, (void*)tp);
		++tp->n_of_threads;
	}

	pthread_cond_signal(&tp->tasks_cond_var);
	pthread_mutex_unlock(&tp->queue_mutex);
}

void*
execute(void *param)
{
	thread_pool *tp = (thread_pool*)param;
	task *t;
	struct timespec ts;
	int rc;
	while(1)
	{
		pthread_mutex_lock(&tp->queue_mutex);
		while(tp->n_of_tasks == 0)
		{
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += tp->timeout.tv_sec;
			ts.tv_nsec += tp->timeout.tv_nsec;
			rc = pthread_cond_timedwait(&tp->tasks_cond_var, &tp->queue_mutex, &ts);
			if((rc == ETIMEDOUT && tp->n_of_threads > tp->min_threads) || tp->d_flag)
			{
				pthread_mutex_unlock(&tp->queue_mutex);
				goto end;
			}
		}
		t = tp->head_task;
		tp->head_task = t->next;
		--tp->n_of_tasks;
		pthread_mutex_unlock(&tp->queue_mutex);
		t->func(t);
	}
end:
	--tp->n_of_threads;
	pthread_exit(NULL);
}
