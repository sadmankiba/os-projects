#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "udp.h"
#include "ufs.h"

#define BUFFER_SIZE (5000)

int getInode(int inum) {
    // FsImg.read(super.inode_reg_addr * block_size + inode-num * sizeof(inode), sizeof(inode))
    // returns inode_t
    return 0;
}

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
        char msg[BUFFER_SIZE];
        int rc = UDP_Read(sd, NULL, msg, BUFFER_SIZE);
        printf("Got message: %s, size: %d\n", msg, rc);
        char *pm = strdup(msg);
        char *op = strsep(&pm, "~");
        if(strcmp(op, "write") == 0) {
            printf("Write..\n");
            // Get inode-num, offset, data, nbytes
            // inode = getInode(inode-num)
            // last_block = inode.direct[just before invalid idx]
            // if last_block.free_space >= nbytes:
            //    FsImg.write(data)
            // else:
            //    new_block = from data bitmap, next available data block.
            //    write possible data in last_block
            //    write remaining data new_block
            //    add new block address to inode.direct
            //    FsImg.write(super.data_bitmap_addr * block_size + bitmap-num / 8, bitmap-set-to-1)
            //    increase file size in inode
            // return success

        } else if (strcmp(op, "read") == 0) {
            printf("Read..\n"); 
            // Get inode-num, offset, nbytes
            // inode = getInode(inode-num)
            // block_start = inode.direct[offset / block_size]
            // block_end =  inode.direct[(offset + nbytes) / block_size]
            // if block_start == block_end:
            //      data = FsImg.read(block_start + offset % block_size, nbytes)
            // else:
            //      data1 = FsImg.read(block_start + offset % block_size, (block_size - offset))
            //      data2 = FsImg.read(block_end, (nbytes - block_size + offset))
            //      data = data1 + data2
            // return data
        } else if (strcmp(op, "create") == 0){
            printf("Create..\n");
            // Get parent-inode-num, type, name
            
            // inode = getInode(inode-num)
            // last_block = inode.direct[just before invalid idx]
            // assume enough free space
            // get new-inode-num 
            // Set bitmap for new-inode-num to 1 
            // free-block = next-available-block-from-data-bitmap
            // assign free-block to inode-num in inode-table
            // FsImg.write(last_block + inode.size % block_size, <new-inode, name-length, name>)
            // return success

        } 
    }
    return 0;
}