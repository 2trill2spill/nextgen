/*
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

/**
*     @file syscall.h
*     @brief This file is the interface to the syscall module.
*
*     @author Harrison Bowden
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "syscall_table.h"
#include "depend-inject/depend-inject.h"

struct test_case;

/**
 *
 */
extern struct syscall_table *get_table(void);

extern struct test_case *create_test_case(void);

/**
 * Return the total amount of arguments that a test case has.
 * @param test A test_case struct.
 * @return A uint32_t that is the total amount of arguments.
 */
 extern inline uint32_t get_total_args(struct test_case *test);

/**
 * Get the array of syscall arguments from the a test case.
 * @param test A pointer to a test case struct/object.
 * @return An array of syscall arguments.
 */
extern inline uint64_t **get_argument_array(struct test_case *test);

/**
 * Picks and returns a syscall entry at random.
 * @param table A system call table to pick a syscall entry from.
 * @return Returns a syscall entry on success and NULL on failure.
 */
extern struct syscall_entry *pick_syscall(struct syscall_table *table);

/**
 * Returns the syscall entry for the syscall selected in a test case.
 * @param test A test case object from which you want the selected syscall entry.
 * @return A syscall entry on success and NULL on failure.
 */
extern inline struct syscall_entry *get_entry(struct test_case *test);

/**
 * Generates system call arguments for a test case. generate_args()
 * trys to generate correct system call arguments. However this correctess
 * is on a best effort basis. This is fine for fuzzing however.
 * @param test A test case struct to generate arguments for.
 * @return Zero on success and -1 on failure.
 */
 extern int32_t generate_args(struct test_case *test);

extern void inject_syscall_deps(struct dependency_context *ctx);

#endif
