#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/slab.h>


static int __init gpio_driver_init(void)
{
	printk("Welcome to my driver!\n");

	return 0;
}

static void __exit gpio_driver_exit(void)
{
	printk("Leaving my driver!\n");
	return;
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("1902765@uad.ac.uk");
MODULE_DESCRIPTION("Driver for control over the mini Kiosk");
MODULE_VERSION("1.0");