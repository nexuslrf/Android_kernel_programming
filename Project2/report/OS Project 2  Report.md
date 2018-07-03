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

  * /kernel/sched/rt.c

    * modify the struct vaiable `.next` in `rt_sched_class` to make it point to `wrr_sched_class`

  * /include/linux/init_task.h :

    * add `.wrr` struct for initialization.

  * /kernel/sched/Makefile ： 

    * add `wrr.o` for compilation.

* Create /kernel/sched/wrr.c. It contains `wrr_sched_class`, which is major part of WRR.  The main idea of implementing  this class and related functions is to imitate and revise codes in rt.c. We omit all codes related to `SMP` and `Preemption` and substitute `bitmap` and `prio_array` struct operations with in-built `list` operations like `list_add` and `list_del`.

  The things we need further add to `wrr_sched_class` is the codes related to judging the state of  tasks, and switching the timeslice the tasks' states changing.  To implement this we have to use function `task_group_path()` in /sched/debug.c to get the state of a task. We add the state checking code in function `enqueue_wrr_entity()`. 

Description above is the general idea of the implementation. After these modifications and creations, we can type `make -jx`(x indicates number of core used) to get the target image file `zImage`.

## Testing Result

Now that we have a new kernel with WRR scheduler, we can load it to Android emulator to see how well it works.

Before testing, we need write a test program to set a task's schedule policy. So we write a program `set_sched` getting some necessary input for changing schedule policy. This program uses in-build syscall `sched_setscheduler()` to finish switching.

Besides, we also write a program `wrr_info` by using syscall `sched_getscheduler()` and `sched_rr_get_interval()` to a task's schedule policy information and execution time interval.

We randomly select an APP in Android emulator and run it. By typing `ps -P | grep [APP's name]`,  we can get sufficient information for `set_sched` . After running `set_sched`, we can get the information from kernel message like fig below.  Apparently, the target task has switched to WRR schedule policy. 

![kernel1](/Users/nexuslrf/Onedrive/CS_Courses/OperatingSystems/Projects/Project2/report/kernel1.png)

If we further run `wrr_info`, we will get the following information. The task is in foreground with timeslice 100ms.

![shell1](/Users/nexuslrf/Onedrive/CS_Courses/OperatingSystems/Projects/Project2/report/shell1.png)

Then we click home button in emulator, to see how kernel information changes. In figure below, we can see the state of this task changes to background and time slice also changes to background timeslice: 10ms.

![kernel2](/Users/nexuslrf/Onedrive/CS_Courses/OperatingSystems/Projects/Project2/report/kernel2.png)

## Obstacles

To implement wrr scheduler, we really need read a lot of codes and refer many documents to understand its mechanism. This project is very different from projects we have done before. We are asked to create something new, but we also need to follow kind of strict programming rule to make WRR work in the whole operating system. 

Firstly, we need to figure out reliance of different code files, which require us to read many related materials. Luckily, I find some useful tips from [stack overflow](https://stackoverflow.com/questions/3086864/how-to-create-a-new-linux-kernel-scheduler), which shows almost every related files. 

Secondly, problems come from compiling stage. For the first several times compilations, I encountered many errors, such as inproper substitution of "rt" variables, lack of declaration, implicit reliance.

Then, problems arise from switching stage. When I lauched an APP and prepared to run `set_sched ` program, the APP crashed with no respond to my `setscheduler` function. This took me a lot of time to solve. Finally, I found the problem was in rt.c file. Now that we added a new sched_class, we need to add `wrr_sched_class` to sched_class link list to make it work.  But we only set `wrr_sched_class`'s next sched_class, so no sched_class points to `wrr_sched_class`. Therefore we need additional modification in rt.c file.

After that I still got problem in `setscheduler`, this time I received kernel-panic message. I really didn't know how to deal with it. This incorrect `wrr_sched_class` uses similar structure with `rt_sched_class`'s, so it contains `prio_array`, `bitmap` and `rt_bandwidth`. By trials and errors, I decide to omit these functions and try to make my `wrr_sched_class` be simple and understandable. 

By now my own `wrr_sched_class` finally works.

## Additional Work/Bonus

My additional work mainly focuses on multi-cpu archecture, actually symmetric multi-processor archecture (SMP for short), and load balance among multiple cpus. 

Due to our provided emulator is a uniprocessor archecture, which is meaningless for this work. And now it turn to use some real devices for development. Luckily, I have a [Google Nexus 7 (2012)](https://www.asus.com/hk/Tablets/Nexus_7/) tablet, which is bought nearly 5 years ago.  This tablet use an ARMv7 quad-core CPU which is embedded in the Nvidia Tegra 3 SoC, so it can well support SMP functions.

Load a kernel on tablet will be another huge work. The kernel our emulator uses cannot load to a real devices for lacking some necessary lower layer API. So we need download a new kerrnel, and do all the work over again. The kernel file can be got from [AOSP web](https://source.android.com/setup/build/building-kernels). 

The tablet uses a linux kernel with 3.1.10 version, which is earlier than our emulator's goldfish version (3.4.67), so some files have different name and some struct have different definitions. Therefore, I need to write a new WRR scheduler from scratch. 

After writing the normal WRR scheduler, I take a easy method to support SMP load balance — when a new wrr task comes to execution, `wrr_sched_class` select an available CPU with the smallest `wrr_rq.total_weight` value and assign new task to this CPU. This feature is implemented by `wrr_sched_class` member function `select_task_rq_wrr`. 

After finishing these modification, I start to build my kernel and load it onto my tablet. This process follows the tutorial from [packtpub](https://hub.packtpub.com/customizing-kernel-and-boot-sequence/) and [stackexchange](https://android.stackexchange.com/questions/69954/how-to-unpack-and-edit-boot-img-for-rom-porting). Then the tablet will run my own kernel.

![buildkl](/Users/nexuslrf/Onedrive/CS_Courses/OperatingSystems/Projects/Project2/report/buildkl.png)

By doing some normal tests, I find that WRR scheduler works fine for this real device. Next, I test how it works for SMP case.

This time, I write a dummy program which runs a meaningless`while` loop. I run 8 dummy programs at the same time (If I run more, my device is too busy to deal with them).

![dummy1](/Users/nexuslrf/Onedrive/CS_Courses/OperatingSystems/Projects/Project2/report/SMP_$$.png)

![smp1](/Users/nexuslrf/Onedrive/CS_Courses/OperatingSystems/Projects/Project2/report/SMP_4.png)

As you can see these 4 dummy programs run in different CPUs at the same time, which shows my load balance policy works.

## Achievements

From this project, I learnt a lot about the linux kernel programming, and experienced the combination of hardware device and software system. This gives me a great impression on the structure of Android operating system. I think this project is beneficial for my future programming development.