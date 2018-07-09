#include "piodirect.h"
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
//unified function to control any set of leds & buttons
void ledcontrol(int *btnstatus, int *ledstatus, int *trigger_flag, GPIO button, GPIO led)
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
		*ledstatus = !*ledstatus;
		onOff(led, *ledstatus);
		*trigger_flag = 0;
	}
}

GPIO led;
GPIO led2;
GPIO button;
GPIO button2;
//ctrl c handling
void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		destroy(led);
		destroy(led2);
		destroy(button);
		destroy(button2);
		printf("Exited program. \n");
	}
	exit(signum);  
}

int main(int argc, char const *argv[])
{
    gpioSetup();

    led = create(_17, OUT_PIN);
	led2 = create(_27, OUT_PIN);
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

	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = 5000000;
	
    while(1)
    {	  
	    ledcontrol(&btnstatus, &ledstatus, &trigger_flag, button, led);
	    ledcontrol(&btnstatus2, &ledstatus2, &trigger_flag2, button2, led2);
    	nanosleep(&t, NULL);
	}
    return 0;
}
