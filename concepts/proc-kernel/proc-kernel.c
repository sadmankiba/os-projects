#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>        /* task_struct */
#include <linux/sched/signal.h> /* for_each_process */


MODULE_DESCRIPTION("Kernel process");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

static int proc_kernel_init(void)
{
	struct task_struct *p, *nxt;

    pr_info("Current process: pid = %u, cmd = %s, utime = %llu", current->pid, current->comm, current->utime);
    pr_info("Parent process: pid = %u, cmd = %s, utime = %llu", current->parent->pid, current->parent->comm, current->parent->utime);
    
    nxt = next_task(current);
    if (nxt)
        pr_info("Next process: pid = %u, cmd = %s, utime = %llu", nxt->pid, nxt->comm, nxt->utime);

    pr_info("Process list");
    for_each_process(p) {
        pr_info("pid = %d, cmd = %s", p->pid, p->comm);
    }
    return 0;
}

static void proc_kernel_exit(void)
{
	pr_info("Current process: pid = %d, cmd = %s", current->pid, current->comm);
}

module_init(proc_kernel_init);
module_exit(proc_kernel_exit);
