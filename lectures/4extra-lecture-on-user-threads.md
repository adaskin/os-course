# Threads in Practice: C, C++, Python, Java  
**Extra Lecture Material**  
*BIL 250 Operating Systems*

---

## Why Threads? – The Problem We Face

Modern computers have multiple CPU cores, but a single-threaded program can only use one core at a time.

**Problems**:
- A program that waits for I/O (disk, network) wastes CPU cycles.
- A computationally intensive task could be finished faster if split across cores.
- Interactive applications (GUIs, web servers) need to remain responsive while doing background work.

---

**Solution**: Concurrency using threads.

- Threads allow a single process to have multiple execution flows.
- They share memory and resources, making communication efficient.
- They can run in parallel on multiple cores.

---

## Threads in C (POSIX Threads - pthreads)

### Key Concepts
- **Library**: `pthread.h`
- **Key functions**:
  - `pthread_create()` – create a new thread
  - `pthread_join()` – wait for thread termination
  - `pthread_exit()` – exit a thread

---

### Simple Example – Code
```c
#include <stdio.h>
#include <pthread.h>

// Function to be executed by the new thread
void* print_message(void* arg) {
    char* msg = (char*)arg;          // Cast argument to string
    printf("%s\n", msg);
    return NULL;                      // Thread exits
}
```
---

```c
int main() {
    pthread_t thread;                  // Thread identifier
    char* message = "Hello from thread!";
    
    // Create a new thread running print_message with message as argument
    pthread_create(&thread, NULL, print_message, (void*)message);
    
    // Wait for the created thread to finish
    pthread_join(thread, NULL);
    
    return 0;
}
```
---

### Explanation
- `pthread_create` starts a new thread; the original thread continues concurrently.
- `pthread_join` blocks until the specified thread terminates.
- Compile with: `gcc -pthread prog.c -o prog`

---

## Threads in C++ (std::thread)

### Key Concepts
- **Header**: `<thread>`
- **Key class**: `std::thread`
- Create a thread with a callable (function, lambda, functor).
- Use `join()` to wait.

---

### Simple Example – Code
```cpp
#include <iostream>
#include <thread>   // For std::thread

// Function to be run in a separate thread
void print_message(const std::string& msg) {
    std::cout << msg << std::endl;
}
```
---

```cpp
int main() {
    // Create a thread t that runs print_message with the given argument
    std::thread t(print_message, "Hello from C++ thread!");
    
    // Wait for thread t to complete
    t.join();
    
    return 0;
}
```

---

### Explanation
- The thread starts immediately upon construction.
- `join()` ensures the main thread waits for the child.
- Compile with: `g++ -std=c++11 -pthread prog.cpp -o prog`

---

## Threads in Python (threading module)

### Key Concepts
- **Module**: `threading`
- Create a `Thread` object and pass a target function.
- Start with `start()`, wait with `join()`.

---

### Simple Example – Code
```python
import threading

def print_message(msg):
    """Function to be executed in a thread."""
    print(msg)
```
---

```python
# Create a Thread object
t = threading.Thread(target=print_message, args=("Hello from Python thread!",))

# Start the thread
t.start()

# Wait for the thread to finish
t.join()
```

---

### Important Note: The Global Interpreter Lock (GIL)
- In CPython, the GIL prevents multiple native threads from executing Python bytecode simultaneously.
- **Consequence**: Python threads are **not** truly parallel for CPU‑bound tasks.
- **But**: They are still excellent for I/O‑bound tasks (network, disk) because the GIL is released during I/O operations.
- For CPU‑bound parallelism in Python, consider `multiprocessing` or C extensions.

**GIL REMOVAL:**
- Python 3.13 (GIL is made optional)
- Python 3.14 (released October 2025) improved upon the **"nogil"** implementation

---

## Threads in Java (java.lang.Thread)

### Key Concepts
- Two ways:
  1. Extend `Thread` class and override `run()`.
  2. Implement `Runnable` and pass to `Thread` (preferred).

---

### Simple Example (Runnable) – Code
```java
public class PrintTask implements Runnable {
    private String message;
    
    // Constructor to set the message
    public PrintTask(String msg) {
        message = msg;
    }
    
    // The run() method contains the code that runs in the thread
    public void run() {
        System.out.println(message);
    }
```

---

```java    
    public static void main(String[] args) {
        // Create a Runnable task
        PrintTask task = new PrintTask("Hello from Java thread!");
        
        // Create a Thread with the task
        Thread t = new Thread(task);
        
        // Start the thread
        t.start();
        
        // Wait for thread to finish
        try {
            t.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
```

---
### Explanation
- Java threads are mapped to OS threads.
- `Runnable` is a functional interface; you can also use lambda in Java 8+.
- Compile and run: `javac PrintTask.java && java PrintTask`

---

## Problem 1: Matrix Multiplication – What Are We Solving?

**The problem**: Multiply two large matrices **A (m×n)** and **B (n×p)** to produce **C (m×p)**.  
For each element C[i][j], we compute the dot product of row i of A and column j of B.

---

**Why is this a good candidate for threading?**  
- The computation of each element (or each row) is independent.
- If we have multiple CPU cores, we can split the work among threads and get a speedup.
- This is **data parallelism**: same operation on different pieces of data.

**Goal**: Use threads to reduce the total computation time.

---

## Matrix Multiplication – Parallelization Strategy

### Approach
- Divide the rows of the result matrix C among threads.
- Each thread computes a contiguous block of rows.
- No synchronization needed because threads write to separate memory locations.

---

### Pseudocode
```
function thread_function(row_start, row_end):
    for i = row_start to row_end-1:
        for j = 0 to p-1:
            C[i][j] = 0
            for k = 0 to n-1:
                C[i][j] += A[i][k] * B[k][j]
```

---

### Why No Synchronization Locks?
- Each thread accesses disjoint rows of C.
- All threads read A and B concurrently – reads are safe.
- Hence, no race conditions – **thread safety by design**.

---

## Matrix Multiplication – C (pthreads) – Setup

### Global Data and Structure
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define M 1000   // rows of A and C
#define N 1000   // cols of A, rows of B
#define P 1000   // cols of B and C
#define NUM_THREADS 4

int A[M][N], B[N][P], C[M][P];   // Shared matrices

// Structure to pass start and end row to each thread
typedef struct {
    int start_row;
    int end_row;
} thread_arg;
```

---

## Matrix Multiplication – C (pthreads) – Thread Function

```c
// Thread function: computes rows from start_row to end_row-1
void* multiply_rows(void* arg) {
    thread_arg* targ = (thread_arg*)arg;
    
    for (int i = targ->start_row; i < targ->end_row; i++) {
        for (int j = 0; j < P; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;   // No conflict: each thread writes to its own rows
        }
    }
    return NULL;
}
```
---

### Explanation
- Each thread receives its own `thread_arg` structure.
- The thread loops over its assigned rows and computes the dot product for each column.
- No locks needed – writes are to disjoint rows of C.

---

## Matrix Multiplication – C (pthreads) – Main Function

```c
int main() {
    // Initialize A and B with random values (for demonstration)
    for (int i = 0; i < M; i++)
        for (int k = 0; k < N; k++)
            A[i][k] = rand() % 10;
    for (int k = 0; k < N; k++)
        for (int j = 0; j < P; j++)
            B[k][j] = rand() % 10;
```

---
```c
    pthread_t threads[NUM_THREADS];
    thread_arg args[NUM_THREADS];
    int rows_per_thread = M / NUM_THREADS;

    // Create threads, each handling a chunk of rows
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i == NUM_THREADS-1) ? M : (i+1) * rows_per_thread;
        pthread_create(&threads[i], NULL, multiply_rows, &args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
```

---
```c
    printf("Multiplication completed.\n");
    // Optionally print a small part of result
    // printf("C[0][0] = %d\n", C[0][0]);
    return 0;
}
```
---

### Explanation
- `rows_per_thread` determines how many rows each thread gets.
- The last thread may get extra rows if M is not evenly divisible.
- `pthread_create` starts each thread; `pthread_join` waits for all.
- The `args` array lives in main, so it remains valid until threads finish.

---

## Problem 2: Drone Shipping Simulation – What Are We Solving?

**Scenario**:  
A warehouse receives customer orders. 
Multiple drones deliver packages. Orders arrive over time. Each drone can carry one package at a time. 

> We want to simulate this system to understand concurrency and resource sharing.

---

**Challenges**:
- Orders must be stored in a shared queue.
- Drones (threads) need to pick orders from the queue.
- If two drones pick the same order, chaos ensues – we need **synchronization**.
- If the queue is empty, drones should wait (not waste CPU).
- If new orders arrive, drones should be notified.

**Goal**: Demonstrate **task parallelism** and the need for thread synchronization.

---

## Drone Shipping – Python Implementation – Imports and Shared Data

```python
import threading
import time
import random

# Shared list of orders (simple list – not thread-safe!)
orders = []

# Lock to protect access to orders list
orders_lock = threading.Lock()

# Event to signal threads to stop
stop_event = threading.Event()
```

---

### Explanation
- `orders` is a simple list; without a lock, concurrent modifications can corrupt it.
- `orders_lock` ensures that only one thread modifies the list at a time.
- `stop_event` allows graceful shutdown.

---

## Drone Shipping – Drone Worker Function

```python
def drone_worker(drone_id):
    """Each drone thread runs this function."""
    while not stop_event.is_set():
        # Safely check and pop an order
        with orders_lock:
            if orders:
                order = orders.pop(0)   # FIFO – but pop(0) is O(n)
            else:
                order = None
        
        if order:
            print(f"Drone {drone_id} delivering order {order}")
            time.sleep(random.uniform(1, 3))  # Simulate delivery
            print(f"Drone {drone_id} completed order {order}")
        else:
            # No order, wait a bit before checking again
            time.sleep(0.1)
```

---

### Important Note on Efficiency
- `pop(0)` on a list is O(n) because it shifts all remaining elements.
- In production code, use `collections.deque` which supports O(1) pops from both ends.
- Here we use a list for simplicity, but be aware of the performance implication.

---

## Drone Shipping – Order Generator Function

```python
def order_generator():
    """Simulates incoming orders."""
    order_id = 0
    while not stop_event.is_set():
        # Random interval between orders
        time.sleep(random.uniform(0.5, 2))
        with orders_lock:
            orders.append(f"order_{order_id}")
            order_id += 1
        print(f"New order placed. Pending orders: {len(orders)}")
```
---

### Explanation
- The generator runs in its own thread.
- It periodically adds a new order to the shared list.
- Lock is used when appending.

---

## Drone Shipping – Main Thread

```python
# Create 3 drone threads
drones = []
for i in range(3):
    t = threading.Thread(target=drone_worker, args=(i,))
    t.start()
    drones.append(t)
```

---

```python
# Start order generator in a separate thread
order_gen = threading.Thread(target=order_generator)
order_gen.start()

```

---

```python
# Let simulation run for 30 seconds
time.sleep(30)

# Signal all threads to stop
stop_event.set()

# Wait for all threads to finish
for t in drones + [order_gen]:
    t.join()
print("Simulation ended.")
```

---

### Explanation
- Drones and generator start concurrently.
- After 30 seconds, `stop_event.set()` signals all loops to exit.
- `join()` ensures clean termination.

---

## Drone Shipping – Observations

- **Concurrency** → multiple deliveries happen simultaneously.
- **Throughput** increases with more drones (up to a point).
- **Load balancing**: drones automatically take next available order.
- **Synchronization** required to safely access shared order queue.
- Without lock, two drones might take the same order or corrupt list.
- **Busy waiting** (the `time.sleep(0.1)`) wastes CPU; better to use condition variables.

---

This example naturally leads into the need for **mutexes**, **condition variables**, and **deadlock** topics (future lectures).

---

## Threads vs. Processes

### Comparison
- **Processes** have separate address spaces; **threads** share memory.
- **Creating processes** is heavier (fork, copy page tables).
- **Context switch** between processes is more expensive than between threads of same process.
- **Communication**: processes need IPC (pipes, sockets, shared memory); threads just use shared variables.
- **Isolation**: processes are isolated (security); threads can interfere easily.

---

### When to Use
- **Processes**: running separate programs, strong isolation, network distribution.
- **Threads**: tightly coupled tasks, overlapping I/O, parallelizing a single task.

---

## Creating Processes in C with `fork()` – Code

```c
#include <stdio.h>
#include <unistd.h>   // for fork(), getpid()
#include <sys/wait.h> // for wait()

int main() {
    pid_t pid = fork();  // Create a new process
    
    if (pid == 0) {
        // Child process: fork() returns 0 to the child
        printf("Child: PID=%d\n", getpid());
        // Could call exec() to run a new program
    } else if (pid > 0) {
        // Parent process: fork() returns child's PID
        wait(NULL);   // Wait for any child to finish
        printf("Parent: child completed\n");
    } else {
        // fork() returned -1 on error
        perror("fork failed");
    }
    return 0;
}
```

---

### Explanation
- `fork()` creates an almost exact copy of the calling process.
- After `fork()`, both processes execute the next instruction independently.
- Child gets a copy of parent's memory (copy‑on‑write).
- `wait()` makes parent block until a child terminates.
- `exec()` family replaces the process's memory with a new program.

---

## Processes vs Threads – Comparison Table

| Feature                | Processes                          | Threads (same process)            |
|------------------------|------------------------------------|-----------------------------------|
| Address space          | Separate                           | Shared                            |
| Creation overhead      | High (fork, copy page tables)      | Low (just new stack/context)      |
| Communication          | IPC (pipes, sockets, shm)          | Direct memory access              |
| Synchronization        | Usually via IPC mechanisms         | Mutexes, condition variables      |
| Fault isolation        | One crash doesn't affect others    | One thread can crash whole process|
| Typical use            | Running different apps, security   | Parallel tasks within one app     |

---

## Beyond Threads: Even Lighter Concurrency

**Observation**: Threads are powerful, but they still have overhead:
- Each thread requires a kernel stack and scheduling.
- Context switching involves kernel entry/exit.
- Creating thousands of threads is impractical.

---

**What if we need massive concurrency (thousands of tasks)?**

**Solution**: User‑level concurrency – coroutines, fibers, goroutines.

These are **cooperatively scheduled** within a single thread, with very low overhead.

---

## Coroutines in Python (async/await) – Code

```python
import asyncio

# Define an asynchronous function (coroutine)
async def fetch_data(delay, name):
    print(f"Start {name}")
    # await yields control to the event loop; other coroutines can run
    await asyncio.sleep(delay)   # Simulate I/O wait
    print(f"End {name}")
    return f"Data from {name}"

async def main():
    # Schedule two coroutines to run concurrently
    task1 = asyncio.create_task(fetch_data(2, "A"))
    task2 = asyncio.create_task(fetch_data(1, "B"))
    
    # Wait for both to complete and collect results
    results = await asyncio.gather(task1, task2)
    print(results)

# Start the event loop with main()
asyncio.run(main())
```

---

### Explanation
- `async def` defines a coroutine.
- `await` suspends the current coroutine, allowing other tasks to run.
- `asyncio.create_task` schedules a coroutine to run concurrently.
- `asyncio.gather` runs multiple coroutines and waits for all.
- This allows handling thousands of I/O operations in a single thread.

---

## Goroutines in Go – Code

```go
package main

import (
    "fmt"
    "time"
)

func worker(id int) {
    for i := 0; i < 3; i++ {
        fmt.Printf("Worker %d: %d\n", id, i)
        time.Sleep(time.Millisecond) // yields control
    }
}

func main() {
    // Start 5 goroutines
    for i := 0; i < 5; i++ {
        go worker(i)   // launches a goroutine
    }
    
    // Give goroutines time to run (in real code use sync.WaitGroup)
    time.Sleep(100 * time.Millisecond)
    fmt.Println("Main exits")
}
```

---

### Explanation
- Goroutines are cheaper than OS threads (stack starts at ~2KB).
- `go func()` starts a goroutine; execution continues immediately in the caller.
- The Go runtime automatically distributes goroutines across available OS threads.
- Use channels or `sync` package for communication and synchronization.

---

## Fibers (C++ with Boost.Fiber) – Code
C++20 introduced `std::jthread` and cooperative cancellation, **but true fibers are not in the standard**.
- [see boost fibers](https://www.boost.org/) 

```cpp
#include <boost/fiber/all.hpp>
#include <iostream>

void task(int id) {
    std::cout << "Fiber " << id << " started\n";
    // Yield control, allowing other fibers to run
    boost::this_fiber::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Fiber " << id << " resumed\n";
}

int main() {
    // Create two fiber objects; they start immediately
    boost::fibers::fiber f1(task, 1);
    boost::fibers::fiber f2(task, 2);
    
    // Wait for fibers to finish
    f1.join();
    f2.join();
    
    return 0;
}
```

---

### Explanation
- Fibers are scheduled cooperatively within a single thread.
- `boost::this_fiber::sleep_for` yields the fiber, allowing another fiber to run.
- Context switch between fibers is just a function call, no kernel involvement.
- Useful for writing synchronous‑style code that doesn't block threads.

---

## Comparison: Threads vs. Coroutines/Fibers

| Feature                | OS Threads                           | Coroutines / Fibers                 |
|------------------------|--------------------------------------|-------------------------------------|
| Scheduling             | Preemptive (kernel)                  | Cooperative (user)                  |
| Creation overhead      | High (syscalls, kernel structures)   | Very low (just stack allocation)    |
| Context switch cost    | High (kernel entry/exit)             | Low (function call)                  |
| Scalability            | Typically hundreds                   | Thousands or millions                |
| Use case               | CPU‑bound, blocking I/O              | I/O‑bound, high‑concurrency servers |

- Modern applications often combine both: OS threads for parallelism across cores, coroutines for concurrency within a thread.

---

## When to Use What?

- **OS Threads**: Use for CPU‑intensive work that requires true parallelism across cores.
- **Coroutines/Async**: Use for I/O‑bound applications (web servers, database clients) where you need to handle many concurrent connections efficiently.
- **Goroutines/Fibers**: Use when you want lightweight concurrency with a familiar synchronous programming style, especially in languages that support them natively (Go) or via libraries.

Understanding these options helps you choose the right tool for concurrency in your applications.

---

# Full Implementations and Exercises

---

## Full Implementation: Matrix Multiplication in C (pthreads)

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define M 1000
#define N 1000
#define P 1000
#define NUM_THREADS 4

int A[M][N], B[N][P], C[M][P];

typedef struct {
    int start_row;
    int end_row;
} thread_arg;

void* multiply_rows(void* arg) {
    thread_arg* targ = (thread_arg*)arg;
    for (int i = targ->start_row; i < targ->end_row; i++) {
        for (int j = 0; j < P; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    return NULL;
}

int main() {
    // Initialize A and B with random values
    for (int i = 0; i < M; i++)
        for (int k = 0; k < N; k++)
            A[i][k] = rand() % 10;
    for (int k = 0; k < N; k++)
        for (int j = 0; j < P; j++)
            B[k][j] = rand() % 10;

    pthread_t threads[NUM_THREADS];
    thread_arg args[NUM_THREADS];
    int rows_per_thread = M / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i == NUM_THREADS-1) ? M : (i+1) * rows_per_thread;
        pthread_create(&threads[i], NULL, multiply_rows, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Multiplication completed.\n");
    return 0;
}
```

---

## Full Implementation: Drone Shipping in C++ (using threads and condition variable)

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <random>

std::queue<std::string> orders;
std::mutex orders_mutex;
std::condition_variable cv;
bool done = false;

void drone_worker(int id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.0, 3.0);
    
    while (true) {
        std::unique_lock<std::mutex> lock(orders_mutex);
        cv.wait(lock, []{ return !orders.empty() || done; });
        
        if (done && orders.empty()) break;
        
        std::string order = orders.front();
        orders.pop();
        lock.unlock();
        
        std::cout << "Drone " << id << " delivering " << order << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(dis(gen)*1000)));
        std::cout << "Drone " << id << " completed " << order << std::endl;
    }
}

void order_generator() {
    int order_id = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.5, 2.0);
    
    for (int i = 0; i < 20; i++) {  // generate 20 orders
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(dis(gen)*1000)));
        {
            std::lock_guard<std::mutex> lock(orders_mutex);
            orders.push("order_" + std::to_string(order_id++));
            std::cout << "New order placed. Queue size: " << orders.size() << std::endl;
        }
        cv.notify_one();
    }
    
    {
        std::lock_guard<std::mutex> lock(orders_mutex);
        done = true;
    }
    cv.notify_all();
}

int main() {
    std::vector<std::thread> drones;
    for (int i = 0; i < 3; i++) {
        drones.emplace_back(drone_worker, i);
    }
    
    std::thread generator(order_generator);
    
    generator.join();
    for (auto& d : drones) d.join();
    
    std::cout << "Simulation ended." << std::endl;
    return 0;
}
```

---

## Full Implementation: Drone Shipping in Java

```java
import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;

public class DroneShipping {
    private static final Queue<String> orders = new LinkedList<>();
    private static boolean done = false;

    static class Drone implements Runnable {
        private final int id;
        private final Random rand = new Random();

        Drone(int id) { this.id = id; }

        @Override
        public void run() {
            while (true) {
                String order;
                synchronized (orders) {
                    while (orders.isEmpty() && !done) {
                        try { orders.wait(); } catch (InterruptedException e) {}
                    }
                    if (done && orders.isEmpty()) break;
                    order = orders.poll();
                }
                System.out.println("Drone " + id + " delivering " + order);
                try { Thread.sleep(1000 + rand.nextInt(2000)); } catch (InterruptedException e) {}
                System.out.println("Drone " + id + " completed " + order);
            }
        }
    }

    static class Generator implements Runnable {
        private final Random rand = new Random();

        @Override
        public void run() {
            for (int i = 0; i < 20; i++) {
                try { Thread.sleep(500 + rand.nextInt(1500)); } catch (InterruptedException e) {}
                synchronized (orders) {
                    orders.add("order_" + i);
                    System.out.println("New order placed. Queue size: " + orders.size());
                    orders.notifyAll();
                }
            }
            synchronized (orders) {
                done = true;
                orders.notifyAll();
            }
        }
    }

    public static void main(String[] args) {
        Thread[] drones = new Thread[3];
        for (int i = 0; i < 3; i++) {
            drones[i] = new Thread(new Drone(i));
            drones[i].start();
        }
        Thread generator = new Thread(new Generator());
        generator.start();

        try {
            generator.join();
            for (Thread d : drones) d.join();
        } catch (InterruptedException e) {}
        System.out.println("Simulation ended.");
    }
}
```

---

## Exercises

1. **Matrix Multiplication Adaptation**
   - Rewrite the matrix multiplication example in C++ using `std::thread`.
   - Modify the Python version (using `threading`) to perform matrix multiplication with threads. (Observe the effect of the GIL – does it speed up? Why?)
   - In Java, implement matrix multiplication using a thread pool (`ExecutorService`).

2. **Drone Shipping Variations**
   - Add a maximum queue size; if the queue is full, the generator should wait (producer‑consumer with bounded buffer). Use condition variables.
   - Implement the drone simulation in C using pthreads (with mutex and condition variables).
   - In Python, replace busy waiting with a `condition` variable.

3. **Comparative Study**
   - Compare performance of matrix multiplication with different numbers of threads (1, 2, 4, 8) on a multicore machine. Plot speedup.
   - Compare the drone simulation implemented in C, C++, Python, and Java – what are the differences in code complexity and performance?

4. **Coroutines/Fibers**
   - Rewrite the drone simulation in Python using `asyncio` and queues (simulate I/O with `asyncio.sleep`). How does it differ from threading version?
   - In Go, implement the drone simulation using goroutines and channels.

5. **Processes vs Threads**
   - Implement a simple server that forks a new process per client vs. one that creates a new thread per client. Compare resource usage.

---

## Summary and Key Takeaways

- **Threads** enable concurrency and parallelism, allowing programs to utilize multiple cores and remain responsive.
- **Data parallelism** (matrix multiplication) divides data among threads – no synchronization needed if data is disjoint.
- **Task parallelism** (drone shipping) shares a resource – **synchronization** (locks, condition variables) is essential.
- **Processes** provide stronger isolation but are heavier; choose based on needs.
- **Lightweight concurrency** (coroutines, goroutines, fibers) offers massive scalability for I/O‑bound tasks.
- Understanding these concepts helps you write efficient, correct concurrent programs.

**Next topics**: Synchronization primitives (mutexes, semaphores), deadlocks, and advanced concurrency patterns.