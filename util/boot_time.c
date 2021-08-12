#include "util/boot_time.h"

#include <time.h>

u64 get_boot_time()
{
  struct timespec current, uptime;
  clock_gettime(CLOCK_REALTIME, &current);
  clock_gettime(CLOCK_MONOTONIC, &uptime);
  u64 boot_time = current.tv_sec * 1000000000uLL + current.tv_nsec - (uptime.tv_sec * 1000000000uLL + uptime.tv_nsec);
  return boot_time;
}
