#ifndef EXT2LUKE_H
#define EXT2LUKE_H

#define _GNU_SOURCE         /* See feature_test_macros(7) */

#include <sys/types.h>
/* #include <sys/stat.h> */
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ext2.h"
#include "SdReader.h"
#include "serial.h"

#define BLK_SIZE 1024

#define MIN(a,b) ((a<b)? a : b)

void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size);
void get_fs_block(uint32_t block, uint8_t *buf);
void print_dir( struct ext2_inode * inode);
void print_file(int inode_num);
uint32_t get_file_inode( uint32_t inodeNum, uint8_t * buf);
int get_file( int inode_num, char * fname);
int find_file( char * path);
FILE * fp;




#endif
