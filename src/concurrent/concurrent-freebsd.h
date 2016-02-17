/**
 * Copyright (c) 2016, Harrison Bowden, Minneapolis, MN
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

#ifndef NX_CONCURRENT_FREEBSD_H
#define NX_CONCURRENT_FREEBSD_H

#include <mqueue.h>

typedef mqd_t msg_port_t;


/* Message IPC sending function. */
extern int32_t msg_send(msg_port_t send_port, msg_port_t remote_port, void *msg_data);

/* This function forks() and passes a message port to the child process.
Then finally the function pointed to by proc_start is executed in the child process,
with the argument arg. This function is necessary to support message ports on Mac OSX,
which are implemented with mach ports. Because mach ports are not passed on fork()
calls, we have to do a little dance to pass mach ports via special ports. On 
systems besides Mac OSX, fork_pass_port() is just a simple wrapper around fork(). */
int32_t fork_pass_port(msg_port_t pass_port, int32_t (*proc_start)(msg_port_t port, void *arg), void *arg);

/* Recieve Message from message queue. */
extern void *msg_recv(msg_port_t recv_port);

/* Initialize a message port. */
extern msg_port_t init_msg_port(void);

#endif
