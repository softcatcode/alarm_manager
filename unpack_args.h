#ifndef UNPACK_ARGS_H
#define UNPACK_ARGS_H

#define ARGS_LEN       256
#define PATH_LEN       128
#define DATE_TIME_LEN  20  // "YYYY-MM-DD HH:MM:SS" + '\0'

struct alarm_args {
    char path[PATH_LEN];
    time64_t t;
};

int unpack_args(char *args_line, struct alarm_args *result);

#endif
