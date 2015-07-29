

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

#include "child.h"
#include "utils.h"
#include "nextgen.h"

#include <string.h>
#include <errno.h>

static void start_file_child(void)
{
	return;
}

static void start_syscall_child(void)
{
	return;
}

void create_syscall_children(void)
{
	/* Walk the child structure and find the first empty child slot. */
	unsigned int i;
	unsigned int number_of_children = map->number_of_children;

	for(i = 0; i < number_of_children; i++)
	{
		/* If the child has a pid of EMPTY let's create a new one. */
		if(map->children[i]->pid == EMPTY)
		{
            map->children[i]->pid = fork();
            if(map->runloop_pid == 0)
            {
            	/* Start the child main loop. */
                start_syscall_child();
            }
            else if(map->runloop_pid > 0)
            {
               /* This is the parent process, so let's keep looping. */
                continue;
            
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

void manage_syscall_children(void)
{
	return;
}

void create_file_children(void)
{
	/* Walk the child structure and find the first empty child slot. */
	unsigned int i;
	unsigned int number_of_children = map->number_of_children;

	for(i = 0; i < number_of_children; i++)
	{
		/* If the child has a pid of EMPTY let's create a new one. */
		if(map->children[i]->pid == EMPTY)
		{
            map->children[i]->pid = fork();
            if(map->runloop_pid == 0)
            {
            	/* Start the child main loop. */
                start_file_child();
            }
            else if(map->runloop_pid > 0)
            {
               /* This is the parent process, so let's keep looping. */
                continue;
            
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

void manage_file_children(void)
{
	return;
}
