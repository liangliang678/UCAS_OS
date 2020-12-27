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

#include <string.h>
#include <os.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <alloca.h>

#define SHELL_BEGIN 15
#define SHELL_END   30
#define SHELL_WIDTH 80
#define MAX_ARGC    10
#define MAX_PREV    5

int main()
{
    char blank_command[] = "                                                      ";
    char prompt[] = "> liangliang678@UCAS_OS: ";

    const int SHELL_COMMAND_BEGIN = strlen(prompt) + 1;
    const int SHELL_COMMAND_END   = SHELL_WIDTH;
    int print_location_x;
    int print_location_y;
    int input, buf_p;
    char buffer[100];

    char prev_command[MAX_PREV][100];
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
        while((input = sys_get_char()) != 13 && input != '\n'){
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

        // parse input (ls, ps, exec, kill, clear, reset)
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
        int ls_command = !strcmp(argv[0], "ls");
        int ps_command = !strcmp(argv[0], "ps");
        int reset_command = !strcmp(argv[0], "reset");
        int clear_command = !strcmp(argv[0], "clear");
        int exec_command = !strcmp(argv[0], "exec");
        int kill_command = !strcmp(argv[0], "kill");
        int taskset_command = !strcmp(argv[0], "taskset");

        // exec command and output
        if(empty_command){
            continue;
        }
        else if(ls_command){
            if(argc >= 2){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command ls!\n");
                print_location_y++;
                continue;
            }
            char ls_context[(SHELL_END - SHELL_BEGIN - 1) * SHELL_WIDTH + 1];
            sys_show_exec(ls_context);

            char ls_buffer[SHELL_WIDTH + 1];
            for(int i = 0; ls_context[i]; i++){
                int j = 0;
                while(ls_context[i] != '\n' && ls_context[i]){
                    ls_buffer[j++] = ls_context[i++];
                }
                ls_buffer[j++] = '\n';
                ls_buffer[j++] = '\0';

                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);
                }
                printf("%s", ls_buffer);
                print_location_y++;
            }
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
            if(argc <= 1){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too few arguments for command exec!\n");
                print_location_y++;
                continue;
            }

            char *_argv[MAX_ARGC - 1];
            for(int i = 0; i < MAX_ARGC - 1; i++){
                _argv[i] = argv[i + 1];
            } 
            int ret = sys_exec(argv[1], argc - 1, _argv, AUTO_CLEANUP_ON_EXIT);
            if(ret == 0){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many tasks! Failed to exec %s\n", argv[1]);
                print_location_y++;
            }
            else if(ret == -1){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Process %s does not exist\n", argv[1]);
                print_location_y++;
            }
            else if(ret == -2){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Memory full! Failed to exec %s\n", argv[1]);
                print_location_y++;
            }
            else{
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("exec process %s\n", argv[1]);
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
        else if(taskset_command){
            if(strcmp(argv[1], "-p")){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("usage: taskset -p mask pid\n");
                print_location_y++;
                continue;
            }
            if(argc >= 5){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command taskset!\n");
                print_location_y++;
                continue;
            }
            else if(argc <=3){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too few arguments for command taskset!\n");
                print_location_y++;
                continue;
            }

            unsigned long mask = atoi(argv[2]);
            int pid = atoi(argv[3]);
            if(!sys_taskset(pid, mask)){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Cannot set mask of task(pid=%d): Task Does Not Exist!\n", pid);
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