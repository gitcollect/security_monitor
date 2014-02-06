#include <linux/module.h>

struct test {
    int value;
};

extern struct test _test;

static int __init hello1_init(void)
{
    printk("test: %d\n", _test.value);
    _test.value ++;
    printk("test: %d\n", _test.value);

    return 0;
}

static void __exit hello1_exit(void)
{
    printk("test: %d\n", _test.value);
    _test.value ++;
    printk("test: %d\n", _test.value);
}

module_init(hello1_init); 
module_exit(hello1_exit);

MODULE_LICENSE("GPL");
