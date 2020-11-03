#include <time.h>
#include <test3.h>
#include <mthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <mailbox.h>
#include <sys/syscall.h>

static struct task_info sq_task = {(uintptr_t)&SunQuan, USER_PROCESS};
static struct task_info lb_task = {(uintptr_t)&LiuBei, USER_PROCESS};
static struct task_info cc_task = {(uintptr_t)&CaoCao, USER_PROCESS};

void SunQuan(void)
{
    mailbox_t pub = mbox_open("SunQuan-Publish-PID");
    pid_t myPid = sys_getpid();

    /* Send PID twice, once for LiuBei,
     * and once for the CaoCao */

    // sys_move_cursor(1, 1);
    // printf("SunQuan(%d): Hello, I am SunQuan          ", myPid);

    mbox_send(pub, &myPid, sizeof(pid_t));
    mbox_send(pub, &myPid, sizeof(pid_t));

    /* Find LiuBei's PID */
    mailbox_t sub = mbox_open("LiuBei-Publish-PID");

    for (;;)
    {
        pid_t liubei;

        sys_move_cursor(1, 1);
        printf("[SunQuan](%d): Where are you Liubei?                ", myPid);
        mbox_recv(sub, &liubei, sizeof(pid_t));

        sys_move_cursor(1, 1);
        printf("[SunQuan](%d): I'm waiting for Liubei (%d)          ", myPid, liubei);
        sys_waitpid(liubei);

        sys_move_cursor(1, 1);
        printf("[SunQuan](%d): I'm coming to save you, LiuBei!", myPid);

        sys_sleep(1);
        sys_spawn(&lb_task, NULL, ENTER_ZOMBIE_ON_EXIT);
        mbox_send(pub, &myPid, sizeof(pid_t));
    }
}

void LiuBei(void)
{
    mailbox_t pub = mbox_open("LiuBei-Publish-PID");
    pid_t myPid = sys_getpid();

    /* Send PID twice, once for sunquan Hood,
     * and once for the CaoCao */
    mbox_send(pub, &myPid, sizeof(pid_t));
    mbox_send(pub, &myPid, sizeof(pid_t));

    /* Find sunquan's PID */
    mailbox_t sub = mbox_open("SunQuan-Publish-PID");

    // sys_move_cursor(1, 2);
    // printf("LiuBei(%d): Hello, I am Liubei          ", myPid);

    for (;;)
    {
        pid_t aramis;

        sys_move_cursor(1, 2);
        printf("[LiuBei](%d): Where are you SunQuan?          ", myPid);
        mbox_recv(sub, &aramis, sizeof(pid_t));

        sys_move_cursor(1, 2);
        printf("[LiuBei](%d): I'm waiting for SunQuan (%d)    ", myPid, aramis);
        sys_waitpid(aramis);

        sys_move_cursor(1, 2);
        printf("[LiuBei](%d): I'm coming to save you, SunQuan!", myPid);

        sys_sleep(1);
        sys_spawn(&sq_task, NULL, ENTER_ZOMBIE_ON_EXIT);
        mbox_send(pub, &myPid, sizeof(pid_t));
    }
}

void CaoCao(void)
{
    pid_t myPid = sys_getpid();

    mailbox_t subSunQuan = mbox_open("SunQuan-Publish-PID");
    mailbox_t subLiuBei = mbox_open("LiuBei-Publish-PID");

    int i;
    pid_t sunquan, liubei;
    mbox_recv(subSunQuan, &sunquan, sizeof(pid_t));
    mbox_recv(subLiuBei, &liubei, sizeof(pid_t));

    for (i = 0;;i++)
    {
        sys_move_cursor(1, 3);
        printf("[CaoCao](%d): I am working... muahaha ", myPid);

        // sys_sleep(1);

        sys_move_cursor(1, 4);
        printf("[CaoCao](%d): I have my decision! ", myPid);

        switch (rand() % 2)
        {
        case 0:
            sys_move_cursor(1, 5);
            printf("[CaoCao](%d): I will kill SunQuan (%d)!  ", myPid, sunquan);
            sys_sleep(1);

            sys_move_cursor(1, 6);
            printf("[CaoCao]biu biu biu ~~~~~~ AAAAAAAA SunQuan is dead QAQ.");
            sys_kill(sunquan);
            mbox_recv(subSunQuan, &sunquan, sizeof(pid_t));

            sys_move_cursor(1, 7);
            printf("[CaoCao](%d): Oops! SunQuan(%d) lives!                 ", myPid, sunquan);
            break;
        case 1:
            sys_move_cursor(1, 5);
            printf("[CaoCao](%d): I will kill LiuBei(%d)! ", myPid, liubei);
            sys_sleep(1);

            sys_move_cursor(1, 6);
            printf("[CaoCao]biu biu biu ~~~~~~ AAAAAAAA Liubei is dead QAQ.");
            sys_kill(liubei);

            sys_move_cursor(1, 7);
            mbox_recv(subLiuBei, &liubei, sizeof(pid_t));
            printf("[CaoCao](%d): Oops! LiuBei(%d) is alive again! ", myPid, liubei);
            break;
        }

        sys_sleep(1);
    }
}
