#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdatomic.h>
#include <mthread.h>
#include <assert.h>

#define MAGIC 0xdeadbeefdeadbeeflu
#define SHMP_KEY 42
#define NUM_CONSENSUS 8

typedef struct consensus_vars {
    atomic_long magic_number;
    atomic_long consensus;
    mthread_barrier_t barrier;
    atomic_int round;
} consensus_vars_t;

int is_first(consensus_vars_t *vars)
{
    unsigned long my = atomic_exchange_d(&vars->magic_number, MAGIC);
    return my != MAGIC;
}

pid_t decide(pid_t prev, pid_t mypid, atomic_long* consensus)
{
    pid_t ret = atomic_compare_exchange(consensus, prev, mypid);
    if (ret == prev) {
        return mypid;
    }
    return ret;
}

int main(int argc, char* argv[])
{
    char *prog_name = argv[0];
    int print_location = 1;
    if (argc > 1) {
        print_location = atol(argv[1]);
    }

    consensus_vars_t *vars = (consensus_vars_t*) sys_shmpage_get(SHMP_KEY);

    if (vars == NULL) {
        sys_move_cursor(1, print_location);
        printf("sys_shmpage_get failed!\n");
        return -1;
    }

    // test sys_shmpage_dt()
    sys_shmpage_dt((void*)vars);
    // touch previous page, OS should insert a page for us
    // then, sys_shmpage_get() should get another virtual address
    vars->magic_number = MAGIC;
    vars = (consensus_vars_t*) sys_shmpage_get(SHMP_KEY);
    sys_move_cursor(1, print_location);

    if (is_first(vars)) {
        mthread_barrier_init(&vars->barrier, NUM_CONSENSUS + 1);
        atomic_exchange_d(&vars->consensus, 0);
        atomic_exchange(&vars->round, 0);
        sys_move_cursor(1, print_location);
        printf("(%d) is the first!\n", sys_getpid());
        char *sub_task_args[2];
        char str_print_loc[10] = {0};
        sub_task_args[0] = argv[0];
        sub_task_args[1] = str_print_loc;
        for (int i = 0; i < NUM_CONSENSUS; ++i) {
            // convert print_location to string
            int _print_loc = i + 2;
            int _pos = 0;
            while (_print_loc > 0) {
                str_print_loc[_pos++] = '0' + _print_loc % 10;
                _print_loc /= 10;
            }
            for (int l = 0, r = _pos - 1;
                 l < r; ++l, --r) {
                // swap(str_print_loc[l], str_print_loc[r])
                str_print_loc[l] ^= str_print_loc[r];
                str_print_loc[r] ^= str_print_loc[l];
                str_print_loc[l] ^= str_print_loc[r];
            }
            str_print_loc[_pos] = 0;

            // printf("loc : %s\n", str_print_loc);
            sys_exec(sub_task_args[0], 2, sub_task_args, AUTO_CLEANUP_ON_EXIT);
        }
    }

    sys_sleep(2);

    mthread_barrier_wait(&vars->barrier);
    pid_t mypid = sys_getpid();
    sys_move_cursor(1, print_location);
    printf("ConsensusTask(%d) is ready at line %d!\n", mypid, print_location);
    pid_t consensus = 0;
    mthread_barrier_wait(&vars->barrier);
    sys_sleep(2);
    int myround = 0;

    while (1) {
        if (consensus != mypid) {
            consensus = decide(consensus, mypid, &vars->consensus);
            if (consensus == mypid) {
                sys_move_cursor(1, print_location);
                printf("(%d) exit now                           \n", consensus);
                myround = fetch_add(&vars->round, 1) + 1;
            } else {
                sys_move_cursor(1, print_location);
                printf("(%d) we selecte (%d)                    \n",
                       mypid, consensus);
            }
        } else {
            sys_move_cursor(1, print_location);
            printf("(%d) I am selected at round %d              \n",
                   consensus, myround);
        }
        sys_sleep(2);
        mthread_barrier_wait(&vars->barrier);
        if (atomic_load(&vars->round) == NUM_CONSENSUS + 1) {
            break;
        }
    }
    mthread_barrier_wait(&vars->barrier);
    sys_shmpage_dt((void*)vars);

    sys_move_cursor(1, print_location);
    printf("(%d) exited!                            \n", mypid);

    return 0;
}
