/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                       System call related processing
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_SYSCALL_H_
#define INCLUDE_SYSCALL_H_

#include <os/syscall_number.h>
#include <stdint.h>
#include <stddef.h>
#include <os.h>

extern long invoke_syscall(long, long, long, long, long);

extern void sys_exit(void);
extern void sys_sleep(uint32_t time);
extern int sys_kill(pid_t pid);
extern int sys_waitpid(pid_t pid);
extern void sys_process_show(char* buffer);
extern pid_t sys_getpid();
extern void sys_yield();
extern int sys_taskset(pid_t pid, unsigned long mask);
pid_t sys_exec(const char *file_name, int argc, char* argv[], spawn_mode_t mode);
void sys_show_exec(int* print_location_y);

extern void sys_futex_wait(volatile uint64_t *val_addr, int binsem_id);
extern void sys_futex_wakeup(volatile uint64_t *val_addr, int num_wakeup);

#define BINSEM_OP_LOCK 0
#define BINSEM_OP_UNLOCK 1
extern int sys_binsem_get(long key);
extern void sys_binsem_op(int binsem_id, int op);

extern void* sys_shmpage_get(int key);
extern void sys_shmpage_dt(void *addr);

extern void sys_write(char *);
extern void sys_move_cursor(int, int);
extern void sys_reflush();
extern void sys_screen_clear(int line1, int line2);
extern void sys_screen_scroll(int line1, int line2);

extern long sys_get_timebase();
extern long sys_get_tick();
extern int sys_get_char();

extern int sys_mbox_open(char *);
extern void sys_mbox_close(int);
extern int sys_mbox_send(int, void *, int);
extern int sys_mbox_recv(int, void *, int);

extern long sys_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength);
extern void sys_net_send(uintptr_t addr, size_t length);
extern void sys_net_irq_mode(int mode);

extern int sys_mkfs(int mode, int* print_location_y);
extern void sys_statfs(int* print_location_y);
extern int sys_mkdir(char* dirname);
extern int sys_rmdir(char* dirname);
extern int sys_ls(char* dirname, int mode, int* print_location_y);
extern int sys_cd(char* dirname);

#endif
