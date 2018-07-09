#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include "piodirect.h"
#include "busywait.h"

GPIO led;
GPIO led2;
struct threadArgs
{
	GPIO led;
	long ontime;
	long offtime;
};

void *task1(void *args)
{
	struct threadArgs *myargs = args;
	struct timespec t;
	t.tv_nsec = myargs->offtime;
	t.tv_sec = 0;
	
	while (1)
	{
		onOff(myargs->led, 1);
		busywait(myargs->ontime);
		
		onOff(myargs->led, 0);
		nanosleep(&t, NULL);
	}
}

void *task2(void *args)
{
	struct threadArgs *myargs = args;
	
	struct timespec it;
	struct timespec ct;
	while (1)
	{
		onOff(myargs->led, 1);
		busywait(myargs->ontime);
		
		onOff(myargs->led, 0);
		it.tv_nsec = myargs->offtime;
		it.tv_sec = 0;
		nanosleep(&it, NULL);
	}
}

void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		destroy(led);
		destroy(led2);
		printf("Exited program. \n");
	}
	exit(signum);  
}

int main()
{
	gpioSetup();
	signal(SIGINT, sigterm_han);
	led = create(_17, OUT_PIN);
	led2 = create(_27, OUT_PIN);
	
	struct threadArgs args;
	args.led = led;
	args.ontime = 2000000;
	args.offtime = 2000000;
	
	struct threadArgs args2;
	args2.led = led2;
	args2.ontime = 3000000;
	args2.offtime = 3000000;
	
	struct sched_param p;
	struct sched_param p2;
	
	pthread_t threadOne;
	pthread_t threadTwo;
	
	pthread_create(&threadTwo, NULL, task2, (void *)&args2);
	pthread_create(&threadOne, NULL, task1, (void *)&args);
	
	p.__sched_priority = sched_get_priority_min(SCHED_FIFO);
	p2.__sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
	
	pthread_setschedparam(threadOne, SCHED_FIFO, &p);
	pthread_setschedparam(threadTwo, SCHED_FIFO, &p2);
	
	pthread_join(threadOne, NULL);
	pthread_join(threadTwo, NULL);
	return 0;
}
