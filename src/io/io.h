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

#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>

#define DEPRECATED __attribute__((deprecated))

/* MACROS for map_file_in(). */
#define READ PROT_READ
#define WRITE PROT_WRITE
#define EXEC PROT_EXEC
#define NONE PROT_NONE

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLD_BLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLD_RED     "\033[1m\033[31m"      /* Bold Red */
#define BOLD_GREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLD_YELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLD_BLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLD_MAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLD_CYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLD_WHITE   "\033[1m\033[37m"      /* Bold White */

static inline void close_fd(int *fd)
{
    if((*fd) == 0)
        return;

    close((*fd));
}

static inline void close_dir(DIR **dir)
{
    if((*dir) == NULL)
        return;

    closedir((*dir));
}

static inline void close_fp(FILE **fp)
{
    if((*fp) == NULL)
        return;

    fclose((*fp));
}

/* The enum used to tell output how to output the message. */
enum out_type { ERROR, STD };

struct output_writter
{
    void (*write)(enum out_type type, const char *format, ...);
};

extern struct output_writter *get_console_writter(void);

#define auto_close_fp __attribute__((cleanup (close_fp)))

#define auto_close __attribute__((cleanup (close_fd)))

#define auto_close_dir __attribute__((cleanup (close_dir)))

/* This function replaces printf and perror in the code so we can aggregate output to one point. */
DEPRECATED void output(enum out_type type, const char *format, ...);

extern void set_verbosity(int32_t val);

extern int32_t get_file_size(int32_t fd, uint64_t *size, struct output_writter *output);

extern int32_t map_file_in(int32_t fd, char **buf, uint64_t *size, int32_t perm, struct output_writter *output);

/* This function maps a buffer of size to a file path.  */
extern int32_t map_file_out(char *path, char *buf, uint64_t size, struct output_writter *output);

/* Copy a file from one file path to another. */
extern int32_t copy_file_to(char *src, char *dst, struct output_writter *output);

#endif
