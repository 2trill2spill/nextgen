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

#ifndef FILE_H
#define FILE_H

#include <stdint.h>

/* File type symbols. */
enum file_type { BINARY, TEXT };

/* Call this function once to start using the functions below. */
extern int32_t setup_file_module(int32_t *stop, char *exec_path, char *input);

extern void start_main_file_loop(void);

extern void start_file_smart_loop(void);

extern int32_t get_exec_path(char **exec_path);

extern int32_t set_end_offset(uint64_t offset);

extern void set_start_addr(uint64_t addr);

extern uint64_t get_start_addr(void);

extern int32_t initial_fuzz_run(void);

//extern int32_t detect_file_type(const char *path, enum file_type *type);

#endif
