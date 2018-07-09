// Mario Ríos Muñoz
// Erik Alm

#include "piodirect.h"
#include <time.h>
#include <signal.h>
// #include <unistd.h>

GPIO led;
struct timespec tr;

void sigterm_han(int signum)
{
    if (signum == SIGINT)
    {
       destroy(led);
       printf("time remaining: %ld\n", tr.tv_nsec);
    }
    exit(signum);  
}


int main(int argc, char const *argv[])
{
    gpioSetup();

    led = create(_17, OUT_PIN);

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 500000000;

    signal(SIGINT, sigterm_han);  

    char c = 0;
    while(1)
    {
        onOff(led,c%2);
        c++;
        nanosleep(&t, &tr);
        if (c == 20)
            break;         
    }
    destroy(led);

    return 0;
}