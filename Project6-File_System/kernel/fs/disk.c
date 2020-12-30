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
    sbi_sd_read(kva2pa(BLOCKMAP_CACHE), 64, BLOCKMAP_BEGIN_ID);
}

void write_blockmap()
{
    sbi_sd_write(kva2pa(BLOCKMAP_CACHE), 64, BLOCKMAP_BEGIN_ID);
}

void read_inodemap()
{
    sbi_sd_read(kva2pa(INODEMAP_CACHE), 1, INODEMAP_ID);
}

void write_inodemap()
{
    sbi_sd_write(kva2pa(INODEMAP_CACHE), 1, INODEMAP_ID);
}

uintptr_t read_inode(uint16_t inode_id)
{
    uint64_t sd_block_id = INODE_BEGIN_ID + inode_id / 8;
    sbi_sd_read(kva2pa(INODE_CACHE), 1, sd_block_id);
    cached_inode_id = inode_id;
    uintptr_t ret = INODE_CACHE + 64 * (inode_id % 8);
    return ret;
}

void write_inode(uint16_t inode_id)
{
    uint64_t sd_block_id = INODE_BEGIN_ID + inode_id / 8;
    sbi_sd_write(kva2pa(INODE_CACHE), 1, sd_block_id);
}

void read_block(uint32_t block_id)
{
    uint64_t sd_block_id = BLOCK_BEGIN_ID + block_id * 8;
    sbi_sd_read(kva2pa(BLOCK_CACHE), 8, sd_block_id);
    cached_block_id = block_id;
}

void write_block(uint32_t block_id)
{
    uint64_t sd_block_id = BLOCK_BEGIN_ID + block_id * 8;
    sbi_sd_write(kva2pa(BLOCK_CACHE), 8, sd_block_id);
}

// Returns inode id
uint16_t alloc_inode()
{
    uint16_t ret = 0xffff;
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
            read_superblock();
            superblock->used_inode_num = superblock->used_inode_num + 1;
            write_superblock();
            return ret;
        }
    }
    return ret;
}

// Returns block id
uint32_t alloc_block()
{
    uint32_t ret = 0xffffffff;
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
        if(ret != 0xffffffff){
            write_blockmap();
            read_superblock();
            superblock->used_block_num = superblock->used_block_num + 1;
            write_superblock();
            return ret;
        }
    }
    return ret;
}

void free_inode(uint16_t inode_id)
{
    read_inodemap();
    uint8_t byte = *(((uint8_t*)inodemap) + inode_id / 8);
    int i = inode_id % 8;
    if(byte & 0x01){
        byte = byte & ~(0x01);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    else if(byte & 0x02){
        byte = byte & ~(0x02);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    else if(byte & 0x04){
        byte = byte & ~(0x04);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    else if(byte & 0x08){
        byte = byte & ~(0x08);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    else if(byte & 0x10){
        byte = byte & ~(0x10);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    else if(byte & 0x20){
        byte = byte & ~(0x20);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    else if(byte & 0x40){
        byte = byte & ~(0x40);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    else if(byte & 0x80){
        byte = byte & ~(0x80);
        *(((uint8_t*)inodemap) + i) = byte;
    }
    write_inodemap();
    read_superblock();
    superblock->used_inode_num = superblock->used_inode_num - 1;
    write_superblock();
}

void free_block(uint32_t block_id)
{
    read_blockmap();
    uint8_t byte = *(((uint8_t*)blockmap) + block_id / 8);
    int i = block_id % 8;
    if(byte & 0x01){
        byte = byte & ~(0x01);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    else if(byte & 0x02){
        byte = byte & ~(0x02);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    else if(byte & 0x04){
        byte = byte & ~(0x04);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    else if(byte & 0x08){
        byte = byte & ~(0x08);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    else if(byte & 0x10){
        byte = byte & ~(0x10);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    else if(byte & 0x20){
        byte = byte & ~(0x20);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    else if(byte & 0x40){
        byte = byte & ~(0x40);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    else if(byte & 0x80){
        byte = byte & ~(0x80);
        *(((uint8_t*)blockmap) + i) = byte;
    }
    write_blockmap();
    read_superblock();
    superblock->used_block_num = superblock->used_block_num - 1;
    write_superblock();
}