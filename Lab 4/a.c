#include "piodirect.h"
#include "pwm.h"
#include "driver.h"
#include <signal.h>

GPIO green, yellow, orange, red;
PWM blue, brown;

struct Engine e;

void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		destroy(e.m[0].dir_pin1);
		destroy(e.m[0].dir_pin2);
		destroy(e.m[1].dir_pin1);
		destroy(e.m[1].dir_pin2);
		pwmDestroy(e.m[0].spd_pin);
		pwmDestroy(e.m[1].spd_pin);
		printf("Exited program. \n");
	}
	exit(signum);  
}

void *_motor_thread(void *in_args)
{
	struct motor_args *args = in_args;
	struct Engine *e = args->e;
	while (1)
		drive_motor(e->m[args->motor_id], e->gear, e->direction);
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, sigterm_han);

	e.m[0].spd_pin = pwmCreate(_26, 60);
	e.m[0].dir_pin1 = create(_19, OUT_PIN);
	e.m[0].dir_pin2 = create(_13, OUT_PIN);

	e.m[1].spd_pin = pwmCreate(_16, 60);
	e.m[1].dir_pin1 = create(_21, OUT_PIN);
	e.m[1].dir_pin2 = create(_20, OUT_PIN);	

	e.gear = 50;
	e.direction = D_FORWARD;
	
	struct motor_args motor_args_1;
	motor_args_1.motor_id = 0;
	motor_args_1.e = &e;

	struct motor_args motor_args_2;
	motor_args_2.motor_id = 1;
	motor_args_2.e = &e;

	pthread_t motor_thread_1, motor_thread_2;

	pthread_create(&motor_thread_1, NULL, _motor_thread, (void *)&motor_args_1);
	pthread_create(&motor_thread_2, NULL, _motor_thread, (void *)&motor_args_2);

	pthread_join(motor_thread_1, NULL);
	pthread_join(motor_thread_2, NULL);

	return 0;
}