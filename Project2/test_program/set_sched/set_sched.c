#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h> /* high-res timers */
#include <sched.h> /* for std. scheduling system calls */

#define SCHED_NORMAL        0
#define SCHED_FIFO      1
#define SCHED_RR        2
#define SCHED_BATCH     3
/* SCHED_ISO: reserved but not implemented yet */
#define SCHED_IDLE      5
#define SCHED_WRR       6
static void change_scheduler()
{
    int ret, tmp;
    struct sched_param param;
    pid_t pid;
    int policy,oldpolicy;
    printf("Input the process id (PID) you want to modify: ");
    scanf("%d", &tmp);
    pid = tmp;
    // struct task_struct *p = find_task_by_vpid(pid);
    if (!pid) {
        perror("Invalid PID. Aborting...");
        exit(-1);
    }
    // oldpolicy = p->policy;
    // printf("Old Policy: ");
    // if(oldpolicy == 0)
    //     printf("NORMAL\n");
    // else if(oldpolicy == 1)
    //     printf("FIFO\n");
    // else if(oldpolicy== 3)
    //     printf("RR\n");
    // else if(oldpolicy == 6)
    //     printf("WRR\n");

    printf("Input the schedule policy you want to change (0-NORMAL, 1-FIFO, 2-RR, 6-WRR):\n");
    scanf("%d", &policy);
   // printf("%d",policy);
    if(policy != 0 && policy != 1 && policy!= 2&& policy!=6)
    {
        perror("Wrong schedule policy. Aborting...");
        exit(-1);
    }

    printf("Set process's priority: ");
    scanf("%d",&tmp);
    if(policy!=6)
        param.sched_priority = tmp;
    else
        param.sched_priority = 0;

    printf("Changing Scheduler for PID %d\n", pid);


    ret = sched_setscheduler(pid, policy, &param);
    if (ret < 0) {
        perror("Changing scheduler failed. Aborting...");
        exit(-1);
    }

}

int main() {
    change_scheduler();
    printf("Switch finish!\n");
}
