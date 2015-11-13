

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

#include "test_utils.h"
#include "stdatomic.h"
#include "../../src/syscall.c"

struct stats *stat;

static int test_syscall_setup(void)
{
    log_test(DECLARE, "Testing syscall module setup");

    /* Call the setup function. */
    int rtrn = setup_syscall_module();

    /* Make sure setup_syscall_module() returns zero. */
    assert_stat(rtrn == 0);

    /* Make sure both atomic values are zero. */
    assert_stat(atomic_load(&running_children) == 0);
    assert_stat(atomic_load(&test_counter) == 0);

    /* Make sure number of children is greater than zero. */
    assert_stat(number_of_children > 0);

    /* Both sys_table and children should not be NULL. */
    assert_stat(sys_table != NULL);
    assert_stat(children != NULL);

    uint32_t i = 0;

    for(i = 0; i < number_of_children; i++)
    {
        assert_stat(children[i] != NULL);
    }

    log_test(SUCCESS, "Syscall module test passed");

    return (0);
}

static int test_init_child_context(void)
{
    log_test(DECLARE, "Testing init_child_context");

    struct child_ctx *child = NULL;

    child = init_child_context();

    assert_stat(child != NULL);

    assert_stat(child->current_arg == 0);

    assert_stat(atomic_load(&child->pid) == 0);

    assert_stat(child->arg_value_index != NULL);

    assert_stat(child->arg_size_index != NULL);

    assert_stat(child->err_value != NULL);

    uint32_t i = 0;

    /* Loop and create the various indecies in the child struct. */
    for(i = 0; i < 7; i++)
    {
        assert_stat(child->arg_value_index[i] != NULL);
    }

    log_test(SUCCESS, "init_child_context test passed");

    return (0);
}

static int test_get_table(void)
{
    struct syscall_table *table = NULL;

    table = get_table();

    assert_stat(table != NULL);

    uint32_t i = 0;

    for(i = 0; i < table->number_of_syscalls; i++)
    {
        assert_stat(&table->sys_entry[i] != NULL);
    }

    return (0);
}

static int test_get_syscall_table(void)
{
    log_test(DECLARE, "Testing get_syscall_table");

    struct syscall_table_shadow *table = NULL;

    table = get_syscall_table();

    assert_stat(table != NULL);

    int32_t rtrn = 0;
    uint32_t i, ii;

    for(i = 0; i < table->number_of_syscalls; i++)
    {
        assert_stat(&table->sys_entry[i] != NULL);

        assert_stat(table->sys_entry[i].name_of_syscall != NULL);

        assert_stat(table->sys_entry[i].syscall_symbol > 0);

        assert_stat(table->sys_entry[i].number_of_args > 0 && table->sys_entry[i].number_of_args <= 7);

        assert_stat(table->sys_entry[i].status == ON || table->sys_entry[i].status == OFF);

        assert_stat(table->sys_entry[i].requires_root == YES || table->sys_entry[i].requires_root == NO);

        assert_stat(table->sys_entry[i].need_alarm == NO || table->sys_entry[i].need_alarm == YES);

        for(ii = 0; ii < table->sys_entry[i].number_of_args; ii++)
        {
            assert_stat(table->sys_entry[i].arg_context_index[ii] != NULL);

            assert_stat(table->sys_entry[i].arg_context_index[ii]->name != NULL);

            assert_stat(table->sys_entry[i].get_arg_index[ii] != NULL);

            uint64_t *arg = NULL;

            rtrn = table->sys_entry[i].get_arg_index[ii](&arg);

            assert_stat(rtrn == 0);

            assert_stat(arg != NULL);
        }
    }

    log_test(SUCCESS, "get_syscall_table test passed");

    return (0);
}

static int test_get_entry(void)
{
    log_test(DECLARE, "Testing get_entry");

    struct syscall_entry_shadow *entry = NULL;

    uint32_t i = 0;
    uint32_t ii = 0;
    int32_t rtrn = 0;

    /* Loop for every syscall entry. */
    for(i = 0; i < sys_table->number_of_syscalls; i++)
    {
        /* Grab the syscall entry. */
        entry = get_entry(i);

        assert_stat(entry != NULL);

        assert_stat(entry->name_of_syscall != NULL);

        assert_stat(entry->syscall_symbol > 0);

        assert_stat(entry->number_of_args > 0 && entry->number_of_args <= 7);

        assert_stat(entry->status == ON || entry->status == OFF);

        assert_stat(entry->requires_root == YES || entry->requires_root == NO);

        assert_stat(entry->need_alarm == NO || entry->need_alarm == YES);

        for(ii = 0; ii < entry->number_of_args; ii++)
        {
            assert_stat(entry->arg_context_index[ii] != NULL);

            assert_stat(entry->arg_context_index[ii]->name != NULL);

            assert_stat(entry->get_arg_index[ii] != NULL);

            uint64_t *arg = NULL;

            rtrn = entry->get_arg_index[ii](&arg);

            assert_stat(rtrn == 0);

            assert_stat(arg != NULL);
        }
    }

    log_test(SUCCESS, "Get_entry test passed");

    return (0);
}

static int test_child_from_index(void)
{
    log_test(DECLARE, "Testing get_child_from_index");

    uint32_t i = 0;

    struct child_ctx *child = NULL;

    /* If we pass a number larger than the number of children
     get_child_from_index() should return NULL. */
    child = get_child_from_index(number_of_children + 1);

    assert_stat(child == NULL);

    for(i = 0; i < number_of_children; i++)
    {
        child = get_child_from_index(i);

        assert_stat(child != NULL);

        mem_free(child);
    }

    log_test(SUCCESS, "get_child_from_index test passed");

    return (0);
}

static int test_pick_syscall(void)
{
    log_test(DECLARE, "Testing pick syscall");

    int32_t rtrn = 0;

    struct child_ctx *child = NULL;

    child = init_child_context();
    assert_stat(child != NULL);

    rtrn = pick_syscall(child);
    assert_stat(rtrn == 0);

    assert_stat(child->number_of_args > 0 && child->number_of_args <= 7);

    assert_stat(child->need_alarm == YES || child->need_alarm == NO);

    assert_stat(child->syscall_symbol > 0);

    assert_stat(child->had_error == NO);

    log_test(SUCCESS, "Pick syscall test passed");

    return (0);
}

int main(void)
{
	/* Map the stats object as shared anonymous memory. */
    stat = init_stats_obj();
    if(stat == NULL)
    {
    	output(ERROR, "Can't init the stats object\n");
    	return (-1);
    }

    int32_t rtrn = 0;

    rtrn = test_syscall_setup();
    if(rtrn < 0)
        return (-1);

    rtrn = test_init_child_context();
    if(rtrn < 0)
        return (-1);

    rtrn = test_get_table();
    if(rtrn < 0)
        return (-1);

    rtrn = test_get_syscall_table();
    if(rtrn < 0)
        return (-1);

    rtrn = test_get_entry();
    if(rtrn < 0)
        return (-1);

    rtrn = test_child_from_index();
    if(rtrn < 0)
        return (-1);

    rtrn = test_pick_syscall();
    if(rtrn < 0)
        return (-1);

    /* Output results. */
	output(STD, "[%d] %ld assertions passed, %ld test passed, and %ld test failed.\n", \
		  (100 * stat->successes) / stat->test_ran, stat->asserts_ran, stat->successes, stat->fails);

	return (0);
}
