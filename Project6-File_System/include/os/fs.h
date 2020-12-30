#ifndef FS_H
#define FS_H

#include <type.h>

#define MAGIC 0x200006042018fffflu

#define SUPERBLOCK_CACHE 0xffffffc05f000000lu
#define INODEMAP_CACHE   0xffffffc05f000200lu
#define BLOCKMAP_CACHE   0xffffffc05f000400lu
#define INODE_CACHE      0xffffffc05f008400lu
#define BLOCK_CACHE      0xffffffc05f008600lu
#define FS_CACHE_END     0xffffffc05f009600lu

// 512 bytes
typedef struct superblock{
    uint64_t magic;
    uint64_t superblock_id;
    uint64_t superblock_num;
    uint64_t blockmap_id;
    uint64_t blockmap_num;
    uint64_t inodemap_id;
    uint64_t inodemap_num;
    uint64_t inode_id;
    uint64_t inode_num;
    uint64_t block_id;
    uint64_t block_num;
    uint64_t root_inode_id;
    uint64_t root_block_id;
    uint64_t used_inode_num;
    uint64_t used_block_num;
    uint8_t empty[392];
} superblock_t;

// 64 bytes
typedef struct inode{
    uint32_t mode;
    uint32_t owner;
    uint32_t size;
    uint32_t direct_blocks[10];
    uint32_t indirect_blocks[2];
    uint32_t empty;
} inode_t;

#define EMPTY 0
#define DIR   1
#define FILE  2
// 32 bytes
typedef struct dir_entry{
    uint8_t type;
    uint8_t name[29];
    uint16_t inode;
} dir_entry_t;
// 4096 bytes
typedef struct dir{
    dir_entry_t entry[128];
} dir_t;

extern volatile superblock_t* superblock;
extern volatile uintptr_t blockmap;
extern volatile uintptr_t inodemap;
extern volatile uintptr_t cached_inode_base;
extern volatile uintptr_t cached_block_base;
extern volatile uint16_t cached_inode_id;
extern volatile uint32_t cached_block_id;

extern uint16_t current_dir_inode;
extern uint32_t current_dir_block;

extern int mkfs(int print);

extern int do_mkfs(int mode, int* print_location_y);
extern void do_statfs(int* print_location_y);
extern int do_mkdir(char* dirname);
extern int do_rmdir(char* dirname);
extern int do_ls(char* dirname, int mode, int* print_location_y);
extern int do_cd(char* dirname);


#endif /* FS_H */