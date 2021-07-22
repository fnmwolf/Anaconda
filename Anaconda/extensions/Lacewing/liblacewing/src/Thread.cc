
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Common.h"

struct ThreadInternal
{
    void * Function, * Parameter;
    String Name;

    #ifdef LacewingWindows
        HANDLE Thread;
    #else
        pthread_t Thread;
        bool Started;
    #endif

    ThreadInternal (const char * _Name, void * _Function)
        : Function (_Function), Name (_Name)
    {
        #ifdef LacewingWindows
            Thread = INVALID_HANDLE_VALUE;
        #else
            Started = false;
        #endif
    }
};

Lacewing::Thread::Thread (const char * Name, void * Function)
{
    InternalTag = new ThreadInternal (Name, Function);
}

Lacewing::Thread::~Thread ()
{
    Join ();

    delete ((ThreadInternal *) InternalTag);
}

int ThreadWrapper (ThreadInternal &Internal)
{
    #ifdef _MSC_VER
        
        struct
        {
              DWORD dwType;
              LPCSTR szName;
              DWORD dwThreadID;
              DWORD dwFlags;

        } ThreadNameInfo;

        ThreadNameInfo.dwFlags     = 0;
        ThreadNameInfo.dwType      = 0x1000;
        ThreadNameInfo.szName      = Internal.Name;
        ThreadNameInfo.dwThreadID  = -1;

        __try
        {   RaiseException(0x406D1388, 0, sizeof(ThreadNameInfo) / sizeof(ULONG), (ULONG *) &ThreadNameInfo);
        }
        __except (EXCEPTION_CONTINUE_EXECUTION)
        {
        }

    #else
        #if HAVE_DECL_PR_SET_NAME != 0
            prctl(PR_SET_NAME, (unsigned long) (const char *) Internal.Name, 0, 0, 0);
        #endif
    #endif
    
    int ExitCode = ((int (*) (void *)) Internal.Function) (Internal.Parameter);

    #ifndef LacewingWindows
        Internal.Started = false;
    #endif

    return ExitCode;
}

void Lacewing::Thread::Start (void * Parameter)
{
    ThreadInternal &Internal = *(ThreadInternal *) InternalTag;

    if (Started ())
        return;

    Internal.Parameter = Parameter;
    
    #ifdef LacewingWindows
        Internal.Thread = (HANDLE) _beginthreadex(0, 0,
                (unsigned (__stdcall *) (void *)) ThreadWrapper, &Internal, 0, 0);
    #else
        Internal.Started = pthread_create
            (&Internal.Thread, 0, (void * (*) (void *)) ThreadWrapper, &Internal);
    #endif
}

bool Lacewing::Thread::Started ()
{
    #ifdef LacewingWindows
        return ((ThreadInternal *) InternalTag)->Thread != INVALID_HANDLE_VALUE;
    #else
        return ((ThreadInternal *) InternalTag)->Started;
    #endif
}

int Lacewing::Thread::Join ()
{
    ThreadInternal &Internal = *(ThreadInternal *) InternalTag;

    if (!Started ())
        return -1;

    #ifdef LacewingWindows

        DWORD ExitCode = -1;

        if (WaitForSingleObject (Internal.Thread, INFINITE) == WAIT_OBJECT_0)
            GetExitCodeThread (Internal.Thread, &ExitCode);

    #else
        
        void * ExitCode;

        if (pthread_join (Internal.Thread, &ExitCode))
            return -1;

    #endif
        
    return (int) (lw_iptr) ExitCode;
}

