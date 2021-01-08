#ifndef FILE_H
#define FILE_H

#include <type.h>

#define O_EMPTY 0
#define O_RDONLY 1 /* read only open */
#define O_WRONLY 2 /* write only open */
#define O_RDWR 3 /* read/write open */

typedef struct fd{
    uint16_t inode;
    uint16_t access;
    uint64_t rd_pos;
    uint64_t wr_pos;
} fd_t;
extern fd_t fd_list[10];

extern int do_touch(char* filename);
extern int do_cat(char* filename, char* buffer);
extern int do_fopen(char* filename, int access);
extern int do_fread(int fd, char* buffer, int size);
extern int do_fwrite(int fd, char* buffer, int size);
extern void do_fclose(int fd);

#endif /* FILE_H */