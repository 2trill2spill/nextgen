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

#include "concurrent.h"
#include "io/io.h"
#include "memory/memory.h"
#include "runtime/platform.h"

#include <ck_pr.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void cas_loop_int32(int32_t *target, int32_t value)
{
    /* Loop until we can succesfully update the the value. */
    while(1)
    {
        /* Grab a snapshot of the value that needs to be updated. */
        int32_t snapshot = ck_pr_load_int(target);

        /* Enforce ordering, ie make sure the load happens
          before the compare and swap below. */
       ck_pr_fence_memory();

        /* Try swaping the variable if successful break from the loop. */
        if(ck_pr_cas_int(target, snapshot, value) == true)
            break;
    }
}

/* CAS loop for swapping atomic uint32 values. */
void cas_loop_uint32(uint32_t *target, uint32_t value)
{
    /* Loop until we can succesfully update the the value. */
    while(1)
    {
        /* Grab a snapshot of the value that needs to be updated. */
        uint32_t snapshot = ck_pr_load_uint(target);

        /* Enforce ordering, ie make sure the load happens
          before the compare and swap below. */
       ck_pr_fence_memory();

        /* Try swaping the variable if successful break from the loop. */
        if(ck_pr_cas_uint(target, snapshot, value) == true)
            break;
    }
}

int32_t wait_on(atomic_int_fast32_t *pid, int32_t *status)
{
    waitpid(atomic_load(pid), status, 0);

    return (0);
}
