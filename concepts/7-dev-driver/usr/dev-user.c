#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* open */
#include <unistd.h> /* close */
#include <sys/ioctl.h> /* ioctl */
#include "dev-driver.h"

#define BUF_LEN 100

int main() {
    int fd = open("/dev/ccdev", O_RDWR);
    char buf[BUF_LEN];
    int err;

    err = read(fd, buf, BUF_LEN);
    if (err < 0) { 
        printf("Error in read");
        return err;
    }
    printf("Message from kernel: %s\n", buf);
    
    strcpy(buf, "write msg from user");
    err = write(fd, buf, BUF_LEN);
    if (err < 0) {
        printf("Error in write");
        return err;
    }
    struct ioc_data idata;
    strcpy(idata.msg, "ioctl msg from user");
    err = ioctl(fd, IOCTL_PRINT_CMD, &idata);
    if(err) {
        printf("error in ioctl");
    }

    close(fd);
    return 0;
}
