

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

#ifndef PROBE_H
#define PROBE_H

#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

struct probe_ctx;

#define REGISTER_IP RIP
#define TRAP_LEN    1
#define TRAP_INST   0xCC
#define TRAP_MASK   0xFFFFFFFFFFFFFF00

extern int32_t inject_probes(pid_t pid);

extern int32_t inject_kernel_probes(struct probe_ctx *probe);

extern int32_t cleanup_kernel_probes(struct probe_ctx *probe);

extern int32_t start_and_pause_target(char *path, pid_t *pid);

extern int32_t inject_fork_server(uint64_t main_address);

extern int32_t setup_probe_module(char *exec_path);

#endif
