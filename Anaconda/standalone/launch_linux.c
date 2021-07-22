// Copyright (c) Mathias Kaerlev 2012.

// This file is part of Anaconda.

// Anaconda is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Anaconda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <libgen.h>
#include <string.h>

char* getexename(char* buf, size_t size)
{
    char linkname[64]; /* /proc/<pid>/exe */
    pid_t pid;
    int ret;
    
    /* Get our PID and build the name of the link in /proc */
    pid = getpid();
    
    if (snprintf(linkname, sizeof(linkname), "/proc/%i/exe", pid) < 0)
    {
        /* This should only happen on large word systems. I'm not sure
           what the proper response is here.
           Since it really is an assert-like condition, aborting the
           program seems to be in order. */
        abort();
    }

    
    /* Now read the symbolic link */
    ret = readlink(linkname, buf, size);
    
    /* In case of an error, leave the handling up to the caller */
    if (ret == -1)
        return NULL;
    
    /* Report insufficient buffer size */
    if (ret >= size)
    {
        errno = ERANGE;
        return NULL;
    }
    
    /* Ensure proper NUL termination */
    buf[ret] = 0;
    
    return buf;
}

/*
 * Trivial test and sample use of getexename().
 */
int main()
{
    char* buf;
    int size;
    
    buf = NULL;
    size = 32; /* Set an initial size estimate */

    for(;;)	
    {
        char* res;
        
        /* Allocate and fill the buffer */
        buf = (char*)malloc(size);
        res = getexename(buf, size);
        
        /* Get out of the loop on success */
        if (res)
            break;
        
        /* Anything but ERANGE indicates a real error */
        if (errno != ERANGE)
        {
            perror("getexename() failed");
            free(buf);
            buf = NULL;
            break;
        }
        
        /* ERANGE means the buffer was too small. Free the current
           buffer and retry with a bigger one. */
        free(buf);
        size *= 2;
    }
    
    /* Exit on failure */
    if (buf == NULL)
        return -1;
    
    buf = basename(buf);

    char newname[size];
    strcpy(newname, buf);
    strcat(newname, " data");
    chdir(newname);

    execl("runtime", "runtime", (char *)0);
    
    free(buf);
    return 0; /* Indicate success */
}
