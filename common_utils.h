#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <linux/ktime.h>

void execute_task(const char *path);

time64_t get_current_time(void);

#endif
