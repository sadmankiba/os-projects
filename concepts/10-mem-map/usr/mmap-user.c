
#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* open */
#include <unistd.h> /* close */
#include <sys/mman.h> /* mmap */

int main() {
    int fd = open("/dev/mdev", O_RDWR);
    void *ret;
    char buf[100];
    ret = mmap("mmap.txt", 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ret != NULL) {
        printf("mmap ret: %p\n", ret);
    }

    close(fd);
    return 0;
}
