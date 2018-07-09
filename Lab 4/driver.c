#include "driver.h"

void drive_motor(struct Motor m, float s, dir_type d)
{
	if (d == D_FORWARD)
	{
		onOff(m.dir_pin1, 1);
		onOff(m.dir_pin2, 0);
	}
	else
	{
		onOff(m.dir_pin1, 0);
		onOff(m.dir_pin2, 1);
	}
	pwmPulse(m.spd_pin, s);
}

void *motor_thread(void* in_args)
{
		struct motor_args *args = in_args;
		struct Engine *e = args->e;
		while (1)
		{
			switch(e->direction)
			{
				case D_FORWARD:
					drive_motor(e->m[args->motor_id], e->gear, e->direction);
					break;
				case D_BACKWARD:
					drive_motor(e->m[args->motor_id], 25, e->direction);
					break;
				case D_LEFT:
					if (args->motor_id == 1)
						drive_motor(e->m[args->motor_id], 0, D_FORWARD);
					else
					{
						e->gear = 0;
						drive_motor(e->m[args->motor_id], 25, D_FORWARD);
					}
					break;
				case D_RIGHT:
					if (args->motor_id == 0)
						drive_motor(e->m[args->motor_id], 0, D_FORWARD);
					else
					{
						e->gear = 0;
						drive_motor(e->m[args->motor_id], 25, D_FORWARD);
					}
					break;
			}
		}
}