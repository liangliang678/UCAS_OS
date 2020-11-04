/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
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

#include <test.h>
#include <string.h>
#include <os.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
/*
struct task_info task_test_waitpid = {(uintptr_t)&wait_exit_task, USER_PROCESS};
struct task_info task_test_semaphore = {(uintptr_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task_test_condition = {(uintptr_t)&test_condition, USER_PROCESS};
struct task_info task_test_barrier = {(uintptr_t)&test_barrier, USER_PROCESS};

struct task_info task13 = {(uintptr_t)&SunQuan, USER_PROCESS};
struct task_info task14 = {(uintptr_t)&LiuBei, USER_PROCESS};
struct task_info task15 = {(uintptr_t)&CaoCao, USER_PROCESS};
struct task_info task_test_multicore = {(uintptr_t)&test_multicore, USER_PROCESS};

static struct task_info *test_tasks[16] = {&task_test_waitpid,
                                           &task_test_semaphore,
                                           &task_test_condition,
                                           &task_test_barrier,
                                           &task13, &task14, &task15,
                                           &task_test_multicore};
static int num_test_tasks = 8;*/

#define SHELL_BEGIN 15
#define SHELL_END   30
#define SHELL_WIDTH 80

void test_shell()
{
    char blank[] = "                                                                                ";
    char prompt[] = "> liangliang678@UCAS_OS: ";

    const int SHELL_COMMAND_BEGIN = strlen(prompt) + 1;
    const int SHELL_COMMAND_END   = SHELL_WIDTH;
    int print_location_x;
    int print_location_y;

    sys_move_cursor(1, SHELL_BEGIN);
    printf("------------------- COMMAND -------------------");
    sys_move_cursor(1, SHELL_END);
    printf("------------------- COMMAND -------------------");
    print_location_y = SHELL_BEGIN + 1;
    sys_move_cursor(1, print_location_y);

    while (1)
    {   
        if(print_location_y == SHELL_END){
            print_location_y--;
            sys_move_cursor(1, print_location_y);
            sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);    
        }
        printf("%s", prompt); 

        // call syscall to read UART port
        int input, buf_p = 0;
        char buffer[100];
        print_location_x = SHELL_COMMAND_BEGIN;
        while((input = getchar()) != 13 && input != '\n'){
            if(input == '\b' || input == 127){
                if(buf_p > 0){
                    print_location_x--;
                    buf_p--;
                    sys_move_cursor(print_location_x, print_location_y);
                    printf(" ");
                    sys_move_cursor(print_location_x, print_location_y);
                }           
            }
            else{
                if(print_location_x < SHELL_COMMAND_END){
                    buffer[buf_p++] = (char)input;
                    print_location_x++;
                    printf("%c",input);
                }
                               
            } 
        }
        buffer[buf_p++] = '\n';
        buffer[buf_p++] = '\0';
        printf("\n");
        print_location_y++;

        // parse input (ps, exec, kill, clear)
        int argc = 0;
        char argv[5][100];
        int argv_p = 0;
        int in_argv_flag = 0;
 
        for(int i = 0; buffer[i]; i++){
            if(buffer[i] == ' ' || buffer[i] == '\n'){
                if(in_argv_flag){
                    argv[argc][argv_p++] = '\n';
                    argv[argc][argv_p++] = '\0';
                    argc++;
                    argv_p = 0;
                }
                in_argv_flag = 0;
            }
            else{
                in_argv_flag = 1;
                argv[argc][argv_p++] = buffer[i];
            }    
        }

        int empty_command = (argc == 0);
        int ps_command = !strcmp(argv[0], "ps\n");
        int clear_command = !strcmp(argv[0], "clear\n");

        if(empty_command){
            ;
        }
        else if(ps_command){
            if(argc >= 2){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command ps!\n");
                print_location_y++;
                continue;
            }
            char ps_context[18 * SHELL_WIDTH];
            sys_process_show(ps_context);

            char ps_buffer[SHELL_WIDTH + 1];
            for(int i = 0; ps_context[i]; i++){
                int j = 0;
                while(ps_context[i] != '\n' && ps_context[i]){
                    ps_buffer[j++] = ps_context[i++];
                }
                ps_buffer[j++] = '\n';
                ps_buffer[j++] = '\0';

                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);
                }
                printf("%s", ps_buffer);
                print_location_y++;
            }
        }
        else if(clear_command){
            if(argc >= 2){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command clear!\n");
                print_location_y++;
                continue;
            }
            for(int i = SHELL_BEGIN + 1; i < SHELL_END; i++){
                sys_move_cursor(1, i);
                printf("%s", blank);
            }
            print_location_y = SHELL_BEGIN + 1;
            sys_move_cursor(1, print_location_y);
        }
        else{
            if(print_location_y == SHELL_END){
                print_location_y--;
                sys_move_cursor(1, print_location_y);
                sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
            }
            printf("Unknown command: ");
            printf("%s", buffer);
            print_location_y++;
        }
    }
}