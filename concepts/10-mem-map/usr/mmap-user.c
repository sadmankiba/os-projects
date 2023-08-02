#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* open */
#include <unistd.h> /* close */
#include <sys/mman.h> /* mmap */

#define MSG "Hello from user space!\n"
#define BUF_LEN 100

int main() {
    int fd = open("/dev/mdev", O_RDWR);
    void *ret;
    char buf[BUF_LEN];
    char rw_buf[BUF_LEN];
    int err;

    strcpy(buf, MSG);
    printf("Userspace buffer addr: %p (%lu)\n", buf, buf);
    printf("Message in userspace: %s\n", buf);
    ret = mmap(buf, BUF_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    printf("mmaped. Now buffer addr: %p (%lu)\n", buf, buf);
    if(ret != NULL) {
        printf("mmap ret: %p (%lu)\n", ret, ret);
    }
    printf("Now message in userspace buf: %s\n", buf);

    err = read(fd, rw_buf, BUF_LEN);
    if (err < 0) { 
        printf("Error in read");
        return err;
    }
    printf("Message from kernel: %s\n", rw_buf);

    strcpy(rw_buf, "write msg from user");
    err = write(fd, rw_buf, BUF_LEN);
    if (err < 0) {
        printf("Error in write");
        return err;
    }
    printf("Now message in userspace buf: %s\n", buf);

    close(fd);
    return 0;
}
