#ifndef DISK_H
#define DISK_H

#include <type.h>

/* * * * * * * * * * * * * * * * * * * * * * * 
Disk Layout
    0 -     2,047 (    2,048): Kernel Image
2,048 -     4,095 (    2,048): Swap
4,096 -     4,096 (        1): Super Block
4,097 -     4,160 (       64): Block Map
4,161 -     4,161 (        1): Inode Map
4,162 -     4,673 (      512): Inode
5,120 - 2,102,271 (2,097,152): Data
* * * * * * * * * * * * * * * * * * * * * * */

#define SUPERBLOCK_ID   4096
#define BLOCK_MAP_BEGIN 4097
#define INODE_MAP_ID    4161
#define INODE_BEGIN     4162
#define DATA_BEGIN      5120

extern void read_superblock();
extern void write_superblock();
extern void read_blockmap();
extern void write_blockmap();
extern void read_inodemap();
extern void write_inodemap();
extern uintptr_t read_inode(uint16_t inode_id);
extern void write_inode(uint16_t inode_id);
extern void read_block(uint32_t block_id);
extern void write_block(uint32_t block_id);

extern uint16_t alloc_inode();
extern uint32_t alloc_block();

#endif /* DISK_H */