#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include "sl_core_log.h"

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

struct sl_core_link {
	unsigned int num;
};

static int log_prac_init(void)
{
	struct sl_core_link link;
	char *state_str = "UP";
	pr_debug("log_prac init!\n");
	sl_core_log_dbg(&link, SL_CORE_LINK_AN_LOG_NAME, "lp caps get");
	sl_core_log_err(&link, SL_CORE_LINK_AN_LOG_NAME,
			"lp caps get - invalid (state = %s)", state_str);
	return 0;
}

static void log_prac_exit(void)
{
	pr_info("log_prac exit!\n");
}

module_init(log_prac_init);
module_exit(log_prac_exit);
