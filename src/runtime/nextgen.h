

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

#ifndef NEXTGEN_H
#define NEXTGEN_H

#include "platform.h"
#include "crypto/crypto.h"
#include "concurrent/concurrent.h"
#include "syscall/syscall_table.h"

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

struct fuzzer_config;

enum fuzz_mode { MODE_FILE, MODE_SYSCALL, MODE_NETWORK };

/* This struct is mapped as shared anonymous memory and is used to communicate between
the various process and threads created by nextgen. */
struct shared_map
{
    /* This tells us what fuzzing mode to execute. */
    enum fuzz_mode mode;

    /* The method with which to derive the random numbers. */
    enum crypto_method method;

    /* The input and output directory paths. */
    char *input_path;
    char *output_path;

    /* Path to target executable. */
    char *exec_path;

    /* The PID of the target app being file fuzzed. */
    int32_t target_pid;

    /* An value used to tell the various processes whether to run or not. */
    int32_t stop;

    /* If this mode is FALSE then we don't use the binary feedback and genetic algorithm. */
    int32_t smart_mode;

    /* The port that the ipv4 socket server is on. The ipv6 port is ipv4 + 1. */
    uint32_t socket_server_port;

    /* Counter for the number of fuzz test perform. */
    uint32_t test_counter;

    const char padding[4];

    /* The epoch object used for coordinating
      reclamation of shared memory objects.  */
    epoch_ctx epoch;

    /* The thread that the genetic algo lives in. */
    pthread_t gen_algo_thread;
};

extern struct shared_map *map;

/**
 * This function parses the command line for various options passed by the user.
 * @param argc The number of command line options passed.
 * @param argv An array with the command line options.
 * @param A output writter object.
 * @param A memory allocator object.
 * @return A fuzzer_config object on success and NULL on failure.
 */
extern struct fuzzer_config *parse_cmd_line(int32_t argc,
                                            char *argv[],
                                            struct output_writter *output,
                                            struct memory_allocator *allocator);
#endif
