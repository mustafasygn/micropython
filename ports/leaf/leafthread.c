/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>

#include "py/obj.h"
#include "gccollect.h"
#include "irq.h"
#include "leafthread.h"

#if MICROPY_PY_THREAD

#define PYB_MUTEX_UNLOCKED ((void*)0)
#define PYB_MUTEX_LOCKED ((void*)1)

// These macros are used when we only need to protect against a thread
// switch; other interrupts are still allowed to proceed.
#define RAISE_IRQ_PRI() raise_irq_pri(IRQ_PRI_PENDSV)
#define RESTORE_IRQ_PRI(state) restore_irq_pri(state)

extern void __fatal_error(const char*);

volatile int leaf_thread_enabled;
leaf_thread_t *volatile leaf_thread_all;
leaf_thread_t *volatile leaf_thread_cur;

static inline void leaf_thread_add_to_runable(leaf_thread_t *thread) {
    thread->run_prev = leaf_thread_cur->run_prev;
    thread->run_next = leaf_thread_cur;
    leaf_thread_cur->run_prev->run_next = thread;
    leaf_thread_cur->run_prev = thread;
}

static inline void leaf_thread_remove_from_runable(leaf_thread_t *thread) {
    if (thread->run_next == thread) {
        __fatal_error("deadlock");
    }
    thread->run_prev->run_next = thread->run_next;
    thread->run_next->run_prev = thread->run_prev;
}

void leaf_thread_init(leaf_thread_t *thread) {
    leaf_thread_enabled = 0;
    leaf_thread_all = thread;
    leaf_thread_cur = thread;
    thread->sp = NULL; // will be set when this thread switches out
    thread->local_state = 0; // will be set by mp_thread_init
    thread->arg = NULL;
    thread->stack = &_heap_end;
    thread->stack_len = ((uint32_t)&_estack - (uint32_t)&_heap_end) / sizeof(uint32_t);
    thread->all_next = NULL;
    thread->run_prev = thread;
    thread->run_next = thread;
    thread->queue_next = NULL;
}

void leaf_thread_deinit() {
    uint32_t irq_state = disable_irq();
    leaf_thread_enabled = 0;
    leaf_thread_all = leaf_thread_cur;
    leaf_thread_cur->all_next = NULL;
    leaf_thread_cur->run_prev = leaf_thread_cur;
    leaf_thread_cur->run_next = leaf_thread_cur;
    enable_irq(irq_state);
}

STATIC void leaf_thread_terminate(void) {
    uint32_t irq_state = disable_irq();
    leaf_thread_t *thread = leaf_thread_cur;
    // take current thread off the run list
    leaf_thread_remove_from_runable(thread);
    // take current thread off the list of all threads
    for (leaf_thread_t **n = (leaf_thread_t**)&leaf_thread_all;; n = &(*n)->all_next) {
        if (*n == thread) {
            *n = thread->all_next;
            break;
        }
    }
    // clean pointers as much as possible to help GC
    thread->all_next = NULL;
    thread->queue_next = NULL;
    thread->stack = NULL;
    if (leaf_thread_all->all_next == NULL) {
        // only 1 thread left
        leaf_thread_enabled = 0;
    }
    // thread switch will occur after we enable irqs
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    enable_irq(irq_state);
    // should not return
    __fatal_error("could not terminate");
}

uint32_t leaf_thread_new(leaf_thread_t *thread, void *stack, size_t stack_len, void *entry, void *arg) {
    uint32_t *stack_top = (uint32_t*)stack + stack_len; // stack is full descending
    *--stack_top = 0x01000000; // xPSR (thumb bit set)
    *--stack_top = (uint32_t)entry & 0xfffffffe; // pc (must have bit 0 cleared, even for thumb code)
    *--stack_top = (uint32_t)leaf_thread_terminate; // lr
    *--stack_top = 0; // r12
    *--stack_top = 0; // r3
    *--stack_top = 0; // r2
    *--stack_top = 0; // r1
    *--stack_top = (uint32_t)arg; // r0
    *--stack_top = 0xfffffff9; // lr (return to thread mode, non-FP, use MSP)
    stack_top -= 8; // r4-r11
    stack_top -= 16; // s16-s31 (we assume all threads use FP registers)
    thread->sp = stack_top;
    thread->local_state = 0;
    thread->arg = arg;
    thread->stack = stack;
    thread->stack_len = stack_len;
    thread->queue_next = NULL;
    uint32_t irq_state = disable_irq();
    leaf_thread_enabled = 1;
    thread->all_next = leaf_thread_all;
    leaf_thread_all = thread;
    leaf_thread_add_to_runable(thread);
    enable_irq(irq_state);
    return (uint32_t)thread; // success
}

void leaf_thread_dump(void) {
    if (!leaf_thread_enabled) {
        printf("THREAD: only main thread\n");
    } else {
        printf("THREAD:\n");
        for (leaf_thread_t *th = leaf_thread_all; th != NULL; th = th->all_next) {
            bool runable = false;
            for (leaf_thread_t *th2 = leaf_thread_cur;; th2 = th2->run_next) {
                if (th == th2) {
                    runable = true;
                    break;
                }
                if (th2->run_next == leaf_thread_cur) {
                    break;
                }
            }
            printf("    id=%p sp=%p sz=%u", th, th->stack, th->stack_len);
            if (runable) {
                printf(" (runable)");
            }
            printf("\n");
        }
    }
}

// should only be called from pendsv_isr_handler
void *leaf_thread_next(void *sp) {
    leaf_thread_cur->sp = sp;
    leaf_thread_cur = leaf_thread_cur->run_next;
    leaf_thread_cur->timeslice = 4; // in milliseconds
    return leaf_thread_cur->sp;
}

void leaf_mutex_init(leaf_mutex_t *m) {
    *m = PYB_MUTEX_UNLOCKED;
}

int leaf_mutex_lock(leaf_mutex_t *m, int wait) {
    uint32_t irq_state = RAISE_IRQ_PRI();
    if (*m == PYB_MUTEX_UNLOCKED) {
        // mutex is available
        *m = PYB_MUTEX_LOCKED;
        RESTORE_IRQ_PRI(irq_state);
    } else {
        // mutex is locked
        if (!wait) {
            RESTORE_IRQ_PRI(irq_state);
            return 0; // failed to lock mutex
        }
        if (*m == PYB_MUTEX_LOCKED) {
            *m = leaf_thread_cur;
        } else {
            for (leaf_thread_t *n = *m;; n = n->queue_next) {
                if (n->queue_next == NULL) {
                    n->queue_next = leaf_thread_cur;
                    break;
                }
            }
        }
        leaf_thread_cur->queue_next = NULL;
        // take current thread off the run list
        leaf_thread_remove_from_runable(leaf_thread_cur);
        // thread switch will occur after we enable irqs
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
        RESTORE_IRQ_PRI(irq_state);
        // when we come back we have the mutex
    }
    return 1; // have mutex
}

void leaf_mutex_unlock(leaf_mutex_t *m) {
    uint32_t irq_state = RAISE_IRQ_PRI();
    if (*m == PYB_MUTEX_LOCKED) {
        // no threads are blocked on the mutex
        *m = PYB_MUTEX_UNLOCKED;
    } else {
        // at least one thread is blocked on this mutex
        leaf_thread_t *th = *m;
        if (th->queue_next == NULL) {
            // no other threads are blocked
            *m = PYB_MUTEX_LOCKED;
        } else {
            // at least one other thread is still blocked
            *m = th->queue_next;
        }
        // put unblocked thread on runable list
        leaf_thread_add_to_runable(th);
    }
    RESTORE_IRQ_PRI(irq_state);
}

#endif // MICROPY_PY_THREAD
