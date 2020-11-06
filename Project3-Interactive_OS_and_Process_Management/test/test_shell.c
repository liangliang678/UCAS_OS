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
static int num_test_tasks = 8;

#define SHELL_BEGIN 15
#define SHELL_END   30
#define SHELL_WIDTH 80
#define MAX_ARGC    5
#define MAX_PREV    5

void test_shell()
{
    char blank_line[] = "                                                                                ";
    char blank_command[] = "                                                      ";
    char prompt[] = "> liangliang678@UCAS_OS: ";

    const int SHELL_COMMAND_BEGIN = strlen(prompt) + 1;
    const int SHELL_COMMAND_END   = SHELL_WIDTH;
    int print_location_x;
    int print_location_y;
    int input, buf_p;
    char buffer[100];

    char prev_command[MAX_PREV][100] = {0};
    for(int i = 0; i < MAX_PREV; i++){
        strcpy(prev_command[i], "\0");
    }
    int back_times = 0;

    sys_screen_clear(1, SHELL_END);
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
        buf_p = 0;
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
            else if(input == 27){
                if(back_times < MAX_PREV){
                    strcpy(buffer, prev_command[back_times++]);
                    print_location_x = SHELL_COMMAND_BEGIN;
                    sys_move_cursor(print_location_x, print_location_y);
                    printf("%s", blank_command);
                    sys_move_cursor(print_location_x, print_location_y);
                    printf("%s", buffer);
                    print_location_x += strlen(buffer);
                    buf_p = strlen(buffer);
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
        buffer[buf_p++] = '\0';
        for(int i = MAX_PREV - 1; i > 0; i--){
            strcpy(prev_command[i], prev_command[i-1]);
        }
        strcpy(prev_command[0], buffer);
        printf("\n");
        print_location_y++;
        back_times = 0;

        // parse input (ps, exec, kill, clear, reset)
        int argc = 0;
        char argv[MAX_ARGC][100];
        int argv_p = 0;
        int in_argv_flag = 0;
 
        for(int i = 0; ; i++){
            if(buffer[i] == ' ' || buffer[i] == '\0'){
                if(in_argv_flag){
                    if(argc >= MAX_ARGC){
                        if(print_location_y == SHELL_END){
                            print_location_y--;
                            sys_move_cursor(1, print_location_y);
                            sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);    
                        }
                        printf("Too many arguments!\n");
                        print_location_y++;
                    }
                    argv[argc][argv_p++] = '\0';
                    argc++;
                    argv_p = 0;
                }
                in_argv_flag = 0;
                if(buffer[i] == '\0'){
                    break;
                }
            }
            else{
                in_argv_flag = 1;
                argv[argc][argv_p++] = buffer[i];
            }    
        }

        int empty_command = (argc == 0);
        int ps_command = !strcmp(argv[0], "ps");
        int reset_command = !strcmp(argv[0], "reset");
        int clear_command = !strcmp(argv[0], "clear");
        int exec_command = !strcmp(argv[0], "exec");
        int kill_command = !strcmp(argv[0], "kill");

        // exec command and output
        if(empty_command){
            continue;
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
            char ps_context[(SHELL_END - SHELL_BEGIN - 1) * SHELL_WIDTH + 1];
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
        else if(reset_command){
            if(argc >= 2){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command reset!\n");
                print_location_y++;
                continue;
            }

            sys_screen_clear(SHELL_BEGIN + 1, SHELL_END - 1);
            print_location_y = SHELL_BEGIN + 1;
            sys_move_cursor(1, print_location_y);
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

            sys_screen_clear(1, SHELL_BEGIN - 1);
            sys_move_cursor(1, print_location_y);
        }
        else if(exec_command){
            if(argc >= 3){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command exec!\n");
                print_location_y++;
                continue;
            }
            else if(argc <= 1){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too few arguments for command exec!\n");
                print_location_y++;
                continue;
            }

            int task_num = atoi(argv[1]);
            if(task_num >= num_test_tasks || task_num < 0){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("No such test tasks!\n");
                print_location_y++;
                continue;
            }
            
            if(sys_spawn(test_tasks[task_num], NULL, AUTO_CLEANUP_ON_EXIT) == -1){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many tasks! Failed to exec process[%d]\n", task_num);
                print_location_y++;
            }
            else{
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("exec process[%d]\n", task_num);
                print_location_y++;
            }
        }
        else if(kill_command){
            if(argc >= 3){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command kill!\n");
                print_location_y++;
                continue;
            }
            else if(argc <= 1){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too few arguments for command kill!\n");
                print_location_y++;
                continue;
            }
            
            int pid = atoi(argv[1]);
            if(pid == 0 || pid == 1){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Cannot kill task(pid=%d): Permission Denied!\n", pid);
                print_location_y++;
            }
            else if(!sys_kill(pid)){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Cannot kill task(pid=%d): Task Does Not Exist!\n", pid);
                print_location_y++;
            }
            else{
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Task(pid=%d) killed\n", pid);
                print_location_y++;
            }
        }
        else{
            if(print_location_y == SHELL_END){
                print_location_y--;
                sys_move_cursor(1, print_location_y);
                sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
            }
            printf("Unknown Command: ");
            printf("%s\n", argv[0]);
            print_location_y++;
        }
    }
}