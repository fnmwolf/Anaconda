
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

struct ErrorInternal
{
    char Error [4096];
    char * Begin;

    ErrorInternal()
    {
        *(Begin = Error + sizeof(Error) - 1) = 0;
    }

    inline void Add(const char * Text)
    {
        int Length = strlen(Text);
        memcpy(Begin -= Length, Text, Length);
    }
};

Lacewing::Error::Error()
{
    InternalTag = new ErrorInternal;
}

Lacewing::Error::~Error()
{
    if(!InternalTag)
        return;

    delete ((ErrorInternal *) InternalTag);
}

Lacewing::Error::operator const char *()
{
    return ToString();
}

const char * Lacewing::Error::ToString()
{
    return ((ErrorInternal *) InternalTag)->Begin;
}

Lacewing::Error * Lacewing::Error::Clone()
{
    ErrorInternal &Internal = *((ErrorInternal *) InternalTag);

    Lacewing::Error * New = new Lacewing::Error;
    New->Add ("%s", ToString());

    return New;
}

void Lacewing::Error::Add(const char * Format, ...)
{
    va_list Arguments;
    va_start (Arguments, Format);

    Add(Format, Arguments);    

    va_end (Arguments);
}

void Lacewing::Error::Add(const char * Format, va_list Arguments)
{
    ErrorInternal &Internal = *((ErrorInternal *) InternalTag);

    char Buffer[2048];
    vsnprintf(Buffer, sizeof(Buffer), Format, Arguments);

    if(*Internal.Begin)
        Internal.Add(" - ");

    Internal.Add(Buffer);
}

void Lacewing::Error::Add(int Error)
{
    #ifdef LacewingWindows

        char * Message;

        if(FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            0, Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *) &Message, 1, 0))
        {
            Add(Error < 0 ? "%s (%08X)" : "%s (%d)", Message, Error);
        }

        LocalFree(Message);

    #else

        Add(strerror(Error));
        
    #endif
}


