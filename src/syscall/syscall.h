/**
 * Copyright (c) 2015, Harrison Bowden, Minneapolis, MN
 * 
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice 
 * and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "context.h"

#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>

enum child_state {EMPTY};

extern uint32_t number_of_children;

extern int32_t get_total_syscalls(uint32_t *total);

extern struct syscall_entry *get_entry(uint32_t syscall_number);

extern struct child_ctx *get_child_from_index(uint32_t i);

extern struct child_ctx *get_child_ctx(void);

extern struct child_ctx *get_child_ctx_from_pid(pid_t pid);

extern struct syscall_table *get_syscall_table(void);

extern void cleanup_syscall_table(struct syscall_table **table);

extern int32_t pick_syscall(struct child_ctx *ctx);

extern int32_t generate_arguments(struct child_ctx *ctx);

extern int32_t test_syscall(struct child_ctx *ctx);

extern void create_syscall_children(void);

extern void kill_all_children(void);

extern int32_t setup_syscall_module(int32_t *stop_ptr, 
	                                uint32_t *counter,
	                                int32_t run_mode);

extern void start_main_syscall_loop(void);

#endif
