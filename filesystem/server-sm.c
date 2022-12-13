#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "mfs.h"
#include "udp.h"
#include "udp.c"
#include "message.h"

int sd;

void intHandler(int dummy) {
    UDP_Close(sd);
    exit(130);
}

/*
getInode function: takes inum, returns inode_t.
    - inode_addr = super.inode_reg_addr * block_size + inode-num * sizeof(inode)
    - FsImg.read(inode_addr, sizeof(inode))

setInode function: takes inode_t, inum
    - inode_addr = super.inode_reg_addr * block_size + inode-num * sizeof(inode)
    - FsImg.write(inode_addr, inode_t, sizeof(inode))

newDataBlock function: returns block address.
    - Find a free data block from data bitmap
    - Set bit to 1 for this data block in data bitmap
    - data block address = (super.data_region_addr + bit index in bitmap) * block_size
    

addDataBlock function: takes inum
    - Create a new data block and get address (newDataBlock)
    - Get inode from inum (getInode).
    - Append data block address to inode.direct
    - Write inode to inum (setInode)

newInode function: takes type, returns inum
    - Create a new data block and get address (newDataBlock)
    - Get new inode-num (maybe current-max-inode + 1)
    - Set bitmap for new-inode-num to 1
    - Write inode(type, size 0, direct=[block address])
    - Return inum

lookupFile function: takes parent-inum, file-name, returns entry address
    - Get inode from parent inum (getInode).
    - Get data block addresses from inode.direct. 
    - Loop through all direct addresses.
        - Start from a data block addr and read data until 
            file found or block end reached.
            - Read sizeof(dir_ent_t) bytes and cast to dir_ent_t struct
            - If name matches file name from client, break
    - Return entry address of found file or err if file not found
*/



int main(int argc, char *argv[])
{

    signal(SIGINT, intHandler);

    if(argc != 3) {
        printf("Usage: server [portnum] [file-system-image]");
        exit(1);
    }

    int port = atoi(argv[1]);
    void *image;

    sd = UDP_Open(port);
    assert(sd > -1);

    struct stat sbuf;
    int fd = open(image, O_RDWR);
    assert(fd > -1);

    int rc = fstat(fd, &sbuf);
    assert(rc > -1);

    int image_size = (int) sbuf.st_size;

    image = mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

 
    while (1) {
        struct sockaddr_in addr;

        message_t msg;

        printf("server: waiting...\n");
        int rc = UDP_Read(sd, &addr, (char *) &msg, sizeof(message_t));

        if(msg.mtype == MFS_INIT){
            /* Do nothing */
        }
        else if(msg.mtype == MFS_LOOKUP){
            /*
            - Get parent inum, file name from message.
            - Lookup file and get entry address (call lookupFile)
            - If found: 
                - Read entry address into dir_ent_t struct
                - Return inum
            - Else throw err
            */
        }
        else if(msg.mtype == MFS_STAT){
            /*
            - Get inum from message
            - Get inode from inum (call getInode)
            - Return MFS-Stat struct with type and size of inode
            */

        }
        else if(msg.mtype == MFS_WRITE){
            /*
            - Get inode-num, offset, data, nbytes
            - inode = getInode(inode-num)
            - Get last_block from inode.direct
            (Comment: nbytes <= 4096, thus write can be from one block or two blocks)
            - if last_block.free_space >= nbytes:
                - FsImg.write(data)
            - else:
                - Add a data block (addDataBlock)
                - Get inode from inum (getInode)
                - Write possible data in second-to-last-block in inode.direct
                - Write remaining data in last block in inode.direct
            - Update file size for inum (setInode)
            - Return success
            */
        }
        else if(msg.mtype == MFS_READ){
            /*
            - Get inode-num, offset, nbytes
            - Get inode from inode-num (getInode)
            (Comment: nbytes <= 4096, thus read can be from one block or two blocks)
            - block_start = inode.direct[int(offset / block_size)]
            - block_end =  inode.direct[int((offset + nbytes) / block_size)]
            - if block_start == block_end:
            -      data = FsImg.read(block_start + offset % block_size, nbytes)
            - else:
            -      data1 = FsImg.read(block_start + offset % block_size, (block_size - offset))
            -      data2 = FsImg.read(block_end, (nbytes - block_size + offset))
            -      data = concatenate(data1, data2)
            - Return data
            */
        }
        else if(msg.mtype == MFS_CREAT){
            /*
            - Get parent inum, new-file type, new-file name
            - If name >= 28 bytes, throw err
            - Get parent-inode from parent inum (getInode)
            - Get last-block addr from parent-inode.direct addresses
            - If last-block.free space < sizeof(dir_ent_t)
                - add new data block to parent inum (addDataBlock)
                - update last-block address in parent-inode (setInode)
            - Create new inode with type and get new-inum (newInode)
            - Create a dir_ent_t with name and new-inum
            - FsImg.write(last-block addr + parent.size % block_size, dir_ent_t, sizeof(dir_ent_t))
            - Update size of parent-inode (setInode)
            - Return success
            */
        }
        else if(msg.mtype == MFS_UNLINK){
            /*
            - Get parent inum, file-name 
            - Get parent-inode from parent inum (call getInode)
            - Lookup file and get entry address of file (call lookupFile)
            - Cast entry to dir_ent_t and get inum.
            - Get inode from file-inum (call getInode)
            - If file type is dir and size > 0, throw err
            - Mark sizeof(dir_ent_t) bytes as invalid at entry address.
            - Return success
            */

        }
        else if(msg.mtype == MFS_SHUTDOWN){
            /*
            - Write any remaining data to image
            - Break from loop
            */
        }
    }

  return 0;
}

