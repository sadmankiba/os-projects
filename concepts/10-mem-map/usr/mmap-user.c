#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* open */
#include <unistd.h> /* close */
#include <sys/mman.h> /* mmap */

#define MSG "Hello from user space!\n"

int main() {
    int fd = open("/dev/mdev", O_RDWR);
    void *ret;
    char buf[100];
    strcpy(buf, MSG);
    printf("Userspace buffer addr: %p (%lu)\n", buf, buf);
    ret = mmap(buf, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    printf("Message in userspace: %s\n", buf);
    if(ret != NULL) {
        printf("mmap ret: %p (%lu)\n", ret, ret);
    }
    printf("Now message in userspace: %s\n", buf);

    close(fd);
    return 0;
}
