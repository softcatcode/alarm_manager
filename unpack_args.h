#ifndef UNPACK_ARGS_H
#define UNPACK_ARGS_H

#define STR_LEN 100
#define PATH_LEN 50
#define DATE_TIME_LEN 20

struct task_args {
    char path[PATH_LEN];
    time_t t;
} *args;

struct task_args *unpack_args(char args_line[STR_LEN]);

#endif
