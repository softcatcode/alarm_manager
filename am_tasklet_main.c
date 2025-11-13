#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <asm/io.h>
#include <linux/rtc.h>
#include "args_vfile.h"
#include "common_utils.h"

#define RTC_IRQ 8

static struct tasklet_struct alarm_tasklet;
static bool should_schedule = false;

void tasklet_handler(unsigned long t)
{
	unsigned long now = get_current_time();
	if (now >= t) {
		notify_alarm_triggered();
		delay_task_callback();
	}
}

irqreturn_t rtc_irq_handler(int irq, void *dev)
{
    if (irq == RTC_IRQ)
    {
		if (should_schedule) {
			tasklet_schedule(&alarm_tasklet);
		}
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static void rtc_enable_periodic(void) {
	char reg;
	outb(0x0B, 0x70);
	reg = inb(0x71);
	reg |= (char) 0x40;
	outb(0x0B, 0x70);
	outb(reg, 0x71);
}

int schedule_task_callback(time64_t t) {
	alarm_tasklet.data = (unsigned long) t;
	should_schedule = true;
	return 0;
}

void delay_task_callback(void) {
	should_schedule = false;
}

static int __init alarm_manager_init(void)
{
	tasklet_init(&alarm_tasklet, tasklet_handler, 0UL);
	rtc_enable_periodic();
	if (request_irq(RTC_IRQ, rtc_irq_handler, IRQF_SHARED, "alarm_tasklet", &rtc_irq_handler) != 0)
    {
        printk(KERN_INFO "++ alarm_manager_init: request_irq failed\n");
        return -1;
    }
	if (create_vfiles() != 0) {
		printk(KERN_ERR "++ alarm_manager_init: create_vfiles failed\n");
		return -1;
	}
    printk(KERN_INFO "++ am_manager_init: module loaded\n");
    return 0;
}

static void __exit alarm_manager_exit(void)
{
	tasklet_kill(&alarm_tasklet);
	free_irq(RTC_IRQ, &rtc_irq_handler);
    delete_vfiles();
    printk(KERN_INFO "++ Alarm: module is unloaded.\n");
}

module_init(alarm_manager_init);
module_exit(alarm_manager_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniil Machilsky");
