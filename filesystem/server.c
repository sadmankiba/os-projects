#include <stdio.h>
#include "udp.h"

int main() {
    int sd = UDP_Open(9500);
    assert(sd > -1);
    while(1) {
        char message[10];
        int rc = UDP_Read(sd, NULL, message, 10);
        printf("Read message: %s, size: %d\n", message, rc);
    }
    return 0;
}