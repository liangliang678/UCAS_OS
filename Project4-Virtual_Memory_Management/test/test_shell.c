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
#include <alloca.h>

// test exit
void test_shell_task1()
{
    int i;

    for (i = 0; i < 500; i++)
    {
        sys_move_cursor(0, 0);
        printf("I am Task A.(%d)           \n", i);
    }

    sys_exit();
}

// test kill & waitpid
void test_shell_task2()
{
    int i;

    sys_move_cursor(0, 1);
    printf("I am waiting Task A to exit.\n");
    sys_waitpid(2);

    for (i = 0;; i++)
    {
        sys_move_cursor(0, 1);
        printf("I am Task B.(%d)           \n", i);
    }
}

// test waitpid
void test_shell_task3()
{
    int i;

    sys_move_cursor(0, 2);
    printf("I am waiting Task B to exit.\n");
    sys_waitpid(3);

    for (i = 0;; i++)
    {
        sys_move_cursor(0, 2);
        printf("I am Task C.(%d)           \n", i);
    }
}

#define SHELL_BEGIN 15
#define SHELL_END   30
#define SHELL_WIDTH 80
#define MAX_ARGC    5
#define MAX_PREV    5

void test_shell()
{
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
        int taskset_command = !strcmp(argv[0], "taskset");

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
        else if(exec_command){/*
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
            }*/
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
        else if(taskset_command){/*
            int spawn = strcmp(argv[1], "-p");
            if((spawn && argc >= 4) || (!spawn && argc >= 5)){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too many arguments for command taskset!\n");
                print_location_y++;
                continue;
            }
            else if((spawn && argc <=2) || (!spawn && argc <=3)){
                if(print_location_y == SHELL_END){
                    print_location_y--;
                    sys_move_cursor(1, print_location_y);
                    sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                }
                printf("Too few arguments for command taskset!\n");
                print_location_y++;
                continue;
            }

            if(spawn){
                unsigned long mask = atoi(argv[1]);
                int task_num = atoi(argv[2]); 
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
                
                int spawn_pid;
                if((spawn_pid = sys_spawn(test_tasks[task_num], NULL, AUTO_CLEANUP_ON_EXIT)) == -1){
                    if(print_location_y == SHELL_END){
                        print_location_y--;
                        sys_move_cursor(1, print_location_y);
                        sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                    }
                    printf("Too many tasks! Failed to exec process[%d]\n", task_num);
                    print_location_y++;
                }
                else{
                    sys_taskset(spawn_pid, mask);
                    if(print_location_y == SHELL_END){
                        print_location_y--;
                        sys_move_cursor(1, print_location_y);
                        sys_screen_scroll(SHELL_BEGIN + 1, SHELL_END - 1);       
                    }
                    printf("exec process[%d] and set mask 0x%x\n", task_num, mask);
                    print_location_y++;
                }
            }
            else{
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
            }*/
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

/*
#define SHELL_BUFF_SIZE 64
#define MAX_ARGS 256

static int shell_tail = 0;
static char shell_buff[SHELL_BUFF_SIZE];

int main()
{
    char ch;
    static char _arg_buf[SHELL_BUFF_SIZE] = {0};
    char *argv[MAX_ARGS] = {0};
    int i, l, pid, command;

    sys_move_cursor(0, SCREEN_HEIGHT / 2);
    printf("------------------- COMMAND -------------------\n");
    printf("> root@UCAS_OS: ");

    while (1)
    {
        command = 0;

        // read UART port
        ch = sys_get_char();

        if (ch == 8 || ch == 127) // Backspace
        {
            if (shell_tail > 0)
            {
                shell_tail--;
            }
            printf("%c", ch);
        }
        else if (ch == 13) // Enter
        {
            command = 1;
            printf("\n");
        }
        else if (ch != 0)
        {
            shell_buff[shell_tail++] = ch;
            printf("%c", ch);
        }

        if (command == 1)
        {
            shell_buff[shell_tail] = '\0';
            l = strlen(shell_buff);

            // process show
            if (memcmp(shell_buff, "ps", 2) == 0)
            {
                sys_process_show();
            }
            // process show
            else if (memcmp(shell_buff, "ls", 2) == 0)
            {
                sys_show_exec();
            }
            // spawn a new task
            else if (memcmp(shell_buff, "exec", 4) == 0)
            {
                pid = 0;
                l = strlen(shell_buff);

                int argc = 0;
                for (i = 4; i < l; i++)
                {
                    if (shell_buff[i] != ' ' && shell_buff[i-1] == ' ')
                    {
                        argc++;
                    }
                }
                strcpy(_arg_buf, &shell_buff[4]);
                int tmp_argc = 0;
                l -= 4;
                for (i = 0; i < l; ++i) {
                    if (_arg_buf[i] == ' ') {
                        _arg_buf[i] = 0;
                    } else if (_arg_buf[i - 1] == 0) {
                        argv[tmp_argc++] = &_arg_buf[i];
                    }
                }

                printf("exec process %s.\n", argv[0]);
                pid_t cmd_pid = sys_exec(argv[0], argc, argv,
                          AUTO_CLEANUP_ON_EXIT);
                printf("success! pid = %d\n",cmd_pid);
            }
            // kill process
            else if (memcmp(shell_buff, "kill", 4) == 0)
            {
                pid = 0;
                l = strlen(shell_buff);

                for (i = 4; i < l; i++)
                {
                    if (shell_buff[i] != ' ')
                    {
                        break;
                    }
                }

                while ((i < l) && (shell_buff[i] != ' '))
                {
                    pid = 10 * pid + (shell_buff[i] - '0');
                    i++;
                }

                if (pid != 0)
                {
                    printf("kill process pid=%d\n", pid);
                    sys_kill(pid);
                }
            }
            // clear the screen
            else if (memcmp(shell_buff, "clear", 5) == 0)
            {
                sys_screen_clear(0, SCREEN_HEIGHT - 1);
                sys_move_cursor(0, SCREEN_HEIGHT / 2);
                printf("------------------- COMMAND -------------------\n");
            }

            printf("> root@UCAS_OS: ");
            shell_tail = 0;
        }
    }
    
    return 0;
}
*/