
#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* open */
#include <unistd.h> /* close */
#include <sys/ioctl.h> /* ioctl */
#include "dev-driver.h"

int main() {
    int fd = open("/dev/ccdev", O_RDWR);
    int err;

    struct ioc_data idata;
    strcpy(idata.msg, "msg from user");
    err = ioctl(fd, IOCTL_PRINT_CMD, &idata);
    if(err) {
        printf("error in ioctl");
    }

    close(fd);
    return 0;
}
