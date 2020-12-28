#ifndef FS_H
#define FS_H

#include <type.h>

#define MAGIC 0x2000060420000604lu

#define SUPERBLOCK_CACHE 0xffffffc05f000000lu
#define INODEMAP_CACHE   0xffffffc05f000200lu
#define BLOCKMAP_CACHE   0xffffffc05f000400lu
#define INODE_CACHE      0xffffffc05f008400lu
#define BLOCK_CACHE      0xffffffc05f008600lu

typedef struct superblock{
    uint64_t magic;
    uint64_t superblock_id;
    uint64_t superblock_num;
    uint64_t blockmap_id;
    uint64_t blockmap_num;
    uint64_t inodemap_id;
    uint64_t inodemap_num;
    uint64_t inode_id;
    uint64_t indoe_num;
    uint64_t data_id;
    uint64_t data_num;
    uint64_t root_inode_id;
    uint64_t root_block_id;
    uint8_t empty[408];
} superblock_t;

typedef struct inode{
    uint8_t mode;
    uint32_t owner;
    uint32_t size;
    uint32_t direct_blocks[10];
    uint32_t indirect_blocks[2];
    uint8_t empty[4];
} inode_t;

#define EMPTY 0
#define DIR 1
#define FILE 2
typedef struct dir_entry{
    uint8_t type;
    uint8_t name[29];
    uint16_t inode;
} dir_entry_t;
typedef struct dir{
    dir_entry_t entry[128];
} dir_t;

extern volatile superblock_t* superblock;
extern volatile uintptr_t blockmap;
extern volatile uintptr_t inodemap;
extern volatile uintptr_t cached_inode_base;
extern volatile uintptr_t cached_data_base;
extern volatile uint16_t cached_inode_id;
extern volatile uint32_t cached_block_id;

extern int mkfs();

#endif /* FS_H */