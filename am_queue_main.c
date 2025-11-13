#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <asm/io.h>
#include <linux/rtc.h>
#include "args_vfile.h"
#include "common_utils.h"

#define RTC_IRQ 8

struct alarm_work_data {
    struct work_struct work;
    time64_t t;
};

static struct workqueue_struct *alarm_wq;
static struct alarm_work_data alarm_work;
static bool should_schedule;

void work_handler(struct work_struct *work)
{
    struct alarm_work_data *data = container_of(work, struct alarm_work_data, work);
    unsigned long now = get_current_time();
    if (now >= data->t) {
        notify_alarm_triggered();
        delay_task_callback();
    }
}

irqreturn_t rtc_irq_handler(int irq, void *dev)
{
    if (irq == RTC_IRQ)
    {
		if (should_schedule)
			queue_work(alarm_wq, &alarm_work.work);
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
    alarm_work.t = t;
    INIT_WORK(&alarm_work.work, work_handler);
    should_schedule = true;
    return 0;
}

void delay_task_callback(void) {
	should_schedule = false;
}

static int __init alarm_manager_init(void)
{
	rtc_enable_periodic();
    if (request_irq(RTC_IRQ, rtc_irq_handler, IRQF_SHARED, "alarm_work", &rtc_irq_handler) != 0)
    {
        printk(KERN_INFO "++ alarm_manager_init: request_irq failed\n");
        destroy_workqueue(alarm_wq);
        return -EIO;
    }
	alarm_wq = alloc_workqueue("alarm_work_queue", WQ_FREEZABLE | WQ_UNBOUND, 1);
    if (!alarm_wq) {
        printk(KERN_ERR "++ alarm_manager_init: failed to create workqueue\n");
        return -ENOMEM;
    }
    if (create_vfiles() != 0) {
        printk(KERN_ERR "++ alarm_manager_init: create_vfiles failed\n");
        free_irq(RTC_IRQ, NULL);
        destroy_workqueue(alarm_wq);
        return -EFAULT;
    }
    printk(KERN_INFO "++ Alarm manager module is REMOVED\n");
    return 0;
}

static void __exit alarm_manager_exit(void)
{
    cancel_work_sync(&alarm_work.work);
    free_irq(RTC_IRQ, &rtc_irq_handler);
    destroy_workqueue(alarm_wq);
    delete_vfiles();
    printk(KERN_INFO "++ Alarm manager module is LOADED.\n");
}

module_init(alarm_manager_init);
module_exit(alarm_manager_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniil Machilsky");
