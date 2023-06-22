#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>

MODULE_DESCRIPTION("Locking example module");
MODULE_AUTHOR("Sadman Sakib");
MODULE_LICENSE("GPL");

static DEFINE_SPINLOCK(my_lock);
static int count;
struct task_struct *thread1, *thread2;

int incr(void *arg)
{
    int i;
    spin_lock(&my_lock);
    /* critical section */
    for (i = 0; i < 1000000; i++)
        count++;
    spin_unlock(&my_lock);
    return 0;
}

static int locking_init(void)
{
	pr_debug("Hello!\n");
    
    thread1 = kthread_create(incr, NULL, "thread1");
    thread2 = kthread_create(incr, NULL, "thread2");
    
    wake_up_process(thread1);
    wake_up_process(thread2);
    return 0;
}

static void locking_exit(void)
{
	kthread_stop(thread1);
    kthread_stop(thread2);
    pr_debug("count = %d\n", count);
    pr_debug("Goodbye!\n");
}

module_init(locking_init);
module_exit(locking_exit);
