#ifndef UNPACK_ARGS_H
#define UNPACK_ARGS_H

#define ARGS_LEN       256
#define PATH_LEN       128
#define DATE_TIME_LEN  20  // "YYYY-MM-DD HH:MM:SS" + '\0'

struct task_args {
    char path[PATH_LEN];
    time64_t t;
};

struct task_args *unpack_args(char *args_line);

#endif
