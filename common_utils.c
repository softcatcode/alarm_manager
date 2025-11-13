#include "common_utils.h"
#include <linux/kernel.h>

void execute_task(const char *path) {
	printk(KERN_INFO "++ program \'%s\' is launched\n", path);
}

time64_t get_current_time(void) {
	return ktime_get_real_seconds() + 3600 * 3;
}
