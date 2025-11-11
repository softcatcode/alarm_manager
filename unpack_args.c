#include <stdio.h>
#include <string.h>
#include <time.h>
#include <linux/vmalloc.h>
#include "unpack_args.h"

struct task_args *unpack_args(char args_line[STR_LEN]) {
    struct task_args *result = kmalloc(sizeof(struct task_args), GFP_KERNEL);
    if (result == NULL) {
        perror("vmalloc error");
        return NULL;
    }
    
    const char *delim = ", ";
    char *path = strtok(args_line, delim);
    size_t n = strlen(path);
    if (path == NULL || n >= PATH_LEN) {
        perror("strtok failed or path is too long");
        return NULL;
    }
    memcpy(result->path, path, n + 1);
    
    char date_time[DATE_TIME_LEN] = { 0 };
    const char *date = strtok(NULL, delim);
    if (date == NULL || strlen(date) != 10) {
        perror("strtok failed or date format is incorrect");
        return NULL;
    }
    memcpy(date_time, date, 10);
    date_time[10] = ' ';
    const char *time = strtok(NULL, delim);
    if (time == NULL || strlen(time) != 8) {
        perror("strtok failed or time format is incorrect");
        return NULL;
    }
    memcpy(date_time + 11, time, 8);
        
    struct tm t = { .tm_isdst = -1 };
    if (strptime(date_time, "%Y-%m-%d %H:%M:%S", &t) == NULL) {
        perror("strptime failed");
        return NULL;
    }
    result->t = mktime(&t);
    return result;
}

int main(void) {
    char args_line[STR_LEN] = "path/to/prog.exe 2025-12-01 15:32:14";
    args = unpack_args(args_line);
    printf("%ld\n", args->t);
    puts(args->path);
    return 0;
}
