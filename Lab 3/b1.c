#include <pthread.h>
#include <signal.h>
#include <time.h>
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
	
	struct timespec it;
	struct timespec ct;
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
	while (1)
	{
		onOff(myargs->led, 1);
		busywait(myargs->ontime);
		
		onOff(myargs->led, 0);
		clock_gettime(CLOCK_REALTIME, &it);
		
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
	
	pthread_create(&threadOne, NULL, task1, (void *)&args);
	pthread_create(&threadTwo, NULL, task2, (void *)&args2);

	
	pthread_join(threadOne, NULL);
	pthread_join(threadTwo, NULL);
	return 0;
}
