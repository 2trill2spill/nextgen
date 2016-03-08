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

#include "nextgen.h"
#include "crypto/crypto.h"
#include "io/io.h"
#include "memory/memory.h"
#include "file/file.h"
#include "reaper/reaper.h"
#include "plugins/plugin.h"
#include "resource/resource.h"
#include "runtime/runtime.h"

#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <dtrace.h>
#include <sys/mman.h>

struct shared_map *map;

struct parser_ctx
{
    enum fuzz_mode mode;
    char *path_to_exec;
    char *path_to_in_dir;
    char *path_to_out_dir;
    enum crypto_method method;
    char *args;
    int32_t smart_mode;
};

static const char *optstring = "p:e:";

static struct option longopts[] = {
    { "in", required_argument, NULL, 'i' },
    { "out", required_argument, NULL, 'o' },
    { "exec", required_argument, NULL, 'e'},
    { "crypto", required_argument, NULL, 'c' },
    { "port", required_argument, NULL, 'p'},
    { "address", required_argument, NULL, 'a'},
    { "protocol", required_argument, NULL, 'c'},
    { "args", required_argument, NULL, 'x'},
    { "file", 0, NULL, 'f'},
    { "network", 0, NULL, 'n'},
    { "syscall", 0, NULL, 's'},
    { "help", 0, NULL, 'h'},
    { "dumb", 0, NULL, 'd'},
    { NULL, 0, NULL, 0 }
};

static void display_help_banner(void)
{
    output(STD, "Nextgen is a Genetic File, Syscall, and Network Fuzzer.\n");
    output(STD, "To use the file fuzzer in smart mode run the command below.\n");
    output(STD, "sudo ./nextgen --file --in /path/to/in/directory --out /path/to/out/directory --exec /path/to/target/exec .\n");

    output(STD, "To use the syscall fuzzer in smart mode run.\n");
    output(STD, "sudo ./nextgen --syscall --out /path/to/out/directory\n");

    output(STD, "To use dumb mode just pass --dumb with any of the above commands.\n");

    return;
}

int32_t init_parser_ctx(struct parser_ctx **ctx)
{
    if((*ctx) != NULL)
    {
        output(ERROR, "Non NULL parser context pointer passed\n");
        return (-1);
    }

    (*ctx) = mem_alloc(sizeof(struct parser_ctx));
    if((*ctx) == NULL)
    {
        output(ERROR, "Can't allocate parser context\n");
        return (-1);
    }

    return (0);
}

int32_t set_fuzz_mode(struct parser_ctx *ctx, enum fuzz_mode mode)
{
    /* Make sure the mode passed is legit. */
    switch((int32_t)mode)
    {
        case MODE_FILE:
        case MODE_SYSCALL:
        case MODE_NETWORK:
            break;

        default:
            output(ERROR, "Unknown fuzz mode\n");
            return (-1);
    }

    /* Set fuzz mode to mode passed by user. */
    ctx->mode = mode;

    return (0);
}

int32_t set_crypto_method(struct parser_ctx *ctx, enum crypto_method method)
{
    switch((int32_t)method)
    {
        case CRYPTO:
        case NO_CRYPTO:
            break;

        default:
            output(ERROR, "Unknown crypto method\n");
            return (-1);
    }

    ctx->method = method;

    return (0);
}

struct parser_ctx *parse_cmd_line(int32_t argc, char *argv[])
{
    int32_t ch = 0;
    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;
    int32_t iFlag = FALSE, oFlag = FALSE, fFlag = FALSE, nFlag = FALSE, 
    sFlag = FALSE, eFlag = FALSE, pFlag = FALSE, aFlag = FALSE, tFlag = FALSE;

    /* Allocate the parser context. */
    rtrn = init_parser_ctx(&ctx);
    if(rtrn < 0)
    {
        output(ERROR, "Can't init parser context\n");
        return (NULL);
    }

    /* Default to smart_mode and crypto numbers. */
    ctx->smart_mode = TRUE;
    rtrn = set_crypto_method(ctx, CRYPTO);
    if(rtrn < 0)
    {
        output(ERROR, "Can't set crypto method\n");
        return (NULL);
    }

    /* Parse the command line. */
    while((ch = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
    {
        switch(ch)
        {
            /* Display banner and exit. */
            case 'h':
                display_help_banner();
                return (NULL);

            case 'p':
                pFlag = TRUE;
                break;

            case 'a':
                aFlag = TRUE;
                break;

            case 't':
                tFlag = TRUE;
                break;

            case 'e':
                rtrn = asprintf(&ctx->path_to_exec, "%s", optarg);
                if(rtrn < 0)
                {
                    output(ERROR, "asprintf: %s\n", strerror(errno));
                    return (NULL);
                }

                eFlag = TRUE;
                break;

            case 'i':
                rtrn = asprintf(&ctx->path_to_in_dir, "%s", optarg);
                if(rtrn < 0)
                {
                    output(ERROR, "asprintf: %s\n", strerror(errno));
                    return (NULL);
                }

                iFlag = TRUE;
                break;

            case 'o':
                rtrn = asprintf(&ctx->path_to_out_dir, "%s", optarg);
                if(rtrn < 0)
                {
                    output(ERROR, "asprintf: %s\n", strerror(errno));
                    return (NULL);
                }

                oFlag = TRUE;
                break;

            case 'f':
                fFlag = TRUE;
                rtrn = set_fuzz_mode(ctx, MODE_FILE);
                if(rtrn < 0)
                {
                    output(ERROR, "Can't set fuzz mode\n");
                    return (NULL);
                }
                break;

            case 'n':
                nFlag = TRUE;
                rtrn = set_fuzz_mode(ctx, MODE_NETWORK);
                if(rtrn < 0)
                {
                    output(ERROR, "Can't set fuzz mode\n");
                    return (NULL);
                }
                break;

            case 's':
                sFlag = TRUE;
                rtrn = set_fuzz_mode(ctx, MODE_SYSCALL);
                if(rtrn < 0)
                {
                    output(ERROR, "Can't set fuzz mode\n");
                    return (NULL);
                }
                break;

            case 'c':
                /* This option allows users to specify the method in which they want to derive
                the random numbers that will be used in fuzzing the application. */
                rtrn = set_crypto_method(ctx, NO_CRYPTO);
                if(rtrn < 0)
                {
                    output(ERROR, "Can't set crypto method\n");
                    return (NULL);
                }
                break;

            case 'd':
                ctx->smart_mode = FALSE;
                break;

            case 'x':
                rtrn = asprintf(&ctx->args, "%s", optarg);
                if(rtrn < 0)
                {
                    output(ERROR, "asprintf: %s\n", strerror(errno));
                    return (NULL);
                }
                break;

            default:
              output(ERROR, "Unknown option\n");
              return (NULL);
        }
    }

    /* Make sure a fuzzing mode was selected. */
    if(fFlag != TRUE && nFlag != TRUE && sFlag != TRUE)
    {
        output(STD, "Specify a fuzzing mode\n");
        return NULL;
    }

    /* If file mode was selected lets make sure all the right args were passed.*/
    if(fFlag == TRUE)
    {
        if(iFlag == FALSE || oFlag == FALSE || eFlag == FALSE)
        {
            output(STD, "Pass --exec , --in and --out for file mode\n");
            return NULL;
        }
    }

    if(nFlag == TRUE)
    {
        if(aFlag == FALSE || tFlag == FALSE || oFlag == FALSE)
        {
            output(STD, "Pass --address , --port, --protocol, and --out for network mode\n");
            return NULL;
        }
    }

    /* Check to see if syscall mode was selected. */
    if(sFlag == TRUE)
    {
        /* Make sure the user set an output path. */
        if(oFlag != TRUE)
        {
            output(STD, "Pass --out for syscall mode\n");
            return NULL;
        }
    }

    return ctx;
}

static void clean_syscall_mapping(void)
{
    return;
}

void clean_shared_mapping(void)
{
    /* Do mode specific shared mapping cleanup. */
    switch((int32_t)map->mode)
    {
        case MODE_SYSCALL:
            clean_syscall_mapping();
            break;

        case MODE_FILE:
           
            break;

        case MODE_NETWORK:
           
            break;

        default:
            output(ERROR, "Unknown fuzzing mode\n");
            return;
    }
    return;
}

static int32_t init_file_mapping(struct shared_map **mapping, struct parser_ctx *ctx)
{
    /* Set the input and output directories. */
    (*mapping)->path_to_out_dir = ctx->path_to_out_dir;
    (*mapping)->path_to_in_dir = ctx->path_to_in_dir;

    (*mapping)->exec_path = ctx->path_to_exec;

    atomic_init(&(*mapping)->target_pid, 0);

    return 0;
}

static int32_t init_network_mapping(struct shared_map **mapping, struct parser_ctx *ctx)
{

    return 0;
}

static int32_t init_syscall_mapping(struct shared_map **mapping, struct parser_ctx *ctx)
{
    /* Set the outpath directory. */
    (*mapping)->path_to_out_dir = ctx->path_to_out_dir;

    /* We use atomic values for the pids, so let's init the reaper pid. */
    atomic_init(&(*mapping)->reaper_pid, 0);
    atomic_init(&(*mapping)->god_pid, 0);

    /* Intialize socket server values.*/
    (*mapping)->socket_server_port = 0;
    atomic_init(&(*mapping)->socket_server_pid, 0);

    /* Set this counter to zero. */
    atomic_init(&(*mapping)->test_counter, 0);

    return 0;
}

/**
 * We use this function to initialize all the shared maps member values.
 **/
int32_t init_shared_mapping(struct shared_map **mapping, 
                            struct parser_ctx *ctx)
{
    int32_t rtrn = 0;

    /* Set the fuzzing mode selected by the user. */
    (*mapping)->mode = ctx->mode;

    /* Set the crypto method. */
    (*mapping)->method = ctx->method;

    /* Set intelligence mode. */
    (*mapping)->smart_mode = ctx->smart_mode;

    /* Set the stop flag to FALSE, when set to TRUE all processes start their exit routines and eventually exit. */
    atomic_init(&(*mapping)->stop, FALSE);

    /* Init the runloop pid. */
    atomic_init(&(*mapping)->runloop_pid, 0);

    /* Do mode specific shared mapping setup. */
    switch((int32_t)ctx->mode)
    {
        case MODE_SYSCALL:
            rtrn = init_syscall_mapping(mapping, ctx);
            if(rtrn < 0)
            {
                output(ERROR, "Can't init syscall mapping\n");
                return (-1);
            }
            break;

        case MODE_FILE:
            rtrn = init_file_mapping(mapping, ctx);
            if(rtrn < 0)
            {
                output(ERROR, "Can't init file mapping\n");
                return (-1);
            }
            break;

        case MODE_NETWORK:
            rtrn = init_network_mapping(mapping, ctx);
            if(rtrn < 0)
            {
                output(ERROR, "Can't init network mapping\n");
                return (-1);
            }
            break;

        default:
            output(ERROR, "Unknown fuzzing mode\n");
            return (-1);
    }
    
    return (0);

}
