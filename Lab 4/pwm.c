#include <time.h>
#include "pwm.h"

//initialize GPIO
PWM pwmCreate(enum Pin pin, unsigned int frequency)
{
	PWM pwm;
	gpioSetup();
	pwm.pio = create(pin, OUT_PIN);	
	pwm.frequency = frequency;

	return pwm;
}

int pwmPulse(PWM pwm, float dutyPercent)
{
	//period stores how long the whole pulse takes
	double period = (double)1/pwm.frequency;
	//the time the led is on
	struct timespec duty_t;
	duty_t.tv_sec = 0;
	duty_t.tv_nsec = 10000000*period*dutyPercent; 
	//how long led is off
	struct timespec off_duty_t;
	off_duty_t.tv_sec = 0;
	off_duty_t.tv_nsec = 10000000*(100-dutyPercent)*period;
	//if on time is 0, we shouldn't turn on led at all
	if (duty_t.tv_nsec != 0)
	{
		onOff(pwm.pio, 1);
		nanosleep(&duty_t, NULL);
	}
	//if off time is 0 no need to turn it off at all
	if (off_duty_t.tv_nsec != 0)
	{
		onOff(pwm.pio, 0);
		nanosleep(&off_duty_t, NULL);
	}
}

//destroy GPIO
int pwmDestroy(PWM pwm)
{
	destroy(pwm.pio);
}
