/*
 * single_prod_cons.c
 *
 * Lock‑free SINGLE‑producer / SINGLE‑consumer ring buffer
 * – implemented exactly as shown on the lecture slides
 *   (7synchronization-II.pdf, slides 133‑134).
 *
 * Compile:
 *   gcc -std=c11 -pthread -o single_prod_cons single_prod_cons.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>        // sched_yield()

/* ------------------------------------------------------------
 * 1. Ring buffer (global variables, exactly as in the slide)
 * ------------------------------------------------------------ */
#define BUF_SIZE  64       // must be power of 2

static _Atomic int in  = 0;    // producer writes this
static _Atomic int out = 0;    // consumer writes this
static int buffer[BUF_SIZE];   // the actual data buffer

/* ------------------------------------------------------------
 * 2. Producer function (from slide 133, rewritten for C11)
 * ------------------------------------------------------------ */
void *producer(void *ignored) {
    for (int item = 1; item <= 1000000; item++) {
        /* ---------- BEGIN slide code ---------- */
        int slot = atomic_load_explicit(&in, memory_order_relaxed);
        int next = (slot + 1) % BUF_SIZE;

        // Wait while buffer is full
        while (atomic_load_explicit(&out, memory_order_acquire) == next)
            sched_yield();  // thread_yield()

        buffer[slot] = item;                          // write data
        atomic_store_explicit(&in, next, memory_order_release);
        /* ---------- END slide code ---------- */
    }
    return NULL;
}

/* ------------------------------------------------------------
 * 3. Consumer function (from slide 134, rewritten for C11)
 * ------------------------------------------------------------ */
void *consumer(void *ignored) {
    unsigned long long sum = 0;
    int consumed = 0;
    while (consumed < 1000000) {
        /* ---------- BEGIN slide code ---------- */
        int myout = atomic_load_explicit(&out, memory_order_relaxed);
        // Wait while buffer is empty
        while (atomic_load_explicit(&in, memory_order_acquire) == myout)
            sched_yield();  // thread_yield()

        int value = buffer[myout];                   // read data
        atomic_store_explicit(&out, (myout + 1) % BUF_SIZE,
                              memory_order_release);
        /* ---------- END slide code ---------- */
        sum += value;
        consumed++;
    }
    printf("Consumer sum = %llu (expected %llu)\n",
           sum, (unsigned long long)1000000 * 1000001 / 2);
    return NULL;
}

/* ------------------------------------------------------------
 * 4. Main
 * ------------------------------------------------------------ */
int main(void) {
    pthread_t prod_tid, cons_tid;

    pthread_create(&prod_tid, NULL, producer, NULL);
    pthread_create(&cons_tid, NULL, consumer, NULL);

    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);

    printf("Done. Items left in buffer: %d\n",
           atomic_load(&in) - atomic_load(&out));
    return 0;
}