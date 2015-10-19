

/**
 * Copyright (c) 2015, Harrison Bowden, Secure Labs, Minneapolis, MN
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

#include "syscall.h"
#include "entry.h"
#include "syscall_table.h"
#include "crypto.h"
#include "signals.h"
#include "io.h"
#include "mutate.h"
#include "arg_types.h"
#include "utils.h"
#include "nextgen.h"
#include "shim.h"

#include <errno.h>
#include <string.h>

#include "stdatomic.h"

/* An index of child context structures. These structures track variables
   local to the child process. */
static struct child_ctx **children;

unsigned int number_of_children;

/* The number of children processes currently running. */
static atomic_uint_fast64_t running_children;

static atomic_uint_fast64_t test_counter;

/* The syscall table. */
static struct syscall_table_shadow *sys_table;

int cleanup_syscall_table(void)
{
    
    return 0;
}

struct syscall_entry_shadow *get_entry(unsigned int syscall_number)
{
    struct syscall_entry_shadow *entry = NULL;

    if(syscall_number > sys_table->number_of_syscalls)
        return NULL;

    entry = mem_alloc(sizeof(struct syscall_entry_shadow));
    if(entry == NULL)
    {
        output(ERROR, "Can't allocate syscall entry\n");
        return NULL;
    }

    entry = &sys_table->sys_entry[syscall_number];

    return entry;
}

struct child_ctx *get_child_from_index(unsigned int i)
{
    if(i > number_of_children)
        return NULL;

    struct child_ctx *child = NULL;

    child = mem_alloc(sizeof(struct child_ctx));
    if(child == NULL)
    {
        output(ERROR, "Can't allocate child context\n");
        return NULL;
    }

    child = children[i];

    return child;
}

static int get_child_index_number(void)
{
    unsigned int i;

    int pid = getpid();

    for(i = 0; i < number_of_children; i++)
    {
        if(atomic_load(&children[i]->pid) == pid)
        {
            return (int)i;
        }
    }
    /* Should not get here. */
    return -1;
}

static void exit_child(void)
{
    int child_number;

    /* Grab the child_ctx index number for this child process. */
    child_number = get_child_index_number();
    if(child_number < 0)
    {
        output(ERROR, "Can't grab child number\n");
        _exit(-1);
    }

    /* Grab child context. */
    struct child_ctx *ctx = children[child_number];

    /* Set the PID as empty. */
    cas_loop_int32(&children[child_number]->pid, EMPTY); 

    /* Decrement the running child counter and exit. */
    atomic_fetch_sub(&running_children, 1);

    _exit(0);
}

static void free_old_arguments(struct child_ctx *ctx)
{
    unsigned int i;
    unsigned int number_of_args = ctx->number_of_args;

    for(i = 0; i < number_of_args; i++)
    {
        if(ctx->arg_value_index[i] == NULL)
        {
            continue;
        }

        free(ctx->arg_value_index[i]);
    }

    return;
}

static void start_smart_syscall_child(void)
{
    int rtrn;
    int child_number;

    /* Grab the child_ctx index number for this child process. */
    child_number = get_child_index_number();
    if(child_number < 0)
    {
        output(ERROR, "Can't grab child number\n");
        exit_child();
    }

    /* Grab child context. */
    struct child_ctx *ctx = children[child_number];

    /* Set the return jump so that we can try fuzzing again on a signal. */
    rtrn = setjmp(ctx->return_jump);
    if(rtrn < 0)
    {
        output(ERROR, "Can't set return jump\n");
        exit_child();
    }

    /* Loop until ctrl-c is pressed by the user. */
    while(atomic_load(&map->stop) != TRUE)
    {
        
    }

    exit_child();
}

/**
 * This is the fuzzing loop for syscall fuzzing mode.
 */
static void start_syscall_child(void)
{
    int rtrn;
    int child_number;

    /* Grab the child_ctx index number for this child process. */
    child_number = get_child_index_number();
    if(child_number < 0)
    {
        output(ERROR, "Can't grab child number\n");
        exit_child();
    }

    /* Grab child context. */
    struct child_ctx *ctx = children[child_number];

    /* Set the return jump so that we can try fuzzing again on a signal. */
    rtrn = setjmp(ctx->return_jump);
    if(rtrn < 0)
    {
        output(ERROR, "Can't set return jump\n");
        exit_child();
    }

    /* Check if we should stop or continue running. */
    while(atomic_load(&map->stop) != TRUE)
    {
        /* Randomly pick the syscall to test. */
        rtrn = pick_syscall(ctx);
        if(rtrn < 0)
        {
            output(ERROR, "Can't pick syscall to test\n");
            exit_child();
        }

        /* Generate arguments for the syscall selected. */
        rtrn = generate_arguments(ctx);
        if(rtrn < 0)
        {
            output(ERROR, "Can't pick syscall to test\n");
            exit_child();
        }

        /* Mutate the arguments randomly. */
        rtrn = mutate_arguments(ctx);
        if(rtrn < 0)
        {
            output(ERROR, "Can't mutate arguments\n");
            exit_child();
        }

        /* Log the arguments before we use them, in case we cause a
        kernel panic, so we know what caused the panic. */
        rtrn = log_arguments(ctx);
        if(rtrn < 0)
        {
            output(ERROR, "Can't log arguments\n");
            exit_child();
        }

        /* Run the syscall we selected with the arguments we generated and mutated. This call usually
        crashes and causes us to jumb back to the setjmp call above.*/
        rtrn = test_syscall(ctx);
        if(rtrn < 0)
        {
            output(ERROR, "Syscall call failed\n");
            exit_child();
        }

        /* Log return values. */
        rtrn = log_results(ctx);
        if(rtrn < 0)
        {
            output(ERROR, "Can't log results\n");
            exit_child();
        }

        /* If we didn't crash cleanup are mess. If we don't do this the generate
        functions will crash in a hard to understand way. */
        free_old_arguments(ctx);
    }

    /* We should not get here, but if we do, exit so we can be restarted. */
    exit_child();
}

struct child_ctx *get_child_ctx(void)
{
    struct child_ctx *child = NULL;

    child = mem_alloc(sizeof(struct child_ctx));
    if(child == NULL)
    {
        output(ERROR, "Can't allocate child context\n");
        return NULL;
    }

    child = get_child_ctx();
    if(child == NULL)
    {
        output(ERROR, "Can't get child context\n");
        mem_free(child); // Free on error so we don't leak memory.
        return NULL;
    }

    return child;
}

static struct arg_context *get_arg_context(enum arg_type type)
{
    switch((int)type)
    {
        case FILE_DESC: return &file_desc_ctx;

        case VOID_BUF: return &void_buf_ctx;
            
        case SIZE: return &size_ctx;

        case FILE_PATH: return &file_path_ctx;

        case OPEN_FLAG: return &open_flag_ctx;
           
        case MODE: return &mode_ctx;

        case STAT_FS: return &stat_fs_ctx;

        case STAT_FLAG: return &stat_flag_ctx;

        case INT: return &int_ctx;
       
        case RUSAGE: return &rusage_ctx;
         
        case PID: return &pid_ctx;
          
        case WAIT_OPTION: return &wait_option_ctx;
         
        case SOCKET: return &socket_ctx;
           
        case WHENCE: return &whence_ctx;
           
        case OFFSET: return &offset_ctx;
        
        case MOUNT_TYPE: return &mount_type_ctx;
         
        case DIR_PATH: return &dir_path_ctx;
          
        case MOUNT_FLAG: return &mount_flag_ctx;

        case UNMOUNT_FLAG: return &unmount_flag_ctx;

        case RECV_FLAG: return &recv_flag_ctx;
         
        case REQUEST: return &request_ctx;

        case MOUNT_PATH: return &mount_path_ctx;

        case DEV: return &dev_ctx;

        default:
            output(ERROR, "Unlisted arg type\n");
            return NULL;
    }
}

static int init_syscall_child(unsigned int i)
{
    int rtrn;

    /* Set the child pid. */
    cas_loop_int32(&children[i]->pid, getpid());

    /* Set up the child signal handler. */
    setup_syscall_child_signal_handler();

    /* If were using a software PRNG we need to seed the PRNG. */
    if(using_hardware_prng() == FALSE)
    {
        /* We got to seed the prng so that the child process trys different syscalls. */
        rtrn = seed_prng();
        if(rtrn < 0)
        {
            output(ERROR, "Can't init syscall\n");
            return -1;
        }
    }

    /* Increment the running child counter. */
    atomic_fetch_add(&running_children, 1);

    /* Inform main loop we are done setting up. */
    write(children[i]->msg_port[1], "1", 1);

    return 0;
}

void create_syscall_children(void)
{
    int rtrn;
    unsigned int i;

    /* Walk the child structure index and find the first empty child slot. */
    for(i = 0; i < number_of_children; i++)
    {
        /* If the child has a pid of EMPTY let's create a new one. */
        if(atomic_load(&children[i]->pid) == EMPTY)
        {
            pid_t child_pid;

            /* Create pipe here so we can communicate with the child and avoid a race condition. */
            rtrn = pipe(children[i]->msg_port);
            if(rtrn < 0)
            {
                output(ERROR, "Can't create msg port: %s\n", strerror(errno));
                return;
            }

            child_pid = fork();
            if(child_pid == 0)
            {
                /* Initialize the new syscall child. */
                init_syscall_child(i);

                /* If were in dumb mode start the dumb syscall loop. */
                if(map->smart_mode != TRUE)
                {
                    start_syscall_child();

                    _exit(0);
                }
                else
                {
                    start_smart_syscall_child();

                    _exit(0);
                }
            }
            else if(child_pid > 0)
            {
                char *msg_buf auto_clean = NULL;

                msg_buf = mem_alloc(2);
                if(msg_buf == NULL)
                {
                    output(ERROR, "Can't create message buf: %s\n", strerror(errno));
                    return;
                }

                /* Wait for the child to be done setting up. */
                ssize_t ret = read(children[i]->msg_port[0], msg_buf, 1);
                if(ret < 1)
                {
                    output(ERROR, "Problem waiting for child setup: %s\n", strerror(errno));
                    return;
                }
            }
            else
            {
                output(ERROR, "Can't create child process: %s\n", strerror(errno));
                return;
            }
        }
    }
    return;
}

struct syscall_table_shadow *get_syscall_table(void)
{
    output(STD, "Building system call table\n");

    /* Grab a copy of the syscall table in on disk format. */ 
    struct syscall_table *syscall_table = get_table();
    if(sys_table == NULL)
    {
        output(STD, "Can't grab syscall table\n");
        return NULL;
    }

    /* Create a shadow syscall table.*/
    struct syscall_table_shadow *shadow_table = mem_alloc(sizeof(struct syscall_table_shadow));
    if(shadow_table == NULL)
    {
        output(ERROR, "Can't create shadow table\n");
        return NULL;
    }

    /* Set the number_of_syscalls. */
    shadow_table->number_of_syscalls = syscall_table->number_of_syscalls;

    /* Our loop incrementers. */
    unsigned int i, ii;

    /* Allocate heap memory for the list of syscalls. */
    shadow_table->sys_entry = mem_alloc(shadow_table->number_of_syscalls * sizeof(struct syscall_entry_shadow));
    if(shadow_table->sys_entry == NULL)
    {
        output(ERROR, "Can't create new entry\n");
        return NULL;
    }

    /* This is the entry offset, it one because the entries start at [1] instead of [0]; */
    unsigned int offset = 1;

    /* We use this to track where to put the new entry in the entry index. */
    unsigned int shadow_entry_offset = 0;

    /* Loop for each entry syscall and build a table from the on disk format. */
    for(i = 0; i < shadow_table->number_of_syscalls; i++)
    {
    	/* Check if the syscall is OFF, this is usually for syscalls in development. */
    	if(syscall_table[i + offset].sys_entry->status == OFF)
    	{
            shadow_table->number_of_syscalls--;
            continue;
    	}

        /* Create and intialize the const value for a shadow struct. */
        struct syscall_entry_shadow entry = {  

            .number_of_args = syscall_table[i + offset].sys_entry->number_of_args, 
            .name_of_syscall = syscall_table[i + offset].sys_entry->name_of_syscall 
        };

        /* Loop for each arg and set the arg index's. */
        for(ii = 0; ii < entry.number_of_args; ii++)
        {
            entry.get_arg_index[ii] = syscall_table[i + offset].sys_entry->get_arg_index[ii];

            entry.arg_context_index[ii] = get_arg_context((enum arg_type)syscall_table[i + offset].sys_entry->arg_type_index[ii]);
            if(entry.arg_context_index[ii] == NULL)
            {
                output(ERROR, "Can't gwt arg context\n");
                return NULL;
            }
        }
     
        /* Init the automic value. */
        atomic_init(&entry.status, ON);

        /* Set the newly created entry in the index. */
        shadow_table->sys_entry[shadow_entry_offset] = entry;

        /* Increment offset. */
        shadow_entry_offset++;
    }

    return shadow_table;
}

/* This function is used to randomly pick the syscall to test. */
int pick_syscall(struct child_ctx *ctx)
{
    /* Use rand_range to pick a number between 0 and the number_of_syscalls.  */
    int rtrn = rand_range(sys_table->number_of_syscalls, &ctx->syscall_number);
    if(rtrn < 0)
    {
        output(ERROR, "Can't generate random number\n");
        return -1;
    }

    /* Copy entry values into the child context. */
    ctx->number_of_args = sys_table->sys_entry[ctx->syscall_number].number_of_args;

    /* Set values in ctx so we can get info from ctx instead using all these pointers. */ 
    ctx->need_alarm = sys_table->sys_entry[ctx->syscall_number].need_alarm;

    ctx->syscall_symbol = sys_table->sys_entry[ctx->syscall_number].syscall_symbol;

    ctx->had_error = NO;

    return 0;
}

int generate_arguments(struct child_ctx *ctx)
{
    int rtrn;
    unsigned int i;
    unsigned int number_of_args = ctx->number_of_args;

    for(i = 0; i < number_of_args; i++)
    {
        ctx->current_arg = i;

        rtrn = sys_table->sys_entry[ctx->syscall_number].get_arg_index[i](&ctx->arg_value_index[i], ctx);
        if(rtrn < 0)
        {
            output(ERROR, "Can't generate arguments for: %s\n", sys_table->sys_entry[ctx->syscall_number].name_of_syscall);
            return -1;
        }
    }

	return 0;
}

static int check_for_failure(int ret_value)
{
    if(ret_value < 0)
        return -1;

    return 0;
}

int test_syscall(struct child_ctx *ctx)
{
    /* Grab argument values. */
    unsigned long *arg1 = ctx->arg_value_index[0];
    unsigned long *arg2 = ctx->arg_value_index[1];
    unsigned long *arg3 = ctx->arg_value_index[2];
    unsigned long *arg4 = ctx->arg_value_index[3];
    unsigned long *arg5 = ctx->arg_value_index[4];
    unsigned long *arg6 = ctx->arg_value_index[5];

    /* Check if we need to set the alarm for blocking syscalls.  */
    if(ctx->need_alarm == YES)
        alarm(1);

    /* Set the time of the syscall test. */
    (void)gettimeofday(&ctx->time_of_syscall, NULL);

#ifndef ASAN

    /* Do the add before the syscall test because we usually crash on the syscall test
    and we don't wan't to do this in a signal handler. */
    atomic_fetch_add(&map->test_counter, 1);

    /* Call the syscall with the args generated. */
    ctx->ret_value = syscall(ctx->syscall_symbol, *arg1, *arg2, *arg3, *arg4, *arg5, *arg6);
    if(check_for_failure(ctx->ret_value) < 0)
    {
        /* If we got here, we had an error, so grab the error string. */
        ctx->err_value = strerror(errno);

        /* Set the error flag so the logging system knows we had an error. */
        ctx->had_error = YES;
    }
    
#endif

	return 0;
}

void kill_all_children(void)
{
    int rtrn;
    unsigned int i;

    for(i = 0; i < number_of_children; i++)
    {
        rtrn = kill(atomic_load(&children[i]->pid), SIGKILL);
        if(rtrn < 0)
        {
            output(ERROR, "Can't kill child: %s\n", strerror(errno));
            return;
        }
    }

    return;
}

void start_main_syscall_loop(void)
{
    output(STD, "Starting fuzzer\n");

    /* Set up signal handler. */
    setup_signal_handler();

    /* Check if we should stop or continue running. */
    while(atomic_load(&map->stop) == FALSE)
    {
        /* Check if we have the right number of children processes running, if not create a new ones until we do. */
        if(atomic_load(&running_children) < number_of_children)
        {
            /* Create children process. */
            create_syscall_children();
        }
    }

    output(STD, "Exiting main loop\n");

    return;
}

static int init_child_context(struct child_ctx *child)
{   
    /* Allocate the child context object. */
    child = mem_alloc(sizeof(struct child_ctx));
    if(child == NULL)
    {
        output(ERROR, "Can't create child context\n");
        return -1;
    }

    /* Set current arg to zero. */
    child->current_arg = 0;
    atomic_init(&child->pid, EMPTY);
     
    /* Create the index where we store the syscall arguments. */
    child->arg_value_index = mem_alloc(7 * sizeof(unsigned long *));
    if(child->arg_value_index == NULL)
    {
        output(ERROR, "Can't create arg value index: %s\n", strerror(errno));
        return -1;
    }

    /* This index tracks the size of of the argument generated.  */
    child->arg_size_index = mem_alloc(7 * sizeof(unsigned long));
    if(child->arg_size_index == NULL)
    {
        output(ERROR, "Can't create arg size index: %s\n", strerror(errno));
        return -1;
    }

    /* This is where we store the error string on each syscall test. */
    child->err_value = mem_alloc(1024);
    if(child->err_value == NULL)
    {
        output(ERROR, "err_value: %s\n", strerror(errno));
        return -1;
    }

    unsigned int ii;

    /* Loop and create the various indecies in the child struct. */
    for(ii = 0; ii < 6; ii++)
    {
        child->arg_value_index[ii] = mem_alloc(sizeof(unsigned long));
        if(child->arg_value_index[ii] == NULL)
        {
            output(ERROR, "Can't create arg value\n");
            return -1;
        }
    }

    return 0;
}

int setup_syscall_module(void)
{
    int rtrn;

    /* Set running children to zero. */
    atomic_init(&running_children, 0);

    /* This counter counts how many syscall test we have done. */
    atomic_init(&test_counter, 0);

    /* Allocate the system call table as shared memory. */
    sys_table = mem_alloc(sizeof(struct syscall_table));
    if(sys_table == NULL)
    {
        output(ERROR, "Can't allocate syscall table\n");
        return -1;
    }

    /* Grab the core count of the machine we are on and set the number
    of syscall children to the core count. */
    rtrn = get_core_count(&number_of_children);
    if(rtrn < 0)
    {
        output(ERROR, "Can't get core count\n");
        return -1;
    }

    /* Create the child process structures. */
    children = mem_alloc(number_of_children * sizeof(struct child_ctx *));
    if(children == NULL)
    {
        output(ERROR, "Can't create children index.\n");
        return -1;
    }

    unsigned int i;

    /* Loop for each child and allocate the child context object. */
    for(i = 0; i < number_of_children; i++)
    {
        struct child_ctx *child = NULL;

        rtrn = init_child_context(child);
        if(rtrn < 0)
        {
            output(ERROR, "Can't init child context\n");
            return -1;
        }
    }

    return 0;
}
