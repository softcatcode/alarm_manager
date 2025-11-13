#ifndef ARGS_VFILE_H
#define ARGS_VFILE_H

#define FILENAME "amFile"
#define DIRNAME  "amDir"

#include "args_parser.h"
#include <linux/ktime.h>

extern int schedule_task_callback(time64_t t);

extern void delay_task_callback(void);

void notify_alarm_triggered(void);

int create_vfiles(void);

void delete_vfiles(void);

#endif
