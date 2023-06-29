#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("To-import");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

extern int square_export(int n);

static int to_import_init(void)
{
	pr_debug("To import init!\n");
	pr_debug("squared %d", square_export(14));
	return 0;
}

static void to_import_exit(void)
{
	pr_debug("To import exit!\n");
}

module_init(to_import_init);
module_exit(to_import_exit);
