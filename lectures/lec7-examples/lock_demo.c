/*
 * lock_demo.c
 *
 * Demonstrates a custom mutex built from Linux fast userspace mutex (futex)
 * or a simple C11 spinlock.
 *
 * Trade‑offs:
 *  - Futex mutex:  Threads that cannot acquire the lock are put to sleep
 *    (FUTEX_WAIT).  This frees the CPU for other work, but the overhead of
 *    entering the kernel on every contention can cause the program to feel
 *    “stuck” when there are many threads or extremely frequent lock/unlock
 *    operations, especially if the lock holder is descheduled.
 *    It is a good choice when locks may be held for a long time and CPU
 *    cycles are precious.
 *
 *  - Spinlock:  Busy‑waits (spins) until the lock becomes free.  No kernel
 *    calls are made, so it is extremely fast when contention is low and the
 *    critical section is short.  However, it wastes CPU time while waiting
 *    and can suffer from priority inversion.
 *
 * For a small number of threads and short critical sections the spinlock
 * often finishes faster; the futex version may even appear to hang for a
 * moment under heavy contention, even though it is making progress.
 *
 * Compile (futex version):
 *   gcc -std=c11 -pthread -DUSE_FUTEX -o lock_demo lock_demo.c
 *
 * Compile (spinlock version):
 *   gcc -std=c11 -pthread -o lock_demo lock_demo.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <linux/futex.h>      /* FUTEX_WAIT, FUTEX_WAKE */
#include <sys/syscall.h>      /* SYS_futex */

/* ------------------------------------------------------------
 * 1. Synchronisation primitives
 * ------------------------------------------------------------ */

#ifdef USE_FUTEX

/* ============================================================
 * 3-state futex mutex (unlocked / locked / contended)
 * ============================================================ */
typedef atomic_int mutex_t;

#define MUTEX_UNLOCKED   0
#define MUTEX_LOCKED     1
#define MUTEX_CONTESTED  2

static inline void mutex_init(mutex_t *m){
    *m = MUTEX_UNLOCKED;
}

static inline void mutex_lock(mutex_t *m)
{
    /* Fast path: 0 -> 1 without a syscall */
    int expected = MUTEX_UNLOCKED;
    if (atomic_compare_exchange_strong_explicit(m, &expected, MUTEX_LOCKED,
            memory_order_acquire, memory_order_relaxed))
        return;          /* uncontested, lock acquired */

    /* Slow path: sleep and wait */
    do {
        /* If the lock is already in CONTESTED state or we failed
           to grab it from UNLOCKED, put ourselves to sleep */
        if (expected == MUTEX_CONTESTED ||
            atomic_exchange_explicit(m, MUTEX_CONTESTED,
                                     memory_order_acquire) != MUTEX_UNLOCKED) {
            syscall(SYS_futex, m, FUTEX_WAIT, MUTEX_CONTESTED,
                    NULL, NULL, 0);
        }
        /* Re-try to take the lock */
        expected = MUTEX_UNLOCKED;
    } while (!atomic_compare_exchange_strong_explicit(m, &expected,
                 MUTEX_CONTESTED, memory_order_acquire, memory_order_relaxed));
    /* Now we own the lock in CONTESTED state */
}

static inline void mutex_unlock(mutex_t *m){
    /* Decrement state.  If previous state was CONTESTED, we need to wake
       someone; otherwise just go to UNLOCKED. */
    int prev = atomic_fetch_sub_explicit(m, 1, memory_order_release);
    if (prev != MUTEX_LOCKED) {
        /* There were waiters (or state was weird).  Reset to UNLOCKED
           and wake one waiting thread. */
        atomic_store_explicit(m, MUTEX_UNLOCKED, memory_order_relaxed);
        syscall(SYS_futex, m, FUTEX_WAKE, 1, NULL, NULL, 0);
    }
    /* If prev was exactly LOCKED, we just transitioned 1->0 and we're done. */
}

#else  /* USE_FUTEX not defined → use spinlock */

/* ============================================================
 * Simple spinlock using C11 atomic_flag
 * ============================================================ */
typedef struct {
    atomic_flag flag;
} spinlock_t;

static inline void spin_init(spinlock_t *lock)
{
    atomic_flag_clear_explicit(&lock->flag, memory_order_release);
}

static inline void spin_lock(spinlock_t *lock)
{
    while (atomic_flag_test_and_set_explicit(&lock->flag,
                                             memory_order_acquire))
        ; /* busy-wait */
}

static inline void spin_unlock(spinlock_t *lock){
    atomic_flag_clear_explicit(&lock->flag, memory_order_release);
}

#endif

/* ------------------------------------------------------------
 * 2. Shared data and thread arguments
 * ------------------------------------------------------------ */
#define NTHREADS  4        /* number of threads */
#define NITER     100  /* increments per thread */

long long counter = 0;     /* shared resource */

#ifdef USE_FUTEX
mutex_t mylock;
#else
spinlock_t mylock;
#endif

typedef struct {
    int id;
} thread_arg_t;

/* ------------------------------------------------------------
 * 3. Worker thread function
 * ------------------------------------------------------------ */
void *worker(void *arg)
{
    thread_arg_t *targ = (thread_arg_t *) arg;
    (void) targ;           /* unused except for potential debugging */

    for (int i = 0; i < NITER; i++) {
#ifdef USE_FUTEX
        mutex_lock(&mylock);
#else
        spin_lock(&mylock);
#endif
        /* --- critical section --- */
        counter++;
        /* --- end critical section --- */
#ifdef USE_FUTEX
        mutex_unlock(&mylock);
#else
        spin_unlock(&mylock);
#endif
    }
    return NULL;
}

/* ------------------------------------------------------------
 * 4. Main program
 * ------------------------------------------------------------ */
int main(void)
{
    pthread_t threads[NTHREADS];
    thread_arg_t args[NTHREADS];

#ifdef USE_FUTEX
    mutex_init(&mylock);
    printf("Using 3‑state futex mutex\n");
#else
    spin_init(&mylock);
    printf("Using C11 spinlock\n");
#endif

    /* Create threads */
    for (int i = 0; i < NTHREADS; i++) {
        args[i].id = i;
        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    /* Wait for all threads to finish */
    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final counter value = %lld (expected %d)\n",
           counter, NTHREADS * NITER);

    return 0;
}