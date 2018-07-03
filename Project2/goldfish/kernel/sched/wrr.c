/*
 * WRR Scheduling Class
 */

#include "sched.h"

#include <linux/slab.h>


#ifdef CONFIG_SMP

static int
select_task_rq_wrr(struct task_struct *p, int sd_flag, int flags) // not required
{
    int cpu, temp, result,tmp_result;
    int minimum_weight;
    struct rq *rq;
    result = task_cpu(p);
    tmp_result = result;
    if (p->wrr.nr_cpus_allowed == 1)
        return result;
    
    minimum_weight = cpu_rq(result)->wrr.total_weight;
    //printk("Selecting task_rq: \n");
    rcu_read_lock();
    for_each_online_cpu(cpu) {
        if (!cpumask_test_cpu(cpu, tsk_cpus_allowed(p)))
            continue;
        rq = cpu_rq(cpu);
        temp = rq->wrr.total_weight;
        //printk("cpu: %d nr_running: %d total_weight: %d\n",cpu,rq->wrr.wrr_nr_running,temp);
        if (temp < minimum_weight) {
            minimum_weight = temp;
            result = cpu;
        }
    }
    rcu_read_unlock();
    //if(tmp_result!=result)
    //printk("SMP: to CPU %d\n", result);
    return result;
}


#endif /*COMFIG_SMP*/

void pull_wrr_task(int dst_cpu)
{
    int src_cpu;
    struct rq *dst_rq = cpu_rq(dst_cpu);
    struct rq *src_rq;
    struct sched_wrr_entity *src_wrr;
    struct task_struct *p;

    if (!cpu_online(dst_cpu))
        return;

    src_rq = NULL;
    for_each_online_cpu(src_cpu) {
        if (src_cpu == dst_cpu)
            continue;

        src_rq = cpu_rq(src_cpu);

        double_rq_lock(dst_rq, src_rq);
        if (list_empty(&src_rq->wrr.queue)) {
            double_rq_unlock(dst_rq, src_rq);
            continue;
        }

        list_for_each_entry(src_wrr, &src_rq->wrr.queue, run_list) {
            p = container_of(src_wrr, struct task_struct, wrr);

            if (task_running(src_rq, p))
                continue;

            if (p->policy != SCHED_WRR)
                continue;

            if (!cpumask_test_cpu(dst_cpu, tsk_cpus_allowed(p)))
                continue;

            if (p->on_rq) {

                deactivate_task(src_rq, p, 0);
                set_task_cpu(p, dst_cpu);
                activate_task(dst_rq, p, 0);

                check_preempt_curr(dst_rq, p, 0);

                double_rq_unlock(dst_rq, src_rq);
                return;
            }
        }

        double_rq_unlock(dst_rq, src_rq);
    }
}

#ifdef CONFIG_SMP
static void set_cpus_allowed_wrr(struct task_struct *p,
                const struct cpumask *new_mask)
{
}
/* Assumes rq->lock is held */
static void rq_online_wrr(struct rq *rq)
{
}
/* Assumes rq->lock is held */
static void rq_offline_wrr(struct rq *rq)
{
}
static void pre_schedule_wrr(struct rq *rq, struct task_struct *prev)
{
}
static void post_schedule_wrr(struct rq *rq)
{
}
static void task_woken_wrr(struct rq *rq, struct task_struct *p)
{
}
static void switched_from_wrr(struct rq *rq, struct task_struct *p)
{
}
#endif

/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 */
static void update_curr_wrr(struct rq *rq)
{
    struct task_struct *curr = rq->curr;
    u64 delta_exec;

    if (curr->sched_class != &wrr_sched_class)
        return;

    delta_exec = rq->clock_task - curr->se.exec_start;
    if (unlikely((s64)delta_exec < 0))
        delta_exec = 0;

    schedstat_set(curr->se.statistics.exec_max,
              max(curr->se.statistics.exec_max, delta_exec));

    curr->se.sum_exec_runtime += delta_exec;
    account_group_exec_runtime(curr, delta_exec);

    curr->se.exec_start = rq->clock_task;
    cpuacct_charge(curr, delta_exec);

}

/*
 * Preempt the current task with a newly woken task if needed: We don't take it into account
 */
static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static struct task_struct *pick_next_task_wrr(struct rq *rq)
{
    struct sched_wrr_entity *result;
    struct task_struct *p;

    if (rq->wrr.wrr_nr_running == 0)
        return NULL;

    result = list_first_entry(&((rq->wrr)).queue, struct sched_wrr_entity, run_list);

    p = container_of(result, struct task_struct, wrr);

    return p;
}

static void enqueue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se, bool head)
{
    struct list_head *queue = &(rq->wrr.queue);
    // struct task_struct *p;

    // p = container_of(wrr_se, struct task_struct, wrr);
    // printk("Enqueuing process: %d\n", p->pid);
    struct task_struct *p;
    p = container_of(wrr_se, struct task_struct, wrr);
    // printk("Dequeuing process: %d\n", p->pid);
    struct task_group *tg = task_group(p);
    char group_path[1024];
    if (!(autogroup_path(tg, group_path, 1024)))
    {
        if (!tg->css.cgroup) {
            group_path[0] = '\0';
        }
        cgroup_path(tg->css.cgroup, group_path, 1024);
    }

    if(group_path[1]=='b'&&p->wrr.weight!=1)
    {
        p->wrr.time_slice = WRR_BG_TIMESLICE;
        p->wrr.weight = 1;
        // rq->wrr.total_weight -= 9;
        printk("Change timeslice to background\n");
    }
    else if(group_path[1]!='b' && p->wrr.weight!=10)
    {
        p->wrr.time_slice = WRR_FG_TIMESLICE;
        p->wrr.weight = 10;
        // rq->wrr.total_weight += 9;
        printk("Change timeslice to foreground\n");
    }

    if(head)
        list_add(&wrr_se->run_list, queue);
    else
        list_add_tail(&wrr_se->run_list, queue);
    
    rq->wrr.total_weight += wrr_se->weight;
    ++rq->wrr.wrr_nr_running;
}

static void dequeue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se)
{
    // struct task_struct *p;
    // p = container_of(wrr_se, struct task_struct, wrr);
    // printk("Dequeuing process: %d\n", p->pid);
    list_del_init(&wrr_se->run_list);
    rq->wrr.total_weight -= wrr_se->weight;
    --rq->wrr.wrr_nr_running;
}

/*
 * Adding/removing a task to/from a priority array:
 */
static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_wrr_entity *wrr_se = &p->wrr;
    
    enqueue_wrr_entity(rq, wrr_se, flags & ENQUEUE_HEAD);
    inc_nr_running(rq);
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_wrr_entity *wrr_se = &p->wrr;

    update_curr_wrr(rq);
    dequeue_wrr_entity(rq, wrr_se);
    dec_nr_running(rq);
}



void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
    
    INIT_LIST_HEAD(&wrr_rq->queue);
    wrr_rq->wrr_nr_running = 0;
    wrr_rq->total_weight = 0;
}

/*
 * Put task to the end of the run list without the overhead of dequeue
 * followed by enqueue.
 */
static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
    printk("Requeuing task: %d\n", p->pid);
    struct sched_wrr_entity *wrr_se = &p->wrr;
    struct list_head *queue = &(rq->wrr.queue);

    if (head)
        list_move(&wrr_se->run_list, queue);
    else
        list_move_tail(&wrr_se->run_list, queue);
}

static void yield_task_wrr(struct rq *rq)
{
    requeue_task_wrr(rq, rq->curr, 0);
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
    update_curr_wrr(rq);
    prev->se.exec_start = 0;
}

static void watchdog(struct rq *rq, struct task_struct *p)
{
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
    struct sched_wrr_entity *wrr_se = &p->wrr;
    update_curr_wrr(rq);

    watchdog(rq, p);

    struct task_group *tg = p->sched_task_group;
    char group_path[1024];
    if (!(autogroup_path(tg, group_path, 1024)))
    {
        if (!tg->css.cgroup) {
            group_path[0] = '\0';
        }
        cgroup_path(tg->css.cgroup, group_path, 1024);
    }

    printk("task_tick_wrr: task_group: %s\n", group_path);
    printk("  cpu: %d task_tick: %d time_slice: %d\n",
     cpu_of(rq),
     p->pid,
     p->wrr.time_slice);

    if (--p->wrr.time_slice)
        return;

    
    if(p->wrr.weight == 1)
        p->wrr.time_slice = WRR_BG_TIMESLICE;
    else
        p->wrr.time_slice = WRR_FG_TIMESLICE;

    if (wrr_se->run_list.prev != wrr_se->run_list.next) {
        requeue_task_wrr(rq, p, 0);
        resched_task(p);
    }
}

static void set_curr_task_wrr(struct rq *rq)
{
    struct task_struct *p = rq->curr;

    p->se.exec_start = rq->clock_task;
}

/*
 * When switching a task to WRR, we may overload the runqueue
 * with WRR tasks. In this case we try to push them off to
 * other runqueues.
 */
static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
    /*
     * If we are already running, then there's nothing
     * that needs to be done. But if we are not running
     * we may need to preempt the current running task.
     * If that current running task is also an WRR task
     * then see if we can move to another run queue.
     */
    struct task_group *tg = p->sched_task_group;
    char group_path[1024];
    if (p->on_rq && rq->curr != p)
    {
        if (rq == task_rq(p) && !rt_task(rq->curr))
            resched_task(rq->curr);
        if (!(autogroup_path(tg, group_path, 1024)))
        {
            if (!tg->css.cgroup) {
                group_path[0] = '\0';
                // return group_path;
            }
            cgroup_path(tg->css.cgroup, group_path, 1024);
        }
        if(group_path[1]=='b')
            p->wrr.weight = 1;
        else
            p->wrr.weight = 10;
    }
    printk("Finish switch_to_wrr pid: %d !\n",p->pid);
}
/* Without prio policy */
static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p)
{
    struct task_group *tg = p->sched_task_group;
    char group_path[1024];
    if (!(autogroup_path(tg, group_path, 1024)))
    {
        if (!tg->css.cgroup) {
            group_path[0] = '\0';
            // return group_path;
        }
        cgroup_path(tg->css.cgroup, group_path, 1024);
    }
    printk("get_wrr_interval_Task_group: %s\n", group_path);
    if(group_path[1]=='\0')
        return WRR_FG_TIMESLICE;
    else
        return WRR_BG_TIMESLICE;
}


#ifdef CONFIG_WRR_GROUP_SCHED



#endif

const struct sched_class wrr_sched_class = {
    .next           = &fair_sched_class,        /*Required*/
    .enqueue_task       = enqueue_task_wrr,     /*Required*/
    .dequeue_task       = dequeue_task_wrr,     /*Required*/
    .yield_task     = yield_task_wrr,       /*Required*/

    .check_preempt_curr = check_preempt_curr_wrr,       /*Required*/

    .pick_next_task     = pick_next_task_wrr,       /*Required*/
    .put_prev_task      = put_prev_task_wrr,        /*Required*/
    // .task_fork          = task_fork_wrr,
#ifdef CONFIG_SMP
    .select_task_rq     = select_task_rq_wrr,          /*Never need impl */

    .set_cpus_allowed       = set_cpus_allowed_wrr,        /*Never need impl */
    .rq_online              = rq_online_wrr,           /*Never need impl */
    .rq_offline             = rq_offline_wrr,          /*Never need impl */
    .pre_schedule       = pre_schedule_wrr,        /*Never need impl */
    .post_schedule      = post_schedule_wrr,           /*Never need impl */
    .task_woken     = task_woken_wrr,          /*Never need impl */
    .switched_from      = switched_from_wrr,           /*Never need impl */
#endif

    .set_curr_task          = set_curr_task_wrr,        /*Required*/
    .task_tick      = task_tick_wrr,        /*Required*/

    .get_rr_interval    = get_rr_interval_wrr,

    .prio_changed       = prio_changed_wrr,        /*Never need impl */
    .switched_to        = switched_to_wrr,         /*Never need impl */
};

#ifdef CONFIG_SMP

#endif