#ifndef _DRIVER_H
#define _DRIVER_H 

#include "piodirect.h"
#include "pwm.h"

typedef enum { D_FORWARD, D_BACKWARD, D_LEFT, D_RIGHT } dir_type;

struct Motor
{
	PWM spd_pin;
	GPIO dir_pin1;
	GPIO dir_pin2;
};

struct Engine
{
	struct Motor m[2];
	int gear;
	dir_type direction;
};

struct motor_args
{
	int motor_id;
	struct Engine *e;
};

void drive_motor(struct Motor m, float s, dir_type d);
void *motor_thread(void* in_args);

#endif