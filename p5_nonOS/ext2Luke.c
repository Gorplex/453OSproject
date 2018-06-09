#include "ext2Luke.h"


/* int main(int argc, char * argv[]) { */
/*    uint8_t buf[1024]; */

/*    int inode_num =2; */

/*    if( ((fp = fopen(argv[1], "r")) ==NULL) &&((fp = fopen(argv[2], "r")) ==NULL)) { */
/*       printf("file not found\n"); */
/*       exit(0); */
/*    } */

/*    if(argc == 4 && (strcmp(argv[1],"-l")==0)) { */
/*       inode_num = find_file(argv[3]); */
/*       print_file(inode_num); */
/*       return 0; */
/*    } else if(argc == 3) { */
/*       inode_num = find_file(argv[2]); */
/*    } else if( argc == 2) { */
/*       inode_num = 2; */
/*    } else{ */
/*       fprintf(stderr, "USAGE: %s image.ext2 [dir/file]\n", argv[0]); */
/*       exit(-1); */
/*    } */

/*    if( get_file_inode( inode_num, buf) == 0) { */
/*       printf("file not found\n"); */
/*       return 0; */
/*    }	  */
/*    print_dir( (struct ext2_inode *) buf); */
   

/*    fclose(fp); */
/*    return 0; */
/* } */
int cmpstr(void const *a, void const *b) { 
    char const *aa = *(char const **)a;
    char const *bb = *(char const **)b;

    return strcmp(aa, bb);
}


int get_file( int inode_num, char * fname) {
   /* returns inode number of the file */
   struct ext2_dir_entry * dirent;
   int i = 0;
   uint64_t dist=0;
   uint8_t inodeBuf[128];
   uint8_t blockIndex=0;
   uint8_t fsBlock[BLK_SIZE];
   uint8_t buf[BLK_SIZE];
   struct ext2_inode * inodeDir;
   
   get_file_inode( inode_num, buf);
   inodeDir = (struct ext2_inode *) buf;

   
   int fname_len = strlen(fname);
   
   get_fs_block(inodeDir->i_block[0],fsBlock);
   dirent = (struct ext2_dir_entry *) fsBlock;
   

   while(dist < inodeDir->i_size) {
      if(dist % BLK_SIZE == 0) {
      	 get_fs_block(inodeDir->i_block[blockIndex++],fsBlock);
      	 dirent = (struct ext2_dir_entry *) fsBlock;
      }

      dist+= dirent->rec_len;
      get_file_inode(dirent->inode, inodeBuf);

      if((dirent->name_len == fname_len) &&
	 (strncmp(dirent->name, fname,fname_len)==0)) {
	 return dirent->inode;
      }
      dirent = (struct ext2_dir_entry *) ((uint8_t *)dirent + dirent->rec_len);
      i++;	    
   }

   fprintf(stderr,"no file found, returning 0\n");
   printf("file not found\n");
   return 0;
   
}


int find_file( char * path) {
   char * token;
   int inode_num=2;
   token = strtok(path, "/");
   inode_num = get_file(inode_num, token);

   while( (token = strtok(NULL, "/")) != NULL) {
      inode_num = get_file(inode_num, token);
   }
   return inode_num;
}


void print_file(int inode_num) {
   struct ext2_inode * inode;
   uint8_t buf[BLK_SIZE];
   uint8_t fsBlock[BLK_SIZE];
   uint32_t indirect[BLK_SIZE/4];
   uint32_t doubly_indirect[BLK_SIZE/4];
   int size;
   get_file_inode( inode_num, buf);
   inode = (struct ext2_inode *) buf;

   size = inode->i_size;
   int i;
   for(i=0; i < 12; i++) {
      get_fs_block(inode->i_block[i],fsBlock);
      fwrite( fsBlock, sizeof(char), MIN(BLK_SIZE, size), stdout);
      size -= BLK_SIZE;
      if(size <= 0) return;
   }

   /* singly indirect */
   get_fs_block(inode->i_block[i],indirect);
   for(i=0; i < 256; i++) {
      get_fs_block(indirect[i],fsBlock);
      fwrite( fsBlock, sizeof(char), MIN(size,BLK_SIZE), stdout);
      size -= BLK_SIZE;
      if(size <= 0) return;
   }

   /* doubly indirect */
   get_fs_block(inode->i_block[13],doubly_indirect);
   int j;
   for(j=0; j < 256; j++) {

      get_fs_block(doubly_indirect[j],indirect);
      for(i=0; i < 256; i++) {
	 get_fs_block(indirect[i],fsBlock);
	 fwrite( fsBlock, sizeof(char), MIN(BLK_SIZE, size), stdout);
	 size -= BLK_SIZE;
	 if(size <= 0) return;
      }
   }
}



void print_dir( struct ext2_inode * inodeDir) {
   struct ext2_dir_entry * dirent;
   int i = 0;
   uint64_t dist=0;
   uint8_t inodeBuf[128];
   uint8_t blockIndex=0;
   /* char * output[256]; */
   uint8_t fsBlock[BLK_SIZE];

   get_fs_block(inodeDir->i_block[0],fsBlock);
   dirent = (struct ext2_dir_entry *) fsBlock;
   
   struct ext2_inode * inode = (struct ext2_inode *) inodeBuf;

   while(dist < inodeDir->i_size) {
      if(dist % BLK_SIZE == 0) {
      	 get_fs_block(inodeDir->i_block[blockIndex++],fsBlock);
      	 dirent = (struct ext2_dir_entry *) fsBlock;
      }

      dist+= dirent->rec_len;
      get_file_inode(dirent->inode, inodeBuf);


      if(inode->i_mode & EXT2_S_IFDIR) {
	 /* sprintf( output[i],"  %-15.*s %10d %10s\r\n", */
	 /* 	   dirent->name_len, dirent->name, */
	 /* 	   0, "D" ); */
      } else {
	 print_stringn(dirent->name, dirent->name_len);
	 print_string("\r\n");
	 /* sprintf( output[i], "  %-15.*s %10d %10s\r\n", */
	 /* 	   dirent->name_len, dirent->name, */
	 /* 	   inode->i_size, "F"); */
      }


      dirent = (struct ext2_dir_entry *) ((uint8_t *)dirent + dirent->rec_len);
      i++;	    
   }

   
}




/* returns file size */
uint32_t get_file_inode( uint32_t inodeNum, uint8_t * inodeBuf) {
   struct ext2_inode * inode;

   uint32_t block = (inodeNum / 7696) * 8192 + 2; /* +1 for boot, +1 for super */
   uint32_t inodeTable = block +3;

   /* read_data */
   sdReadData( 2*(inodeTable+(((inodeNum-1) %7696)/8)) +(128 * (((inodeNum-1) %7696)%8)) /512,
	      (128 * (((inodeNum-1) %7696)%8)) %512,
	      inodeBuf,		/* change size of buf??? TODO */
	      sizeof(struct ext2_inode));

   inode = (struct ext2_inode *) (inodeBuf);


   
   return inode->i_size;
}

void get_fs_block(uint32_t block, uint8_t *buf) {
   /* read_data */
   sdReadData( 2 * block     , 0, buf,     512);
   /* read_data */
   sdReadData( 2 * block + 1 , 0, buf+512, 512);
}
   

/* /\* the block argument is in terms of SD card 512 byte sectors *\/ */
/* void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size) { */
/*    if (offset > 511) { */
/*       printf ("Offset (%d) greater than 511.\n",offset); */
/*       exit(0); */
/*    } */
/*    fseek(fp,block*512 + offset,SEEK_SET); */
/*    fread(data,size,1,fp); */
/* } */
