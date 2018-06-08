#ifndef ext2Reader_h
#define ext2Reader_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

#include "ext2.h"
#include "SdReader.h"
#include "serial.h"

#define BLOCK_SIZE 1024
#define EXT2_BGDT 2
#define MAX_FILES 256
#define READ_BUF_SIZE 256

#define MIN(a,b) a<b?a:b
#define MAX(a,b) a>b?a:b


void readRoot(uint16_t *index, char *name, uint32_t *len, struct ext2_inode *inode);
void readFile(struct ext2_inode inode, uint32_t bufNum, uint8_t *buf);

#endif

