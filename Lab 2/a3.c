#include "piodirect.h"
#include <time.h>
#include "pwm.h"
#include <signal.h>

PWM pwm; 
//destroy GPIO when ctrl c is capture
void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		pwmDestroy(pwm);
		printf("Exited program. \n");
	}
	exit(signum);  
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, sigterm_han);	
	pwm = pwmCreate(_17, 60);
	int i;
	//if up = 1 c increases, otherwise c decreases
	int up = 1;
	//c is duty percent, changed over time
	int c = 0;
	for (i = 0; i < 1000; i++)
	{
		pwmPulse(pwm, c);
		
		if (up)
		{
			c++;
		}
		else
		{
			c--;
		}
		//if c reaches limits, direction is changed	
		if (c <= 0 || c >= 100)
		{
			up = !up;
		}
	}
	
	pwmDestroy(pwm);

    return 0;
}
