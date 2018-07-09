#include <pthread.h>
#include <signal.h>
#include "piodirect.h"
#include "busywait.h"

GPIO led;

void sigterm_han(int signum)
{
	if (signum == SIGINT)
	{
		destroy(led);
		printf("Exited program. \n");
	}
	exit(signum);  
}

int main()
{
	gpioSetup();
	signal(SIGINT, sigterm_han);
	led = create(_17, OUT_PIN);
	while(1)
	{
		onOff(led, 1);
		busywait(1000000);
		
		onOff(led, 0);
		busywait(3000000);
	}

return 0;
}
