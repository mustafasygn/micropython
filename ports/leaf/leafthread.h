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
#ifndef MICROPY_INCLUDED_STMHAL_PYBTHREAD_H
#define MICROPY_INCLUDED_STMHAL_PYBTHREAD_H

typedef struct _leaf_thread_t {
    void *sp;
    uint32_t local_state;
    void *arg;                  // thread Python args, a GC root pointer
    void *stack;                // pointer to the stack
    size_t stack_len;           // number of words in the stack
    uint32_t timeslice;
    struct _leaf_thread_t *all_next;
    struct _leaf_thread_t *run_prev;
    struct _leaf_thread_t *run_next;
    struct _leaf_thread_t *queue_next;
} leaf_thread_t;

typedef leaf_thread_t *leaf_mutex_t;

extern volatile int leaf_thread_enabled;
extern leaf_thread_t *volatile leaf_thread_all;
extern leaf_thread_t *volatile leaf_thread_cur;

void leaf_thread_init(leaf_thread_t *th);
void leaf_thread_deinit();
uint32_t leaf_thread_new(leaf_thread_t *th, void *stack, size_t stack_len, void *entry, void *arg);
void leaf_thread_dump(void);

static inline uint32_t leaf_thread_get_id(void) {
    return (uint32_t)leaf_thread_cur;
}

static inline void leaf_thread_set_local(void *value) {
    leaf_thread_cur->local_state = (uint32_t)value;
}

static inline void *leaf_thread_get_local(void) {
    return (void*)leaf_thread_cur->local_state;
}

static inline void leaf_thread_yield(void) {
    if (leaf_thread_cur->run_next == leaf_thread_cur) {
        __WFI();
    } else {
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    }
}

void leaf_mutex_init(leaf_mutex_t *m);
int leaf_mutex_lock(leaf_mutex_t *m, int wait);
void leaf_mutex_unlock(leaf_mutex_t *m);

#endif // MICROPY_INCLUDED_STMHAL_PYBTHREAD_H
