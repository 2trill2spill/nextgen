

/**
 * Copyright (c) 2015, Harrison Bowden, Secure Labs, Minneapolis, MN
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

#include "utils.h"
#include "memory.h"
#include "crypto.h"
#include "nextgen.h"
#include "io.h"

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syslimits.h>
#include <errno.h>
#include <pwd.h>

int32_t check_root(void)
{
    output(STD, "Making sure nextgen has root privileges\n");

    uid_t check = 0;

    check = getuid();
    if(check == 0)
        return (0);

    return (-1);
}

int32_t get_file_size(int32_t fd, uint64_t *size)
{
    struct stat stbuf;

    if((fstat(fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode)))
    {
        output(ERROR, "fstat: %s\n", strerror(errno));
        return (-1);
    }

    (*size) = stbuf.st_size;

    return (0);
}

int32_t get_extension(char *path, char **extension)
{
    char *pointer = strrchr(path, '.');
    if(pointer == NULL)
    {
        output(ERROR, "Can't find . :\n", strerror(errno));
        return -1;
    }

    int32_t rtrn = asprintf(extension, "%s\n", pointer + 1);
    if(rtrn < 0)
    {
        output(ERROR, "Can't alloc extension: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int32_t get_core_count(uint32_t *core_count)
{
    int32_t mib[4];

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;

    size_t len = sizeof(uint32_t);

    int32_t rtrn = sysctl(mib, 2, core_count, &len, NULL, 0);
    if(rtrn < 0)
    {
        output(ERROR, "sysctl: %s\n", strerror(errno));
        return -1;
    }

    if(*core_count < 1)
    	*core_count = (uint32_t) 1;

	return 0;
}

int32_t generate_name(char **name, char *extension, enum name_type type)
{
    /* Declare some variables. */
    int32_t rtrn = 0;
    char *random_data auto_clean = NULL;
    char *tmp_buf auto_clean = NULL;
    
    /* Create some space in memory. */
    random_data = mem_alloc(PATH_MAX + 1);
    if(random_data == NULL)
    {
        output(STD, "Can't Allocate Space\n");
        return -1;
    }
    
    /* Grab some random bytes. */
    rtrn = rand_bytes(&random_data, (PATH_MAX));
    if(rtrn < 0)
    {
        output(STD, "Can't get random bytes\n");
        return -1;
    }

    /* Null terminate the random byte string. */
    random_data[PATH_MAX] = '\0';
    
    /* SHA 256 hash the random output string. */
    rtrn = sha256(random_data, &tmp_buf);
    if(rtrn < 0)
    {
        output(STD, "Can't Hash Random Data\n");
        return -1;
    }
    
    switch((int)type)
    {
        case DIR_NAME:
            
            rtrn = asprintf(name, "%s", tmp_buf);
            if(rtrn < 0)
            {
                output(STD, "Can't create dir path: %s\n", strerror(errno));
                return -1;
            }
            
            break;
            
        case FILE_NAME:
            
            rtrn = asprintf(name, "%s%s", tmp_buf, extension);
            if(rtrn < 0)
            {
                output(STD, "Can't create file path: %s\n", strerror(errno));
                return -1;
            }
            
            break;
            
        default:
            output(STD, "Should not get here\n");
            return -1;
    }

    return 0;
}

int get_home(char **home)
{
    uid_t uid;
    struct passwd *pwd;
    int rtrn;
    
    /* Get User UID */
    uid = getuid();
    
    /* If getuid succedes get user password struct */
    if(!(pwd = getpwuid(uid)))
    {
        output(ERROR, "Can't Get pwd struct: %s\n", strerror(errno));
        return -1;
    }
    
    /* Copy User Home Directory to buffer supplied to function */
    rtrn = asprintf(home, "%s", pwd->pw_dir);
    if(rtrn < 0)
    {
        output(ERROR, "Can't Create home string: %s\n", strerror(errno));
        return -1;  
    }
    
    /* Exit */
    return 0;
}
