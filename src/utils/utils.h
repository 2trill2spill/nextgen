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

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>

#include "stdatomic.h"

enum yes_no { NX_YES, NX_NO };

enum csp { SET, UNSET };

enum name_type { DIR_NAME, FILE_NAME };

#define NX_NO_RETURN __attribute__((noreturn))

/* This function as the name implies gets the file size. */
extern int32_t get_file_size(int32_t fd, uint64_t *size);

/* Get file extension */
extern int32_t get_extension(char *path, char **extension);

/* Get the core count of the system we are on. This will include virtual cores on hyperthreaded systems. */
extern int32_t get_core_count(uint32_t *core_count);

/* Can be used to create random file and directory names. */
extern int32_t generate_name(char **name, char *extension, enum name_type type);

/* Grabs the path to the user's home directory. */
extern int32_t get_home(char **home);

/* Convert the ascii string to a binary string upto len bytes length. */
extern int32_t ascii_to_binary(char *input, char **out, uint64_t input_len, uint64_t *out_len);

/* Convert binary string to a ascii string. */
extern int32_t binary_to_ascii(char *input, char **out, uint64_t input_len, uint64_t *out_len);

/* This function will return zero if the process calling it has root privileges. */
extern int32_t check_root(void);

/* Create a random file at the path root with the extension, ext, the path created
 will be put in the buffer path. */
extern int32_t create_random_file(char *root, char *ext, char **path, uint64_t *size);

/* Drop privileges.  */
extern int32_t drop_privileges(void);

/* Delete a directory and all it's contents. */
extern int32_t delete_directory(char *path);

/* Delete the contents of the directory but not the directory it's self. */
extern int32_t delete_dir_contents(char *dir);

/* Count files directory. */
extern int32_t count_files_directory(uint32_t *count, char *dir);

extern void SetProcessName(char *name);

/* End of header. */
#endif
