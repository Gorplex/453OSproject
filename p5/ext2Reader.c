#include "ext2Reader.h"

static FILE *fp = 0;

uint32_t block_size;
uint32_t blocks_per_group;
uint32_t inodes_per_group;
uint32_t inode_table;

void read_block(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size) {
    sdReadData(block*2+offset/512, offset%512, data, MIN(512-(offset%512), size));   
    if(size>=512){
        sdReadData(block*2+1, offset%512, data+512-offset, size-512-offset);
    }
}

void read_super(){
    struct ext2_super_block su_blk;
    read_block(1, 0, (uint8_t *)&su_blk, sizeof(struct ext2_super_block));
    block_size = 1024 << su_blk.s_log_block_size;
    blocks_per_group = su_blk.s_blocks_per_group;
    inodes_per_group = su_blk.s_inodes_per_group;
}

void read_bgdt(){
    struct ext2_group_desc bgdt;
    read_block(2,0, (void *) &bgdt, sizeof(struct ext2_group_desc));
    inode_table = bgdt.bg_inode_table;
}

void read_inode(uint32_t inodeNum, struct ext2_inode *inode){
    uint32_t BGNum;
    uint32_t index;
    uint32_t block;

    BGNum = (inodeNum-1) / inodes_per_group; 
    index = (inodeNum-1) % inodes_per_group; 
    block = (index*EXT2_GOOD_OLD_INODE_SIZE)/block_size;
    read_block(inode_table+block+BGNum*blocks_per_group,
        (index*EXT2_GOOD_OLD_INODE_SIZE)%block_size, 
        (void *)inode, EXT2_GOOD_OLD_INODE_SIZE);
}

uint32_t readDirBlock(uint32_t block, uint16_t *curIndex, uint16_t *index, char *name){
   struct ext2_inode inode;
   struct ext2_dir_entry *dirEnt;
   uint8_t buffer[BLOCK_SIZE];

   read_block(block, 0, (void *)buffer, block_size); 
   
   while((((void *)dirEnt) - ((void *)buffer)) < block_size && dirEnt->rec_len){
      if(*curIndex == *index){
         read_inode(dirEnt->inode, &inode);
         if(inode.i_mode & EXT2_S_IFREG){
            memcpy(name, dirEnt->name, dirEnt->name_len);
            name[dirEnt->name_len] = '\0';
            return dirEnt->inode;
         }
         *index = *index+1;
      }
      *curIndex = *curIndex+1;
      dirEnt = (struct ext2_dir_entry *) (dirEnt->rec_len + ((void *)dirEnt));
   }
   return 0;
}

uint32_t readRoot(uint16_t *index, char *name){
   struct ext2_inode inode;
   uint16_t curIndex;
   uint16_t blockNum;
   uint32_t inodeNum;

   read_super();
   read_bgdt();
   read_inode(EXT2_ROOT_INO, &inode);
   
   while(blockNum*block_size < inode.i_size){
      if(inodeNum = readDirBlock(inode.i_block[blockNum], &curIndex, index, name)){
         return inodeNum;      
      }
      blockNum++;
   }

   return inodeNum;
}

void readFile(uint32_t inodeNum, uint32_t bufNum, uint8_t *buf){
   struct ext2_inode inode;
   uint8_t links[256];
   uint16_t i;

   read_inode(inodeNum, &inode);
   
   if(bufNum/4 < 12){
      read_block(inode.i_block[bufNum/4], (bufNum%4)*READ_BUF_SIZE, ((void *)buf)+(bufNum%2)*READ_BUF_SIZE, MIN(block_size, inode.i_size-bufNum*READ_BUF_SIZE));
   }else if(bufNum/4 >= 12){
      read_block(inode.i_block[12], 0, links, block_size);
      read_block(links[bufNum/4-12], (bufNum%4)*READ_BUF_SIZE, ((void *)buf)+(bufNum%2)*READ_BUF_SIZE, MIN(block_size, inode.i_size-bufNum*READ_BUF_SIZE));
   }
   //NO DOUBLE LINKS
}

/*OLD
void print_file(struct ext2_inode *inode){
    uint32_t buf[256];
    uint32_t bufofbuf[256];
    uint32_t size;
    uint16_t i;
    uint16_t j;

    size=inode->i_size;
    for(i=0;size>0 && i<12;i++){
        print_block(inode->i_block[i], MIN(block_size, size));
        size -= MIN(block_size, size);
    }
    read_block(inode->i_block[12], 0, (void *)&buf, block_size);
    for(i=0;size>0 && i<256;i++){
        print_block(buf[i], MIN(block_size, size));
        size -= MIN(block_size, size);
    }
    read_block(inode->i_block[13], 0, (void *)bufofbuf, block_size);
    for(j=0;size>0 && j<256;j++){
        read_block(bufofbuf[j], 0, (void *)buf, block_size);
        for(i=0;size>0 && i<256;i++){
            print_block(buf[i], MIN(block_size, size));
            size -= MIN(block_size, size);
        }
    }
}

uint32_t check_dir_block(uint32_t block, char * dirName){
    struct ext2_dir_entry *dirEnt;
    uint8_t buffer[BLOCK_SIZE];
    uint16_t nameLen = strlen(dirName);
     
    read_block(block, 0, (void *)buffer, block_size); 
    dirEnt=(void *)buffer;
    while((((void *)dirEnt) - ((void *)buffer)) < block_size){
        if(dirEnt->name_len == nameLen){
            if(!strncmp(dirEnt->name, dirName, nameLen)){
                return dirEnt->inode; 
            }
        }
        dirEnt = (struct ext2_dir_entry *) (dirEnt->rec_len + ((void *)dirEnt));
   } 
   return 0;
}

void follow_path(uint32_t inodeNum, char *dirName){
    struct ext2_inode inode;
    uint32_t blockNum;
    uint32_t nextInode;
    char *nextName;
    
    read_inode(inodeNum, &inode); 
    blockNum = 0;
    nextInode = 0;
    while(blockNum*BLOCK_SIZE < inode.i_size){
        nextInode = check_dir_block(inode.i_block[blockNum], dirName);
        if(nextInode){
            nextName = strtok(NULL, "/");
            if(nextName){
                follow_path(nextInode, nextName);
            }else{
               print_inode(nextInode); 
            }
            break;
        }
        blockNum++;
    }
    if(!nextInode){
        printf("file not found\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
}
*/
