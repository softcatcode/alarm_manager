#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/errno.h>
#include "args_parser.h"

static int parse_date(const char *date_str, struct tm *tm)
{
	char year_str[5] = { date_str[0], date_str[1], date_str[2], date_str[3], '\0' };
    char mon_str[3]  = { date_str[5], date_str[6], '\0' };
    char day_str[3]  = { date_str[8], date_str[9], '\0' };
	
    if (strlen(date_str) != 10)
        return -EINVAL;

    // Формат: YYYY-MM-DD
    if (date_str[4] != '-' || date_str[7] != '-')
        return -EINVAL;

    if (kstrtouint(year_str, 10, (unsigned int *)&tm->tm_year) != 0 ||
        kstrtouint(mon_str, 10, (unsigned int *)&tm->tm_mon) != 0 ||
        kstrtouint(day_str, 10, (unsigned int *)&tm->tm_mday) != 0)
        return -EINVAL;

    return 0;
}

static int parse_time(const char *s, struct tm *tm)
{
	// s == "HH:MM:SS"
    tm->tm_hour = (s[0] - '0') * 10 + s[1] - '0';
    tm->tm_min = (s[3] - '0') * 10 + s[4] - '0';
    tm->tm_sec = (s[6] - '0') * 10 + s[7] - '0';
    return 0;
}

static void split_to_arguments(char *arguments, char **path, char **date ,char **time)
{
	size_t n;
	size_t i;
	
	*path = *date = *time = NULL;
    if (arguments == NULL)
		return;
	
	// Вычисление длины строки, содержащей аргументы.
	n = 0;
	while (n < ARGS_LEN && arguments[n] != 0)
		n++;
	if (n > ARGS_LEN || (n == ARGS_LEN && arguments[n - 1] != 0))
		return;
	
	// Все пробельные символы заменяются нулями.
	for (i = 0; i < n; i++)
		if (arguments[i] == ' ' || arguments[i] == '\t')
			arguments[i] = 0;
	
	// Осталось найти положение аргументов в строке arguments.
    for (; n > 0 && *arguments == 0; n--)
		arguments++;
	if (n == 0)
		return;
	*path = arguments;
	for (; *arguments != 0; n--)
		arguments++;
	for (; n > 0 && *arguments == 0; n--)
		arguments++;
	if (n == 0)
		return;
	*date = arguments;
	for (; *arguments != 0; n--)
		arguments++;
	for (; n > 0 && *arguments == 0; n--)
		arguments++;
	if (n == 0)
		return;
	*time = arguments;
}

int unpack_args(char *args_line, struct alarm_args *result)
{
	size_t path_len;
	char *path_arg;
	char *date_arg;
	char *time_arg;
	struct tm tm;
	
    if (!args_line)
        return 1;

    split_to_arguments(args_line, &path_arg, &date_arg, &time_arg);
    if (path_arg == NULL || date_arg == NULL || time_arg == NULL) {
		printk(KERN_INFO "++ unpack_args: arguments format is incorrect\n");
		return 1;
	}
	
	path_len = strnlen(path_arg, PATH_LEN);
	memcpy(result->path, path_arg, path_len);
	result->path[path_len] = 0;
    
    memset(&tm, 0, sizeof(struct tm));
    if (parse_date(date_arg, &tm) != 0) {
        printk(KERN_INFO "++ unpack_args: date format is incorrect\n");
        return 1;
    }
    if (parse_time(time_arg, &tm) != 0) {
        printk(KERN_INFO "++ unpack_args: invalid time format (expected HH:MM:SS)\n");
        return 1;
    }
    
    result->t = mktime64(tm.tm_year, tm.tm_mon, tm.tm_mday,
                         tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (result->t < 0) {
        printk(KERN_INFO "++ unpack_args: mktime64 failed (invalid date/time)\n");
        return 1;
    }

    return 0;
}
