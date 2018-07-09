// Mario Ríos Muñoz
// Erik Alm


#include "piodirect.h"
#include <time.h>

int main(int argc, char const *argv[])
{
    gpioSetup();

    GPIO led = create(_17, OUT_PIN);

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 500000000;

    char c = 0;
    while(1)
    {
        onOff(led,c%2);
        c++;
        nanosleep(&t, NULL);
        if (c == 20)
            break;         
    }
    destroy(led);

    return 0;
}