#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h> /* register, MKDEV, file, inode, file_operations */
#include <linux/cdev.h> /* struct cdev, cdev_* */
#include <linux/sched.h> /* set_current_state */
#include <linux/string.h> /* strlen */

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

#define DEV_MAJOR 175
#define DEV_MINOR 2
#define NUM_DEVS 1
#define DEV_NAME "sadman-dev"

#define DEV_FREE 0
#define DEV_BUSY 1

#define MSG "You are reading sadman-dev!"

struct dev_info {
	struct cdev cdev;
	atomic_t opened;
	char data[100];
};

struct dev_info dinfo;

int cdev_open(struct inode *ind, struct file *f) {
	struct dev_info *dinfo;

	pr_debug(DEV_NAME " opened");
	
	dinfo = container_of(ind->i_cdev, struct dev_info, cdev);
	f->private_data = dinfo;

	if(atomic_cmpxchg(&dinfo->opened, DEV_FREE, DEV_BUSY) == DEV_BUSY) {
		return -EBUSY;
	}
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(1000);
	return 0;
}

int cdev_release(struct inode *ind, struct file *f) {
	struct dev_info *dinfo;

	pr_debug(DEV_NAME " released");
	
	dinfo = (struct dev_info *) f->private_data;
	atomic_cmpxchg(&dinfo->opened, DEV_BUSY, DEV_FREE);
	return 0;
}

ssize_t cdev_read(struct file *f, char *buf, size_t size, loff_t *offset) {
	struct dev_info *dinfo;
	int err;

	pr_debug(DEV_NAME " reading at offset %lld, size %lu", *offset, size);
	dinfo = (struct dev_info *) f->private_data;
	if (size > strlen(dinfo->data) - *offset) {
		size = strlen(dinfo->data) - *offset;
	}
	err = copy_to_user(buf, dinfo->data, size);
	if (err) 
		return err;

	*offset = *offset + size;
	return size;
}

ssize_t cdev_write(struct file *f, const char *buf, size_t size, loff_t *offset) {
	struct dev_info *dinfo;
	int err;

	pr_debug(DEV_NAME " writing at offset %lld, size %lu", *offset, size);
	dinfo = (struct dev_info *) f->private_data;
	// if (size > strlen(dinfo->data) - *offset) {
	// 	size = strlen(dinfo->data) - *offset;
	// }
	err = copy_from_user(dinfo->data, buf, size);
	if (err) 
		return err;

	// *offset = *offset + size;
	return size;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = cdev_open,
	.release = cdev_release, 
	.read = cdev_read,
	.write = cdev_write
};

static int dev_driver_init(void)
{
	int err; 

	pr_debug("dev_driver init!\n");
	err = register_chrdev_region(MKDEV(DEV_MAJOR, DEV_MINOR), NUM_DEVS, DEV_NAME);
	if (err) {
		pr_debug("Error in register chrdev: %d", err);
		return err;
	}
	pr_debug(DEV_NAME " registered");

	cdev_init(&dinfo.cdev, &fops);
	cdev_add(&dinfo.cdev, MKDEV(DEV_MAJOR, DEV_MINOR), NUM_DEVS);
	atomic_set(&dinfo.opened, 0);
	memcpy((char *) dinfo.data, MSG, strlen(MSG) + 1);
	return 0;
}

static void dev_driver_exit(void)
{	
	cdev_del(&dinfo.cdev);
	unregister_chrdev_region(MKDEV(DEV_MAJOR, DEV_MINOR), NUM_DEVS); 
	pr_debug(DEV_NAME " unregistered");
	pr_debug("dev_driver exit!\n");
}

module_init(dev_driver_init);
module_exit(dev_driver_exit);
