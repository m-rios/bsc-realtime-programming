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

	while (1)
	{
		onOff(myargs->led, 1);
		busywait(myargs->ontime);
		
		onOff(myargs->led, 0);
		busywait(myargs->offtime);
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
	args.ontime = 1000000;
	args.offtime = 3000000;
	
	struct threadArgs args2;
	args2.led = led2;
	args2.ontime = 1000000;
	args2.offtime = 5000000;
	
	pthread_t threadOne;
	pthread_t threadTwo;
	
	pthread_attr_t low;
	pthread_attr_t high;
	struct sched_param lowp;
	struct sched_param highp;
	
	pthread_attr_init(&low);
	pthread_attr_init(&high);
	
	lowp.sched_priority = sched_get_priority_min(SCHED_FIFO);
	highp.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
	
	
	pthread_attr_setschedpolicy(&low, SCHED_FIFO);
	pthread_attr_setschedpolicy(&high, SCHED_FIFO);
	
	pthread_attr_setschedparam(&low, &lowp);
	pthread_attr_setschedparam(&high, &highp);
	
	
	pthread_attr_setinheritsched(&low, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched(&high, PTHREAD_EXPLICIT_SCHED);
	
	pthread_create(&threadOne, &high, task1, (void *)&args);
	printf("main thread \n");
	pthread_create(&threadTwo, &low, task2, (void *)&args2);
	
	pthread_join(threadOne, NULL);
	pthread_join(threadTwo, NULL);
	return 0;
}
