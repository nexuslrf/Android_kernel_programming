#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <ctype.h>
#include <sys/time.h> /* high-res timers */

int main(int argc, char *argv[])
{
    int k = 0x0fffffff;
    int i,j=0;
    pid_t pid;
    struct timespec wrr_time;
    struct sched_param param;
    param.sched_priority = 0;
    printf("Input number of dummy: ");
    scanf("%d", &j);
	printf("%d\n", getpid());
    for(i=0;i<j;i++)
    {
        pid = fork();
        if(pid)
        {
            printf("chd pid %d: %d\n",i,pid);
        }
        else
        {

            sched_setscheduler(getpid(), 6, &param);
            while (k--) 
            {
                if(k%0x0ffffff)
                    sched_rr_get_interval(getpid(),&wrr_time);
               // printf("block\n");
                ;
            }
            return;
        }
    }
    // wait();
	return 0;
}