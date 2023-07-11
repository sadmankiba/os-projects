#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h> /* kzalloc */
#include <linux/ioport.h> /* request_region, release_region */
#include <linux/interrupt.h> /* request_irq, free_irq, irqreturn_t */

MODULE_DESCRIPTION("kbd_io module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

#define IOP_START 0x61
#define IOP_LEN 1
#define IRQ_NUM 13

struct devio_info {
	int data;
};

struct devio_info *dioinf;

irqreturn_t kbd_intr_handler(int irq, void *dev_id)
{
	struct devio_info *info = (struct devio_info *) dev_id;
	pr_debug("kbd_intr_handler called!\n");
	return IRQ_HANDLED;
}

static int kbd_io_init(void)
{
	int err; 

	pr_debug("kbd_io init!\n");
	
	dioinf = (struct devio_info *) kzalloc(sizeof(*dioinf), GFP_KERNEL);
	
	request_region(IOP_START, IOP_LEN, "kbd_io");
	err = request_irq(IRQ_NUM, kbd_intr_handler, IRQF_SHARED, "kbd_intr", dioinf);
	if(err) {
		pr_debug("request_irq failed!\n");
		return err;
	} else
		pr_debug("request_irq succeeded!\n");

	return 0;
}

static void kbd_io_exit(void)
{
	release_region(IOP_START, 1);
	free_irq(IRQ_NUM, dioinf);
	pr_debug("kbd_io exit!\n");
}

module_init(kbd_io_init);
module_exit(kbd_io_exit);
