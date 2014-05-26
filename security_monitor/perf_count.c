#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/delay.h>

#include "perf_count.h"

static uint64_t SYS_CALL_TABLE_ADDR = 0xffffffff81801360;

asmlinkage int (*kernel_call_perf_event_open)(struct perf_event_attr *attr_uptr, pid_t pid, int cpu, int group_fd, unsigned long flags);
asmlinkage int (*kernel_call_close)(unsigned int fd);
asmlinkage int (*kernel_call_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);
asmlinkage int (*kernel_call_read)(unsigned int fd, int64_t *buf, size_t count);

void ** kernel_call_sys_call_table;

int perf_count_start(pid_t pid, int *file_p) {

    struct perf_event_attr pe;
    int fd;
    int ret;
    mm_segment_t fs;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 1;
    pe.exclude_kernel = 0;
    pe.exclude_hv = 1;
    pe.inherit = 1;
    pe.inherit_stat = 1;
    pe.enable_on_exec = 1;


    kernel_call_sys_call_table = (void**) SYS_CALL_TABLE_ADDR;
    kernel_call_perf_event_open = kernel_call_sys_call_table[__NR_perf_event_open];
    kernel_call_ioctl = kernel_call_sys_call_table[__NR_ioctl];
    kernel_call_close = kernel_call_sys_call_table[__NR_close];
    kernel_call_read = kernel_call_sys_call_table[__NR_read];


    fs = get_fs();     
    set_fs (get_ds());
    fd = kernel_call_perf_event_open(&pe, pid, -1, -1, 0);
 
    set_fs(fs);

    if (fd == -1) {
        printk("open perf_event errors!\n");
        return -1;
    }

    ret = kernel_call_ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    if (ret == -1) {
        printk("reset counter errors!\n");
        return -1;
    }

    *file_p = fd;

    ret = kernel_call_ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
    if (ret == -1) {
        printk("enable counter errors!\n");
        return -1;
    }

    printk("measuring instruction counter!\n");

    return 0;
}


int perf_count_stop(int *file_p, int64_t *count_p) {
    int fd;
    int ret;
    int64_t count;
    mm_segment_t fs;

    fd = *file_p;

    ret = kernel_call_ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    if (ret == -1) {
        printk("disable counter errors!\n");
        return -1;
    }

    fs = get_fs();     
    set_fs (get_ds());
    ret = kernel_call_read(fd, &count, sizeof(int64_t));
    set_fs(fs);

    if (ret == -1) {
        printk("read counter errors!\n");
        return -1;
    }
    printk("Used %lld instructions\n", count);

    ret = kernel_call_close(fd);
    if (ret == -1) {
        printk("close file errors!\n");
        return -1;
    }

    *count_p = count;

    return 0;
}
