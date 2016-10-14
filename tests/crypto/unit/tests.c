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

#include "unity.h"
#include "crypto/crypto.h"
#include "memory/memory.h"

static uint32_t iterations = 1000;

static void test_get_random_generator(void)
{
    struct random_generator *random = NULL;
		struct memory_allocator *allocator = NULL;
		struct output_writter *output = NULL;

		output = get_console_writter();
	  TEST_ASSERT_NOT_NULL(output);

		allocator = get_default_allocator();
	  TEST_ASSERT_NOT_NULL(allocator);

		random = get_default_random_generator(allocator, output);
		TEST_ASSERT_NOT_NULL(random);
	  TEST_ASSERT_NOT_NULL(random->range);

		int32_t rtrn = 0;
		uint32_t number = 0;
		uint32_t i = 0;

		for(i = 0; i < iterations; i++)
		{
				rtrn = random->range(1000, &number);
				// TEST_ASSERT(rtrn == 0);
				// TEST_ASSERT(number <= 1000);
		}
}

static void test_sha256(void)
{
	  uint32_t i = 0;
    int32_t rtrn = 0;
    char *buf = NULL;
    char *hash = NULL;

    /* Loop and test sha256 several times. */
	for(i = 0; i < iterations; i++)
	{
        /* Allocate a buffer to place random bytes in. */
        buf = mem_alloc(1001);
        TEST_ASSERT_NOT_NULL(buf);

		/* Create a random buffer with rand_bytes. */
        rtrn = rand_bytes(&buf, 1000);
        TEST_ASSERT(rtrn == 0);
        TEST_ASSERT_NOT_NULL(buf);

        /* sha256 hash the random buffer. */
        rtrn = sha256(buf, &hash);
        TEST_ASSERT(rtrn == 0);
        TEST_ASSERT_NOT_NULL(hash);

        /* The hash should be 64 bytes long. */
        TEST_ASSERT(strlen(hash) == 64);

        /* Clean up the buffers. */
        mem_free((void **)&hash);
        mem_free((void **)&buf);
	}

	return;
}

static void test_sha512(void)
{
	uint32_t i = 0;
    int32_t rtrn = 0;
    char *buf = NULL;
    char *hash = NULL;

    /* Loop and test sha512 several times. */
	for(i = 0; i < iterations; i++)
	{
       /* Allocate a buffer to place random bytes in. */
        buf = mem_alloc(1001);
        TEST_ASSERT_NOT_NULL(buf);

		/* Create a random buffer with rand_bytes. */
        rtrn = rand_bytes(&buf, 1000);
        TEST_ASSERT(rtrn == 0);
        TEST_ASSERT_NOT_NULL(buf);

        /* sha256 hash the random buffer. */
        rtrn = sha512(buf, &hash);
        TEST_ASSERT(rtrn == 0);
        TEST_ASSERT_NOT_NULL(hash);

        /* The hash should be 128 bytes long. */
        TEST_ASSERT(strlen(hash) == 128);

        /* Clean up the buffers. */
        mem_free((void **)&hash);
        mem_free((void **)&buf);
	}

	return;
}

static void test_setup_crypto(void)
{
    int32_t rtrn = 0;

    rtrn = setup_crypto_module(CRYPTO);
    TEST_ASSERT(rtrn == 0);

	return;
}

static void test_rand_range(void)
{
    int32_t rtrn = 0;
    uint32_t number = 0;
    uint32_t i = 0;

    for(i = 0; i < iterations; i++)
    {
        rtrn = rand_range(1000, &number);
        TEST_ASSERT(rtrn == 0);
        TEST_ASSERT(number <= 1000);
    }

	return;
}

static void test_rand_bytes(void)
{
    int32_t rtrn = 0;
    char *buf = NULL;
    uint32_t i = 0;

    for(i = 0; i < iterations; i++)
    {
        buf = mem_alloc(1001);
        TEST_ASSERT_NOT_NULL(buf);

        rtrn = rand_bytes(&buf, 1000);
        TEST_ASSERT(rtrn == 0);
        TEST_ASSERT(strlen(buf) <= 1000);

        mem_free((void **)&buf);
    }

	return;
}

int main(void)
{
	test_setup_crypto();
	test_sha256();
	test_sha512();
	test_rand_bytes();
	test_rand_range();
	test_get_random_generator();

	return (0);
}
