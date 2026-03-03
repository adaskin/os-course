---
title: ammar daskin - Operating Systems (Spring 2026 at Istanbul Medeniyet University)
---

**Friday  9:30 pm @B501**

## Prerequisites 
**Basic C programming skill is required:**  
- C Programming, a little Unix or Linux, Data Structures and Algorithms  
- You are expected to have prior programming experience in C and a basic understanding of data structures and algorithms. Familiarity with the Linux command line is helpful but not assumed – essential tools will be reviewed early in the course.

**This semester (Spring 26) the course is being taught without the assumption that students have already taken [System Programming](https://github.com/adaskin/sysprog-course).**  
That course has moved to the third year and become a technical elective. **Consequently, the lectures and projects have been redesigned** to incorporate fundamental system programming topics alongside the traditional OS concepts.

---

## Course Description
This course introduces the fundamental principles and internal workings of modern operating systems, following the [ACM 2023 curriculum guidelines](https://dl.acm.org/doi/epdf/10.1145/3664191) for the Operating Systems knowledge area in CS. Topics include processes, threads, concurrency, synchronization, CPU scheduling, memory management, virtual memory, file systems, I/O, and security. Students will learn how operating systems virtualize hardware, manage resources, and provide protection. Programming projects provide hands-on experience with Linux system calls, multithreading, and kernel compilation. 

**This semester (Spring 2026)** assumes no prior system programming background; essential concepts are integrated into lectures and projects. For example:
- When discussing **processes**, we will examine `fork()`, `exec()`, `wait()` and implement a simple shell.  
- During **threads and concurrency**, we will write multithreaded programs with pthreads and use synchronization primitives.  
- For **file systems and I/O**, we will work with low‑level file descriptors, redirection, and pipes.  
The course retains its strong emphasis on **concurrency, synchronization, and kernel internals**, but now builds this knowledge from the ground up. We will still include kernel‑level projects – such as adding system calls, modifying kernel modules, and working with the `/proc` file system, but we will try to make them simpler and include some introductory examples in the projects.

---

## Course Objectives

Upon completing this course, students will be able to explain the role of an operating system in managing hardware, virtualizing resources, and enforcing protection. Analyze design tradeoffs in operating systems (performance vs. flexibility, security vs. usability). Understand concurrency, synchronization, and the mechanisms used to manage concurrent processes. Understand and implement
inter-process communication mechanisms safely. Compare scheduling, memory management, and file system algorithms. Describe how operating systems implement virtualization, protection, and isolation. understand the impact and implications of operating system.

---

## Learning Outcomes

By the end of this course, students will be able to:

- Identify the core functions of an operating system and the purpose of system calls.
- Explain process and thread management, including states, context switching, and interprocess communication.
- Describe concurrency issues (race conditions, deadlocks) and how synchronization primitives address them.
- Compare CPU scheduling algorithms and their impact on system performance.
- Explain memory management techniques: paging, segmentation, virtual memory, and page replacement.
- Describe file system implementation, device management, and I/O handling.
- Understand protection, security mechanisms, and virtualization concepts in modern operating systems.

## Textbooks and Course Material

*   Operating System Concepts, 10th Edition Abraham Silberschatz, Greg Gagne, Peter B. Galvin, [https://www.wiley.com/en-us/Operating+System+Concepts%2C+10th+Edition-p-9781119320913](https://www.wiley.com/en-us/Operating+System+Concepts%2C+10th+Edition-p-9781119320913)
    
* ### Lectures slides are based on the slides 
    *   [https://www.scs.stanford.edu/24wi-cs212/notes/](https://www.scs.stanford.edu/24wi-cs212/notes/)         
    *   [https://www.os-book.com/OS10/slide-dir/index.html](https://www.os-book.com/OS10/slide-dir/index.html)         
    *   and [https://linux-kernel-labs.github.io/](https://linux-kernel-labs.github.io/) 
* ### Other resources
    *   [Linux Kernel Documentation](https://www.kernel.org/doc/html/latest/)         
    *   [intel CPU manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)         
    *   OS security and more: [https://www.ics.uci.edu/~goodrich/teach/cs201P/notes/](https://www.ics.uci.edu/~goodrich/teach/cs201P/notes/)         
    *   ebook for synchronization [https://dl.acm.org/doi/book/10.5555/2385452](https://dl.acm.org/doi/book/10.5555/2385452)         
        *   [https://booksite.elsevier.com/9780123973375/](https://booksite.elsevier.com/9780123973375/)            
    *   [kernel source code](https://elixir.bootlin.com/linux/latest/source/kernel)        
    *   [https://linux-kernel-labs.github.io/refs/heads/master/index.html](https://linux-kernel-labs.github.io/refs/heads/master/index.html)         
    *   Testing Linux: [https://linux-test-project.github.io/](https://linux-test-project.github.io/)
        
## Weekly topics and lecture notes
* Week-1:
  1. Administrivia [⟶0administrivia.pdf](lectures/0administrivia.pdf)
  2. Intro: running a software on a machine [⟶0intro-os-vms-running-sw.pdf](lectures/0intro-os-vms-running-sw.pdf)
     1. An extra: How to interact with OS: some terminal commands [⟶0intro-how-to-interact-with-os](lectures/0intro-how-to-interact-with-os)
  3. Chapter 1: Introduction to OS  [⟶1intro.pdf](lectures/1intro.pdf) 

2. Chapter 2: Operating System Services-structures-linkers/loaders  [⟶2services-structs.pdf](lectures/2services-structs.pdf) 
3. Chapter 3: Processes     [⟶3processes.pdf](lectures/3processes.pdf) 
4. Chapter 4: Threads & Concurrency   [⟶4concurrency-threads.pdf](lectures/4concurrency-threads.pdf)   
5. Chapter 5: CPU Scheduling    [⟶5cpu-scheduling.pdf](lectures/5cpu-scheduling.pdf) 
   - some readings: 
     - [Old CFS load balancing issues on multicore](https://people.ece.ubc.ca/sasha/papers/eurosys16-final29.pdf)
     - [New Linux Scheduler](https://docs.kernel.org/scheduler/sched-eevdf.html)
6. Intro to Synchronization: Peterson solution, spin-locks, atomic instructions, memory barriers (e.g., mb, fence, volatile), C11 atomic library (relaxed, acquire, release) 
    [⟶6synchronization-intro.pdf](lectures/6synchronization-intro.pdf) 
7.  Midterm exam 
8.  Synchronization II (implementation of locks, low level locks: disabling interrupts and spin-locks, improving spinlock efficiency, lock free programming, cache coherency, deadlock, transactional memory)    
    [⟶7synchronization-II.pdf](lectures/7synchronization-II.pdf)  
9.  Synchronization review  
    [⟶8synchronization-review.pdf](lectures/8synchronization-review.pdf)  
10.  Chapter 9: Main Memory  
    [⟶9vm-hw.pdf](lectures/9vm-hw.pdf)  
11.  Chapter 10: Virtual Memory    
    [⟶10vm-os.pdf](lectures/10vm-os.pdf) 
12.  Chapter 11-12: I/O Systems   
    [⟶11io-systems.pdf](lectures/11io-systems.pdf) 
13. Disk and storage
    [⟶12disk-io.pdf](lectures/12disk-io.pdf)   
14.  Chapter 13-14-15: File-System Interface, Implementation, and Internals    
    [⟶13file-systems.pdf](lectures/13file-systems.pdf)   
15.  Protection, Security
    [⟶14protection-security.pdf](lectures/14protection-security.pdf)  
    

## Homeworks and exams
*   Assigned via classroom.google.com or from github    
*   3-5 projects: includes programming assignments that may require compiling and configuring Linux-kernel, adding system calls to kernel that reads kernel structures and copies this info to user programs, changing kernel parameters or programs, adding new modules, /proc file system, and device drivers    
*   No late submission    
*   Submissions through classroom.google.com
*   1-midterm    
*   1-final
  
## Grading      
*   30% projects/hw
*   30% midterm exam    
*   40% final exam
    
### **Grade Letters**
- I will use university's letters.. 
- but **if the distribution is not "ideal"**, I may adapt something similar to following
  - $> 90$ or $\approx$ top 5-10% AA
  - $<35$ is FF.
  - For a median $x$ and a standard deviation $2s$ (we use half)
    - BA: $\approx[x+3s, x+4s)$ 
    - BB: $\approx[x+2s, x+3s]$ 
    - BC: $\approx[x+1s, x+2s)$ 
    - CC: $\approx[x-1s, x+1s)$ 
    - DC: $\approx[x-2s, x-1s)$ 
    - Depending on the value of $s$ and $x$, some slight changes may occur...  

## Discussions
For the assignment submission/grading and discussions, we will use [https://classroom.google.com](https://classroom.google.com/) and for public discussions, we will use [https://piazza.com](https://piazza.com/) for this course.  In discussions and questions:

*   Do not post solutions or any significant part of an assignment.    
*   Do not post anything not related to the course.    
*   Ask a question when you would like some help with something    
*   Post something when you would like to help others with something.
    

## Collaboration and Cheating Policy
*   Any kind of plagiarism and cheating are prohibited (Please, refer to the university cheating policy).    
*   If you benefit from some work of others, list them as references (online references or books)     
*   Discussing the assignments or projects with your friends is allowed; but, all the submitted work should be yours alone. List your collaborators (if you discuss your homework with your friends) in your assignments.