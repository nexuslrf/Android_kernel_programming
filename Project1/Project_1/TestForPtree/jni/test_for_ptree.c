/*
Problem 2
To test our system call, and display our result.
*/
#include<stdio.h>
#define MAX_NUM_PROC 10000

struct prinfo 
{
    pid_t parent_pid; /* process id of parent */
    pid_t pid;   /* process id */
    pid_t first_child_pid; /* pid of youngest child */
    pid_t next_sibling_pid; /* pid of older sibling */
    long state;  /* current state of process */
    long uid;   /* user id of process owner */
    char comm[64];   /* name of program excuted */
    int level;  /* to identify the hierarchy of a process*/
};

int main()
{
    struct prinfo buf[MAX_NUM_PROC],*p;
    int nr,i,j;
    /* call our system call by its system call number */
    if(syscall(287, buf, &nr)<0)
    {
        fprintf(stderr, "Syscall Failed"); /* exception case */
        return 1;
    }
    printf("There are %d processes.\n", nr); 
    /* printing the ptree */
    for(i = 0; i<nr; i++)
    {
        p = buf+i;
        for(j = 0; j<p->level; j++)
            printf("\t");
        printf("%s, %d, %ld, %d, %d, %d, %ld\n", p->comm, p->pid, p->state,
            p->parent_pid, p->first_child_pid, p->next_sibling_pid, p->uid);
    }
    return 0;
}
