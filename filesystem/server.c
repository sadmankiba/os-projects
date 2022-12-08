#include <stdio.h>
#include <string.h>
#include "udp.h"

int main() {
    int sd = UDP_Open(9500);
    assert(sd > -1);
    while(1) {
        char message[10];
        int rc = UDP_Read(sd, NULL, message, 10);
        printf("Read message: %s, size: %d\n", message, rc);
        if(strcmp(message, "wr") == 0) {
            printf("Write..\n");
        } else if (strcmp(message, "rd") == 0) {
            printf("Read..\n"); // doesn't enter here :(
        }
    }
    return 0;
}