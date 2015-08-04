

#include "syscall.h"
#include "entry.h"
#include "syscall_table.h"
#include "crypto.h"
#include "utils.h"
#include "shim.h"

int get_syscall_table(void)
{
    output(STD, "Building system call table\n");

	/* Grab a copy of the system table. */ 
	struct syscall_table *sys_table = get_table();

    /* Check how many syscalls there are in the table. */
    unsigned int number_of_syscalls = map->sys_table->number_of_syscalls;

    /* Create a shadow syscall table. */
    struct syscall_table_shadow *shadow_table = malloc(sizeof(struct syscall_table));
    if(shadow_table == NULL)
    {
        output(ERROR, "Can't create shadow table\n");
        return -1;
    }

    unsigned int i;

    /* Allocate heap memory for the list of syscalls. */
    shadow_table->sys_entry = malloc(number_of_syscalls * sizeof(struct syscall_entry));
    if(shadow_table->sys_entry == NULL)
    {
        output(ERROR, "Can't create new entry\n");
        return -1;
    }

    /* Loop for each entry syscall and build a table from the on disk format. */
    for(i = 0; i < number_of_syscalls; i++)
    {
        struct syscall_entry_shadow entry;

        entry.name_of_syscall = sys_table->sys_entry[i].name_of_syscall;
        entry.number_of_args = sys_table->sys_entry[i].number_of_args;

        if(sys_table->sys_entry[i].status == ON)
        {
            atomic_init(&entry.status, ON);
        }
        else
        {
            atomic_init(&entry.status, OFF);	
        }

        shadow_table->sys_entry[i] = entry;
    }

	return 0;
}

/* This function is used to randomly pick the syscall to test. */
int pick_syscall(struct child_ctx *ctx)
{

	/* Our variables we will be using. */
    int rtrn;
    unsigned int syscall_number;

    /* Use rand_range to pick a number between 0 and the number_of_syscalls.  */
    rtrn = rand_range(ctx->sys_table->number_of_syscalls, &syscall_number);
    if(rtrn < 0)
    {
        output(ERROR, "Can't generate random number\n");
        return -1;
    }

    ctx->syscall_number = syscall_number + 1;

    return 0;
}

int generate_arguments(struct child_ctx *ctx)
{
    unsigned int i;
    unsigned int number_of_args = ctx->sys_table->sys_entry[ctx->syscall_number].number_of_args;
    int rtrn;

    for(i = 0; i < number_of_args; i++)
    {
    	/* This crazy line allows us to avoid a large switch stament in the code. */
        rtrn = ctx->sys_table->sys_entry[ctx->syscall_number].get_arg_index[i](&ctx->arg_value_index[i]);
        if(rtrn < 0)
        {
        	output(ERROR, "Can't generate arguments for: %s\n", ctx->sys_table->sys_entry[ctx->syscall_number].name_of_syscall);
            return -1;
        }
    }

	return 0;
}