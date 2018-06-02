#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h> 

#include "ext2.h"
#include "SdReader.h"

#define BLOCK_SIZE 1024
#define EXT2_BGDT 2
#define MAX_FILES 256

#define MIN(a,b) a<b?a:b
#define MAX(a,b) a>b?a:b

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

void format_inode(uint32_t inodeNum, uint16_t len, char *name, char **output){
    struct ext2_inode inode;
    
    read_inode(inodeNum, &inode);
    if(inode.i_mode & EXT2_S_IFREG){
        asprintf(output, "%-20.*s\t%-10d\tF\n", len, name, inode.i_size);
    }else{
        asprintf(output, "%-20.*s\t0\t\tD\n", len, name);
    }
}

void read_dir_block(uint32_t block, uint8_t * numLines, char *lines[]){
    struct ext2_dir_entry *dirEnt;
    //struct ext2_dir_entry *l;
    uint8_t buffer[BLOCK_SIZE];
    uint16_t num;

    //printf("TEST: %d\n", buffer[1000]);
    read_block(block, 0, (void *)buffer, block_size); 
    //printf("TEST: %d\n", buffer[1000]);
    //printf("TEST: %.*s\n",9, buffer+340-12);
    
    dirEnt=(void *)buffer;
    //printf("LEN2: %d\n", dirEnt->rec_len);
    while((((void *)dirEnt) - ((void *)buffer)) < block_size && dirEnt->rec_len){
        //printf("NumLines: %d\n", *numLines);
        //printf("TEST: %.*s\n",9, buffer+340-12);
        format_inode(dirEnt->inode, dirEnt->name_len, 
            dirEnt->name, &lines[(*numLines)++]);
        //printf("TEST: %.*s\n",9, buffer+340-12);
        //printf("LEN: %d\n", dirEnt->rec_len);
        //printf("LENN: %d\n", dirEnt->name_len);
        //l=dirEnt;
        //printf("inode %.*s: %d\n", dirEnt->name_len, dirEnt->name, dirEnt->inode);
        dirEnt = (struct ext2_dir_entry *) (dirEnt->rec_len + ((void *)dirEnt));
        //printf("DIF: %d\n", ((void *)dirEnt)-((void *)buffer));
        //printf("PTR: %p\n\n", dirEnt);
        //printf("buf: %p\n\n", buffer);
        //printf("\n");
    }
}

int string_cmp(const void *a, const void *b){
    return strcmp(*((const char **)a),*((const char **)b));
}

void print_block(uint32_t block, uint16_t len){
    uint8_t buf[BLOCK_SIZE];
    read_block(block, 0, (void *)buf, len);
    fwrite(buf, 1, len, stdout);
}

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

void print_dir(struct ext2_inode *inode){
    char * lines[MAX_FILES];
    uint8_t numLines;
    uint32_t blockNum;
    uint16_t i;

    numLines = 0;
    blockNum=0;
    while(blockNum*block_size < inode->i_size){
        read_dir_block(inode->i_block[blockNum], &numLines, lines);
        blockNum++;
    }
    qsort(lines, numLines, sizeof(char *), string_cmp);
    printf("name\t\t\tsize\t\ttype\n");
    for(i=0;i<numLines;i++){
        printf(lines[i]);
        free(lines[i]);
    }
}

void print_inode(uint32_t inodeNum){
    struct ext2_inode inode;
   
    read_inode(inodeNum, &inode);
    if(inode.i_mode & EXT2_S_IFREG){
        //fprintf(stderr, "ext2reader: Not a directory\n");
        //fclose(fp);
        //exit(EXIT_FAILURE);
        print_file(&inode);
    }else{
        print_dir(&inode);    
    }
}

void read_bgdt(){
    struct ext2_group_desc bgdt;
    read_block(2,0, (void *) &bgdt, sizeof(struct ext2_group_desc));
    inode_table = bgdt.bg_inode_table;
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
/*
void main(int argc, char **argv){
    char *dirName;

    checkArgs(argc, argv);
    read_super();
    read_bgdt();

    if(argc == 2){
        print_inode(EXT2_ROOT_INO); 
    }else if(argc == 3){
        
        dirName = strtok(argv[2], "/");
        if(dirName){
            follow_path(EXT2_ROOT_INO, dirName);
        }else{
            print_inode(EXT2_ROOT_INO); 
        }
    }else{
        dirName = strtok(argv[3], "/");
        if(dirName){
            follow_path(EXT2_ROOT_INO, dirName);        
        }
    }
    fclose(fp);
}
*/
