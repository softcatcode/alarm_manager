#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/kmod.h>
#include <linux/ktime.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include "unpack_args.h"

#define FILENAME "amFile"
#define DIRNAME  "amDir"
#define ARGS_BUF_SIZE 128

static bool alarm_triggered;
static char args_buffer[ARGS_BUF_SIZE];
static struct task_args *args = NULL;
static struct task_struct *alarm_thread = NULL;
static struct proc_dir_entry *proc_file;
static struct proc_dir_entry *proc_dir;
static bool read_flag = false;

void exec_task(void) {
    printk(KERN_INFO "++ Alarm manager: target time reached!\n");
    alarm_triggered = true;
}

time64_t get_current_time(void) {
	return ktime_get_real_seconds() + 3600 * 3;
}

static int alarm_thread_func(void *data) {
	time64_t delay, now;
	
    now = get_current_time();
    if (now >= args->t) {
        printk(KERN_INFO "++ alarm_thread_func: attempt to plan on a past time\n");
        return -EFAULT;
    }
    delay = args->t - now;
    printk(KERN_INFO "++ Alarm will be triggered after %llu\n", delay);
    alarm_triggered = false;
    mdelay(delay * 1000);
    exec_task();
    return 0;
}

static ssize_t args_write(struct file *f, const char __user *buf, size_t len, loff_t *fpos) {
    if (!alarm_triggered) {
		kthread_stop(alarm_thread);
		printk(KERN_INFO "++ previous task canceled: %s\n", args->path);
		kfree(args);
	}
    if (copy_from_user(args_buffer, buf, len) != 0) {
        printk("++ args_write: copy_from user failed\n");
        return -EFAULT;
    }
    args_buffer[len] = 0;
    args = unpack_args(args_buffer);
    alarm_thread = kthread_run(alarm_thread_func, NULL, "alarm_manager_thread");
    if (IS_ERR(alarm_thread)) {
        printk(KERN_INFO "++ args_write: failed to create kernel thread\n");
        alarm_thread = NULL;
        return len;
    }
    return len;
}

static ssize_t args_read(struct file *f, char __user *buf, size_t len, loff_t *fpos) {
	size_t n;
	
	if (read_flag) {
		read_flag = false;
		return 0;
	}
	read_flag = true;
	
    if (alarm_triggered) {
        printk(KERN_INFO "++ args_read: arguments are not set\n");
        sprintf(args_buffer, "arguments are not set");
    } else {
        sprintf(args_buffer, "path=\'%s\', time left: %llu\n", args->path, args->t - get_current_time());
        printk(KERN_INFO "++ %s", args_buffer);
    }
    n = strlen(args_buffer);
    if (copy_to_user(buf, args_buffer, n) != 0) {
        printk(KERN_INFO "++ args_read: copy_to_user failed");
        return -EFAULT;
    }
    
    return n;
}

static int args_release(struct inode *ind, struct file *f) {
    printk(KERN_INFO "++ args_release called\n");
    return 0;
}

static int args_open(struct inode *ind, struct file *f) {
    printk(KERN_INFO "++ args_open called\n");
    return 0;
}

void free_resources(void) {
    printk(KERN_INFO "++ free_resources: cleaning alarm args\n");
    if (args != NULL) {
        kfree(args);
        args = NULL;
    }
    printk(KERN_INFO "++ free_resources: cleaning proc file system\n");
    if (proc_file != NULL)
        remove_proc_entry(FILENAME, proc_dir);
    if (proc_dir != NULL)
        remove_proc_entry(DIRNAME, NULL);
    printk(KERN_INFO "++ free_resources: cleaning successful\n");
}

static const struct proc_ops fops =
{
    .proc_open = args_open,
    .proc_read = args_read,
    .proc_write = args_write,
    .proc_release = args_release
};

static int __init alarm_manager_init(void)
{
    if ((proc_dir = proc_mkdir(DIRNAME, NULL)) == NULL) {
        printk(KERN_INFO "++ fortune: create dir err\n");
        free_resources();
        return -EFAULT;
    }
    if ((proc_file = proc_create(FILENAME, 666, proc_dir, &fops)) == NULL) {
        printk(KERN_INFO "++ fortune: create file err\n");
        free_resources();
        return -EFAULT;
    }
    printk(KERN_INFO "++ Alarm manager is installed!\n");
    alarm_triggered = true;
    return 0;
}

static void __exit alarm_manager_exit(void)
{
    printk(KERN_INFO "++ RTC Alarm: module unloaded.\n");
    free_resources();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniil Machilsky");
MODULE_DESCRIPTION("Alarm manager implemented by sleep syscall.");
MODULE_VERSION("0.1");
module_init(alarm_manager_init);
module_exit(alarm_manager_exit);
