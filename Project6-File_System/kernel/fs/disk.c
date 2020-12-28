#include <os/fs.h>
#include <os/disk.h>
#include <sbi.h>
#include <pgtable.h>

void read_superblock()
{
    sbi_sd_read(kva2pa(SUPERBLOCK_CACHE), 1, SUPERBLOCK_ID);
}

void write_superblock()
{
    sbi_sd_write(kva2pa(SUPERBLOCK_CACHE), 1, SUPERBLOCK_ID);
}

void read_blockmap()
{
    sbi_sd_read(kva2pa(BLOCKMAP_CACHE), 64, BLOCK_MAP_BEGIN);
}

void write_blockmap()
{
    sbi_sd_write(kva2pa(BLOCKMAP_CACHE), 64, BLOCK_MAP_BEGIN);
}

void read_inodemap()
{
    sbi_sd_read(kva2pa(INODEMAP_CACHE), 1, INODE_MAP_ID);
}

void write_inodemap()
{
    sbi_sd_write(kva2pa(INODEMAP_CACHE), 1, INODE_MAP_ID);
}

uintptr_t read_inode(uint16_t inode_id)
{
    uint64_t sd_block_id = INODE_BEGIN + inode_id / 8;
    sbi_sd_read(kva2pa(INODE_CACHE), 1, sd_block_id);
    cached_inode_id = inode_id;
    uintptr_t ret = INODE_CACHE + 64 * (inode_id % 8);
    return ret;
}

void write_inode(uint16_t inode_id)
{
    uint64_t sd_block_id = INODE_BEGIN + inode_id / 8;
    sbi_sd_write(kva2pa(INODE_CACHE), 1, sd_block_id);
}

void read_block(uint32_t block_id)
{
    uint64_t sd_block_id = DATA_BEGIN + block_id * 8;
    sbi_sd_read(kva2pa(BLOCK_CACHE), 8, sd_block_id);
    cached_block_id = block_id;
}

void write_block(uint32_t block_id)
{
    uint64_t sd_block_id = DATA_BEGIN + block_id * 8;
    sbi_sd_write(kva2pa(BLOCK_CACHE), 8, sd_block_id);
}

// Returns inode id
uint16_t alloc_inode()
{
    uint16_t ret = 0xff;
    read_inodemap();
    for(int i = 0; i < 512; i++){
        uint8_t byte = *(((uint8_t*)inodemap) + i);
        if((byte & 0x01) == 0x00){
            ret = 8 * i + 0;
            byte = byte | 0x01;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        else if((byte & 0x02) == 0x00){
            ret = 8 * i + 1;
            byte = byte | 0x02;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        else if((byte & 0x04) == 0x00){
            ret = 8 * i + 2;
            byte = byte | 0x04;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        else if((byte & 0x08) == 0x00){
            ret = 8 * i + 3;
            byte = byte | 0x08;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        else if((byte & 0x10) == 0x00){
            ret = 8 * i + 4;
            byte = byte | 0x10;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        else if((byte & 0x20) == 0x00){
            ret = 8 * i + 5;
            byte = byte | 0x20;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        else if((byte & 0x40) == 0x00){
            ret = 8 * i + 6;
            byte = byte | 0x40;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        else if((byte & 0x80) == 0x00){
            ret = 8 * i + 7;
            byte = byte | 0x80;
            *(((uint8_t*)inodemap) + i) = byte;
        }
        if(ret != 0xff){
            write_inodemap();
            return ret;
        }
    }
    return ret;
}

// Returns block id
uint32_t alloc_block()
{
    uint32_t ret = 0xffff;
    read_blockmap();
    for(int i = 0; i < 64 * 512; i++){
        uint8_t byte = *(((uint8_t*)blockmap) + i);
        if((byte & 0x01) == 0x00){
            ret = 8 * i + 0;
            byte = byte | 0x01;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        else if((byte & 0x02) == 0x00){
            ret = 8 * i + 1;
            byte = byte | 0x02;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        else if((byte & 0x04) == 0x00){
            ret = 8 * i + 2;
            byte = byte | 0x04;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        else if((byte & 0x08) == 0x00){
            ret = 8 * i + 3;
            byte = byte | 0x08;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        else if((byte & 0x10) == 0x00){
            ret = 8 * i + 4;
            byte = byte | 0x10;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        else if((byte & 0x20) == 0x00){
            ret = 8 * i + 5;
            byte = byte | 0x20;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        else if((byte & 0x40) == 0x00){
            ret = 8 * i + 6;
            byte = byte | 0x40;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        else if((byte & 0x80) == 0x00){
            ret = 8 * i + 7;
            byte = byte | 0x80;
            *(((uint8_t*)blockmap) + i) = byte;
        }
        if(ret != 0xffff){
            write_blockmap();
            return ret;
        }
    }
    return ret;
}