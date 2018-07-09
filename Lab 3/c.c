#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include "piodirect.h"
#include "busywait.h"
#include "pwm.h"

PWM led;
PWM led2;

double globalcmd;
pthread_mutex_t lock;

struct threadArgs
{
	PWM led;
	int led_id;
};

void *ledcontrol(void *args)
{
	struct threadArgs *myargs = args;
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = 10000000;
	
	int str = 0;
	
	while (1)
	{
		double n;
		pthread_mutex_lock(&lock);
		double m = modf(globalcmd, &n);
		if (n == myargs->led_id)
		{
			str = m * 1000;
			globalcmd = 0;
		}
		pthread_mutex_unlock(&lock);
		pwmPulse(myargs->led, str);
	}
}

void *readinput()
{
	while (1)
	{
		double tmp;
		scanf("%lf", &tmp);
		
		pthread_mutex_lock(&lock);
		globalcmd = tmp;
		pthread_mutex_unlock(&lock);
	}
}

void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		pwmDestroy(led);
		pwmDestroy(led2);
		printf("Exited program. \n");
	}
	exit(signum);  
}

int main()
{
	gpioSetup();
	signal(SIGINT, sigterm_han);
	led = pwmCreate(_17, 60);
	led2 = pwmCreate(_27, 60);
	
	struct threadArgs args;
	args.led = led;
	args.led_id = 1;
	
	struct threadArgs args2;
	args2.led = led2;
	args2.led_id = 2;
	
	pthread_mutex_init(&lock, NULL);
	
	struct sched_param p;
	struct sched_param p2;
	struct sched_param p3;
	
	pthread_t taskOne;
	pthread_t taskTwo;
	pthread_t taskThree;
	
	pthread_create(&taskOne, NULL, ledcontrol, (void *)&args);
	pthread_create(&taskTwo, NULL, ledcontrol, (void *)&args2);
	pthread_create(&taskThree, NULL, readinput, NULL);
	
	p.__sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
	p2.__sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
	p3.__sched_priority = sched_get_priority_min(SCHED_FIFO);
	
	pthread_setschedparam(taskOne, SCHED_FIFO, &p);
	pthread_setschedparam(taskTwo, SCHED_FIFO, &p2);
	pthread_setschedparam(taskThree, SCHED_FIFO, &p3);
	
	
	pthread_join(taskOne, NULL);
	pthread_join(taskTwo, NULL);
	pthread_join(taskThree, NULL);
	pthread_mutex_destroy(&lock);
	return 0;
}
