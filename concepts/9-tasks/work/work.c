#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
// #include <linux/workqueue.h>

MODULE_DESCRIPTION("work module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

void my_work_handler(struct work_struct *work);

// struct work_struct my_work;
// INIT_WORK(&my_work, my_work_handler);

DECLARE_WORK(my_work, my_work_handler);

static int work_init(void)
{
	pr_debug("work init!");
	schedule_work(&my_work);
	return 0;
}

static void work_exit(void)
{
	flush_scheduled_work();
	cancel_work_sync(&my_work);
	pr_debug("work exit!");
}

void my_work_handler(struct work_struct *work)
{
	pr_debug("my_work_handler.");
}

module_init(work_init);
module_exit(work_exit);
