#include "ext2Reader.h"

uint32_t block_size;
uint32_t blocks_per_group;
uint32_t inodes_per_group;
uint32_t inode_table;

void read_block(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size) {
   if(offset < 512){
      //read first 1/2
      sdReadData(block*2, offset, data, MIN(size, 512-offset));
      //if size > ammount read
      if(size > 512-offset){
         sdReadData(block*2+1, 0, data+(512-offset), size-(512-offset));
      }
   }else{
      sdReadData(block*2+1, offset%512, data, MIN(size, 512));
   }
}

void read_super(){
   struct ext2_super_block su_blk;
   memset(&su_blk, 0, sizeof(struct ext2_super_block));

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

uint32_t readDirBlock(uint32_t block, uint16_t *curIndex, uint16_t *index, char *name, uint32_t *len, struct ext2_inode *inode){
   struct ext2_dir_entry *dirEnt;
   uint8_t buffer[BLOCK_SIZE];

   read_block(block, 0, (void *)buffer, block_size); 
   dirEnt=(struct ext2_dir_entry *)&buffer; 

   while((((uint8_t *)dirEnt) - ((uint8_t *)buffer)) < block_size && dirEnt->rec_len){
      if(*curIndex == *index){
         read_inode(dirEnt->inode, inode);
         if(inode->i_mode & EXT2_S_IFREG){
            *len = inode->i_size;
            name[dirEnt->name_len] = '\0';
            for(int i=0;i<dirEnt->name_len && i<255;i++){
               name[i]=dirEnt->name[i];
            }
            return dirEnt->inode;
         }
         *index = *index+1;
      }
      *curIndex = *curIndex+1;
      dirEnt = (struct ext2_dir_entry *) (dirEnt->rec_len + ((uint8_t *)dirEnt));
   }
   return 0;
}

void readRoot(uint16_t *index, char *name, uint32_t *len, struct ext2_inode *inode){
   struct ext2_inode root;
   uint16_t curIndex;
   uint16_t blockNum;
   uint32_t inodeNum;

   read_super();
   read_bgdt();
   read_inode(EXT2_ROOT_INO, &root);
   
   curIndex=0;
   blockNum=0;
   while(blockNum*block_size < 1){    //while(blockNum*block_size < root.i_size){
      if((inodeNum = readDirBlock(root.i_block[blockNum], &curIndex, index, name, len, inode))){
         break;      
      }
      blockNum++;
   }
}

void readFile(struct ext2_inode inode, uint32_t bufNum, uint8_t *buf){
   static int32_t links[256];
   

   if(bufNum/4 < 12){
      read_block(inode.i_block[bufNum/4], (bufNum%4)*READ_BUF_SIZE, 
         (buf)+(bufNum%2)*READ_BUF_SIZE, READ_BUF_SIZE);
   }else if(bufNum/4 >= 12){
      if(bufNum == 12*4){
         //first read of links
         read_block(inode.i_block[12], 0, (uint8_t *)links, block_size);
      }
      read_block(links[bufNum/4-12], (bufNum%4)*READ_BUF_SIZE, 
         buf+(bufNum%2)*READ_BUF_SIZE, READ_BUF_SIZE);
   }
   //NO DOUBLE LINKS
}

