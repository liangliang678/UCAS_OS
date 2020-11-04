#include <os/func.h>
#include <os/sched.h>
#include <os/string.h>

void process_show(char* buffer)
{
    buffer[0] = '\0';
    kstrcat(buffer, "[PROCESS TABLE]\n");
    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid != -1){
            kstrcat(buffer, "PID: ");
            char pid_c[3];
            pid_c[0] = (char)((pcb[i].pid / 10) + '0');
            pid_c[1] = (char)((pcb[i].pid % 10) + '0');
            pid_c[2] = '\0';
            kstrcat(buffer, pid_c);
            kstrcat(buffer, "\tSTATUS: ");
            if(pcb[i].status == TASK_BLOCKED){
                kstrcat(buffer, "BLOCKED\n");
            }
            else if(pcb[i].status == TASK_RUNNING){
                kstrcat(buffer, "RUNNING\n");
            }
            else if(pcb[i].status == TASK_READY){
                kstrcat(buffer, "READY\n");
            }
        }
    }
    return buffer;
}