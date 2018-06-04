#ifndef ext2Reader_h
#define ext2Reader_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

#include "ext2.h"
#include "SdReader.h"


#define BLOCK_SIZE 1024
#define EXT2_BGDT 2
#define MAX_FILES 256
#define READ_BUF_SIZE 256

#define MIN(a,b) a<b?a:b
#define MAX(a,b) a>b?a:b


uint32_t readRoot(uint16_t *index, char *name);
void readFile(uint32_t inodeNum, uint32_t bufNum, uint8_t *buf);

#endif

