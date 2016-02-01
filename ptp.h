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

#pragma once

#include <pthread.h>
#include <time.h>

struct _task;
typedef void (*task_func)(struct _task *task);
typedef int (*create_new_thread_func)(unsigned int min_threads, unsigned int n_of_tasks);

/** The task. */
struct _task
{
	/** The parameters of the task. */
	void *params;

	/** The function to be called. */
	task_func func;

	/** The next task, in linked list. */
	struct _task *next;
};

typedef struct _task task;

struct _worker
{
	/** Pthread ID. */
	pthread_t id;

	/** Next worker, linked list. */
	struct _worker *next;
	
	/** Previous worker, linked list. */
	struct _worker *prev;
};

typedef struct _worker worker;

/** The thread pool. */
struct _thread_pool
{
	/** Pointer to the head of the workers list. */
	worker *head_worker;

	/** Pointer to the head of the tasks list. */
	task *head_task;

	/** Pointer to the tail of the tasks list. */
	task *tail_task;

	/** Number of tasks in queue. */
	unsigned int n_of_tasks;

	/** Min number of threads. */
	unsigned int min_threads;

	/** Max number of threads. */
	unsigned int max_threads;

	/** Current number of threads. */
	unsigned int n_of_threads;

	/** Destroy flag. */
	int d_flag;

	/** The timeout interval. */
	struct timespec timeout;

	/** Mutex to control access to the linked list (the queue). */
	pthread_mutex_t queue_mutex;

	/** Condition variable to control threads wakes/sleeps. */
	pthread_cond_t tasks_cond_var;

	/** This is the function that will be called when a new task is added
	  * to know if it is necessary to create another thread.
	  * This function must receive two unsigned ints (min_threads and n_of_tasks)
	  * and return an int (bool, necessary or not).
	  * The function must be passed in init_thread_pool.
	  */
	create_new_thread_func create_thread;
};

typedef struct _thread_pool thread_pool;

/** Inits the thread pool.
  *
  * @param tp The pointer to the thread pool to be initialized.
  * @param min_threads The min number of threads.
  * @param max_threads The max number of threads.
  * @param timeout The time (in milliseconds) that a thread must sleep to be destroyed (if its an extra thread).
  * @param cntf The function that will say if its necessary to create another thread when a new task is added.
  */
void init_thread_pool(thread_pool *tp, unsigned int min_threads, unsigned int max_threads, unsigned int timeout, create_new_thread_func cntf);

/** Adds a new task.
  *
  * @param tp The pointer to the thread pool.
  * @param t The task to be added.
  */
void add_task(thread_pool *tp, task *t);

/** This is the function that will be executed by the threads.
  * Here is the main loop of the threads.
  *
  * @param param Parameter(s) needed by the thread.
  */
void* execute(void *param);

/** When this function is called, all threads are destroyed (after executing) */
void wait_and_destroy(thread_pool *tp);

