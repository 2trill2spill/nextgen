/*
 * Copyright (c) 2017, Harrison Bowden, Minneapolis, MN
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

#include "unity.h"
#include "io/io.h"
#include "crypto/crypto.h"
#include "crypto/random.h"
#include "mutate/mutate.h"
#include "memory/memory.h"
#include "syscall/syscall.h"

#include <string.h>
#include <stdlib.h>

static void test_mutate_buffer(void)
{
	int32_t rtrn = 0;
	char *str = "hfd94hf497grh49fh";
	char *buffer = malloc(strlen(str));
	TEST_ASSERT_NOT_NULL(buffer);

	memcpy(buffer, str, strlen(str));

	TEST_ASSERT(strncmp(buffer, str, strlen(str)) == 0);

	rtrn = mutate_buffer((void **)&buffer, strlen(str));
	TEST_ASSERT(rtrn == 0);
	TEST_ASSERT(strncmp(buffer, str, strlen(str)) != 0);

    return;
}

static void setup_tests(void)
{
	struct dependency_context *ctx = NULL;
    struct output_writter *output = NULL;

    output = get_console_writter();
    TEST_ASSERT_NOT_NULL(output);

    struct memory_allocator *allocator = NULL;

    allocator = get_default_allocator();
    TEST_ASSERT_NOT_NULL(allocator);

    ctx = create_dependency_ctx(create_dependency(output, OUTPUT),
                                create_dependency(allocator, ALLOCATOR),
                                NULL);
    inject_crypto_deps(ctx);

    struct random_generator *random_gen = NULL;

    random_gen = get_default_random_generator();
    TEST_ASSERT_NOT_NULL(random_gen);

    add_dep(ctx, create_dependency(random_gen, RANDOM_GEN));

    inject_mutate_deps(ctx);
}

int main(void)
{
	setup_tests();
	test_mutate_buffer();

	return (0);
}