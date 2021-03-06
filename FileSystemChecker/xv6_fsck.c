#include "xv6_fsck.h"


void
ERROR(char* err) {
  fprintf(stderr, "ERROR: %s.\n", err);
 // fprintf(stderr, "%s.\n", err);
  exit(1);
}

void*
getAddr(uint bn) {
  if(bn < sb->size - sb->nblocks) {
printf("bn: %u\n", bn);
    return NULL;
}
  if(bn >= sb->size) {

printf("bn: %u\n", bn);
    return NULL;
}
  return img_ptr + (bn*BSIZE);
}

void
checkAddr() {
  struct dinode* ip;
  uint ind,min, max, *addrs, *indaddrs;

  min =  DATABLOCK_BN(sb->ninodes); 
  max =  DATABLOCK_BN(sb->ninodes) + sb->nblocks;

  for(ip= ntable; ip< &ntable[sb->ninodes]; ip++) {
    if(ip->type != 0) {
      addrs = ip->addrs;
      for(ind = 0; ind < NDIRECT; ind ++) {
        if(addrs[ind] == 0)
          break;
        if(addrs[ind] < min||
            addrs[ind] >= max)
          ERROR("bad direct address in inode");
      }

      if(addrs[NDIRECT] == 0)
        continue;
      if(addrs[NDIRECT] < min || addrs[NDIRECT] >= max)
        ERROR("bad indirect address in inode");

      indaddrs = (uint*) getAddr(addrs[NDIRECT]);
      if(indaddrs == NULL)
        ERROR("getAddr failed"); 
      
      for(ind = 0; ind < NINDIRECT; ind++) {
        if ( indaddrs[ind] == 0 )
          break;
        if (indaddrs[ind] < min || indaddrs[ind] > max)
          ERROR("bad indirect address in inode");
      }
      
    }
  }
}
void
validType(struct dinode *ip) {
  if(ip->type < 0 || ip->type > 3){
    ERROR("bad inode");
  }
}


void
check_inode(struct dinode *ip, int inum) {
 int type;
  if(ip == NULL) {
    ERROR("passed invalid ip to check_inode()");
  }
  type = ip->type;
  switch (type) {
    case T_DIR:
       check_dir(ip, inum);
       break;
    case T_FILE:
       check_file(ip, inum);
       break;
    default : break;
  }
}
 
int
main (int argc, char* argv[]) {
int fd, ind; 
struct stat buf;
struct dinode *dip, *i;

  fd = open(argv[1], O_RDONLY);
  if ( fd < 0 ) {
    fprintf(stderr, "image not found.\n");
    exit(1);
   // ERROR("image not found");
  }
  if ( fstat(fd, &buf) < 0 ) {
    ERROR("fstat failed");
  }
  img_ptr = (void*) mmap(NULL, buf.st_size,
            PROT_READ, MAP_PRIVATE, fd, 0);
  if ( img_ptr < 0 ) {
    ERROR("mmap failed");
  }
  sb = (struct superblock*) (img_ptr + SB_OFFSET);
  ntable = dip = (struct dinode*)(img_ptr + NODE_OFFSET);
  bmap = (uchar*)(img_ptr + BMAP_OFFSET(sb->ninodes));
  datablock = img_ptr + DATABLOCK_OFFSET(sb->ninodes);

  check_root();
  checkAddr();
  check_extralink();
  bitmap_check();
  for(i = ntable; i<&ntable[sb->ninodes]; i++) {
    ind = (int) (i - ntable);
    validType(i);
    check_inode(i, ind);
  }
  //FIGURE HOW ROOT IS PASSED, ask about extern
return 0;
}
