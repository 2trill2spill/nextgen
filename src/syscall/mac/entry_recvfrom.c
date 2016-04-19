

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

#include "syscall_list.h"

struct syscall_entry entry_recvfrom = {

    .syscall_name = "recvfrom",
    .syscall_symbol = SYS_sendmsg,
    .total_args = 6,
    .status = ON,
    .requires_root = NX_NO,
    .need_alarm = NX_YES,

    .arg_type_array[FIRST_ARG] = SOCKET,
    .get_arg_array[FIRST_ARG] = &generate_socket,

    .arg_type_array[SECOND_ARG] = VOID_BUF,
    .get_arg_array[SECOND_ARG] = &generate_message,

    .arg_type_array[THIRD_ARG] = SIZE,
    .get_arg_array[THIRD_ARG] = &generate_length,

    .arg_type_array[FOURTH_ARG] = RECV_FLAG,
    .get_arg_array[FOURTH_ARG] = &generate_recv_flags,

    .arg_type_array[FIFTH_ARG] = SOCKADDR,
    .get_arg_array[FIFTH_ARG] = &generate_sockaddr,

    .arg_type_array[SIXTH_ARG] = SOCKLEN,
    .get_arg_array[SIXTH_ARG] = &generate_socklen,
};
