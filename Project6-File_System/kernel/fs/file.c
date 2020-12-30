#include <os/fs.h>
#include <os/disk.h>
#include <os/file.h>
#include <os/sched.h>
#include <os/irq.h>
#include <assert.h>

fd_t fd_list[10];

int do_touch(char* filename)
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
            return 0;
        }

        filename_pos++;
        read_block(nextdir_block_id);
        cur_dir = cached_block_base;
    }

    read_block(lastdir_block);
    cur_dir = cached_block_base;
    for(int i = 0; i < 128; i++){
        if(cur_dir->entry[i].type == EMPTY){
            uint16_t inode_id = alloc_inode();

            // Modify Current Dir
            cur_dir->entry[i].type = FILE;
            strcpy(cur_dir->entry[i].name, nextdirname);
            cur_dir->entry[i].inode = inode_id;
            write_block(lastdir_block);

            // Modify New Inode
            inode_t* new_inode = read_inode(inode_id);
            new_inode->mode = 0;
            new_inode->owner = current_running[cpu_id]->pid;
            new_inode->size = 0;
            write_inode(inode_id);

            disable_sum();
            return 1;
        }
    }

    disable_sum();
    return 0;
}

int do_cat(char* filename, char* buffer)
{
    /* Search for Parent Dir */
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

    /* Modify Buffer */
    inode_t* inode;
    read_block(lastdir_block);
    cur_dir = cached_block_base;
    for(int i = 0; i < 128; i++){
        if(cur_dir->entry[i].type == FILE && !strcmp(cur_dir->entry[i].name, nextdirname)){
            inode = read_inode(cur_dir->entry[i].inode);
        }
    }

    memset(buffer, 0, inode->size + 1);
    int block_num = (inode->size == 0) ? 0 : (inode->size / 4096 + 1);
    int copied_byte = 0;
    int i;
    for(i = 0; i < block_num - 1; i++){
        int block_id = inode->direct_blocks[i];
        read_block(block_id);
        memcpy(buffer + copied_byte, cached_block_base, 4096);
        copied_byte += 4096;
    }
    int block_id = inode->direct_blocks[i];
    read_block(block_id);
    memcpy(buffer + copied_byte, cached_block_base, inode->size - copied_byte);
    copied_byte += 4096;
    disable_sum();
    return 1;
}

int do_fopen(char* filename, int access)
{
    enable_sum();

    /* Search for Parent Dir */
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
            return 0;
        }

        filename_pos++;
        read_block(nextdir_block_id);
        cur_dir = cached_block_base;
    }

    /* Modify FD */
    read_block(lastdir_block);
    cur_dir = cached_block_base;
    int fd_id;
    for(fd_id = 0; fd_id < 10; fd_id++){
        if(fd_list[fd_id].access == O_EMPTY){
            break;
        }
    }
    
    for(int i = 0; i < 128; i++){
        if(cur_dir->entry[i].type == FILE && !strcmp(cur_dir->entry[i].name, nextdirname)){
            fd_list[fd_id].access = access;
            fd_list[fd_id].inode = cur_dir->entry[i].inode;
            fd_list[fd_id].rd_pos = 0;
            fd_list[fd_id].wr_pos = 0;
        }
    }

    disable_sum();
    return fd_id;
}

int do_fread(int fd, char* buffer, int size)
{
    if(fd_list[fd].access == O_WRONLY){
        return 0;
    }

    inode_t* inode = read_inode(fd_list[fd].inode);
    if(fd_list[fd].rd_pos + size > inode->size){
        size = inode->size - fd_list[fd].rd_pos;
    }
     
    int copied_byte = 0;
    while(copied_byte < size){
        int block_num = fd_list[fd].rd_pos / 4096;
        int block_id;
        if(block_num < 10){
            block_id = inode->direct_blocks[block_num];
        }
        else if(block_num >= 10 && block_num < 10 + 1024){
            block_id = inode->indirect_blocks[0];
            read_block(block_id);
            uint32_t *pointer = cached_block_base;
            block_id = *(pointer + block_num - 10);
        }
        else if(block_num >= 10 + 1024 && block_num < 10 + 1024 + 1024){
            block_id = inode->indirect_blocks[1];
            read_block(block_id);
            uint32_t *pointer = cached_block_base;
            block_id = *(pointer + block_num - 10 - 1024);
        }
        else{
            assert(0);
        }
        
        read_block(block_id);

        int offset = fd_list[fd].rd_pos % 4096;
        int copy_byte;
        if(size - copied_byte <= 4096 - offset){
            copy_byte = size - copied_byte;
        }
        else{
            copy_byte = 4096 - offset;
        }

        enable_sum();
        memcpy(buffer + copied_byte, ((uint8_t*)cached_block_base) + offset, copy_byte);
        disable_sum();

        copied_byte += copy_byte;
        fd_list[fd].rd_pos += copy_byte;
    }

    return size;
}

int do_fwrite(int fd, char* buffer, int size)
{
    if(fd_list[fd].access == O_RDONLY){
        return 0;
    }

    inode_t* inode = read_inode(fd_list[fd].inode);
    int cur_block = (inode->size == 0) ? 0 : (inode->size / 4096 + 1);
    inode->size = inode->size + size;
    int total_block = (inode->size == 0) ? 0 : (inode->size / 4096 + 1);
    int needed_block = total_block - cur_block;

    if(total_block >= 10 && cur_block < 10){
        inode->indirect_blocks[0] = alloc_block();
    }
    else if(total_block >= 10 + 1024 && cur_block < 10 + 1024){
        inode->indirect_blocks[1] = alloc_block();
    }

    for(int i = 0; i < needed_block; i++){
        if(cur_block + i < 10){
            inode->direct_blocks[cur_block + i] = alloc_block();
        }
        else if(cur_block + i >= 10 && cur_block + i < 10 + 1024){
            int block_id = inode->indirect_blocks[0];
            read_block(block_id);
            uint32_t *pointer = cached_block_base;
            *(pointer + cur_block + i - 10) = alloc_block();
            write_block(block_id);
        }
        else if(cur_block + i >= 10 + 1024 && cur_block + i < 10 + 1024 + 1024){
            int block_id = inode->indirect_blocks[1];
            read_block(block_id);
            uint32_t *pointer = cached_block_base;
            *(pointer + cur_block + i - 10 - 1024) = alloc_block();
            write_block(block_id);
        }
        else{
            assert(0);
        }
    }
    
    write_inode(fd_list[fd].inode);

    int copied_byte = 0;
    while(copied_byte < size){
        int block_num = fd_list[fd].wr_pos / 4096;
        int block_id; 
        if(block_num < 10){
            block_id = inode->direct_blocks[block_num];
        }
        else if(block_num >= 10 && block_num < 10 + 1024){
            block_id = inode->indirect_blocks[0];
            read_block(block_id);
            uint32_t *pointer = cached_block_base;
            block_id = *(pointer + block_num - 10);
        }
        else if(block_num >= 10 + 1024 && block_num < 10 + 1024 + 1024){
            block_id = inode->indirect_blocks[1];
            read_block(block_id);
            uint32_t *pointer = cached_block_base;
            block_id = *(pointer + block_num - 10 - 1024);
        }
        else{
            assert(0);
        }

        read_block(block_id);

        int offset = fd_list[fd].wr_pos % 4096;
        int copy_byte;
        if(size - copied_byte <= 4096 - offset){
            copy_byte = size - copied_byte;
        }
        else{
            copy_byte = 4096 - offset;
        }

        enable_sum();
        memcpy(((uint8_t*)cached_block_base) + offset, buffer + copied_byte, copy_byte);
        write_block(block_id);
        disable_sum();

        copied_byte += copy_byte;
        fd_list[fd].wr_pos += copy_byte;
    }

    return size;
}

void do_fclose(int fd)
{
    fd_list[fd].access = O_EMPTY;
}