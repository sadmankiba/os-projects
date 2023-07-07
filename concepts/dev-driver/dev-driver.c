#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h> /* register, MKDEV, file, inode, file_operations */
#include <linux/cdev.h> /* cdev_* */

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

#define DEV_MAJOR 175
#define DEV_MINOR 2
#define NUM_DEVS 1
#define DEV_NAME "sadman-dev"

struct dev_info {
	struct cdev cdev;
};

struct dev_info dinfo;

int cdev_open(struct inode *ind, struct file *f) {
	return 0;
}

int cdev_release(struct inode *ind, struct file *f) {
	return 0;
}

ssize_t cdev_read(struct file *f, char *buf, size_t size, loff_t *offset) {
	return 0;
}

ssize_t cdev_write(struct file *f, const char *buf, size_t size, loff_t *offset) {
	return 0;
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
		pr_debug("Error in register chrdev");
	}
	pr_debug(DEV_NAME " registered");

	cdev_init(&dinfo.cdev, &fops);
	cdev_add(&dinfo.cdev, MKDEV(DEV_MAJOR, DEV_MINOR), NUM_DEVS);
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
