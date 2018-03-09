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

typedef struct _robot_thread_t {
    void *sp;
    uint32_t local_state;
    void *arg;                  // thread Python args, a GC root pointer
    void *stack;                // pointer to the stack
    size_t stack_len;           // number of words in the stack
    uint32_t timeslice;
    struct _robot_thread_t *all_next;
    struct _robot_thread_t *run_prev;
    struct _robot_thread_t *run_next;
    struct _robot_thread_t *queue_next;
} robot_thread_t;

typedef robot_thread_t *robot_mutex_t;

extern volatile int robot_thread_enabled;
extern robot_thread_t *volatile robot_thread_all;
extern robot_thread_t *volatile robot_thread_cur;

void robot_thread_init(robot_thread_t *th);
void robot_thread_deinit();
uint32_t robot_thread_new(robot_thread_t *th, void *stack, size_t stack_len, void *entry, void *arg);
void robot_thread_dump(void);

static inline uint32_t robot_thread_get_id(void) {
    return (uint32_t)robot_thread_cur;
}

static inline void robot_thread_set_local(void *value) {
    robot_thread_cur->local_state = (uint32_t)value;
}

static inline void *robot_thread_get_local(void) {
    return (void*)robot_thread_cur->local_state;
}

static inline void robot_thread_yield(void) {
    if (robot_thread_cur->run_next == robot_thread_cur) {
        __WFI();
    } else {
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    }
}

void robot_mutex_init(robot_mutex_t *m);
int robot_mutex_lock(robot_mutex_t *m, int wait);
void robot_mutex_unlock(robot_mutex_t *m);

#endif // MICROPY_INCLUDED_STMHAL_PYBTHREAD_H
