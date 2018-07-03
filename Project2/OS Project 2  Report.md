# OS Project 2  Report

## Introduction

CPU scheduling is the basis of multiprogrammed operating systems. By switching the CPU among processes, the operating system can make the computer more productive. In this project, we are asked to implement a particular CPU scheduler for Linux based Android operating system.

Our new Android scheduler is a case-distinct Round-robin schedule policy  ---- weighted Round-robin (WRR for short).  WRR will assign more execution time unit for foreground tasks and less time for background tasks.

## Problem Analysis & Implementation

To better understand how linux scheduler works, especially how the existing RR scheduler works, we need to refer the provided [website](https://helix979.github.io/jkoo/post/os-scheduler/), and to see the source code in "/kernel/sched" like "core.c" and "rt.c".

After some investigation, we can get a big picture of the Linux scheduler structure, like the figure below.

![overview](http://zzjlzx.blog.chinaunix.net/attachment/201401/13/29060569_1389579925Y6Yq.png)

So our job is to write a new sched_class  similar with existing rt and cfs sched_classes. And define corresponding running queue struct and entity structure to store some necessary information. 

 Based on analysis above, we have the following implementation:

* Modify some existing files: 

  * goldfish_armv7_defconfig : add `CONFIG_WRR_GROUP_SCHED`  option.

  *  /include/linux/sched.h : 

    * define `SCHED_WRR` with const value 6 to indicate our WRR policy.
    * define `WRR_BG_TIMESLICE` & `WRR_FG_TIMESLICE` with const value 10ms and 100ms respectively.
    * define `sched_wrr_entity` structure to store some necessary information for each wrr task. The information contains:
      *  `timeslices`: used as a counter to indicate how much time per RR turn.
      * `weight` : to indicate the state of a task: when a task is in foreground, its `weight` will be 10, when it is in background, its `weight` will be 1. This weight is also used for load-balance purpose.
      * `run_list` : a `link_head` instance to link neighbor tasks on the same running queue.

    In addition to these definitions, we also need to declare some  wrr variables in proper places:

    * add `sched_wrr_entity` member `wrr` to `task_structure`.
    * declare `wrr_rq` structure.

  * /kernel/sched/sched.h :

    * define `wrr_rq` structure, and add it to `rq` structure. As running queue, `wrr_rq` maintains general information of `wrr_entity`s, like `wrr_nr_running` (the number of tasks in this queue), `total_weight`.

      Unlike `rt_rq` , we don't define `prio_array `. `prio_array` need to work with `bitmap`. If we include `prio_array` struct in WRR, our project will be more difficult for coding and rebalancing. 

    * declare some external function which will be implemented later on.

  * /kernel/sched/core.c :

    * revise function `__sched_fork()`, `sched_fork()`,  `wake_up_new_task()`, `scheduler_tick()`, `rt_mutex_setprio()`,  `__setscheduler()`, `__sched_setscheduler()`, `sched_init()` to make WRR has the same behavior with RT and FAIR.

  * /include/linux/init_task.h :

    * add `.wrr` struct for initialization.

  * /kernel/sched/Makefile ： 

    * add `wrr.o` for compilation.

* Create /kernel/sched/wrr.c. It contains `wrr_sched_class`, which is major part of WRR.  The main idea of implementing  this class and related functions is to imitate and revise codes in rt.c. We omit all codes related to `SMP` and `Preemption` and substitute `bitmap` and `prio_array` struct operations with in-built `list` operations like `list_add` and `list_del`.

  The things we need further add to `wrr_sched_class` is the codes related to judging the state of  tasks, and switching the timeslice the tasks' states changing.  To implement this we have to use function `task_group_path()` in /sched/debug.c to get the state of a task. We add the state checking code in function `enqueue_wrr_entity()`. 

Description above is the general idea of the implementation. After these modifications and creations, we can type `make -jx`(x indicates number of core used) to get the target image file `zImage`.

# Note!  Add timeslice changing in task tick

## Testing Result

Now that we have a new kernel with WRR scheduler, we can load it to Android emulator to see how well it works.

Before testing, we need write a test program to set a task's schedule policy. So we write a program `set_sched` getting some necessary input for changing schedule policy. This program uses in-build syscall `sched_setscheduler()` to finish switching.

Besides, we also write a program `wrr_info` by using syscall `sched_getscheduler()` and `sched_rr_get_interval()` to a task's schedule policy information and execution time interval.

We randomly select an APP in Android emulator and run it. By typing `ps -P | grep [APP's name]`,  we can get sufficient information for `set_sched` . After running `set_sched`, we can get the information from kernel message like fig below.  Apparently, the target task has switched to WRR schedule policy. 



If we further run `wrr_info`, we will get the following information. The task is in foreground with timeslice 100ms.



Then we click home button in emulator, to see how kernel information changes. In figure below, we can see the state of this task changes to background and time slice also changes to background timeslice: 10ms.



## Obstacles

To 

## Additional Work/Bonus

## Future Work

## Achievements

