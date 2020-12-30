#include <os/fs.h>
#include <os/disk.h>
#include <os/mm.h>
#include <os/sched.h>
#include <os/irq.h>

volatile superblock_t* superblock = SUPERBLOCK_CACHE;
volatile uintptr_t blockmap = BLOCKMAP_CACHE;
volatile uintptr_t inodemap = INODEMAP_CACHE;
volatile uintptr_t cached_inode_base = INODE_CACHE;
volatile uintptr_t cached_block_base = BLOCK_CACHE;
volatile uint16_t cached_inode_id;
volatile uint32_t cached_block_id;

uint16_t current_dir_inode;
uint32_t current_dir_block;

int mkfs(int print)
{
    // Read superblock and check magic
    read_superblock();
    if(superblock->magic == MAGIC){
        current_dir_inode = superblock->root_inode_id;
        current_dir_block = superblock->root_block_id;
        return 0;
    }

    // Clear block map
    if(print){
        prints("[FS] Setting inode-map...\n");
    }
    memset(blockmap, 0, 512 * 64);
    write_blockmap();

    // Clear inode map
    if(print){
        prints("[FS] Setting block-map...\n");
    }
    memset(inodemap, 0, 512);
    write_inodemap();

    // Form root dir
    if(print){
        prints("[FS] Setting root dir...\n");
    }
    uint16_t inode_id = alloc_inode();
    uint32_t block_id = alloc_block();

    inode_t* root_inode = read_inode(inode_id);
    root_inode->mode = 0;
    root_inode->owner = 0;
    root_inode->size = 4096;
    root_inode->direct_blocks[0] = block_id;
    write_inode(inode_id);

    read_block(block_id);
    memset(cached_block_base, 0, 4096);
    dir_t* root_dir = cached_block_base;
    root_dir->entry[0].type = DIR;
    strcpy(root_dir->entry[0].name, ".");
    root_dir->entry[0].inode = inode_id;
    root_dir->entry[1].type = DIR;
    strcpy(root_dir->entry[1].name, "..");
    root_dir->entry[1].inode = inode_id;
    write_block(block_id);

    current_dir_inode = inode_id;
    current_dir_block = block_id;

    // Write superblock
    if(print){
        prints("[FS] Setting superblock...\n");
        prints("     magic: 0x%lx\n", MAGIC);
        prints("     superblock: %d (%d)\n", SUPERBLOCK_ID, 1);
        prints("     block-map:  %d (%d)\n", BLOCKMAP_BEGIN_ID, 64);
        prints("     inode-map:  %d (%d)\n", INODEMAP_ID, 1);
        prints("     inode    :  %d (%d)\n", INODE_BEGIN_ID, 512);
        prints("     block    :  %d (%d)\n", BLOCK_BEGIN_ID, 2097152);
        prints("     inode entry size: %dB    dir entry size: %dB\n", sizeof(inode_t), sizeof(dir_entry_t));
    }
    superblock->magic = MAGIC;
    superblock->superblock_id = SUPERBLOCK_ID;
    superblock->superblock_num = 1;
    superblock->blockmap_id = BLOCKMAP_BEGIN_ID;
    superblock->blockmap_num = 64;
    superblock->inodemap_id = INODEMAP_ID;
    superblock->inodemap_num = 1;
    superblock->inode_id = INODE_BEGIN_ID;
    superblock->inode_num = 512;
    superblock->block_id = BLOCK_BEGIN_ID;
    superblock->block_num = 2097152;
    superblock->root_inode_id = inode_id;
    superblock->root_block_id = block_id;
    superblock->used_inode_num = 1;
    superblock->used_block_num = 1;
    write_superblock();

    return 1;
}

int do_mkfs(int mode, int* print_location_y)
{
    if(mode == 1){
        superblock->magic = 0x0;
        write_superblock();
    }

    if(superblock->magic == MAGIC){
        return 0;
    
    }

    mkfs(1);
    enable_sum();
    *print_location_y = *print_location_y + 11;
    disable_sum();
    return 1;
}

void do_statfs(int* print_location_y)
{
    read_superblock();
    prints("magic: 0x%lx\n", MAGIC);
    prints("used inode: %d/%d\n", superblock->used_inode_num, superblock->inode_num);
    prints("used block: %d/%d\n", superblock->used_block_num, superblock->block_num);
    prints("inode entry size: %dB    dir entry size: %dB\n", sizeof(inode_t), sizeof(dir_entry_t));
    enable_sum();
    *print_location_y = *print_location_y + 4;
    disable_sum();
}

int do_mkdir(char* dirname)
{
    enable_sum();

    int dirname_pos = 0;
    dir_t* cur_dir;
    uint16_t lastdir_inode;
    uint32_t lastdir_block;

    if(dirname[dirname_pos] == '/'){
        read_superblock();
        read_block(superblock->root_block_id);
        cur_dir = cached_block_base;
        lastdir_inode = superblock->root_inode_id;
        lastdir_block = superblock->root_block_id;
        dirname_pos++;    
    }
    else{
        read_block(current_dir_block);
        cur_dir = cached_block_base;
        lastdir_inode = current_dir_inode;
        lastdir_block = current_dir_block;
    }

    char nextdirname[29];
    while(1){
        // Get name of next dir
        int nextdirname_pos = 0;
        while(dirname[dirname_pos] != '/' && dirname[dirname_pos] != 0){
            nextdirname[nextdirname_pos++] = dirname[dirname_pos++];
        }
        nextdirname[nextdirname_pos] = 0;

        // Last?
        if(dirname[dirname_pos] == 0 || 
           (dirname[dirname_pos + 1] == 0 && dirname[dirname_pos] == '/')){
            break;
        }

        // Search
        int i = 0;
        uint16_t nextdir_inode_id;
        uint32_t nextdir_block_id;
        for(i = 0; i < 128; i++){
            if(cur_dir->entry[i].type == DIR && !strcmp(cur_dir->entry[i].name, nextdirname)){
                nextdir_inode_id = cur_dir->entry[i].inode;
                inode_t* nextdir_inode = read_inode(nextdir_inode_id);
                nextdir_block_id = nextdir_inode->direct_blocks[0];
                lastdir_inode = nextdir_inode_id;
                lastdir_block = nextdir_block_id;
                break;
            }
        }
        if(i == 128){
            return 0;
        }

        dirname_pos++;
        read_block(nextdir_block_id);
        cur_dir = cached_block_base;
    }

    read_block(lastdir_block);
    cur_dir = cached_block_base;
    for(int i = 0; i < 128; i++){
        if(cur_dir->entry[i].type == DIR && !strcmp(cur_dir->entry[i].name, nextdirname)){
            disable_sum();
            return 0;
        }
    }

    for(int i = 0; i < 128; i++){
        if(cur_dir->entry[i].type == EMPTY){
            uint16_t inode_id = alloc_inode();
            uint32_t block_id = alloc_block();

            // Modify Current Dir
            cur_dir->entry[i].type = DIR;
            strcpy(cur_dir->entry[i].name, nextdirname);
            cur_dir->entry[i].inode = inode_id;
            write_block(lastdir_block);

            // Modify New Inode
            inode_t* new_inode = read_inode(inode_id);
            new_inode->mode = 0;
            new_inode->owner = current_running[cpu_id]->pid;
            new_inode->size = 4096;
            new_inode->direct_blocks[0] = block_id;
            write_inode(inode_id);
            
            //Modify New Dir
            read_block(block_id);
            memset(cached_block_base, 0, 4096);
            dir_t* new_dir = cached_block_base;
            new_dir->entry[0].type = DIR;
            strcpy(new_dir->entry[0].name, ".");
            new_dir->entry[0].inode = inode_id;
            new_dir->entry[1].type = DIR;
            strcpy(new_dir->entry[1].name, "..");
            new_dir->entry[1].inode = lastdir_inode;
            write_block(block_id);

            disable_sum();
            return 1;
        }
    }

    disable_sum();
    return 0;
}

int do_rmdir(char* dirname)
{
    enable_sum();

    int dirname_pos = 0;
    dir_t* cur_dir;
    uint16_t lastdir_inode;
    uint32_t lastdir_block;

    if(dirname[dirname_pos] == '/'){
        read_superblock();
        read_block(superblock->root_block_id);
        cur_dir = cached_block_base;
        lastdir_block = superblock->root_block_id;
        lastdir_inode = superblock->root_inode_id;
        dirname_pos++;    
    }
    else{
        read_block(current_dir_block);
        cur_dir = cached_block_base;
        lastdir_block = current_dir_block;
        lastdir_inode = current_dir_inode;
    }

    char nextdirname[29];
    while(1){
        // Get name of next dir
        int nextdirname_pos = 0;
        while(dirname[dirname_pos] != '/' && dirname[dirname_pos] != 0){
            nextdirname[nextdirname_pos++] = dirname[dirname_pos++];
        }
        nextdirname[nextdirname_pos] = 0;

        // Last?
        if(dirname[dirname_pos] == 0 || 
           (dirname[dirname_pos + 1] == 0 && dirname[dirname_pos] == '/')){
            break;
        }

        // Search
        int i = 0;
        uint16_t nextdir_inode_id;
        uint32_t nextdir_block_id;
        for(i = 0; i < 128; i++){
            if(cur_dir->entry[i].type == DIR && !strcmp(cur_dir->entry[i].name, nextdirname)){
                nextdir_inode_id = cur_dir->entry[i].inode;
                inode_t* nextdir_inode = read_inode(nextdir_inode_id);
                nextdir_block_id = nextdir_inode->direct_blocks[0];
                lastdir_inode = nextdir_inode_id;
                lastdir_block = nextdir_block_id;
                break;
            }
        }
        if(i == 128){
            return 0;
        }

        dirname_pos++;
        read_block(nextdir_block_id);
        cur_dir = cached_block_base;
    }

    read_block(lastdir_block);
    cur_dir = cached_block_base;
    for(int i = 0; i < 128; i++){
        if(cur_dir->entry[i].type == DIR && !strcmp(cur_dir->entry[i].name, nextdirname)){
            cur_dir->entry[i].type = EMPTY;
            uint16_t inode_id = cur_dir->entry[i].inode;
            inode_t* inode = read_inode(inode_id);
            int mode = inode->mode;
            uint32_t block_id = inode->direct_blocks[0];
            if(mode == 0){
                free_block(block_id);
            }
            free_inode(inode_id);
            write_block(lastdir_block);
            disable_sum();
            return 1;
        }
    }
    return 0;
}

int do_ls(char* dirname, int mode, int* print_location_y)
{
    enable_sum();

    int dirname_pos = 0;
    dir_t* cur_dir;
    uint32_t lastdir_block;

    if(dirname[dirname_pos] == '/'){
        read_superblock();
        read_block(superblock->root_block_id);
        cur_dir = cached_block_base;
        lastdir_block = superblock->root_block_id;
        dirname_pos++;    
    }
    else{
        read_block(current_dir_block);
        cur_dir = cached_block_base;
        lastdir_block = current_dir_block;
    }

    while(1){
        // Get name of next dir
        char nextdirname[29];
        int nextdirname_pos = 0;
        while(dirname[dirname_pos] != '/' && dirname[dirname_pos] != 0){
            nextdirname[nextdirname_pos++] = dirname[dirname_pos++];
        }
        nextdirname[nextdirname_pos] = 0;
        if(nextdirname_pos == 0){
            break;
        }

        // Search
        int i = 0;
        uint16_t nextdir_inode_id;
        uint32_t nextdir_block_id;
        for(i = 0; i < 128; i++){
            if(cur_dir->entry[i].type == DIR && !strcmp(cur_dir->entry[i].name, nextdirname)){
                nextdir_inode_id = cur_dir->entry[i].inode;
                inode_t* nextdir_inode = read_inode(nextdir_inode_id);
                nextdir_block_id = nextdir_inode->direct_blocks[0];
                break;
            }
        }
        if(i == 128){
            return 0;
        }

        // Last?
        if(dirname[dirname_pos] == 0 || 
           (dirname[dirname_pos + 1] == 0 && dirname[dirname_pos] == '/')){
            lastdir_block = nextdir_block_id;
            break;
        }
        else{
            dirname_pos++;
            read_block(nextdir_block_id);
            cur_dir = cached_block_base;
        }
    }

    read_block(lastdir_block);
    cur_dir = cached_block_base;
    if(mode == 0){
        for(int i = 0; i < 128; i++){
            if(cur_dir->entry[i].type != EMPTY){
                prints("%s    ", cur_dir->entry[i].name);
            }
        }
        prints("\n");
        enable_sum();
        *print_location_y = *print_location_y + 1;
    }
    else if(mode == 1){
        int count = 0;
        for(int i = 0; i < 128; i++){
            if(cur_dir->entry[i].type != EMPTY){
                uint16_t inode_id = cur_dir->entry[i].inode;
                inode_t* inode = read_inode(inode_id);
                prints("name: %s  type: %s  owner: %ld  size: %ldB\n", cur_dir->entry[i].name, 
                       cur_dir->entry[i].type == DIR ? "dir" : "file", inode->owner, inode->size);
                count++;
            }
        }
        enable_sum();
        *print_location_y = *print_location_y + count;
    }

    
    disable_sum();
    return 1;
}

int do_cd(char* dirname)
{
    enable_sum();

    int dirname_pos = 0;
    dir_t* cur_dir;
    uint16_t lastdir_inode;
    uint32_t lastdir_block;

    if(dirname[dirname_pos] == '/'){
        read_superblock();
        read_block(superblock->root_block_id);
        cur_dir = cached_block_base;
        lastdir_inode = superblock->root_inode_id;
        lastdir_block = superblock->root_block_id;
        dirname_pos++;    
    }
    else{
        read_block(current_dir_block);
        cur_dir = cached_block_base;
        lastdir_inode = current_dir_inode;
        lastdir_block = current_dir_block;
    }

    while(1){
        // Get name of next dir
        char nextdirname[29];
        int nextdirname_pos = 0;
        while(dirname[dirname_pos] != '/' && dirname[dirname_pos] != 0){
            nextdirname[nextdirname_pos++] = dirname[dirname_pos++];
        }
        nextdirname[nextdirname_pos] = 0;
        

        // Search
        int i = 0;
        uint16_t nextdir_inode_id;
        uint32_t nextdir_block_id;
        for(i = 0; i < 128; i++){
            if(cur_dir->entry[i].type == DIR && !strcmp(cur_dir->entry[i].name, nextdirname)){
                nextdir_inode_id = cur_dir->entry[i].inode;
                inode_t* nextdir_inode = read_inode(nextdir_inode_id);
                nextdir_block_id = nextdir_inode->direct_blocks[0];
                break;
            }
        }
        if(i == 128){
            return 0;
        }

        // Last?
        if(dirname[dirname_pos] == 0 || 
           (dirname[dirname_pos + 1] == 0 && dirname[dirname_pos] == '/')){
            lastdir_inode = nextdir_inode_id;
            lastdir_block = nextdir_block_id;
            break;
        }
        else{
            dirname_pos++;
            read_block(nextdir_block_id);
            cur_dir = cached_block_base;
        }
    }

    current_dir_inode = lastdir_inode;
    current_dir_block = lastdir_block;

    disable_sum();
    return 1;
}

int do_link(char* filename, char* newfile, int mode)
{
    enable_sum();  

    int filename_pos = 0;
    dir_t* cur_dir;
    uint16_t lastdir_inode;
    uint32_t lastdir_block;

    if(filename[filename_pos] == '/'){
        read_superblock();
        read_block(superblock->root_block_id);
        cur_dir = cached_block_base;
        lastdir_block = superblock->root_block_id;
        filename_pos++;    
    }
    else{
        read_block(current_dir_block);
        cur_dir = cached_block_base;
        lastdir_block = current_dir_block;
    }

    char nextdirname[29];
    while(1){
        // Get name of next dir
        int nextdirname_pos = 0;
        while(filename[filename_pos] != '/' && filename[filename_pos] != 0){
            nextdirname[nextdirname_pos++] = filename[filename_pos++];
        }
        nextdirname[nextdirname_pos] = 0;

        // Last?
        if(filename[filename_pos] == 0 || 
           (filename[filename_pos + 1] == 0 && filename[filename_pos] == '/')){
            break;
        }

        // Search
        int i = 0;
        uint16_t nextdir_inode_id;
        uint32_t nextdir_block_id;
        for(i = 0; i < 128; i++){
            if(cur_dir->entry[i].type == DIR && !strcmp(cur_dir->entry[i].name, nextdirname)){
                nextdir_inode_id = cur_dir->entry[i].inode;
                inode_t* nextdir_inode = read_inode(nextdir_inode_id);
                nextdir_block_id = nextdir_inode->direct_blocks[0];
                lastdir_inode = nextdir_inode_id;
                lastdir_block = nextdir_block_id;
                break;
            }
        }
        if(i == 128){
            disable_sum();
            return 0;
        }

        filename_pos++;
        read_block(nextdir_block_id);
        cur_dir = cached_block_base;
    }

    uint16_t inode_id;
    int type;
    read_block(lastdir_block);
    cur_dir = cached_block_base;
    for(int i = 0; i < 128; i++){
        if(cur_dir->entry[i].type != EMPTY && !strcmp(cur_dir->entry[i].name, nextdirname)){
            inode_id = cur_dir->entry[i].inode;
            type = cur_dir->entry[i].type;
            break;
        }
    }

    if(mode == 0){
        if(type == DIR){
            return 0;
        }
        read_block(current_dir_block);
        cur_dir = cached_block_base;
        for(int i = 0; i < 128; i++){
            if(cur_dir->entry[i].type == EMPTY){
                cur_dir->entry[i].type = FILE;
                strcpy(cur_dir->entry[i].name, newfile);
                cur_dir->entry[i].inode = inode_id;
                write_block(current_dir_block);
                return 1;
            }
        }
    }
    else if(mode == 1){
        read_block(current_dir_block);
        cur_dir = cached_block_base;
        for(int i = 0; i < 128; i++){
            if(cur_dir->entry[i].type == EMPTY){
                uint16_t new_inode_id = alloc_inode();
                cur_dir->entry[i].type = type;
                strcpy(cur_dir->entry[i].name, newfile);
                cur_dir->entry[i].inode = inode_id;
                write_block(current_dir_block);
                
                inode_t* inode = read_inode(inode_id);
                uint32_t direct[10];
                uint32_t indirect[2];
                uint64_t size;
                for(int j = 0; j < 10; j++){
                    direct[i] = inode->direct_blocks[i];
                }
                indirect[0] = inode->indirect_blocks[0];
                indirect[1] = inode->indirect_blocks[1];
                size = inode->size;

                inode = read_inode(new_inode_id);
                inode->mode = 1;
                inode->owner = current_running[cpu_id]->pid;
                inode->size = size;
                for(int j = 0; j < 10; j++){
                    inode->direct_blocks[i] = direct[i];
                }
                inode->indirect_blocks[0] = indirect[0];
                inode->indirect_blocks[1] = indirect[1];
                read_inode(new_inode_id);

                return 1;
            }
        }
    }
}