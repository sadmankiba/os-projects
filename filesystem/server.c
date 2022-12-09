#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "udp.h"
#include "ufs.h"

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Usage: server [portnum] [file-system-image]");
        exit(1);
    }
    int port = atoi(argv[1]);
    char fsimg[100]; 
    sprintf(fsimg, "%s", argv[2]);
    
    int sd = UDP_Open(port);
    assert(sd > -1);

    int fd = open(fsimg, O_RDWR);
    super_t s;
    pread(fd, &s, sizeof(super_t), 0);
    printf("#inodes: %d #data: %d\n", s.num_inodes, s.num_data);

    while(1) {
        char message[10];
        int rc = UDP_Read(sd, NULL, message, 10);
        printf("Read message: %s, size: %d\n", message, rc);
        if(strcmp(message, "wr") == 0) {
            printf("Write..\n");
        } else if (strcmp(message, "rd") == 0) {
            printf("Read..\n"); 
        }
    }
    return 0;
}