#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/delay.h>

int perf_count_start(pid_t pid, int *file_p);

int perf_count_stop(int *file_p, uint64_t *count_p);
