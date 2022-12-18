#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "mfs.h"
#include "udp.h"
#include "message.h"
#include "ufs.h"
#include "debug.h"

int fd = -1;
track_t* p_cr = NULL;
super_t super;
unsigned int highest_inode = 0;
unsigned int hghst_alloc_dblk = 0;

// set up the needed functions
inode_t * read_inode(unsigned int);
dir_ent_t* lookup_file(int, char*);
int write_file(int inum, void *buf, unsigned int offset, int nbytes, int type);
int alloc_dblk(void);

int initialize_serv(char* );
int run_udp(int);
int end_serv();

int fsread(int addr, void *ptr, size_t nbytes) {
  lseek(fd, addr, SEEK_SET);
  return read(fd, ptr, nbytes);
}

int fswrite(int addr, void *ptr, size_t nbytes) {
  lseek(fd, addr, SEEK_SET);
  return write(addr, ptr, nbytes);
}

void inode_dbg(int inum) {
  inode_t * ind = read_inode(inum);
  debug("type: %d ", ind->type);
  debug("size: %d ", indsize);
  debug("direct: ");
  for(int i = 0; i < DIRECT_PTRS; i++) {
    debug("%d ", ind->direct[i]);
  }
  debug("\n");
}

void dir_dbg(int inum) {
  debug("dir entries: ");
  inode_t *ind = read_inode(inum);
  for(int i = 0; i < (ind->size / sizeof(dir_ent_t)); i++) {
    dir_ent_t de;
    fsread(ind->direct[0] * UFS_BLOCK_SIZE + i * sizeof(dir_ent_t), 
      &de, sizeof(dir_ent_t)); 
    debug("%s %d, ", de.name, de.inum);
  }
  debug("\n");
}

/* a 32-bit mask for a number starting from leftmost bit*/
unsigned int mask(unsigned int num) {
  return 0x1 << (8 * sizeof(unsigned int) - (num % (8 * sizeof(unsigned int))) - 1);
}

inode_t * read_inode(unsigned int inum) {
  unsigned int ibm; 
  fsread(super.inode_bitmap_addr * UFS_BLOCK_SIZE + (inum / 32) * 4, 
    &ibm, sizeof(unsigned int));
  int valid = ibm && mask(inum);
  if (!valid) return NULL;

  inode_t *ind = (inode_t *) malloc(sizeof(inode_t));
  fsread(super.inode_region_addr * UFS_BLOCK_SIZE + inum * sizeof(inode_t), 
    ind, sizeof(inode_t));
  return ind;
}

void write_inode(int inum, inode_t inode) {
  fswrite(super.inode_region_addr * UFS_BLOCK_SIZE + inum * sizeof(inode_t),
    &inode, sizeof(inode_t));
}

/*
lookup_file: Find a file in a parent directory
params: parent-inum, file-name, 
returns: file inode number if found, -1 if not found

Iterates over directory entries in data block of parent to find a file. 
*/
dir_ent_t* lookup_file(int pinum, char* name){
  inode_t *nd = read_inode(pinum);
  
  if(nd == NULL) return NULL;

  if(nd->type != UFS_DIRECTORY) {
    perror("lookup_file: invalid pinum_4");
    return NULL;
  }
    
  unsigned int mxb = ceil(1.0 * nd->size / UFS_BLOCK_SIZE); 

  for (int i = 0; i < mxb; i++) {
    unsigned int ndes = (i == (mxb - 1)) && (nd->size % UFS_BLOCK_SIZE != 0)? 
      (nd->size % UFS_BLOCK_SIZE) / sizeof(dir_ent_t) :
      UFS_BLOCK_SIZE / (sizeof(dir_ent_t));
    
    debug("In lookup_file: reading direct block %d (addr %d entries %d)\n", i, nd->direct[i], ndes);

    dir_ent_t * de = (dir_ent_t *) malloc(sizeof(dir_ent_t));
    for (int j = 0; j < ndes; j++) {
      fsread(nd->direct[i] * UFS_BLOCK_SIZE + j * sizeof(dir_ent_t), de, sizeof(dir_ent_t));
      if(strcmp(de->name, name) == 0 && de->inum != -1) {
        debug("In lookup_file: file found. inum %d\n", de->inum);
        return de;
      }
    }
  }
  debug("In lookup_file: file not found. returning NULL\n");
  return NULL;
}

/*
creat_file: 
params: parent inum, new-file type, new-file name
return new inode_num on success
- Get last-block addr from parent-inode.direct addresses
- If last-block.free space < sizeof(dir_ent_t)
    - add new data block to parent inum (addDataBlock)
    - update last-block address in parent-inode (setInode)
- FsImg.write(last-block addr + parent.size % block_size, dir_ent_t, sizeof(dir_ent_t))

*/
int creat_file(int pinum, int type, char *name) {
  /* Check if name already exists */
  dir_ent_t* lde = lookup_file(pinum, name);
  if (lde != NULL) return 0;

  /* set in i-bitmap*/
  highest_inode += 1;
  unsigned int bits;
  fsread(super.inode_bitmap_addr * UFS_BLOCK_SIZE 
    + highest_inode % (8 * sizeof(unsigned int)), 
    &bits, sizeof(unsigned int)); 
  bits |= mask(highest_inode);
  fswrite(super.inode_bitmap_addr * UFS_BLOCK_SIZE 
    + highest_inode % (8 * sizeof(unsigned int)), 
    &bits, sizeof(unsigned int));

  /* write in inode table */
  inode_t newnd;
  newnd.type = type;
  newnd.size = 0;
  for (int i = 0; i < DIRECT_PTRS; i++) {
    newnd.direct[i] = -1;
  }

  fswrite(super.inode_region_addr * UFS_BLOCK_SIZE + highest_inode * sizeof(inode_t), 
          &newnd, sizeof(inode_t));

  /* write in parent data*/
  inode_t *pind = read_inode(pinum);
  dir_ent_t de;
  de.inum = highest_inode;
  strcpy(de.name, name);

  if (pind->direct[0] == -1) pind->direct[0] = alloc_dblk();
  write_file(pinum, &de, pind->size, sizeof(dir_ent_t), UFS_DIRECTORY);
  inode_dbg(pinum);
  dir_dbg(pinum);
  return 0; 
}

/*
write_file
param: inode-num, offset (0-indexed), data, nbytes
returns: nbytes written

Writes to one block or two blocks. 
Adds a data block if not enough space in current block.
Updates inode size and direct fields.
*/

int write_file(int inum, void *buf, unsigned int offset, int nbytes, int type) {
  inode_t *fnd = read_inode(inum);
  if (fnd == NULL || fnd->type != type) return -1;

  unsigned int ofd = ceil(1.0 * offset / UFS_BLOCK_SIZE);
  unsigned int ofr = offset % UFS_BLOCK_SIZE;
  unsigned int offree = UFS_BLOCK_SIZE - ofr;
  unsigned int d = ofd;
  while(d >= 0 && fnd->direct[d] == -1) {
    fnd->direct[d] = alloc_dblk();
    d--;
  } 
  if(nbytes <= offree) {
    fswrite(fnd->direct[ofd] * UFS_BLOCK_SIZE + ofr, buf, nbytes);
  } else {
    fnd->direct[ofd + 1] = alloc_dblk();
    fswrite(fnd->direct[ofd] * UFS_BLOCK_SIZE + ofr, buf, offree);
    fswrite(fnd->direct[ofd + 1] * UFS_BLOCK_SIZE, 
      buf + sizeof(char) * offree, nbytes - offree);
  }
  fnd->size = (offset + nbytes) > fnd->size ? offset + nbytes: fnd->size;
  write_inode(inum, *fnd);
  fsync(fd);
  return 0;
}

int alloc_dblk() {
  /* set in d-bitmap */
  unsigned int bits;
  hghst_alloc_dblk += 1;
  fsread(super.data_bitmap_addr * UFS_BLOCK_SIZE, 
    &bits, sizeof(unsigned int)); /* assume <= 32 alloc blocks */
  bits |= (0x1 << (sizeof(unsigned int) * 8 - hghst_alloc_dblk - 1));
  fswrite(super.data_bitmap_addr * UFS_BLOCK_SIZE, &bits, sizeof(unsigned int));
  return super.data_region_addr + hghst_alloc_dblk;
}

/*
read_file
param: inode-num, buf, offset, nbytes
returns: nbytes read

Read from one block or two blocks as nbytes <= 4096.
*/
int read_file(int inum, char* buf, int offset, int nbytes) {
  inode_t *fnd = read_inode(inum);
  if (fnd == NULL) return -1;

  unsigned int rdb = ceil(1.0 * offset / UFS_BLOCK_SIZE);
  unsigned int rds = offset % UFS_BLOCK_SIZE;
  unsigned int rdf = UFS_BLOCK_SIZE - rds;
  if (nbytes <= rdf) {
    if(fnd->direct[rdb] == -1) return -1;
    fsread(fnd->direct[rdb] * UFS_BLOCK_SIZE + rds, buf, nbytes);
  } else {
    if(fnd->direct[rdb] == -1 || fnd->direct[rdb + 1] == -1) return -1;
    fsread(fnd->direct[rdb] * UFS_BLOCK_SIZE + rds, buf, rdf);
    fsread(fnd->direct[rdb + 1] * UFS_BLOCK_SIZE, buf + sizeof(char) * rdf, nbytes - rdf);
  }
  return 0;
}

/*
params: parent inum, file-name

Remove a regular file or directory name from the parent dir.

- Get parent-inode from parent inum (call getInode)
- Lookup file and get entry address of file (call lookupFile)
- Cast entry to dir_ent_t and get inum.
- Get inode from file-inum (call getInode)
- If file type is dir and size > 0, throw err
- Mark sizeof(dir_ent_t) bytes as invalid at entry address.
- Return success
*/
int unlink_file(int pinum, char *name) {
  dir_ent_t *de = lookup_file(pinum, name);
  if (de != NULL && de->inum != -1) {
    inode_t *ind = read_inode(de->inum);
    if (ind->type == UFS_DIRECTORY && ind->size > 0) {
      return -1;
    }
    strcpy(de->name, "");
    de->inum = -1; 
  }
  return 0;
}

/*
newDataBlock function: returns block address.
    - Find a free data block from data bitmap
    - Set bit to 1 for this data block in data bitmap
    - data block address = (super.data_region_addr + bit index in bitmap) * block_size
    
*/
/*
addDataBlock function: takes inum
    - Create a new data block and get address (newDataBlock)
    - Get inode from inum (getInode).
    - Append data block address to inode.direct
    - Write inode to inum (setInode)
*/


/*
newInode function: takes type, returns inum
    - Create a new data block and get address (newDataBlock)
    - Get new inode-num (maybe current-max-inode + 1)
    - Set bitmap for new-inode-num to 1
    - Write inode(type, size 0, direct=[block address])
    - Return inum
*/

int end_serv() {
  fsync(fd);
  exit(0);
}

int initialize_serv(char* image_path) {
  fd = open(image_path, O_RDWR | O_CREAT, S_IRWXU);

  struct stat fs;
  if(fstat(fd, &fs) < 0) {
    perror("initialize_serv: Cannot open file");
  }

  int rc, i, j;

  fsread(0, &super, sizeof(super_t));
  debug("Read super block. Inode rgn addr: %d, #inodes: %d\n", 
    super.inode_region_addr, super.num_inodes);
  debug("inode 0:\n");
  inode_dbg(0);
  p_cr = (track_t *)malloc(sizeof(track_t));
  int sz = 0;
  int offset = 0, step = 0;
	
  lseek(fd,0, SEEK_SET);
  read(fd, p_cr, sizeof(track_t));

  return 0;
}

int run_udp(int port) { 
  int sd=-1;
  if((sd =   UDP_Open(port))< 0){
    perror("initialize_serv: port open fail");
    return -1;
  }

  struct sockaddr_in s;
  message_t buf_pk,  rx_pk;

  while (1) {
    if( UDP_Read(sd, &s, (char *)&buf_pk, sizeof(message_t)) < 1)
      continue;


    if(buf_pk.msg == MFS_LOOKUP){
      /*
        - Get parent inum, file name from message.
        - Lookup file and get entry address (call lookupFile)
        - If found: 
            - Read entry address into dir_ent_t struct
            - Return inum
        - Else throw err
        */
      dir_ent_t *de = lookup_file(buf_pk.node_num, buf_pk.name);
      if (de != NULL) {
        rx_pk.node_num = de->inum;
      } else {
        rx_pk.node_num = -1;
      }
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));
    }
    else if(buf_pk.msg == MFS_STAT){
        /*
        - Get inum from message
        - Get inode from inum (call getInode)
        - Return MFS-Stat struct with type and size of inode
        */
      inode_t *ind = read_inode(buf_pk.node_num);
      rx_pk.st.size = ind->size;
      rx_pk.st.type = ind->type;
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));

    }
    else if(buf_pk.msg == MFS_WRITE){
      rx_pk.node_num = write_file(buf_pk.node_num, buf_pk.buf, 
        buf_pk.offset, buf_pk.nbytes, UFS_REGULAR_FILE);
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));

    }
    else if(buf_pk.msg == MFS_READ){
      rx_pk.node_num = read_file(buf_pk.node_num, rx_pk.buf, buf_pk.offset, buf_pk.nbytes);
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));

    }
    else if(buf_pk.msg == MFS_CREAT){
      rx_pk.node_num = creat_file(buf_pk.node_num, buf_pk.mtype, buf_pk.name);
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));

    }
    else if(buf_pk.msg == MFS_UNLINK){
      rx_pk.node_num = unlink_file(buf_pk.node_num, buf_pk.name);
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));

    }
    else if(buf_pk.msg == MFS_SHUTDOWN) {
     /*
      - Write any remaining data to image
      - Break from loop
      */
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));
      end_serv();
    }
    else if(buf_pk.msg == MFS_FEEDBACK) {
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));
    }
    else {
      perror("invalid MFS function");
      return -1;
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
	if(argc != 3) {
		perror("Usage: server <portnum> <image>\n");
		exit(1);
	}

	initialize_serv(argv[2]);
  run_udp(atoi(argv[1]));

	return 0;
}