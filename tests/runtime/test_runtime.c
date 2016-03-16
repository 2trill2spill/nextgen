/**
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
 **/

#include "test_utils.h"
#include "runtime/nextgen.c"
#include "runtime/runtime.h"

#include <stdint.h>

static struct shared_map *mapping;

static int32_t test_setup_runtime(void)
{
	log_test(DECLARE, "Testing setup runtime");

	int32_t rtrn = 0;

	rtrn = setup_runtime(mapping);
	assert_stat(rtrn = 0);

	log_test(SUCCESS, "Setup runtime test passed");

	return (0);
}

static int32_t test_init_file_mapping(void)
{
    log_test(DECLARE, "Testing init shared file mapping");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;

    rtrn = init_parser_ctx(&ctx);
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);

    /* Setup the shared mapping for file fuzzing mode with 
    cryptographic numbers and in smart mode. */
    rtrn = init_shared_mapping(&mapping, ctx);
    assert_stat(rtrn == 0);

    log_test(SUCCESS, "Init shared file mapping test passed");

    return (0);
}

static int32_t test_init_syscall_mapping(void)
{
    log_test(DECLARE, "Testing init shared syscall mapping");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;

    rtrn = init_shared_mapping(&mapping, ctx);
    assert_stat(rtrn == 0);

    log_test(SUCCESS, "Init shared syscall mapping test passed");

    return (0);
}

static int32_t test_init_network_mapping(void)
{
    log_test(DECLARE, "Testing init shared network mapping");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;

    rtrn = init_shared_mapping(&mapping, ctx);
    assert_stat(rtrn == 0);

    log_test(SUCCESS, "Init shared network mapping test passed");

    return (0);
}

static int32_t test_cmd_parser(void)
{
    log_test(DECLARE, "Testing command line parser");

    int32_t argc = 1;
    int32_t rtrn = 0;
    char **args = NULL;
    struct parser_ctx *ctx = NULL;
    uint32_t i;

    args = mem_alloc(sizeof(char *) * 6);
    if(args == NULL)
        return (-1);

    for(i = 0; i < 6; i++)
    {
        args[i] = mem_alloc(1025);
        if(args[i] == NULL)
            return (-1);
    }

    /* Make sure a null ctx struct is returned because we didn't
    specify a fuzzing mode. */
    ctx = parse_cmd_line(argc, args);
    assert_stat(ctx == NULL);

    /* If argc is zero return a NULL parser context struct. */
    argc = 0;
    args[1] = "--syscall"; /* First arg */

    ctx = parse_cmd_line(argc, args);
    assert_stat(ctx == NULL);

    char *home = NULL;
    char *arg_string = NULL;

    rtrn = get_home(&home);
    if(rtrn < 0)
        return -1;

    rtrn = asprintf(&arg_string, "%s/syscall_results", home);
    if(rtrn < 0)
        return -1;

    /* Make sure syscall mode works. */
    argc = 3;
    args[0] = "nextgen"; /* Program name */
    args[1] = "--syscall"; /* First arg */
    args[2] = "--out"; /* Second arg */
    args[3] = arg_string;

    ctx = parse_cmd_line(argc, args);
    assert_stat(ctx != NULL);
    assert_stat(ctx->mode == MODE_SYSCALL);
    assert_stat(ctx->output_path != NULL);
    assert_stat(strncmp(ctx->output_path, arg_string, strlen(arg_string)) == 0);

    log_test(SUCCESS, "Passed command line parser test");

    return 0;
}

static int32_t test_init_parser_ctx(void)
{
    log_test(DECLARE, "Testing init parser context");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;

    rtrn = init_parser_ctx(&ctx);
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);

    log_test(SUCCESS, "Init parser context test passed");

    return (0);
}

static int32_t test_set_fuzz_mode(void)
{
    log_test(DECLARE, "Testing set fuzz mode");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;
    enum fuzz_mode mode = MODE_SYSCALL;

    rtrn = init_parser_ctx(&ctx);
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);

    int32_t bad_mode = 33;

    rtrn = set_fuzz_mode(ctx, (enum fuzz_mode)bad_mode);    
    assert_stat(rtrn == -1);

    rtrn = set_fuzz_mode(ctx, mode);    
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);
    assert_stat(ctx->mode == mode);

    log_test(SUCCESS, "Set fuzz mode test passed");

    return (0);
}

static int32_t test_set_crypto_method(void)
{
    log_test(DECLARE, "Testing set crypto method");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;
    enum crypto_method method = CRYPTO;

    rtrn = init_parser_ctx(&ctx);
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);

    int32_t bad_method = 33;

    rtrn = set_crypto_method(ctx, (enum crypto_method)bad_method);    
    assert_stat(rtrn == -1);

    rtrn = set_crypto_method(ctx, method);    
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);
    assert_stat(ctx->method == method);

    log_test(SUCCESS, "Set crypto method test passed");

    return (0);
}

static int32_t test_set_output_path(void)
{
    log_test(DECLARE, "Testing set output path");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;

    rtrn = init_parser_ctx(&ctx);
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);

    char *path = "/usr/bin/ls";

    /* Should fail with a file path. */
    rtrn = set_output_path(ctx, path);
    assert_stat(rtrn < 0);

    rtrn = get_home(&path);
    assert_stat(rtrn == 0);

    rtrn = set_output_path(ctx, path);
    assert_stat(rtrn < 0);
    assert_stat(path != NULL);
    assert_stat(ctx != NULL);
    assert_stat(strncmp(ctx->output_path, path, 11) == 0);

    mem_free((void **)&path);

    log_test(SUCCESS, "Set output path test passed");

    return (0);
}

static int32_t test_set_input_path(void)
{
    log_test(DECLARE, "Testing set input path");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;

    rtrn = init_parser_ctx(&ctx);
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);

    char *path = "/usr/bin/ls";

    /* Should fail with a file path instead of a directory being passed. */
    rtrn = set_input_path(ctx, path);
    assert_stat(rtrn < 0);

    rtrn = get_home(&path);
    assert_stat(rtrn == 0);

    /* Should work with directory path. */
    rtrn = set_input_path(ctx, path);
    assert_stat(rtrn == 0);
    assert_stat(path != NULL);
    assert_stat(ctx != NULL);
    assert_stat(ctx->input_path != NULL);
    assert_stat(strncmp(ctx->input_path, path, 11) == 0);

    mem_free((void **)&path);

    log_test(SUCCESS, "Set input path test passed");

    return (0);
}


static int32_t test_set_exec_path(void)
{
    log_test(DECLARE, "Testing set exec path");

    int32_t rtrn = 0;
    struct parser_ctx *ctx = NULL;

    char *path = "/bin/ls";

    /* Should fail with a NULL parser context. */
    rtrn = set_exec_path(ctx, path);
    assert_stat(rtrn < 0);
    assert_stat(ctx == NULL);

    /* Allocate the parser context/ */
    rtrn = init_parser_ctx(&ctx);
    assert_stat(rtrn == 0);
    assert_stat(ctx != NULL);

    /* Grab our home directory. */
    rtrn = get_home(&path);
    assert_stat(rtrn == 0);

    /* Should fail with a directory path. */
    rtrn = set_exec_path(ctx, path);
    assert_stat(rtrn == -1);

    /* Free the home path pointer. */
    mem_free((void **)&path);

    path = "/bin/ls";
    
    /* Should work with valid executable. */
    rtrn = set_exec_path(ctx, path);
    assert_stat(rtrn < 0);
    assert_stat(path != NULL);
    assert_stat(ctx != NULL);
    assert_stat(ctx->exec_path != NULL);
    assert_stat(strncmp(ctx->exec_path, path, 7) == 0);

    log_test(SUCCESS, "Set exec path test passed");

    return (0);
}

int main(void)
{
    test_stat = init_test_framework();
    if(test_stat == NULL)
    {
        output(ERROR, "Can't init test framework");
        return (-1);
    }

    int32_t rtrn = 0;

    rtrn = test_cmd_parser();
    if(rtrn < 0)
        log_test(FAIL, "Command line parser test failed");

    rtrn = test_init_parser_ctx();
    if(rtrn < 0)
        log_test(FAIL, "Init parser context test failed");

    rtrn = test_set_fuzz_mode();
    if(rtrn < 0)
        log_test(FAIL, "Set fuzz mode test failed");

    rtrn = test_set_crypto_method();
    if(rtrn < 0)
        log_test(FAIL, "Set crypto method test failed");

    rtrn = test_set_input_path();
    if(rtrn < 0)
        log_test(FAIL, "Set input path test failed");

    rtrn = test_set_exec_path();
    if(rtrn < 0)
        log_test(FAIL, "Set exec path test failed");

    rtrn = test_init_file_mapping();
    if(rtrn < 0)
        log_test(FAIL, "Init shared mapping test failed");

    rtrn = test_init_syscall_mapping();
    if(rtrn < 0)
        log_test(FAIL, "Init shared mapping test failed");

    rtrn = test_init_network_mapping();
    if(rtrn < 0)
        log_test(FAIL, "Init shared mapping test failed");

    rtrn = test_set_output_path();
    if(rtrn < 0)
        log_test(FAIL, "Set output path test failed");

    rtrn = test_setup_runtime();
    if(rtrn < 0)
    	log_test(FAIL, "Setup runtime test failed");

	return (0);
}
