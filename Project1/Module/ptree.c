#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/unistd.h>
#include<linux/list.h>
#include<linux/string.h>
#include<linux/syscalls.h>
#include<linux/slab.h>          
#include<linux/uaccess.h>      

MODULE_LICENSE("Dual BSD/GPL");

#define __NR_ptree 287
#define MAX_NUM_PROC 60000

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
int nr_tmp;
struct prinfo buf_tmp[MAX_NUM_PROC];
void DFS(struct task_struct *entry,int level)
{
    struct task_struct *sibling_ptr=NULL, *child_ptr=NULL;
    struct list_head *item;
    if(entry == NULL)
        return;
    
   // copying
    buf_tmp[nr_tmp].level = level;
    buf_tmp[nr_tmp].pid = entry->pid;
    buf_tmp[nr_tmp].state = entry->state;
    buf_tmp[nr_tmp].uid = entry->cred->uid;
    get_task_comm(buf_tmp[nr_tmp].comm, entry);
    if(list_empty(&entry->children))
        buf_tmp[nr_tmp].first_child_pid=0;
    else
    {
        child_ptr = list_entry((&entry->children)->next, struct task_struct, sibling);
        if(child_ptr != NULL)
            buf_tmp[nr_tmp].first_child_pid = child_ptr->pid;
        else
            buf_tmp[nr_tmp].first_child_pid = 0;
    }

    if(list_empty(&entry->sibling))
        buf_tmp[nr_tmp].next_sibling_pid=0;
    else
    {
        sibling_ptr = list_entry((&entry->sibling)->next, struct task_struct, sibling);
        if(sibling_ptr != NULL)  // no sibling will point to parent.
            buf_tmp[nr_tmp].next_sibling_pid = sibling_ptr->pid;
        else
            buf_tmp[nr_tmp].next_sibling_pid = 0;
    }

    nr_tmp++;

    printk("Copy~ comm: %s, pid: %d, ppid: %d, uid: %ld\n", buf_tmp[nr_tmp].comm,
        buf_tmp[nr_tmp].pid,buf_tmp[nr_tmp].parent_pid,buf_tmp[nr_tmp].uid);
    if (!list_empty(&entry->children))
        list_for_each(item, &entry->children)
        {
            child_ptr = list_entry(item, struct task_struct, sibling);
            DFS(child_ptr,level+1);
        }
}



static int (*oldcall)(void);
static int ptree(struct prinfo *buf, int *nr)
{
    int i;
    nr_tmp = 0;
    printk("Constructing!\n");
    // To copy processes info to buf by DFS.
    read_lock(&tasklist_lock);
    DFS(&init_task,0); 
    read_unlock(&tasklist_lock);
    printk("Complete ptree!\n");
    *nr = nr_tmp;
    for(i=0; i<nr_tmp; i++)
    {
        strcpy(buf[i].comm, buf_tmp[i].comm);
        buf[i].pid = buf_tmp[i].pid;
        buf[i].state = buf_tmp[i].state;
        buf[i].parent_pid = buf_tmp[i].parent_pid;
        buf[i].first_child_pid = buf_tmp[i].first_child_pid;
        buf[i].next_sibling_pid = buf_tmp[i].next_sibling_pid;
        buf[i].uid = buf_tmp[i].uid;
        buf[i].level = buf_tmp[i].level;
    }
    return 0;
}
static int addsyscall_init(void)
{
    long *syscall = (long*)0xc000d8c4;
    oldcall = (int(*)(void))(syscall[__NR_ptree]);
    syscall[__NR_ptree] = (unsigned long) ptree;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void)
{
    long *syscall = (long*) 0xc000d8c4;
    syscall[__NR_ptree] = (unsigned long) oldcall;
    printk(KERN_INFO "module exit!\n");
}
module_init(addsyscall_init);
module_exit(addsyscall_exit);
