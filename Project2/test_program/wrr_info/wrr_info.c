#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h> /* high-res timers */
#include <sched.h> /* for std. scheduling system calls */


static void get_wrr_info()
{
    int  policy,tmp;
    struct timespec wrr_time;
    pid_t pid;
    printf("Input the process id (PID) you want to check: ");
    scanf("%d", &tmp);
    pid = tmp;
    // struct task_struct *p = find_task_by_vpid(pid);
    if (!pid) {
        perror("Invalid PID. Aborting...");
        exit(-1);
    }
    sched_rr_get_interval(pid,&wrr_time);
    policy = sched_getscheduler(pid);
    printf("Schedule policy: ");
    switch(policy)
	{
		case 0: printf("normal \n"); break;
		case 1: printf("fifo \n"); break;
		case 2: printf("rr \n"); break;
		case 3: printf("batch \n"); break;
		case 5: printf("idle \n"); break;
		case 6: printf("wrr \n"); break;
	}
    tmp = wrr_time.tv_nsec/1000000;
    printf("Timeslice: %d milisec\n", tmp);

}

int main() {
    get_wrr_info();
}
