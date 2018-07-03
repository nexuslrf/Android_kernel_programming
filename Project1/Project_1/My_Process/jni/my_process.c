/*
Problem 3
generating child process and call ptree to show relationship of these 2 processes
function used: fork & execl
*/
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
int main()
{
    pid_t pid;
    printf("516030910383_Parent %d\n",(int)getpid());
    pid = fork(); /* creating child process */

    /* exception case */
    if(pid<0)
    {
        fprintf(stderr, "Fork Failed"); 
        return 1;
    }
    /* child process */
    else if(pid==0)
    {
	    printf("516030910383_Child %d\n",(int)getpid());
        execl("./ptreeARM","ptree",(char *) NULL); /* execute ptree */
    }
    /* parent process */
    else
    {
        /* wait until child process finishes */
        wait(NULL);
    }
    return 0;
}
