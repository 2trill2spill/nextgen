

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

#include "runtime.h"
#include "signals.h"
#include "network.h"
#include "genetic.h"
#include "resource.h"
#include "concurrent.h"
#include "syscall.h"
#include "utils.h"
#include "plugin.h"
#include "crypto.h"
#include "disas.h"
#include "probe.h"
#include "reaper.h"
#include "mutate.h"
#include "nextgen.h"
#include "platform.h"
#include "file.h"
#include "io.h"
#include "log.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>

static int32_t start_network_mode_runtime(void)
{
    return (0);
}

static int32_t start_syscall_mode_runtime(void)
{
    pid_t runloop_pid = 0;

    /* Create a second process. */
    runloop_pid = fork();
    if(runloop_pid == 0)
    {
        /* Set the pid in the global shared mapping so that other processes
        know this process's pid, so they can wait for it or kill it. */
        cas_loop_int32(&map->runloop_pid, getpid());

        /* Start the main loop for the syscall fuzzer. This function should not 
        return except when the user set's ctrl-c or there is an unrecoverable error. */
        start_main_syscall_loop();

        /* Clean up this process and exit. */
        _exit(0);
    }
    else if(runloop_pid > 0)
    {
        int32_t status = 0;

        /* Wait for the other process's created by nextgen. */
        wait_on(&map->runloop_pid, &status);
        wait_on(&map->reaper_pid, &status);
        wait_on(&map->socket_server_pid, &status);

        /* If were running in smart mode wait for the genetic algorithm(god) to exit. */
        if(map->smart_mode)
            wait_on(&map->god_pid, &status);

        /* Display stats for the user. */
        output(STD, "Sycall test completed: %ld\n", atomic_load(&map->test_counter));

        return (0);
    }
    else
    {
        output(ERROR, "runloop fork failed: %s\n", strerror(errno));
        return (-1);
    }
}

static int32_t start_file_mode_runtime(void)
{
    start_main_file_loop();
    return (0);
}

static int setup_file_mode_runtime(void)
{
    int32_t rtrn = 0;

    /* Create a directory at the path specified by by the user.
    We will store data generated by nextgen that is relevant to
    the user in this directory. */
    rtrn = create_out_directory(map->path_to_out_dir);
    if(rtrn < 0)
    {
        output(ERROR, "Can't create output directory: %s\n", map->path_to_out_dir);
        return (-1);
    }

    /* Setup the file fuzzing module. */
    rtrn = setup_file_module(map->exec_path, map->path_to_in_dir);
    if(rtrn < 0)
    {
        output(ERROR, "Can't setup file module\n");
        return (-1);
    }

    /* Setup the probe module. */
    rtrn = setup_probe_module(map->exec_path);
    if(rtrn < 0)
    {
        output(ERROR, "Can't setup probe module\n");
        return (-1);
    } 

    /* Setup the plugin module. */
    rtrn = setup_plugin_module();
    if(rtrn < 0)
    {
        output(ERROR, "Can't setup plugin module\n");
        return (-1);
    }

    /* Check if the user want's dumb or smart mode. */
    if(map->smart_mode == TRUE)
    {
        output(STD, "Starting file fuzzer in smart mode\n");

        /* Lets parse the binary and figure out the virtual memory address it's going to be loaded at. */
        rtrn = get_load_address();
        if(rtrn < 0)
        {
            output(ERROR, "Can't get load address for target executable\n");
            return -1;
        }

        /* Get address of each branch in binary and store it in map for later. */
        rtrn = disas_executable_and_examine();
        if(rtrn < 0)
        {
            output(ERROR, "Can't disas executable\n");
            return -1;
        }

        /* Start the target executable and pause it before it executes. */
        rtrn = start_and_pause_target();
        if(rtrn < 0)
        {
            output(ERROR, "Can't start target exacutable\n");
            return -1;
        }

        /* Now inject probes into the target process. We will use these probes to feed information into the genetic alorithm
        and to calculate the code coverage of fuzzing proccess. We must do this before injecting the fork server so we can
        avoid injecting probes on each fork(). */
        rtrn = inject_probes();
        if(rtrn < 0)
        {
            output(ERROR, "Can't inject instrumentation probes\n");
            return -1;
        }
    
        /* Inject the fork server. One can learn more about the fork server idea at:
        http://lcamtuf.blogspot.com/2014/10/fuzzing-binaries-without-execve.html .
        We use the fork server so that we can avoid the cost of probe injection and execv
        calls on each fuzz test. This implementation of the fork server use's mach traps and ptrace on non mac osx systems
        as opposed to a C/C++ compiler plugin like the orignal implementation. */
        rtrn = inject_fork_server();
        if(rtrn < 0)
        {
           output(ERROR, "Can't inject fork server\n");
           return -1;
        }

        /* Run through the input files so we can prime the genetic algorithm. */
        rtrn = initial_fuzz_run();
        if(rtrn < 0)
        {
            output(ERROR, "Can't make initial fuzz run\n");
            return -1;
        }
    }
    else
    {
        output(STD, "Starting file fuzzer in dumb mode\n");

    }

    /* Lets set up the signal handler for the main process. */
    setup_signal_handler();

    return 0;
}

static int32_t setup_network_mode_runtime(void)
{
    return (0);
}

/* Set up the various modules we need for syscall fuzzing and then
   create the output directory. If were running in smart mode start the
   genetic module. */
static int32_t setup_syscall_mode_runtime(void)
{
    int32_t rtrn = 0;
    pid_t reaper_pid = 0;

    /* Intialize the reaper module. */
    rtrn = setup_reaper_module(&reaper_pid);
    if(rtrn < 0)
    {
        output(ERROR, "Can't set up the reaper module\n");
        return (-1);
    }

    /* Set the atomic pid in the global shared mapping. */
    cas_loop_int32(&map->reaper_pid, reaper_pid);

    /* Setup the resource module. */
    rtrn = setup_resource_module("/tmp");
    if(rtrn < 0)
    {
        output(ERROR, "Can't setup resource module\n");
        return (-1);
    }

    /* Setup the syscall module. */
    rtrn = setup_syscall_module();
    if(rtrn < 0)
    {
        output(ERROR, "Can't create syscall module\n");
        return (-1);
    }

    /* Create the output directory. This directory is where we store
      test cases and state so that we can resume. */
    rtrn = create_out_directory(map->path_to_out_dir);
    if(rtrn < 0)
    {
        output(ERROR, "Can't create output directory: %s\n", map->path_to_out_dir);
        return (-1);
    }

    /* Check if the user want's dumb or smart mode. */
    if(map->smart_mode == TRUE)
    {
        /* Do init work specific to smart mode. */
        output(STD, "Starting syscall fuzzer in smart mode\n");

        /* Start the genetic algorithm. */
        rtrn = setup_genetic_module();
        if(rtrn < 0)
        {
            output(ERROR, "Can't setup the genetic module\n");
            return (-1);
        }
    }
    else
    {
        /* Do init work specific to dumb mode. */
        output(STD, "Starting syscall fuzzer in dumb mode\n");
    }

    /* Set up the signal handler for the main process. */
    setup_signal_handler();

    return (0);
}

int32_t setup_runtime(void)
{
    output(STD, "Setting up fuzzer\n");

    /* Here we do work that's common across different fuzzing modes, 
     then we call the init function for the selected fuzzing. */
    int32_t rtrn = 0;

    /* This function sets up the crypto functions and crypto library.  */
    rtrn = setup_crypto_module(map->method);
    if(rtrn < 0)
    {
        output(ERROR, "Can't set up crypto\n");
        return (-1);
    }

    /* We are done doing common init work now we call the specific init routines. */ 
    switch((int32_t)map->mode)
    {
        case MODE_FILE:
            rtrn = setup_file_mode_runtime();
            if(rtrn < 0)
            {
                output(ERROR, "Can't setup file mode runtime\n");
                return (-1);
            }
            break;

        case MODE_NETWORK:
            rtrn = setup_network_mode_runtime();
            if(rtrn < 0)
            {
                output(ERROR, "Can't setup network mode runtime\n");
                return (-1);
            }
            break;

        case MODE_SYSCALL:
            rtrn = setup_syscall_mode_runtime();
            if(rtrn < 0)
            {
                output(ERROR, "Can't setup syscall mode runtime\n");
                return (-1);
            }
            break;

        default:
            output(ERROR, "Unknown fuzzing mode\n");
            return (-1);
    }

    return (0);
}

int32_t start_runtime(void)
{
    /* Start the selected fuzzer runtime. */ 
    switch((int32_t)map->mode)
    {
        case MODE_FILE:
            start_file_mode_runtime();
            break;

        case MODE_NETWORK:
            start_network_mode_runtime();
            break;

        case MODE_SYSCALL:
            start_syscall_mode_runtime();
            break;

        default:
            output(ERROR, "Don't understand fuzzing mode selected.\n");
            return (-1);
    }

    return (0);
}

int32_t shutdown(void)
{
    /* Set the atomic flag so that other processes know to start their shutdown procedure. */
    cas_loop_int32(&map->stop, TRUE);

    return (0);
}
