#include <linux/module.h>

struct test {
    int value;
};

struct test _test;

//int test = 0;
EXPORT_SYMBOL(_test);

static int __init hello_init(void)
{
    printk("test: %d\n", _test.value);
    _test.value ++;
    printk("test: %d\n", _test.value);

    return 0;
}

static void __exit hello_exit(void)
{
    printk("test: %d\n", _test.value);
    _test.value ++;
    printk("test: %d\n", _test.value);
}

module_init(hello_init); 
module_exit(hello_exit);

MODULE_LICENSE("GPL");
