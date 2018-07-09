#include "piodirect.h"
#include "pwm.h"
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

int arr[] = { 0, 25, 50, 75, 100 };

//unified function to control any set of leds & buttons
void ledcontrol(int *btnstatus, int *ledstatus, int *trigger_flag, GPIO button, PWM led)
{
	//if button is released trigger event
	if (*btnstatus == 0 && readIn(button) == 1)
	{
		*trigger_flag = 1;
		*btnstatus = 1;
	}
	//if button is pressed untrigger event
	if (*btnstatus == 1 && readIn(button) == 0)
	{
		*trigger_flag = 0;
		*btnstatus = 0;
	}
	//if event is triggered, invert led status
	if (*trigger_flag)
	{
		//increase ledstatus to next status
		*ledstatus = (*ledstatus + 1) % 5;
		*trigger_flag = 0;
	}
	//to keep up with the frequency, this fucntion is always called
	pwmPulse(led, arr[*ledstatus]);
}

PWM led;
PWM led2;
GPIO button;
GPIO button2;

void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		pwmDestroy(led);
		pwmDestroy(led2);
		destroy(button);
		destroy(button2);
		printf("Exited program. \n");
	}
	exit(signum);  
}

int main(int argc, char const *argv[])
{	
	led = pwmCreate(_17, 120);
	led2 = pwmCreate(_27, 120);
	
	button = create(_5, IN_PIN);
	button2 = create(_6, IN_PIN);

    pullUpDown(button, PUD_UP);
	pullUpDown(button2, PUD_UP);
	
	signal(SIGINT, sigterm_han);
	
	int ledstatus = 0;
	int btnstatus = 1;
	int trigger_flag = 0;
	
	int ledstatus2 = 0;
	int btnstatus2 = 1;
	int trigger_flag2 = 0;
	
    while(1)
    {	  
	    ledcontrol(&btnstatus, &ledstatus, &trigger_flag, button, led);
	    ledcontrol(&btnstatus2, &ledstatus2, &trigger_flag2, button2, led2);
    }
	
    return 0;
}
