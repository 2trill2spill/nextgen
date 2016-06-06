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

#ifndef CONCURRENT_H
#define CONCURRENT_H

#include "ck_queue.h"
#include "ck_spinlock.h"
#include "stdatomic.h"

#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

#ifdef FREEBSD

#include <mqueue.h>

typedef mqd_t msg_port_t;

#elif MAC_OSX

#include <mach/port.h>
#include <mach/mach.h>

typedef mach_port_t msg_port_t;

#elif COMMON

struct msg_port;

typedef struct msg_port msg_port_t;

#endif

#define NX_SPINLOCK_INITIALIZER CK_SPINLOCK_INITIALIZER

#define NX_LIST_ENTRY(x) CK_LIST_ENTRY(x) list_entry
#define NX_SLIST_ENTRY(x) CK_SLIST_ENTRY(x) list_entry

#define NX_LIST_HEAD(name,type) 
#define NX_SLIST_HEAD(name,type) CK_SLIST_HEAD(name, type) name

#define NX_SLIST_HEAD_INITIALIZER CK_SLIST_HEAD_INITIALIZER

#define NX_SLIST_INIT(list) CK_SLIST_INIT(list)

#define NX_SLIST_INSERT_HEAD(pool, blk) CK_SLIST_INSERT_HEAD(pool, blk, list_entry)

#define NX_SLIST_FOREACH(block, list) CK_SLIST_FOREACH(block, list, list_entry)

#define NX_SLIST_REMOVE(list, blk, type) CK_SLIST_REMOVE(list, blk, type, list_entry)

#define nx_spinlock_lock(lock) ck_spinlock_lock(lock)

#define nx_spinlock_unlock(lock) ck_spinlock_unlock(lock)

typedef ck_spinlock_t nx_spinlock_t;

/* CAS loop for swapping atomic int32 values. */ 
extern void cas_loop_int32(int32_t *target, int32_t value);

/* CAS loop for swapping atomic uint32 values. */ 
extern void cas_loop_uint32(uint32_t *target, uint32_t value);

/* Simple wrapper function so we can wait on atomic pid values.  */
extern int32_t wait_on(atomic_int_fast32_t *pid, int32_t *status);

/* Message IPC sending function. */
extern int32_t msg_send(msg_port_t remote_port, void *data, uint32_t size);

/* This function forks() and passes a message port to the child process.
Then finally the function pointed to by proc_start is executed in the child process,
with the argument arg. This function is necessary to support message ports on Mac OSX,
which are implemented with mach ports. Because mach ports are not passed on fork()
calls, we have to do a little dance to pass mach ports via special ports. On 
systems besides Mac OSX, fork_pass_port() is just a simple wrapper around fork(). */
int32_t fork_pass_port(msg_port_t *pass_port, int32_t (*proc_start)(msg_port_t port, void *arg), void *arg);

/* Recieve Message from message queue. */
extern void *msg_recv(msg_port_t recv_port);

/* Initialize a message port. */
extern int32_t init_msg_port(msg_port_t *msg_port);

extern int32_t destroy_msg_port(msg_port_t *msg_port);

extern int32_t recv_port(msg_port_t recv_port, msg_port_t *port);

extern int32_t send_port(msg_port_t remote_port, msg_port_t port);

#endif
