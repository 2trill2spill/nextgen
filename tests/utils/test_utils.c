/**
 * Copyright (c) 2015, Harrison Bowden, Minneapolis, MN
 * 
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice 
 * and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY å
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **/

#include "test_utils.h"
#include "io/io.h"
#include "utils/utils.c"
#include "crypto/crypto.h"
#include "memory/memory.h"

#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>

static uint32_t number_of_extensions = 11;

static uint32_t size_array[] = { 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 2};

/* Array of extensions. */
static char *ext[] = { "txt", "mp3", "mp4", "m4v", "jpg",
                       "pdf", "dmg", "c", "h", "png", "go" };

/* The number of ascii to binary comparison test. */
static uint32_t binary_test = 6;

/* Array of strings to covert to binary. */
static char *string_array[] = { "a", "b", "c", "d", "e", "f", NULL };

/* Array of binary strings to compare against. */
static char *binary_array[] = { "01100001", "01100010", "01100011", "01100100",
                                "01100101", "01100110", NULL };

static int32_t test_delete_directory(void)
{
    log_test(DECLARE, "Testing delete directory");

    int32_t rtrn = 0;
    char *dir auto_free = NULL;
    char *name auto_free = NULL;

    rtrn = generate_name(&name, ".txt", DIR_NAME);
    if(rtrn < 0)
    {
        output(ERROR, "Can't generate directory name\n");
        return (-1);
    }

    rtrn = asprintf(&dir, "/tmp/%s\n", name);
    if(rtrn < 0)
    {
        output(ERROR, "Can't create directory\n");
        return (-1);
    }

    /* Let's make a directory in /temp for testing delete_directory(). */
    rtrn = mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(rtrn < 0)
    {
        output(ERROR, "Can't create directory\n");
        return (-1);
    }

    uint32_t total_files = 0;

    /* Pick a random amount of files to create. */
    rtrn = rand_range(25, &total_files);
    if(rtrn < 0)
    {
        output(ERROR, "Can't pick random number\n");
        return (-1);
    }

    uint32_t i;

    /* Loop and create a bunch of random text files. */
    for(i = 0; i < total_files; i++)
    {
        uint64_t size = 0;
        char *path auto_free = NULL;
        
        /* Create random file. */
        rtrn = create_random_file(dir, ".txt", &path, &size);
        if(rtrn < 0)
        {
            output(ERROR, "Can't create random file\n");
            return (-1);
        }
    }

    /* Now that were done setting up a test directory let's start 
    testing delete_directory(). Pass a NULL path and make sure delete_directory
    returns an error. */
    char *dir_path = NULL;

    rtrn = delete_directory(dir_path);
    assert_stat(rtrn < 0);

    char *file_path auto_free = NULL;
    uint64_t size = 0;

    /* Create a file path. */
    rtrn = create_random_file("/tmp", ".txt", &file_path, &size);
    assert_stat(rtrn == 0);

    /* If we pass delete_directory() a file path then it should fail. */
    rtrn = delete_directory(file_path);
    assert_stat(rtrn < 0);

    /* Create a directory path that does not exist. */
    char *random_path auto_free = NULL;

    rtrn = generate_name(&random_path, NULL, DIR_NAME);
    if(rtrn < 0)
    {
        output(ERROR, "Can't generate dir name\n");
        return (-1);
    }

    rtrn = delete_directory(random_path);
    assert_stat(rtrn < 0);

    /* Now let's try deleting the directory we made. */
    rtrn = delete_directory(dir);
    assert_stat(rtrn == 0);

    struct stat buf;

    /* Stat should fail on the directory. */
    assert_stat(stat(dir, &buf) < 0);

    log_test(SUCCESS, "Delete directory test passed");

    return (0);
}

static int32_t test_check_root(void)
{
	log_test(DECLARE, "Testing check root");

	int32_t rtrn = 0;
    uid_t uid = getuid();

    rtrn = check_root();

    if(uid != 0)
    {
        assert_stat(rtrn != 0);
    }
    else
    {
        assert_stat(rtrn == 0);
    }

    log_test(SUCCESS, "Check root test passed");

	return (0);
}

static int32_t test_get_file_size(void)
{
    log_test(DECLARE, "Testing get file size");

    uint32_t i;
    int32_t fd = 0;
    int32_t rtrn = 0;
    char *path = NULL;
    uint64_t size = 0;
    uint64_t file_size = 0;

    for(i = 0; i < 100; i++)
    {
        rtrn = create_random_file("/tmp", ".txt", &path, &size);
        assert_stat(rtrn == 0);
        assert_stat(path != NULL);

        fd = open(path, O_RDONLY);
        if(fd < 0)
        {
            output(ERROR, "Can't open file: %s\n", strerror(errno));
            return (-1);
        }

        rtrn = get_file_size(fd, &file_size);
        assert_stat(rtrn == 0);
        assert_stat(size == file_size);

        /* Clean up. */
        unlink(path);
    }

    log_test(SUCCESS, "Get file size test passed");

    return (0);
}

static int32_t test_ascii_to_binary(void)
{
    log_test(DECLARE, "Testing ascii to binary");

    uint32_t i;
    int32_t rtrn = 0;

    char *string = "dn39r93ho*(#98893rnf@N(EHM(EI";
    char *binary_string = NULL;
    uint64_t size = 0;

    /* When we pass a zero size, ascii_to_binary should fail. */
    rtrn = ascii_to_binary(string, &binary_string, 0, &size);
    assert_stat(rtrn < 0);
    assert_stat(binary_string == NULL);
    assert_stat(size == 0);

    /* Should work now that we have a non zero length argument. */
    rtrn = ascii_to_binary(string, &binary_string, strlen(string), &size);
    assert_stat(rtrn == 0);
    assert_stat(binary_string != NULL);
    assert_stat(size > 0);

    /* Clean up. */
    mem_free((void **)&binary_string);

    /* Loop and compare conversions
    to known good values to validate ascii_to_binary(); */
    for(i = 0; i < binary_test; i++)
    {
        /* Convert the string in  */
        rtrn = ascii_to_binary(string_array[i], &binary_string, strlen(string_array[i]), &size);
        assert_stat(rtrn == 0);
        assert_stat(binary_string != NULL);
        assert_stat(size > 0);
        assert_stat(strncmp(binary_array[i], binary_string, 8) == 0);
        mem_free((void **)&binary_string);
    }
    
    log_test(SUCCESS, "ascii to binary test passed");

    return (0);
}

/*
static int32_t test_binary_to_ascii(void)
{
    log_test(DECLARE, "Testing Binary to ascii");

    uint32_t i;
    int32_t rtrn = 0;

    uint64_t size = 0;
    char *binary_string = "01100001011000100110010101100010011001100110100101110101";
    char *ascii_string = NULL;

    rtrn = binary_to_ascii(binary_string, &ascii_string, 0, &size);
    assert_stat(rtrn == -1);
    assert_stat(ascii_string == NULL);
    assert_stat(size == 0);

    for(i = 0; i < binary_test; i++)
    {
        rtrn = ascii_to_binary(binary_array[i], &ascii_string, strlen(ascii_string), &size);
        assert_stat(rtrn == 0);
        assert_stat(ascii_string != NULL);
        assert_stat(binary_array[i] != NULL);
        assert_stat(size > 0);
        mem_free((void **)&ascii_string);
    }
    
    log_test(SUCCESS, "Binary to ascii test passed");

    return (0);
}
*/

static int32_t test_get_core_count(void)
{
    log_test(DECLARE, "Testing get core count");

    int32_t rtrn = 0;
    uint32_t count = 0;

    rtrn = get_core_count(&count);
    assert_stat(rtrn == 0);
    assert_stat(count > 0);

    /* A second call should have the same result. */
    uint32_t second_count = 0;

    rtrn = get_core_count(&second_count);
    assert_stat(rtrn == 0);
    assert_stat(second_count > 0);
    assert_stat(count == second_count);

    log_test(SUCCESS, "Get core count test passed");

    return (0);
}

static int32_t test_get_extension(void)
{
    log_test(DECLARE, "Testing get extension");

    uint32_t i;
    int32_t rtrn = 0;
    char *path = NULL;
    uint64_t size = 0;
    char *extension = NULL;

    for(i = 0; i < number_of_extensions; i++)
    {
        /* Create a random file. */
        rtrn = create_random_file("/tmp", ext[i], &path, &size);
        assert_stat(rtrn == 0);
        assert_stat(path != NULL);

        /* Check the extension of the file against the extension it should be. */
        rtrn = get_extension(path, &extension);
        assert_stat(rtrn == 0);
        assert_stat(extension != NULL);
        assert_stat(strncmp(extension, ext[i], size_array[i]) == 0);

        mem_free((void **)&path);   
    }

    log_test(SUCCESS, "Get extension test passed");

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

    /* The crypto module most be setup first before
    using the utils module. */
    rtrn = setup_crypto_module(CRYPTO);
    if(rtrn < 0)
    {
        output(ERROR, "Can't setup crypto module");
        return (-1);
    }

    rtrn = test_check_root();
    if(rtrn < 0)
    	log_test(FAIL, "Check root test failed");

    rtrn = test_get_file_size();
    if(rtrn < 0)
        log_test(FAIL, "Get file size test failed");

    rtrn = test_get_core_count();
    if(rtrn < 0)
        log_test(FAIL, "Get core count test failed");

    rtrn = test_get_extension();
    if(rtrn < 0)
        log_test(FAIL, "Get extension test failed");

    rtrn = test_ascii_to_binary();
    if(rtrn < 0)
        log_test(FAIL, "Ascii to binary test failed");

    rtrn = test_delete_directory();
    if(rtrn < 0)
        log_test(FAIL, "Delete directory test failed");

    /* This function is not implemented so no need to test it.
    rtrn = test_binary_to_ascii();
    if(rtrn < 0)
        log_test(FAIL, "Binary to ascii test failed"); */

    /* Clean up our mess in temp. */
    rtrn = delete_dir_contents("/tmp");
    if(rtrn < 0)
    {
        output(ERROR, "Can't delete directory contents\n");
        return (-1);
    }

    _exit(0);
}
