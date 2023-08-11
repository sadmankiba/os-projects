#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/pci.h> /* pci_driver, PCI_DEVICE */

MODULE_DESCRIPTION("pci_prac module");
MODULE_AUTHOR("Kernel Practitioner");
MODULE_LICENSE("GPL");

static int my_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
void my_pci_remove(struct pci_dev *pdev);

#define MY_VENDOR_ID 0x1af4
#define MY_DEVICE_ID 0x1005

static struct pci_device_id my_pci_dev_ids[] = {
    { PCI_DEVICE(MY_VENDOR_ID, MY_DEVICE_ID) },
    { }
};
MODULE_DEVICE_TABLE(pci, my_pci_dev_ids);

static struct pci_driver my_pci_driver = {
    .name = "pci_prac_drvr",
    .id_table = my_pci_dev_ids,
    .probe = my_pci_probe,
    .remove = my_pci_remove,
};

static int my_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int err;
    pr_debug("my_pci_probe called\n");
    err = pci_enable_device(pdev);
    if (err) 
        return err;
    return 0;
}

void my_pci_remove(struct pci_dev *pdev) {
    int err;
    pr_debug("my_pci_remove called\n");
    pci_disable_device(pdev);
}

static int pci_prac_init(void)
{
	int err;
    pr_debug("pci_prac init!");
    err = pci_register_driver(&my_pci_driver);
    if (err) {
        return err;
    }
    return 0;
}

static void pci_prac_exit(void)
{
	pr_debug("pci_prac exit!");
    pci_unregister_driver(&my_pci_driver);
}

module_init(pci_prac_init);
module_exit(pci_prac_exit);
