#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h> /* kmalloc */
#include <linux/fs.h> /* register, MKDEV, file, inode, file_operations */
#include <linux/cdev.h> /* struct cdev, cdev_* */
#include <linux/mm.h> /* remap_pfn_range */
#include <linux/highmem.h> /* SetPageReserved, ClearPageReserved */
#include <asm/io.h> /* virt_to_phys */


#include "dev-driver.h"

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

#define DEV_MAJOR 175
#define DEV_MINOR 2
#define NUM_DEVS 1
#define DEV_NAME "memmap-dev"

#define NPAGES 5

struct dev_info {
	struct cdev cdev;
	char *data;
};

struct dev_info dinfo;

void alloc_my_pages(void);
void free_my_pages(void);

int cdev_open(struct inode *ind, struct file *f) {
	struct dev_info *dinfo;

	pr_debug(DEV_NAME " opened");
	
	dinfo = container_of(ind->i_cdev, struct dev_info, cdev);
	f->private_data = dinfo;

	return 0;
}

int cdev_release(struct inode *ind, struct file *f) {
	struct dev_info *dinfo;

	pr_debug(DEV_NAME " released");
	
	dinfo = (struct dev_info *) f->private_data;
	return 0;
}

int cdev_mmap(struct file *f, struct vm_area_struct *vma) {
	unsigned long phys, pfn;
	unsigned long start = (unsigned long) vma->vm_start;
	unsigned long size = (unsigned long) (vma->vm_end - vma->vm_start);
	struct dev_info *dinfo = (struct dev_info *) f->private_data;

	pr_debug("mmap called, start: %lu, size: %lu\n", start, size);

	if (size > PAGE_SIZE) {
		pr_debug("Size too large\n");
		return -EINVAL;
	}

	phys = virt_to_phys((void *) dinfo->data);
	pfn = phys >> PAGE_SHIFT;
	pr_debug("phy addr: %lu (%lx), pfn: %lu (%lx)\n", phys, phys, pfn, pfn);

	if (remap_pfn_range(vma, start, pfn, size, vma->vm_page_prot)) {
		pr_debug("remap failed\n");
		return -EAGAIN;
	}

	return 0;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = cdev_open,
	.release = cdev_release, 
	.mmap = cdev_mmap,
};

static int mem_map_init(void)
{
	int err; 

	pr_debug("mem_map init!\n");
	err = register_chrdev_region(MKDEV(DEV_MAJOR, DEV_MINOR), NUM_DEVS, DEV_NAME);
	if (err) {
		pr_debug("Error in register chrdev: %d", err);
		return err;
	}
	pr_debug(DEV_NAME " registered");

	cdev_init(&dinfo.cdev, &fops);
	cdev_add(&dinfo.cdev, MKDEV(DEV_MAJOR, DEV_MINOR), NUM_DEVS);
	pr_debug(DEV_NAME " added");

	alloc_my_pages();
	pr_debug("Allocated %d pages\n", NPAGES);

	return 0;
}

static void mem_map_exit(void)
{	
	cdev_del(&dinfo.cdev);
	unregister_chrdev_region(MKDEV(DEV_MAJOR, DEV_MINOR), NUM_DEVS); 
	pr_debug(DEV_NAME " unregistered");

	free_my_pages();
	pr_debug("Freed %d pages\n", NPAGES);
	
	pr_debug("mem_map exit!\n");
}

void alloc_my_pages(void) {
	int i;
	dinfo.data = kmalloc(NPAGES * PAGE_SIZE, GFP_KERNEL);
	if (!dinfo.data) {
		pr_debug("kmalloc failed\n");
		return;
	}
	for (i = 0; i < NPAGES; i++) {
		SetPageReserved(virt_to_page(dinfo.data + i * PAGE_SIZE));
	}
}

void free_my_pages(void) {
	int i;
	for (i = 0; i < NPAGES; i++) {
		ClearPageReserved(virt_to_page(dinfo.data + i * PAGE_SIZE));
	}
	kfree(dinfo.data);
}

module_init(mem_map_init);
module_exit(mem_map_exit);
