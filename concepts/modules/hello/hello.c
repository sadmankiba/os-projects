#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

static int hello_init(void)
{
	pr_debug("Hello!\n");
	return 0;
}

static void hello_exit(void)
{
	pr_debug("Goodbye!\n");
}

module_init(hello_init);
module_exit(hello_exit);
