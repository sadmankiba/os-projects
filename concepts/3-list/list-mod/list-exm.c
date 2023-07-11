#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("List example");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

/* Example of list, atomic and rwlock */

struct port_rec {
    struct list_head list;
    int num;
    unsigned long timestamp;
    atomic_t count;
};

struct port_rec * search_port(int num);

/*
struct list_head my_list;
INIT_LIST_HEAD(my_list);
Or,
LIST_HEAD(my_list);
*/
LIST_HEAD(my_list); 

DEFINE_RWLOCK(lock);

int add_port(int num) {
    struct port_rec *pr;
    if((pr = search_port(num)) != NULL) {
        write_lock(&lock);
        atomic_inc(&pr->count);
        write_unlock(&lock);
        return 0;
    }
    pr = kzalloc(sizeof(struct port_rec), GFP_KERNEL);
    pr->num = num;
    pr->timestamp = jiffies;
    atomic_set(&pr->count, 1);
    write_lock(&lock);
    list_add(&pr->list, &my_list);
    write_unlock(&lock);
    return 0;
}

int delete_port(int num) {
    struct list_head *temp;
    write_lock(&lock);
    list_for_each(temp, &my_list) {
        struct port_rec *pr = list_entry(temp, struct port_rec, list);
        if (pr->num == num) {
            list_del(temp);
            kfree(pr);
            write_unlock(&lock);
            return 0;
        }
    }
    write_unlock(&lock);
    return -EINVAL; 
}

void delete_all(void) {
    struct list_head *p, *q;
    struct port_rec *pr;

    write_lock(&lock);
    list_for_each_safe(p, q, &my_list) {
        pr = list_entry(p, struct port_rec, list);
        list_del(p);
        kfree(pr);
    }
    write_unlock(&lock);
}

struct port_rec * search_port(num) {
    struct port_rec *pr;
    read_lock(&lock);
    list_for_each_entry(pr, &my_list, list) {
        if(pr->num == num) {
            read_unlock(&lock);
            return pr;
        }
    }
    read_unlock(&lock);
    return NULL;
}

void print_list(void) {
    struct port_rec *pr;
    pr_debug("List: ");
    
    read_lock(&lock);
    list_for_each_entry(pr, &my_list, list) {
        pr_debug("num = %d, timestamp = %lu, count = %d", pr->num, pr->timestamp, atomic_read(&pr->count));
    }
    read_unlock(&lock);
}

static int listex_init(void)
{
	pr_debug("init listex");
    add_port(4000);
    add_port(3000);
    add_port(8000);
    add_port(4000);
    add_port(8000);
    print_list();
	return 0;
}

static void listex_exit(void)
{
    delete_port(3000);
    print_list();
    delete_all();
    print_list();
	pr_debug("exit listex");
}

module_init(listex_init);
module_exit(listex_exit);