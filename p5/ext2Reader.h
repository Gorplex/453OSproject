#ifndef ext2Reader_h
#define ext2Reader_h

uint32_t readRoot(uint16_t *index, char *name);
void readInodeData(uint32_t inodeNum);

#endif

