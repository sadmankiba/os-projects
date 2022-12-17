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
int highest_inode = 0;
int hghst_alloc_dblk = 0;

// set up the needed functions
inode_t read_inode(int);
dir_ent_t* lookup_file(int, char*);
int alloc_dblk(void);

int initialize_serv(char* );
int run_udp(int);

int set_inode(int , MFS_Stat_t* );
int write_to(int , char* , int );
int read_from(int , char* , int );
int new_file(int , int , char* );
int remove_serv(int , char* );
int end_serv();
int add_data_block(int, int);
int print_dir(int);

int fsread(int addr, void *ptr, size_t nbytes) {
  lseek(fd, addr, SEEK_SET);
  return read(fd, ptr, nbytes);
}

int fswrite(int addr, void *ptr, size_t nbytes) {
  lseek(fd, addr, SEEK_SET);
  return write(addr, ptr, nbytes);
}

void inode_dbg(int inum) {
  inode_t ind = read_inode(inum);
  debug("type: %d ", ind.type);
  debug("size: %d ", ind.size);
  debug("direct: ");
  for(int i = 0; i < DIRECT_PTRS; i++) {
    debug("%d ", ind.direct[i]);
  }
  debug("\n");
}

void dir_dbg(int inum) {
  debug("dir entries: ");
  inode_t ind = read_inode(inum);
  for(int i = 0; i < (ind.size / sizeof(dir_ent_t)); i++) {
    dir_ent_t de;
    fsread(ind.direct[0] * UFS_BLOCK_SIZE + i * sizeof(dir_ent_t), 
      &de, sizeof(dir_ent_t)); 
    debug("%s %d, ", de.name, de.inum);
  }
  debug("\n");
}

inode_t read_inode(int inum) {
  inode_t *ind = (inode_t *) malloc(sizeof(inode_t));
  fsread(super.inode_region_addr * UFS_BLOCK_SIZE + inum * sizeof(inode_t), 
    ind, sizeof(inode_t));
  return *ind;
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
  inode_t nd = read_inode(pinum);
  
  /* assert dir */
  if(nd.type != MFS_DIRECTORY) {
    perror("lookup_file: invalid pinum_4");
    return NULL;
  }
    
  unsigned int mxb = ceil(1.0 * nd.size / UFS_BLOCK_SIZE); 

  for (int i = 0; i < mxb; i++) {
    unsigned int ndes = (i == (mxb - 1)) && (nd.size % UFS_BLOCK_SIZE != 0)? 
      (nd.size % UFS_BLOCK_SIZE) / sizeof(dir_ent_t) :
      UFS_BLOCK_SIZE / (sizeof(dir_ent_t));
    
    debug("In lookup_file: reading direct block %d (addr %d entries %d)\n", i, nd.direct[i], ndes);

    dir_ent_t * de;
    for (int j = 0; j < ndes; j++) {
      lseek(fd, nd.direct[i] * UFS_BLOCK_SIZE + j * sizeof(dir_ent_t), SEEK_SET);
      read(fd, de, sizeof(dir_ent_t));
      if(strcmp(de->name, name) == 0) {
        debug("In lookup_file: file name matched. inum %d\n", de->inum);
        return de;
      }
    }
    
  }
  debug("In lookup_file: file not found. returning -1\n");
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
  /* set in i-bitmap*/
  highest_inode += 1;
  unsigned int bits;
  fsread(super.inode_bitmap_addr * UFS_BLOCK_SIZE, 
    &bits, sizeof(unsigned int)); /* assume <= 32 inodes */
  bits |= (0x1 << (sizeof(unsigned int) * 8 - highest_inode - 1));
  fswrite(super.inode_bitmap_addr * UFS_BLOCK_SIZE, &bits, sizeof(unsigned int));

  /* write in inode table */
  inode_t newnd;
  newnd.type = type;
  newnd.size = 0;
  for (int i = 0; i < DIRECT_PTRS; i++) {
    newnd.direct[i] = -1;
  }
  newnd.direct[0] = alloc_dblk();
  fswrite(super.inode_region_addr * UFS_BLOCK_SIZE + highest_inode * sizeof(inode_t), 
          &newnd, sizeof(inode_t));

  /* write in parent data*/
  inode_t pind = read_inode(pinum);
  dir_ent_t de;
  de.inum = highest_inode;
  strcpy(de.name, name);
  fswrite(pind.direct[0] * UFS_BLOCK_SIZE + pind.size, 
    &de, sizeof(dir_ent_t)); /* assume dir size <= 1 block*/
  pind.size += sizeof(dir_ent_t);
  write_inode(pinum, pind);
  fsync(fd);
  inode_dbg(pinum);
  dir_dbg(pinum);
  return highest_inode; 
}

/*
write_file
param: inode-num, offset, data, nbytes
returns: nbytes written

Writes to one block or two blocks. 
Adds a data block if not enough space in current block.
Updates inode size and direct fields.
*/

int write_file(int inum, char *buf, int offset, int nbytes) {
  inode_t fnd = read_inode(inum);
  unsigned int mxd = ceil(1.0 * fnd.size / UFS_BLOCK_SIZE);
  unsigned int mxe = fnd.size % UFS_BLOCK_SIZE;
  unsigned int mxfree = UFS_BLOCK_SIZE - mxe;
  if(nbytes <= mxfree) {
    lseek(fd, fnd.direct[mxd] * UFS_BLOCK_SIZE + offset, SEEK_SET);
    write(fd, buf, nbytes);
  } else {
    fnd.direct[mxd + 1] = alloc_dblk();
    lseek(fd, fnd.direct[mxd] * UFS_BLOCK_SIZE + mxe, SEEK_SET);
    write(fd, buf, mxfree);
    lseek(fd, fnd.direct[mxd + 1] * UFS_BLOCK_SIZE, SEEK_SET);
    write(fd, buf + sizeof(char) * mxfree, nbytes - mxfree);
  }
  fnd.size += nbytes;
  write_inode(inum, fnd);
  fsync(fd);
  return nbytes;
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
  inode_t fnd = read_inode(inum);
  unsigned int mxb = ceil(1.0 * fnd.size / UFS_BLOCK_SIZE);
  unsigned int rdb = ceil(1.0 * offset / UFS_BLOCK_SIZE);
  unsigned int rds = offset % UFS_BLOCK_SIZE;
  unsigned int rdf = UFS_BLOCK_SIZE - rds;
  if (nbytes <= rdf) {
    lseek(fd, fnd.direct[rdb] * UFS_BLOCK_SIZE + rds, SEEK_SET);
    read(fd, buf, nbytes);
  } else {
    lseek(fd, fnd.direct[rdb] * UFS_BLOCK_SIZE + rds, SEEK_SET);
    read(fd, buf, rdf);
    lseek(fd, fnd.direct[rdb + 1] * UFS_BLOCK_SIZE, SEEK_SET);
    read(fd, buf + rdf, nbytes - rdf);
  }
  return nbytes;
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
    inode_t ind = read_inode(de->inum);
    if (ind.type == UFS_DIRECTORY && ind.size > 0) {
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



int new_file(int pinum, int type, char* name) {
// interable vars
  int i=0;
  int j=0;
  int k=0;
  int l=0;

  int offset = 0, step =0;

  int is_old_mp = 0;
  int is_old_nd = 0;
  int is_old_block = 0;

  int fp_mp = -1, fp_nd = -1, fp_block = -1;

  // find parent inode
  inode_t nd;
  N_Trace mp;

  int len_name = 0;
  for(i=0; name[i] != '\0'; i++, len_name ++)
    ;
  if(lookup_file(pinum, name) != NULL) {
    return 0;
  }


  k = pinum / 16; 
  fp_mp =  p_cr->node_array[k];

  l = pinum % 16;
  N_Trace mp_par;	       
  lseek(fd, fp_mp, SEEK_SET);
  read(fd, &mp_par, sizeof(N_Trace));
  fp_nd = mp_par.inodes[l];

  inode_t nd_par;
  lseek(fd, fp_nd, SEEK_SET);
  read(fd, &nd_par, sizeof(inode_t));

  int free_inum = -1;
  int is_free_inum_found = 0;
  for(i=0; i<256; i++) {
    fp_mp =  p_cr->node_array[i];

    if(fp_mp != -1) {

      N_Trace mp_par;	     
      lseek(fd, fp_mp, SEEK_SET);
      read(fd, &mp_par, sizeof(N_Trace));
      for(j=0; j<16; j++) {
        fp_nd = mp_par.inodes[j]; 
        if(fp_nd == -1) {
          free_inum = i*16 + j;
          is_free_inum_found = 1;
          break;
        }
      }
    }
    else {

      N_Trace mp_new;
      for(j = 0; j< 16; j++) mp_new.inodes[j] = -1 ; 

      offset = p_cr->tfinal;
      step = sizeof(N_Trace);
      p_cr->tfinal += step;
      lseek(fd, offset, SEEK_SET);
      write(fd, &mp_new, step);

      p_cr->node_array[i] = offset;	
      lseek(fd, 0, SEEK_SET);
      write(fd, p_cr, sizeof(track_t));

      fsync(fd);

      for(j=0; j<16; j++) {
        fp_nd = mp_new.inodes[j]; 
        if(fp_nd == -1) {
          free_inum = i*16 + j;
          is_free_inum_found = 1;
          break;
        }
      }
    }

    if (is_free_inum_found) break;
  }


  char data_buf[4096]; 
  Block_t* dir_buf = NULL;
  int flag_found_entry = 0;
  int block_par = 0;
  inode_t* p_nd = &nd_par;


  for(i=0; i< 14; i++) {

    fp_block = p_nd->direct[i];
    block_par = i;
    if(fp_block == -1) {
      Block_t* p_dir = (Block_t*) data_buf;
      for(i=0; i< 64; i++){
        strcpy(p_dir->data_blocks[i].name, "\0");
        p_dir->data_blocks[i].inum = -1;
      }
      offset = p_cr->tfinal;
      step = 4096; 
      p_cr->tfinal += step;
      lseek(fd, offset, SEEK_SET);
      write(fd, p_dir, sizeof(Block_t)); 

      fp_block = offset;

      inode_t nd_dir_new;
      nd_dir_new.size = nd_par.size; 
      nd_dir_new.type = MFS_DIRECTORY;
      for (i = 0; i < 14; i++) nd_dir_new.direct[i] = nd_par.direct[i]; 
      nd_dir_new.direct[block_par] = offset; 	
      p_nd = &nd_dir_new;		


      offset = p_cr->tfinal;
      step = sizeof(inode_t); 
      p_cr->tfinal += step;
      lseek(fd, offset, SEEK_SET);
      write(fd, &nd_dir_new, step);	


      N_Trace mp_dir_new;
      for(i = 0; i< 16; i++) mp_dir_new.inodes[i] = mp_par.inodes[i]; 
      mp_dir_new.inodes[l] = offset;

      offset = p_cr->tfinal;
      step = sizeof(N_Trace);
      p_cr->tfinal += step;
      lseek(fd, offset, SEEK_SET);
      write(fd, &mp_dir_new, step);	

      p_cr->node_array[k] = offset;
      lseek(fd, 0, SEEK_SET);
      write(fd, p_cr, sizeof(track_t));

      fsync(fd);

    }

    lseek(fd, fp_block, SEEK_SET);
    read(fd, data_buf, 4096);
	  
    dir_buf = (Block_t*)data_buf;
    for(j=0; j<64; j++) {
      MFS_DirEnt_t* p_de = &dir_buf->data_blocks[j];
      if(p_de->inum == -1) {
        p_de->inum = free_inum;
        strcpy(p_de->name, name);
        flag_found_entry = 1;
        break;
      }
    }

    if(flag_found_entry)
      break;
  }
  offset = p_cr->tfinal;
  step = 4096; 
  p_cr->tfinal += step;
  lseek(fd, offset, SEEK_SET);
  write(fd, dir_buf, sizeof(Block_t)); 

  inode_t nd_par_new;
  nd_par_new.size = p_nd->size; 
  nd_par_new.type = MFS_DIRECTORY;
  for (i = 0; i < 14; i++) nd_par_new.direct[i] = p_nd->direct[i]; 
  nd_par_new.direct[block_par] = offset;


  offset = p_cr->tfinal;
  step = sizeof(inode_t); 
  p_cr->tfinal += step;
  lseek(fd, offset, SEEK_SET);
  write(fd, &nd_par_new, step);	


  N_Trace mp_par_new;
  for(i = 0; i< 16; i++) mp_par_new.inodes[i] = mp_par.inodes[i]; 
  mp_par_new.inodes[l] = offset;

  offset = p_cr->tfinal;
  step = sizeof(N_Trace); 
  p_cr->tfinal += step;
  lseek(fd, offset, SEEK_SET);
  write(fd, &mp_par_new, step);	

  p_cr->node_array[k] = offset;
  lseek(fd, 0, SEEK_SET);
  write(fd, p_cr, sizeof(track_t));


  fsync(fd);

  char* ip = NULL;
  char wr_buffer[ 4096 ];
  for(i=0, ip=wr_buffer; i<4096; i++) {
    wr_buffer[i] = '\0';
  }

  int inum = free_inum;
  is_old_mp = 0, is_old_nd = 0, is_old_block = 0;
  fp_mp = -1, fp_nd = -1, fp_block = -1;
  if(type == MFS_DIRECTORY) {
    Block_t* p_dir = (Block_t*) wr_buffer;
    for(i=0; i< 64; i++){
      strcpy(p_dir->data_blocks[i].name, "\0");
      p_dir->data_blocks[i].inum = -1;
    }
    strcpy(p_dir->data_blocks[0].name, ".\0");
    p_dir->data_blocks[0].inum = inum; 
    strcpy(p_dir->data_blocks[1].name, "..\0");
    p_dir->data_blocks[1].inum = pinum; 

    offset = p_cr->tfinal;	
    step = 4096; 
    p_cr->tfinal += step;
    lseek(fd, offset, SEEK_SET);
    write(fd, wr_buffer, step);

  }

  k = inum / 16; 
  fp_mp =  p_cr->node_array[k];
  if(fp_mp != -1){
    is_old_mp = 1;

    l = inum % 16; 
    lseek(fd, fp_mp, SEEK_SET);
    read(fd, &mp, sizeof(N_Trace));
    fp_nd = mp.inodes[l]; 

  }

  inode_t nd_new;
  nd_new.size = 0;		
  nd_new.type = type;			  
  for (i = 0; i < 14; i++) nd_new.direct[i] = -1;
  if(type == MFS_DIRECTORY)
    nd_new.direct[0] = offset;		

  offset = p_cr->tfinal;
  step = sizeof(inode_t); 
  p_cr->tfinal += step;
  lseek(fd, offset, SEEK_SET);
  write(fd, &nd_new, step);


  N_Trace mp_new;
  if(is_old_mp) {
    for(i = 0; i< 16; i++) mp_new.inodes[i] = mp.inodes[i] ; 
    mp_new.inodes[l] = offset;
  }
  else {
    for(i = 0; i< 16; i++) mp_new.inodes[i] = -1 ; 
    mp_new.inodes[l] = offset;
  }

  offset = p_cr->tfinal;
  step = sizeof(N_Trace);
  p_cr->tfinal += step;
  lseek(fd, offset, SEEK_SET);
  write(fd, &mp_new, step);

  p_cr->node_array[k] = offset; 
  lseek(fd, 0, SEEK_SET);
  write(fd, p_cr, sizeof(track_t));

  fsync(fd);

  return 0;
}

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

  lseek(fd, 0, SEEK_SET);
  read(fd, &super, sizeof(super_t));
  debug("Read super block. Inode rgn addr: %d, #inodes: %d\n", 
    super.inode_region_addr, super.num_inodes);
  debug("inode 0:\n");
  inode_dbg(0);
  p_cr = (track_t *)malloc(sizeof(track_t));
  int sz = 0;
  int offset = 0, step = 0;
	
  if(fs.st_size < sizeof(track_t)){

    fd = open(image_path, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    if(fd <0)
      return -1;

    if(fstat(fd,&fs) < 0) {
      perror("initialize_serv: fstat error");
      return -1;
    }
       
    p_cr->inode_count = 0;
    p_cr->tfinal = sizeof(track_t);
    for(i=0; i<256; i++)
      p_cr->node_array[i] = -1;

    lseek(fd, 0, SEEK_SET);
    write(fd, p_cr, sizeof(track_t));

    Block_t db;
    for(i=0; i< 64; i++){
      strcpy(db.data_blocks[i].name, "\0");
      db.data_blocks[i].inum = -1;
    }
    strcpy(db.data_blocks[0].name, ".\0");
    db.data_blocks[0].inum = 0; 
    strcpy(db.data_blocks[1].name, "..\0");
    db.data_blocks[1].inum = 0;

    offset = p_cr->tfinal;
    step = 4096; 
    p_cr->tfinal += step;
    lseek(fd, offset, SEEK_SET);
    write(fd, &db, sizeof(Block_t));

    inode_t nd;
    nd.size = 0; 
    nd.type = MFS_DIRECTORY;
    for (i = 0; i < 14; i++) nd.direct[i] = -1; 
    nd.direct[0] = offset;			   

    offset = p_cr->tfinal;
    step = sizeof(inode_t);
    p_cr->tfinal += step;
    lseek(fd, offset, SEEK_SET);
    write(fd, &nd, step);


    N_Trace mp;
    for(i = 0; i< 16; i++) mp.inodes[i] = -1;
    mp.inodes[0] = offset;

    offset = p_cr->tfinal;
    step = sizeof(N_Trace);
    p_cr->tfinal += step;
    lseek(fd, offset, SEEK_SET);
    write(fd, &mp, step);

    p_cr->node_array[0] = offset;
    lseek(fd, 0, SEEK_SET);
    write(fd, p_cr, sizeof(track_t));

    fsync(fd);

  }
  else {
    lseek(fd,0, SEEK_SET);
    read(fd, p_cr, sizeof(track_t));
  }

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
      if (de != NULL && de->inum != -1) {
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
      inode_t ind = read_inode(buf_pk.node_num);
      rx_pk.st.size = ind.size;
      rx_pk.st.type = ind.type;
      rx_pk.msg = MFS_FEEDBACK;
      UDP_Write(sd, &s, (char*)&rx_pk, sizeof(message_t));

    }
    else if(buf_pk.msg == MFS_WRITE){
      rx_pk.node_num = write_file(buf_pk.node_num, buf_pk.buf, buf_pk.offset, buf_pk.nbytes);
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