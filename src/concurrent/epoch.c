/*
 * Copyright (c) 2016, Harrison Bowden, Minneapolis, MN
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
 */

#include "epoch.h"
#include "io/io.h"
#include "utils/utils.h"
#include "memory/memory.h"

struct thread_ctx
{
	  epoch_record *record;

	  epoch_section **section;

	  uint32_t section_count;

    uint32_t buf_size;
};

struct thread_ctx *init_thread(epoch_ctx *epoch)
{
	struct thread_ctx *thread = NULL;

    /* Allocate the thread context. */
	thread = mem_alloc(sizeof(struct thread_ctx));
	if(thread == NULL)
	{
		output(ERROR, "Thread context allocation failed\n");
		return (NULL);
	}

	thread->record = mem_alloc(sizeof(epoch_record));
	if(thread->record == NULL)
	{
		output(ERROR, "Epoch record allocation failed\n");
		mem_free((void **)&thread);
		return (NULL);
	}

	thread->buf_size = 8;
	thread->section_count = 0;

	thread->section = mem_alloc(sizeof(epoch_section *) * thread->buf_size);
	if(thread->section == NULL)
	{
		output(ERROR, "Epoch section array allocation failed\n");
		mem_free((void **)&thread);
		mem_free((void **)&thread->record);
		return (NULL);
	}

    /* Initialize the epoch record. */
	epoch_register(epoch, thread->record);

	return (thread);
}

void clean_thread(struct thread_ctx **thread)
{
	epoch_unregister((*thread)->record);

	mem_free((void **)&(*thread)->section);
	mem_free((void **)&(*thread)->record);
	mem_free((void **)thread);

	return;
}

epoch_record *get_record(struct thread_ctx *thread)
{
	return (thread->record);
}

int32_t epoch_start(struct thread_ctx *thread)
{
	epoch_section section;
	uint32_t count = thread->section_count;

	/* Make sure the count is less than the buffer length. */
	if(count > thread->buf_size)
	{
		thread->section = reallocarray(thread->section,
			                           thread->buf_size * 2,
			                           sizeof(epoch_section *));
		if(thread->section == NULL)
		{
			output(ERROR, "Can't reallocate epoch section array\n");
			return (-1);
		}

		thread->buf_size = thread->buf_size * 2;
	}

	thread->section[count] = mem_alloc(sizeof(epoch_section));
	if(thread->section[count] == NULL)
	{
		output(ERROR, "Epoch section allocation failed\n");
		return (-1);
	}

	/* Start the epoch protected section. */
	epoch_begin(thread->record, &section);

	/* Move the epoch section from the stack to the heap. */
	memmove(thread->section[count], &section, sizeof(epoch_section));

	thread->section_count++;

	return (0);
}

void epoch_stop(struct thread_ctx *thread)
{
	thread->section_count--;
	uint32_t count = thread->section_count;
	epoch_end(thread->record, thread->section[count]);
	mem_free((void **)&thread->section[count]);

	return;
}

void stop_all_sections(struct thread_ctx *thread)
{
	uint32_t i;

	for(i = 0; i < thread->section_count; i++)
	{
		epoch_end(thread->record, thread->section[i]);
		mem_free((void **)&thread->section[i]);
	}

	thread->section_count = 0;
}
