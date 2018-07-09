#include "piodirect.h"
#include "pwm.h"
#include "driver.h"
#include <pthread.h>
#include <signal.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define TOTAL_MSG 100
#define QUEUE_NAME "/cmd_queue"
#define MAX_MSG_NUM 10
#define MAX_MSG_SIZE 4

pthread_mutex_t gear_mtx, direction_mtx, panic_mtx;

GPIO blue, green, red, white;
mqd_t queue;
int gear;
int PANIC_STATE = 0;
dir_type direction;

struct Engine engine;
struct led_args _red_green_args;
struct led_args _steering_led_args;

struct timespec red_press;
struct timespec exec_stopped;

struct led_args
{
	PWM led1;
	PWM led2;
};

void clean_up()
{
	destroy(blue);
	destroy(white);
	destroy(green);
	destroy(red);
	mq_unlink(QUEUE_NAME);
	pwmDestroy(engine.m[0].spd_pin);
	pwmDestroy(engine.m[1].spd_pin);
	pwmDestroy(_red_green_args.led1);
	pwmDestroy(_red_green_args.led2);
	pwmDestroy(_steering_led_args.led1);
	pwmDestroy(_steering_led_args.led2);
	destroy(engine.m[0].dir_pin1);
	destroy(engine.m[0].dir_pin2);
	destroy(engine.m[1].dir_pin1);
	destroy(engine.m[1].dir_pin2);
	pthread_mutex_destroy(&gear_mtx);	
	pthread_mutex_destroy(&direction_mtx);	
}

void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		clean_up();
		printf("Exited program. \n");
	}
	exit(signum);  
}

//compute elapsed time between two timestamps
int sub_timespec(struct timespec *start, struct timespec *stop, struct timespec *result)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0)
	{
		result->tv_sec = stop->tv_sec - start->tv_sec -1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		result->tv_sec = stop->tv_sec -start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
}

int read_button(GPIO button)
{
	int status = readIn(button);
	struct timespec first_press;
	struct timespec present;
	struct timespec result;
	struct timespec sleep;
	sleep.tv_sec = 0;
	sleep.tv_nsec = 1000000;
	if (status == 0)
	{
		clock_gettime(CLOCK_REALTIME, &first_press);
		while (readIn(button) == 0)
			nanosleep(&sleep, NULL);
		clock_gettime(CLOCK_REALTIME, &present);
		sub_timespec(&first_press, &present, &result);
		if (result.tv_sec >= 3)
			return 2;
		if (result.tv_sec == 0 && result.tv_nsec <= 10000000)
			return 0;
		return 1;
	}
	return 0;
}

void *cmd_thread(void * in_args)
{
	// open queue
	struct mq_attr qAttr;
	qAttr.mq_msgsize = MAX_MSG_SIZE;
	qAttr.mq_maxmsg = MAX_MSG_NUM;
	mqd_t q = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0700, &qAttr);

	int b_status, g_status, r_status, w_status;
	b_status = 1; g_status = 1; r_status = 1; w_status = 1;
	char msg[MAX_MSG_SIZE - 1];
	int err;
	struct timespec present;
	present.tv_sec = 0;
	present.tv_nsec = 0;
	int event_started = 0;
	int longpress = 0;
	while(1)
	{
		int _blue = read_button(blue);
		int _green = read_button(green);
		int _red = read_button(red);
		int _white = read_button(white);
		if (_red == 1)
		{
			printf("R\n");
			clock_gettime(CLOCK_REALTIME, &red_press);
			strcpy(msg, "R");
			// message sent with higher priority
			err = mq_send(q, msg, strlen(msg), 1);
			pthread_mutex_lock(&panic_mtx);
			PANIC_STATE++;
			pthread_mutex_unlock(&panic_mtx);
			if (PANIC_STATE > 1)
			{
				clean_up();
				printf("Emergency exit\n");
				exit(0);
			}
		}
		else if (_green == 1)
		{
			printf("G\n");
			pthread_mutex_lock(&panic_mtx);
			PANIC_STATE = 0;
			pthread_mutex_unlock(&panic_mtx);
			strcpy(msg, "G");
			err = mq_send(q, msg, strlen(msg), 0);
		}
		if (_green == 2)
		{
			printf("RG\n");
			pthread_mutex_lock(&panic_mtx);
			PANIC_STATE = 0;
			pthread_mutex_unlock(&panic_mtx);
			strcpy(msg, "GL");
			err = mq_send(q, msg, strlen(msg), 0);
		}
		else if (_blue == 1)
		{
			printf("B\n");
			pthread_mutex_lock(&panic_mtx);
			PANIC_STATE = 0;
			pthread_mutex_unlock(&panic_mtx);
			strcpy(msg, "B");
			err = mq_send(q, msg, strlen(msg), 0);
		}
		else if (_white == 1)
		{
			printf("W\n");
			pthread_mutex_lock(&panic_mtx);
			PANIC_STATE = 0;
			pthread_mutex_unlock(&panic_mtx);
			strcpy(msg, "W");
			err = mq_send(q, msg, strlen(msg), 0);	
		}
		if (err < 0)
		{
			printf("unable to send message - error code: %d\n", errno);
		}
	}
}


int breathing_led(PWM pwm, int *i, int *up)
{
	pwmPulse(pwm, *i);
	if (*up)
		*i = *i + 1;
	else
		*i = *i - 1;
	if (*i <= 0 || *i >= 100)
	{
		*up = !*up;
	}
	return *i;
}
// continuosly poll the command queue, and updates the state of the motor for the other threads to read it
void *dispatcher(void * in_args)
{
	// open queue
	struct mq_attr qAttr;
	qAttr.mq_msgsize = MAX_MSG_SIZE;
	qAttr.mq_maxmsg = MAX_MSG_NUM;
	mqd_t q2 = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0700, &qAttr);
	char msg[MAX_MSG_SIZE + 1] = "";
	while (1)
	{
		if (mq_receive(q2, msg, MAX_MSG_SIZE, NULL) < 0)
		{
			printf("recieve %d \n", errno);
		}
		if (strcmp(msg, "R") == 0)
		{
			//kill swithc pressed, empty message queue
			while (qAttr.mq_curmsgs > 0)
			{
				mq_receive(q2, msg, MAX_MSG_SIZE, 0);
			}
			//reset motor status
			pthread_mutex_lock(&gear_mtx);
			engine.gear = 0;
			pthread_mutex_unlock(&gear_mtx);
			
			pthread_mutex_lock(&direction_mtx);
			engine.direction = D_FORWARD;
			pthread_mutex_unlock(&direction_mtx);
			clock_gettime(CLOCK_REALTIME, &exec_stopped);
			sub_timespec(&red_press, &exec_stopped, &red_press);
			printf("Stopped in %ld ms\n", red_press.tv_nsec/1000000);	
		}
		else if (strcmp(msg, "G") == 0)
		{
			// short green press message (shift gear up)
			pthread_mutex_lock(&gear_mtx);
			engine.gear = (engine.gear + 25) % 125;
			pthread_mutex_unlock(&gear_mtx);
			pthread_mutex_lock(&direction_mtx);
			engine.direction = D_FORWARD;
			pthread_mutex_unlock(&direction_mtx);
		}
		else if (strcmp(msg, "GL") == 0)
		{
			// long green press message, shift to reversed gear
			pthread_mutex_lock(&direction_mtx);
			engine.direction = D_BACKWARD;
			pthread_mutex_unlock(&direction_mtx);
			pthread_mutex_lock(&gear_mtx);
			//when 100, next G commmand will switch to 125, wich is 0 %125
			engine.gear = 100;
			pthread_mutex_unlock(&gear_mtx);	
		}
		else if (strcmp(msg, "B") == 0)
		{
			// blue press detected, turn left
			pthread_mutex_lock(&direction_mtx);
			engine.direction = D_LEFT;
			pthread_mutex_unlock(&direction_mtx);
		}
		else if (strcmp(msg, "W") == 0)
		{
			// white press detected, turn right
			pthread_mutex_lock(&direction_mtx);
			engine.direction = D_RIGHT;
			pthread_mutex_unlock(&direction_mtx);
		}
		memset(msg, 0, strlen(msg));
	}
}

// controlls red and green leds
void *go_stop_led_thread(void *in_args)
{
	struct led_args *args = in_args;
	int up = 1;
	int i = 0;
	int _p_state;
	while (1)
	{
		pthread_mutex_lock(&panic_mtx);
		_p_state = PANIC_STATE;
		pthread_mutex_unlock(&panic_mtx);
		if (_p_state)
		{
			// panic button has been pressed
			args->led2.frequency = 4;
			pwmPulse(args->led2, 50);
		}
		else {
			// normal functioning state
			pthread_mutex_lock(&gear_mtx);
			int _gear = engine.gear;
			pthread_mutex_unlock(&gear_mtx);
			pthread_mutex_lock(&direction_mtx);
			int _direction = engine.direction;
			pthread_mutex_unlock(&direction_mtx);
			
			if (_direction == D_FORWARD)
			{			
				// change blinking frequency depending on gear
				switch (_gear)
				{
					case 0:
					args->led1.frequency = 60;
					i = breathing_led(args->led1, &i, &up);
					break;
					case 25:
					args->led1.frequency = 2;
					pwmPulse(args->led1, 50);
					break;
					case 50:
					args->led1.frequency = 4;
					pwmPulse(args->led1, 50);
					break;
					case 75:
					args->led1.frequency = 10;
					pwmPulse(args->led1, 50);
					break;
					case 100:
					args->led1.frequency = 20;
					pwmPulse(args->led1, 50);
					break;
				}
			}
			else if(_direction == D_BACKWARD)
			{
				// if reversed gear, green led is off
				args->led1.frequency = 1;
				pwmPulse(args->led1, 0);
			}	
		}	
	}
}

// controls the steering leds
void *steering_led_thread(void *in_args)
{
	struct led_args *args = in_args;
	while (1)
	{
		pthread_mutex_lock(&direction_mtx);
		dir_type dir = engine.direction;
		pthread_mutex_unlock(&direction_mtx);
		if (dir == D_LEFT)
			pwmPulse(args->led1, 50);
		else if (dir == D_RIGHT)
			pwmPulse(args->led2, 50);
	}
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, sigterm_han);
	gear = 0;
	gpioSetup();
	//Button definition
	blue = create(_5, IN_PIN);
	pullUpDown(blue, PUD_UP);
	green = create(_12, IN_PIN);
	pullUpDown(green, PUD_UP);
	red = create(_23, IN_PIN);
	pullUpDown(red, PUD_UP);
	white = create(_27, IN_PIN);
	pullUpDown(white, PUD_UP);

	// specify the pins for each motor of the engine
	engine.m[0].spd_pin = pwmCreate(_26, 60);
	engine.m[0].dir_pin1 = create(_19, OUT_PIN);
	engine.m[0].dir_pin2 = create(_13, OUT_PIN);

	engine.m[1].spd_pin = pwmCreate(_16, 60);
	engine.m[1].dir_pin1 = create(_21, OUT_PIN);
	engine.m[1].dir_pin2 = create(_20, OUT_PIN);	

	// arguments for the motor threads (specifies wich motor each thread is controlling)
	struct motor_args motor_args_1;
	motor_args_1.motor_id = 0;
	motor_args_1.e = &engine;


	struct motor_args motor_args_2;
	motor_args_2.motor_id = 1;
	motor_args_2.e = &engine;

	// led pins for the led controler threads
	_red_green_args.led1 = pwmCreate(_25, 60);
	_red_green_args.led2 = pwmCreate(_24, 60);

	_steering_led_args.led1 = pwmCreate(_6, 60);	
	_steering_led_args.led1.frequency = 2;
	_steering_led_args.led2 = pwmCreate(_17, 60);
	_steering_led_args.led2.frequency = 2;

	// struct sched_param s_critical;
	// struct sched_param s_high;
	// struct sched_param s_medium;
	// struct sched_param s_low;

	// pthread_attr_t critical;
	// pthread_attr_t high;
	// pthread_attr_t medium;
	// pthread_attr_t low;

	// pthread_attr_init(&critical);
	// pthread_attr_init(&high);
	// pthread_attr_init(&medium);
	// pthread_attr_init(&low);

	// s_critical.sched_priority = sched_get_priority_max(SCHED_FIFO);
	// s_high.sched_priority = sched_get_priority_max(SCHED_FIFO)-1;
	// s_medium.sched_priority = sched_get_priority_max(SCHED_FIFO)-2;
	// s_low.sched_priority = sched_get_priority_max(SCHED_FIFO)-3;

	// pthread_attr_setschedpolicy(&low, SCHED_FIFO);
	// pthread_attr_setschedpolicy(&medium, SCHED_FIFO);
	// pthread_attr_setschedpolicy(&high, SCHED_FIFO);
	// pthread_attr_setschedpolicy(&critical, SCHED_FIFO);

	// pthread_attr_setschedparam(&low, &s_low);
	// pthread_attr_setschedparam(&medium, &s_medium);
	// pthread_attr_setschedparam(&high, &s_high);
	// pthread_attr_setschedparam(&critical, &s_critical);

	// pthread_attr_setinheritsched(&low, PTHREAD_EXPLICIT_SCHED);
	// pthread_attr_setinheritsched(&medium, PTHREAD_EXPLICIT_SCHED);
	// pthread_attr_setinheritsched(&high, PTHREAD_EXPLICIT_SCHED);
	// pthread_attr_setinheritsched(&critical, PTHREAD_EXPLICIT_SCHED);


	pthread_mutex_init(&gear_mtx, NULL);
	pthread_mutex_init(&direction_mtx, NULL);

	pthread_t cmd_th, motor_thread_1, motor_thread_2, dispatcher_th, go_stop_thread, steering_thread;

	pthread_create(&cmd_th, NULL, cmd_thread, NULL);
	pthread_create(&dispatcher_th, NULL, dispatcher, NULL);
	pthread_create(&motor_thread_1, NULL, motor_thread, (void *)&motor_args_1);
	pthread_create(&motor_thread_2, NULL, motor_thread, (void *)&motor_args_2);
	pthread_create(&go_stop_thread, NULL, go_stop_led_thread, (void *)&_red_green_args);
	pthread_create(&steering_thread, NULL, steering_led_thread, (void *)&_steering_led_args);

	pthread_join(cmd_th, NULL);
	pthread_join(motor_thread_1, NULL);
	pthread_join(motor_thread_2, NULL);
	pthread_join(dispatcher_th, NULL);
	pthread_join(steering_thread, NULL);
	pthread_join(go_stop_thread, NULL);

	clean_up();
	return 0;
}