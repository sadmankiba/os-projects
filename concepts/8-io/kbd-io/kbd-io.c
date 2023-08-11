#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h> /* register, file, inode, file_operations, MKDEV */
#include <linux/cdev.h> /* struct cdev, cdev_* */
#include <linux/slab.h> /* kzalloc */
#include <linux/ioport.h> /* request_region, release_region */
#include <linux/interrupt.h> /* request_irq, free_irq, irqreturn_t */

MODULE_DESCRIPTION("kbd_io module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

#define KBD_DEV_MAJOR 174
#define KBD_DEV_MINOR 2
#define KBD_DEV_NR_DEVS 1
#define KBD_DEV_NAME "kbd_dev"

#define IOP_START 0x61
#define IOP_LEN 1
#define KBD_IRQ_NUM 1
#define CURSOR_IRQ_NUM 38
#define UNK_IRQ_NUM 65
#define IRQ_NUM UNK_IRQ_NUM

#define DATA_BUF_SIZE 100

struct devio_info {
	char data[DATA_BUF_SIZE];
	struct cdev cdev;
	unsigned short cur_pos;
	unsigned short len;
};

struct devio_info *dioinf;

irqreturn_t intr_handler(int irq, void *dev_id)
{
	struct devio_info *info = (struct devio_info *) dev_id;
	int i;
	unsigned int addr = 0x1026;

	pr_debug("intr_handler called for IRQ %d!", irq);
	
	for (i = 0; i < 10; i++) {
		pr_debug("%x: %u", addr, inb(addr));
		addr += 16;
	}
	return IRQ_NONE;
}

int kbd_open(struct inode *ind, struct file *f)
{
	struct devio_info *dioinf;
	pr_debug(KBD_DEV_NAME " opened!\n");
	dioinf = container_of(ind->i_cdev, struct devio_info, cdev);
	f->private_data = dioinf;
	return 0;
}

ssize_t kbd_read(struct file *f, char *buf, size_t size, loff_t *offset)
{
	struct devio_info *dioinf;
	int err;

	pr_debug(KBD_DEV_NAME " read!\n");
	dioinf = (struct devio_info *) f->private_data;

	while(dioinf->len > 0) {
		err = copy_to_user(buf, &dioinf->data[dioinf->cur_pos], 1);
		if(err) {
			pr_debug("copy_to_user failed!\n");
			return -EFAULT;
		}
		dioinf->cur_pos = (dioinf->cur_pos + 1) % DATA_BUF_SIZE;
		dioinf->len--;
		return 1;
	}
	
	return 0;
}

ssize_t kbd_write(struct file *f, const char *buf, size_t size, loff_t *offset)
{
	struct devio_info *dioinf;
	int err;
	int rem;

	pr_debug(KBD_DEV_NAME " write!\n");
	dioinf = (struct devio_info *) f->private_data;
	rem = size;

	while(rem > 0) {
		err = copy_from_user(&dioinf->data[dioinf->cur_pos], buf, 1);
		if(err) {
			pr_debug("copy_from_user failed!\n");
			return -EFAULT;
		}
		dioinf->cur_pos = (dioinf->cur_pos + 1) % DATA_BUF_SIZE;
		dioinf->len++;
		rem--;
	}
	
	return size;
}


int kbd_release(struct inode *ind, struct file *f)
{
	pr_debug(KBD_DEV_NAME " released!\n");
	return 0;
}

static const struct file_operations kbd_fops = {
	.owner = THIS_MODULE,
	.open = kbd_open,
	.release = kbd_release,
	.read = kbd_read,
	.write = kbd_write,
};

static int kbd_io_init(void)
{
	int err; 

	pr_debug("kbd_io init!\n");

	err = register_chrdev_region(MKDEV(KBD_DEV_MAJOR, KBD_DEV_MINOR), 
		KBD_DEV_NR_DEVS, KBD_DEV_NAME);
	if(err) {
		pr_debug(KBD_DEV_NAME " register failed!");
		return err;
	}
	pr_debug(KBD_DEV_NAME " registered!");
	dioinf = (struct devio_info *) kzalloc(sizeof(*dioinf), GFP_KERNEL);
	dioinf->cur_pos = 0;
	dioinf->len = 0;

	cdev_init(&dioinf->cdev, &kbd_fops);
	cdev_add(&dioinf->cdev, MKDEV(KBD_DEV_MAJOR, KBD_DEV_MINOR), KBD_DEV_NR_DEVS);
	
	request_region(IOP_START, IOP_LEN, "kbd_io");
	pr_debug("I/O port %d requested!", IOP_START);
	err = request_irq(IRQ_NUM, intr_handler, IRQF_SHARED, "kbd_intr", dioinf);
	if(err) {
		pr_debug("IRQ %d register failed!", IRQ_NUM);
		return err;
	} else
		pr_debug("IRQ %d register succeeded!", IRQ_NUM);

	return 0;
}

static void kbd_io_exit(void)
{
	release_region(IOP_START, 1);
	pr_debug("I/O port %d released!", IOP_START);
	free_irq(IRQ_NUM, dioinf);
	pr_debug("IRQ %d freed!", IRQ_NUM);
	
	cdev_del(&dioinf->cdev);
	unregister_chrdev_region(MKDEV(KBD_DEV_MAJOR, KBD_DEV_MINOR), KBD_DEV_NR_DEVS);
	pr_debug(KBD_DEV_NAME " unregistered!");

	kfree(dioinf);

	pr_debug("kbd_io exit!");
}

module_init(kbd_io_init);
module_exit(kbd_io_exit);
