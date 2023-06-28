#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("List example");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

struct port_rec {
    struct list_head list;
    int num;
};

/*
struct list_head my_list;
INIT_LIST_HEAD(my_list);
Or,
LIST_HEAD(my_list);
*/
LIST_HEAD(my_list); 

int add_port(int num) {
    struct port_rec *pr = kzalloc(sizeof(struct port_rec), GFP_KERNEL);
    pr->num = num;
    list_add(&pr->list, &my_list);
    return 0;
}

int delete_port(int num) {
    struct list_head *temp;
    list_for_each(temp, &my_list) {
        struct port_rec *pr = list_entry(temp, struct port_rec, list);
        if (pr->num == num) {
            list_del(temp);
            kfree(pr);
            return 0;
        }
    }
    return -EINVAL; 
}

void delete_all(void) {
    struct list_head *p, *q;
    struct port_rec *pr;
    list_for_each_safe(p, q, &my_list) {
        pr = list_entry(p, struct port_rec, list);
        list_del(p);
        kfree(pr);
    }
}

void print_list(void) {
    struct port_rec *pr;
    pr_debug("List: ");
    list_for_each_entry(pr, &my_list, list) {
        pr_debug("%d ", pr->num);
    }
}

static int listex_init(void)
{
	pr_debug("init listex");
    add_port(4000);
    add_port(3000);
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
