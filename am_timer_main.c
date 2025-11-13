#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/hrtimer.h>
#include "args_vfile.h"
#include "common_utils.h"

static struct hrtimer htimer;

static enum hrtimer_restart timer_function(struct hrtimer * timer) {
    notify_alarm_triggered();
    return HRTIMER_NORESTART;
}

int schedule_task_callback(time64_t t) {
	time64_t delay, now;
	static ktime_t kt_periode;
	
    now = get_current_time();
    if (now >= t) {
        printk(KERN_INFO "++ alarm_thread_func: attempt to plan on a past time\n");
        return -1;
    }
    delay = args.t - now;
    
    kt_periode = ktime_set(delay, 0);
	hrtimer_init(&htimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
	htimer.function = timer_function;
	printk(KERN_INFO "++ Alarm will be triggered after %llu\n", delay);
	hrtimer_start(&htimer, kt_periode, HRTIMER_MODE_REL);
	return 0;
}

void delay_task_callback(void) {
	hrtimer_cancel(&htimer);
}

static int __init alarm_manager_init(void)
{
	if (create_vfiles() != 0) {
		printk(KERN_ERR "++ alarm_manager_init: create_vfiles failed\n");
		return -1;
	}
    printk(KERN_INFO "++ Alarm manager is installed!\n");
    return 0;
}

static void __exit alarm_manager_exit(void)
{
    delete_vfiles();
    printk(KERN_INFO "++ Alarm manager module is unloaded.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniil Machilsky");
MODULE_DESCRIPTION("Alarm manager implemented by sleep syscall.");
MODULE_VERSION("0.1");
module_init(alarm_manager_init);
module_exit(alarm_manager_exit);
