#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/workqueue.h> /* work, workqueue */
#include <linux/delay.h> /* msleep */

MODULE_DESCRIPTION("work module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

void my_work_handler(struct work_struct *work);

DECLARE_WORK(my_work, my_work_handler);

struct workqueue_struct *my_wq;
bool use_queue;

/* 
 * A work is only executed once whether it is scheduled or queued.
 * If it is scheduled again, it is executed again. 
 */

static int work_init(void)
{
	pr_debug("work init!");
	use_queue = true;
	if (use_queue) {
		my_wq = create_workqueue("my_wq");
		queue_work(my_wq, &my_work);
	} else 
		schedule_work(&my_work);
	return 0;
}

static void work_exit(void)
{
	if(use_queue) {
		destroy_workqueue(my_wq);
		flush_workqueue(my_wq);
	} else {
		cancel_work_sync(&my_work);
		flush_scheduled_work();
	}

	pr_debug("work exit!");
}

void my_work_handler(struct work_struct *work)
{
	pr_debug("my_work_handler enter.");
	msleep(500);
	if(use_queue) {
		queue_work(my_wq, &my_work);
	} else {
		schedule_work(&my_work);
	}
	pr_debug("my_work_handler exit.");
}

module_init(work_init);
module_exit(work_exit);
