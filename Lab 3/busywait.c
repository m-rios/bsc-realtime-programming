#include "busywait.h"

void busywait(long wait)
{
	struct timespec it;
	struct timespec ct;
	
	clock_gettime(CLOCK_REALTIME, &it);
	clock_gettime(CLOCK_REALTIME, &ct);
	while (ct.tv_nsec < it.tv_nsec + wait && ct.tv_sec == it.tv_sec)
	{
		clock_gettime(CLOCK_REALTIME, &ct);
	}
}