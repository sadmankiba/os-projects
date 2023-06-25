#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/sched.h>    /* task_struct */
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_DESCRIPTION("Locking example module");
MODULE_AUTHOR("Sadman Sakib");
MODULE_LICENSE("GPL");

// static DEFINE_SPINLOCK(my_lock);
static int count = 0;
struct task_struct *thread1, *thread2;

int incr(void *arg)
{
    int i;
    // spin_lock(&my_lock);
    /* critical section */
    for (i = 0; i < 10000; i++) {
        count++;
        // msleep(1);
    }
    // spin_unlock(&my_lock);
    return 0;
}

static int locking_init(void)
{
	pr_debug("Hello!\n");
    // spin_lock_init(&my_lock);
    
    thread1 = kthread_create(incr, NULL, "thread1");
    if (thread1) {
        wake_up_process(thread1);
        pr_debug("Thread 1 is running now");    
    } else {
        pr_debug("Thread 1 did not run");
        return -1;
    }
    
    thread2 = kthread_run(incr, NULL, "thread2");
    if (thread2) {
        pr_debug("Thread 2 is running now");    
    } else {
        kthread_stop(thread2);
        pr_debug("Thread 2 did not run");
        return -1;
    }

    return 0;
}

static void locking_exit(void)
{
	// kthread_stop(thread1);
    // kthread_stop(thread2);
    pr_debug("count = %d\n", count);
    pr_debug("Goodbye!\n");
}

module_init(locking_init);
module_exit(locking_exit);
