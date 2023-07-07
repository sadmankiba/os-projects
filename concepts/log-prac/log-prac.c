#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include "mod_log.h"

MODULE_DESCRIPTION("log_prac module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

struct mod_comp {
	unsigned int num;
};

static int log_prac_init(void)
{
	struct mod_comp comp;
	char *state_str = "UP";
	pr_debug("log_prac init!\n");
	mod_log_dbg(&comp, MOD_PART1_NAME, "lp caps get");
	mod_log_err(&comp, MOD_PART2_NAME,
			"lp caps get - invalid (state = %s)", state_str);
	return 0;
}

static void log_prac_exit(void)
{
	pr_info("log_prac exit!\n");
}

module_init(log_prac_init);
module_exit(log_prac_exit);
