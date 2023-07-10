#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ioport.h> /* request_region, release_region */

MODULE_DESCRIPTION("kbd_io module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

#define IOP_START 0x61

static int kbd_io_init(void)
{
	pr_debug("kbd_io init!\n");
	request_region(IOP_START, 1, "kbd_io");
	return 0;
}

static void kbd_io_exit(void)
{
	release_region(IOP_START, 1);
	pr_debug("kbd_io exit!\n");
}

module_init(kbd_io_init);
module_exit(kbd_io_exit);
