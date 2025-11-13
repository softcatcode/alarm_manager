#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include "args_vfile.h"
#include "common_utils.h"

static bool alarm_triggered;
static bool read_flag;
static char args_buffer[ARGS_LEN];
static struct alarm_args args;
static struct proc_dir_entry *proc_file;
static struct proc_dir_entry *proc_dir;
extern const struct proc_ops fops;

void notify_alarm_triggered(void) {
	printk(KERN_INFO "++ Alarm manager: target time reached!\n");
	if (alarm_triggered)
		return;
	alarm_triggered = true;
	execute_task(args.path);
}

static ssize_t args_write(struct file *f, const char __user *buf, size_t len, loff_t *fpos) {
	if (!alarm_triggered) {
		delay_task_callback();
		alarm_triggered = true;
	}
	
    if (copy_from_user(args_buffer, buf, len) != 0) {
        printk("++ args_write: copy_from user failed\n");
        return -EFAULT;
    }
    args_buffer[len] = 0;
    if (unpack_args(args_buffer, &args) != 0) {
		printk(KERN_INFO "++ args_write: unpack_args failed\n");
		return -1;
	}
	alarm_triggered = schedule_task_callback(args.t) != 0;
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
        sprintf(args_buffer, "arguments are not set\n");
    } else {
        sprintf(args_buffer, "path=\'%s\', time left: %llu\n", args.path, args.t - get_current_time());
    }
    printk(KERN_INFO "++ %s", args_buffer);
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

const struct proc_ops fops = {
    .proc_open = args_open,
    .proc_read = args_read,
    .proc_write = args_write,
    .proc_release = args_release
};

int create_vfiles(void) {
	proc_file = proc_dir = NULL;
	alarm_triggered = true;
	read_flag = false;
	
	if ((proc_dir = proc_mkdir(DIRNAME, NULL)) == NULL) {
        printk(KERN_INFO "++ fortune: create dir err\n");
        delete_vfiles();
        return -EFAULT;
    }
    if ((proc_file = proc_create(FILENAME, 666, proc_dir, &fops)) == NULL) {
        printk(KERN_INFO "++ fortune: create file err\n");
        delete_vfiles();
        return -EFAULT;
    }
    return 0;
}

void delete_vfiles(void) {
	if (proc_file != NULL)
        remove_proc_entry(FILENAME, proc_dir);
    if (proc_dir != NULL)
        remove_proc_entry(DIRNAME, NULL);
    if (!alarm_triggered)
		delay_task_callback();
}
