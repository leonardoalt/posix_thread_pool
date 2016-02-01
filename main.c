#include <stdio.h>
#include <stdlib.h>

#include "ptp.h"

void
test(task *t)
{
	printf("Inside task %p.\n", t);
	//sleep(2);
	int i;
	for(i = 0; i < 999999999; ++i);
}

int
create_thread_func(unsigned int min_th, unsigned int n_of_ta)
{
	return (n_of_ta > (2 * min_th));
}

int
main()
{
	thread_pool *tp = (thread_pool*)malloc(sizeof(thread_pool));
	task** t = malloc(10 * sizeof(task*));
	init_thread_pool(tp, 2, 4, 1000, create_thread_func);
	int i;
	for(i = 0; i < 10; ++i)
	{
		t[i] = malloc(sizeof(task));
		t[i]->func = test;
		add_task(tp, t[i]);
	}

	wait_and_destroy(tp);
}

