#ifndef _DEV_DRIVER_H
#define _DEV_DRIVER_H

#include <linux/ioctl.h>

struct ioc_data {
    char msg[100];
};

#define IOCTL_PRINT_CMD _IOC(_IOC_WRITE, 'm', 1, sizeof(struct ioc_data))

#endif
