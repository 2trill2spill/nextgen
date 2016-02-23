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

#include "test_utils.h"
#include "syscall/syscall.h"
#include "genetic/genetic.c"

#include "stdatomic.h"

#include <stdint.h>

static int32_t test_init_world(void)
{
	log_test(DECLARE, "Testing init world");

    int32_t rtrn = 0;

    /* Initialize all the data structures. */
    rtrn = init_world();   

    /* init_world() should return on zero. */ 
    assert_stat(rtrn == 0);

    /* World should not be NULL. */
    assert_stat(world != NULL);
    assert_stat(world->number_of_species > 0);
    assert_stat(world->current_generation == 0);
    assert_stat(world->species != NULL);

    uint32_t i;

    for(i = 0; i < world->number_of_species; i++)
    {
    	assert_stat(world->species[i] != NULL);
    	assert_stat(world->species[i]->species_name != NULL);
    	assert_stat(world->species[i]->species_population == 0);
    	//assert_stat(world->species[i]->average_species_fitness == 0);

    	struct organism_ctx *organism = NULL;

    	CK_SLIST_FOREACH(organism, &world->species[i]->organism_list, list_entry)
    	{
    		assert_stat(organism != NULL);

    		assert_stat(organism->chromosome != NULL);

    		//assert_stat(organism->fitness == 0);
    	}
    }

    log_test(SUCCESS, "init world test passed");

    return (0);
}

int main(void)
{
    int32_t rtrn = 0;

    test_stat = init_test_framework();
    if(test_stat == NULL)
    {
        output(ERROR, "Can't init test framework");
        return (-1);
    }

    atomic_int_fast32_t stop;
    atomic_uint_fast64_t counter;

    atomic_init(&stop, FALSE);
    atomic_init(&counter, 0);

    /* Initialize the syscall module before using the genetic module. */
    rtrn = setup_syscall_module(&stop, &counter, TRUE);
    assert_stat(rtrn == 0);

    rtrn = test_init_world();
    if(rtrn < 0)
    {
        log_test(FAIL, "init world test failed");
        return (-1);
    }

    _exit(0);
}
