#include "mfs.h"
#include "udp.h"

#define BUFFER_SIZE (5000)

struct sockaddr_in addrSnd;
int sd;
int rc;

int sendToServer(char *msg) {
    printf("Client: send msg: %s\n", msg);
    rc = UDP_Write(sd, &addrSnd, msg, BUFFER_SIZE);
    if (rc < 0) {
        printf("Client: failed to send");
        exit(1);
    }
    return rc;
}

int MFS_Init(char *hostname, int port) {
    sd = UDP_Open(35000);
    rc = UDP_FillSockAddr(&addrSnd, hostname, port);
    return 1;
}

int MFS_Lookup(int pinum, char *name) {
    return 0;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
    return 0;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes) {
    char msg[BUFFER_SIZE];
    sprintf(msg, "write");
    sendToServer(msg);
    return 0;
}

int MFS_Read(int inum, char *buffer, int offset, int nbytes) {
    char msg[BUFFER_SIZE];
    sprintf(msg, "read");
    sendToServer(msg);
    return 0;
}

int MFS_Creat(int pinum, int type, char *name) {
    char msg[BUFFER_SIZE];
    sprintf(msg, "create~%d~%d~%s", pinum, type, name);
    sendToServer(msg);

    return 0;
}

int MFS_Unlink(int pinum, char *name) {
    return 0;
}

int MFS_Shutdown() {
    return 0;
}

int main() {
    MFS_Init("localhost", 9500);
    MFS_Write(0, NULL, 0, 0);
    MFS_Read(0, NULL, 0, 0);
    MFS_Creat(0, MFS_REGULAR_FILE, "a.txt");
}