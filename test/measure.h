#include <sys/time.h>
#include <string.h>
#include <sys/time.h>

void start_timer (struct timeval *tv1)
{
  struct timezone tz;
  gettimeofday(tv1, &tz);
}

long long stop_timer (struct timeval *tv1)
{
  struct timeval tv2;
  struct timezone tz;
  gettimeofday(&tv2, &tz);
  return ((tv2.tv_sec-tv1->tv_sec) * 1000000L + \
           (tv2.tv_usec-tv1->tv_usec));  
}
