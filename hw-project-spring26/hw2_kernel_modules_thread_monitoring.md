# Homework 2: Kernel Module for Thread Monitoring and User‑Space Drone Simulation

*Prepared wıth the help of DeepSeek AI*  
**Due date:** TBA  

---

## Objectives

- Write a Linux kernel module that can be loaded and unloaded dynamically.
- Use the kernel’s process/thread data structures to retrieve information about all tasks (processes and threads).
- Create a `/proc` entry that exposes thread information to user space.
- Implement a multi‑threaded user‑space program simulating a drone delivery system.
- Use the kernel module to observe how the scheduler manages threads and how thread states change under different workloads.

This homework aims to help you observe the effects of concurrency from the kernel’s perspective.

---

## Background

### Kernel Modules
A kernel module is a piece of code that can be loaded into the running kernel without rebooting. This allows us to extend kernel functionality dynamically. In this assignment, you will write a module that creates an entry in the `/proc` filesystem, reads information about running tasks (processes and threads), and makes that information available to user‑space programs.

### Threads in the Kernel
In Linux, each thread is represented by a `struct task_struct` (the same structure used for processes). The kernel sees no fundamental difference between a process with one thread and a process with many threads – they are all “tasks”. The `task_struct` contains fields such as:
- `pid` – process ID (unique for each task)
- `tgid` – thread group ID (the PID of the main thread of the process)
- `__state` – current state (running, sleeping, etc.)
- `utime`, `stime` – user and system CPU time
- `se.vruntime` – virtual runtime used by the scheduler (for Completely Fair Scheduler)

### User‑Space Threads
In user space, we use the POSIX threads library (`pthreads`) to create multiple threads within a single process. Each such thread is backed by a kernel task (a lightweight process). Thus, when you create a thread with `pthread_create()`, the kernel creates a new `task_struct` that shares the same memory space and other resources as the parent process.

### Drone Simulation Example
From the lecture notes, we have a drone shipping simulation where multiple “drone” threads concurrently pick orders from a shared queue and deliver them. This example demonstrates:
- Thread creation and management.
- Sharing a resource (the order queue).
- Synchronization using mutexes and condition variables.

In this assignment, you will adapt that example to also monitor the drone threads using your kernel module.

---

## Part 1: Kernel Module Basics

Before building the full monitoring module, you will write a simple module to practice loading, unloading, and viewing kernel messages.

### 1.1 Create a Simple Module

Write a file `simple.c`:

```c
#include <linux/kernel.h>   /* for pr_info() */
#include <linux/module.h>   /* for module_init, module_exit, MODULE_LICENSE */
#include <linux/init.h>     /* for __init, __exit */

static int __init simple_init(void)
{
    pr_info("Simple module loaded\n");
    return 0;
}

static void __exit simple_exit(void)
{
    pr_info("Simple module unloaded\n");
}

module_init(simple_init);
module_exit(simple_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A very simple kernel module");
MODULE_AUTHOR("Your Name");
```

### 1.2 Create a Makefile

Create a `Makefile` in the same directory:

```make
obj-m += simple.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### 1.3 Compile and Load the Module

- Run `make` to build the module. You should get a file `simple.ko`.
- Load the module with `sudo insmod simple.ko`.
- Verify it is loaded with `lsmod | grep simple`.
- View the kernel log with `dmesg | tail` – you should see “Simple module loaded”.
- Remove the module with `sudo rmmod simple` and check the log again.

**Record** a screenshot showing the output of `lsmod` after loading and the relevant lines from `dmesg`.

---

## Part 2: Thread Information Module

Now you will extend the simple module to create a `/proc` entry that lists all threads in the system with their PID, TGID, state, and CPU times.

### 2.1 Adding a /proc Entry

Modify the previous module (or create a new one) to include:

- A function that iterates over all tasks using `for_each_process(task)`.
- Builds a string with one line per thread.
- Uses `copy_to_user()` to transfer data to user space when the file is read.
- Handles partial reads (using the `*off` parameter) to allow `cat` and other tools to work.

**Use the provided template** (see `thread_monitor.c` below) as a starting point.

### 2.2 Template: `thread_monitor.c`

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>   /* for for_each_process */
#include <linux/uaccess.h>
#include <linux/init.h>

#define PROC_NAME "thread_monitor"

static const char * const task_state_array[] = {
    "R (running)",   /* 0x00 */
    "S (sleeping)",  /* 0x01 */
    "D (disk sleep)",/* 0x02 */
    "T (stopped)",   /* 0x04 */
    "t (tracing stop)",/* 0x08 */
    "X (dead)",      /* 0x10 */
    "Z (zombie)",    /* 0x20 */
    "P (parked)",    /* 0x40 */
    "I (idle)",      /* 0x80 */
};

static ssize_t thread_monitor_read(struct file *file, char __user *buf,
                                   size_t len, loff_t *off)
{
    char *kbuf;
    size_t kbuf_size = PAGE_SIZE;
    size_t written = 0;
    struct task_struct *task;

    kbuf = kmalloc(kbuf_size, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    rcu_read_lock();

    for_each_process(task) {
        int state_idx = task_state_index(task);
        const char *state_str = task_state_array[state_idx];
        unsigned long long utime = task->utime;
        unsigned long long stime = task->stime;

        written += snprintf(kbuf + written, kbuf_size - written,
                            "PID: %d, TGID: %d, State: %s, utime: %llu, stime: %llu\n",
                            task->pid, task->tgid, state_str, utime, stime);

        if (written >= kbuf_size - 128)
            break;
    }

    rcu_read_unlock();

    /* Handle partial reads */
    if (*off >= written) {
        kfree(kbuf);
        return 0;
    }
    if (len > written - *off)
        len = written - *off;

    if (copy_to_user(buf, kbuf + *off, len)) {
        kfree(kbuf);
        return -EFAULT;
    }
    *off += len;
    kfree(kbuf);
    return len;
}

static const struct proc_ops thread_monitor_ops = {
    .proc_read = thread_monitor_read,
};

static int __init thread_monitor_init(void)
{
    proc_create(PROC_NAME, 0444, NULL, &thread_monitor_ops);
    pr_info("Thread monitor module loaded\n");
    return 0;
}

static void __exit thread_monitor_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("Thread monitor module unloaded\n");
}

module_init(thread_monitor_init);
module_exit(thread_monitor_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Thread monitor via /proc/thread_monitor");
MODULE_AUTHOR("Your Name");
```

### 2.3 Update the Makefile

If you named the module `thread_monitor`, update `obj-m += thread_monitor.o`.

### 2.4 Test the Module

- Compile with `make`.
- Load: `sudo insmod thread_monitor.ko`.
- Check the output: `cat /proc/thread_monitor | head -20`.
- You should see a list of all threads (including kernel threads).

**Record** a screenshot showing the output of `cat /proc/thread_monitor` for at least the first 20 lines.

---

## Part 3: Drone Simulation with Thread Monitoring

Now you will write a user‑space program that simulates a drone delivery system using multiple threads. The program will:

- Create a number of drone threads (e.g., 4).
- Each drone thread repeatedly picks an order from a shared queue, “delivers” it (simulate with a sleep), and repeats.
- A separate order generator thread adds orders to the queue.
- Periodically (every 2 seconds), the main thread reads `/proc/thread_monitor` and prints the information about the drone threads and the generator thread (you can filter by TGID or just print all).

**You will adapt the drone shipping example from the lecture** (see `4b-multi-thread-programming-examples.md` for reference).

### 3.1 Implementation Outline

Create a C file `drone_sim.c` with the following components:

- Global data structures: a queue (linked list or `std::queue` if using C++) and associated mutex/condition variable. Use POSIX threads.
- Drone worker function: loop forever, take an order, simulate delivery, repeat.
- Generator function: add orders to the queue at random intervals.
- Main function: create threads, then monitor periodically.

**Important**: Use `pthread_create()` and `pthread_join()`. Include proper synchronization to avoid race conditions.

### 3.2 Monitoring

The monitoring part should:

- Open `/proc/thread_monitor` for reading.
- Read its contents (or just the first few lines) and print them.
- Optionally, filter to show only threads belonging to your program (you can get the PID of the main thread via `getpid()` and compare TGID with that). This makes the output less cluttered.

**Example monitoring loop in `main()`:**

```c
while (1) {
    sleep(2);
    FILE *fp = fopen("/proc/thread_monitor", "r");
    if (!fp) {
        perror("fopen");
        break;
    }
    char line[256];
    printf("\n--- Thread snapshot ---\n");
    while (fgets(line, sizeof(line), fp)) {
        // Optionally filter by TGID
        printf("%s", line);
    }
    fclose(fp);
}
```

### 3.3 Compile and Run

- Compile with `gcc -pthread -o drone_sim drone_sim.c`
- Load the kernel module first (if not already loaded).
- Run `./drone_sim`. You should see periodic snapshots of all threads, including your drone threads.

**Record** a screenshot showing the output of your drone simulation, especially the monitoring snapshots. Highlight the lines corresponding to your drone threads (they should appear with state `R` when running, or `S` when sleeping).

---

## Part 4: Analysis and Report

Answer the following questions in a short report (max 2 pages) and include screenshots:

1. When you run the drone simulation with 4 drones, how many threads are created? How do they appear in `/proc/thread_monitor` (state, PID, TGID)?
2. What is the difference between the `pid` and `tgid` of a thread? Which one is the same for all threads of the same process?
3. Observe the output while the simulation is running. How does the state of a drone thread change when it is waiting for an order vs. when it is delivering? Explain based on the code.
4. The drone simulation uses `pthreads`, which are kernel threads. If we instead implemented user‑level threads (e.g., with a library like GNU Portable Threads), how would the output of `/proc/thread_monitor` differ? (You do not need to implement this; just reason theoretically.)
5. Briefly describe the steps required to load and unload a kernel module. What kernel functions are called at each step?

---

## Submission

Submit the following files as a single compressed archive (`.zip` or `.tar.gz`):

1. **Source code**:
   - `simple.c` and its `Makefile` (or `thread_monitor.c` and its `Makefile`).
   - `drone_sim.c`.
2. **Screenshots** (as images or in a PDF):
   - `lsmod` output after loading your module.
   - `dmesg` output showing module load/unload messages.
   - Output of `cat /proc/thread_monitor` (first 20 lines).
   - Output of running `./drone_sim` (showing at least two monitoring snapshots).
3. **Report** (PDF) answering the analysis questions.

**Note:** All group members must submit individually (or as per course policy), but you may collaborate. Indicate your group members in the `README.txt` file included in the archive.

---

## Evaluation Criteria

| Component | Points |
|-----------|--------|
| **Part 1** (simple module compiles, loads, unloads) | 15 |
| **Part 2** (thread monitor module works, produces correct output) | 30 |
| **Part 3** (drone simulation compiles, runs, monitors) | 35 |
| **Report** (answers, analysis, screenshots) | 20 |
| **Total** | 100 |

**Deductions**:
- Code with compilation warnings: -5
- Missing or incomplete screenshots: -5 per missing item
- Plagiarism or unauthorized code sharing: 0 on the assignment (see course policy)

---

## Hints and Resources

- Use the provided kernel module template; ensure you understand the use of `copy_to_user` and handling of `*off`.
- When iterating over processes, use `rcu_read_lock()` and `rcu_read_unlock()` to prevent the task list from changing during traversal.
- The drone simulation requires careful synchronization; use a mutex for the queue and a condition variable to signal drones when new orders arrive.
- You can limit the number of orders generated to avoid infinite loops, or let it run until you manually kill the program (Ctrl+C).
- To view only your own threads, you can get the main thread’s TGID (which is the PID of the main thread) and filter lines in the user‑space monitor. For example, when you read `/proc/thread_monitor`, parse each line and compare the TGID with your main PID.

Good luck!