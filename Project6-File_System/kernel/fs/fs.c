#include <os/fs.h>
#include <os/disk.h>
#include <os/mm.h>

volatile superblock_t* superblock = SUPERBLOCK_CACHE;
volatile uintptr_t blockmap = BLOCKMAP_CACHE;
volatile uintptr_t inodemap = INODEMAP_CACHE;
volatile uintptr_t cached_inode_base = INODE_CACHE;
volatile uintptr_t cached_data_base = BLOCK_CACHE;
volatile uint16_t cached_inode_id;
volatile uint32_t cached_block_id;

int mkfs()
{
    // Read superblock and check magic
    read_superblock();
    if(superblock->magic == MAGIC){
        return 0;
    }

    // Form root dir
    uint16_t inode_id = alloc_inode();
    uint32_t block_id = alloc_block();
    inode_t* cached_inode = read_inode(inode_id);
    read_block(block_id);
    dir_t* cached_data = cached_data_base;

    cached_inode->mode = 0;
    cached_inode->owner = 1;
    cached_inode->size = 4096;
    cached_inode->direct_blocks[0] = block_id;
    write_inode(inode_id);

    cached_data->entry[0].type = DIR;
    strcpy(&cached_data->entry[0].name, "..");
    cached_data->entry[0].inode = inode_id;
    write_block(block_id);

    // Write superblock
    superblock->magic = MAGIC;
    superblock->superblock_id = SUPERBLOCK_ID;
    superblock->superblock_num = 1;
    superblock->blockmap_id = BLOCK_MAP_BEGIN;
    superblock->blockmap_num = 8;
    superblock->inodemap_id = INODE_MAP_ID;
    superblock->inodemap_num = 1;
    superblock->inode_id = INODE_BEGIN;
    superblock->indoe_num = 512;
    superblock->data_id = DATA_BEGIN;
    superblock->data_num = 2097152;
    superblock->root_inode_id = inode_id;
    superblock->root_block_id = block_id;
    write_superblock();

    // Clear block map
    memset(blockmap, 0, 512 * 64);
    write_blockmap();

    // Clear inode map
    memset(inodemap, 0, 512);
    write_inodemap();

    return 1;
}
