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
#include <os.h>

extern long invoke_syscall(long, long, long, long);

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode);
void sys_exit(void);
void sys_sleep(uint32_t time);
int sys_kill(pid_t pid);
int sys_waitpid(pid_t pid);
void sys_process_show(char* buffer);
pid_t sys_getpid();
void sys_yield();
int sys_taskset(pid_t pid, unsigned long mask);

void sys_futex_wait(volatile uint64_t *val_addr);
void sys_futex_wakeup(volatile uint64_t *val_addr, int num_wakeup);

#define BINSEM_OP_LOCK 0
#define BINSEM_OP_UNLOCK 1
int sys_binsem_get(int key);
void sys_binsem_op(int binsem_id, int op);

void sys_write(char *);
void sys_move_cursor(int, int);
void sys_reflush();
void sys_screen_clear(int line1, int line2);
void sys_screen_scroll(int line1, int line2);

long sys_get_timebase();
long sys_get_tick();
int sys_get_char();

int sys_mbox_open(char *);
void sys_mbox_close(int);
int sys_mbox_send(int, void *, int);
int sys_mbox_recv(int, void *, int);

#endif
