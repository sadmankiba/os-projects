#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("To export");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

int square_export(int n) {
	return n * n;
}
EXPORT_SYMBOL(square_export);

static int to_export_init(void)
{
	pr_debug("To export init!\n");
	return 0;
}

static void to_export_exit(void)
{
	pr_debug("To export exit!\n");
}

module_init(to_export_init);
module_exit(to_export_exit);
