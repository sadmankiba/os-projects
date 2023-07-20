#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>	/* timer */
#include <linux/mutex.h>

MODULE_DESCRIPTION("timer_test module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

struct timer_list my_timer;
unsigned int timeout_s = 2;

DEFINE_MUTEX(my_mutex);

void my_timer_callback(struct timer_list *timer)
{
	pr_debug("%d seconds passed. jiffies: (%ld).", timeout_s, jiffies);

	/* mutex_lock is blocking. So a second timer interrupt will cause a deadlock. */
	// mutex_lock(&my_mutex);
	// pr_debug("mutex locked.");
	
	/* To make periodic */
	mod_timer(&my_timer, jiffies + timeout_s * HZ);
}

static int timer_test_init(void)
{
	pr_debug("timer_test init!");
	timer_setup(&my_timer, my_timer_callback, 0);
	pr_debug("jiffies: (%ld).", jiffies);
	mod_timer(&my_timer, jiffies + timeout_s * HZ);
	return 0;
}

static void timer_test_exit(void)
{
	del_timer_sync(&my_timer);
	mutex_destroy(&my_mutex);
	pr_debug("timer_test exit!");
}

module_init(timer_test_init);
module_exit(timer_test_exit);
