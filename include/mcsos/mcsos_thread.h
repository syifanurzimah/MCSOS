#ifndef MCSOS_THREAD_H
#define MCSOS_THREAD_H

#include <stdint.h>

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
} thread_state_t;

typedef struct thread_context {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;

    uint64_t rip;
    uint64_t rflags;
    uint64_t rsp;
} thread_context_t;

typedef struct thread {
    int id;
    thread_state_t state;
    thread_context_t context;
} thread_t;

#endif
