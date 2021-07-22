
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

const long WriterPresent = 0x8000;

struct SpinSyncInternal
{
    SpinSyncInternal()
    {
        Readers        = 0;
        WritersWaiting = 0;
    }

    ~SpinSyncInternal()
    {
    }

    volatile long Readers;
    volatile long WritersWaiting;
};

Lacewing::SpinSync::SpinSync()
{
    InternalTag = new SpinSyncInternal;
    Tag         = 0;
}

Lacewing::SpinSync::~SpinSync()
{
    delete ((SpinSyncInternal *) InternalTag);
}

Lacewing::SpinSync::ReadLock::ReadLock(Lacewing::SpinSync &Object)
{
    if(!(InternalTag = Object.InternalTag))
        return;

    SpinSyncInternal &Internal = *((SpinSyncInternal *) InternalTag);

    for(;;)
    {
        if(Internal.WritersWaiting)
        {
            LacewingYield();
            continue;
        }

        long OldReaders = Internal.Readers;

        if(OldReaders == WriterPresent)
        {
            LacewingYield();
            continue;
        }

        if(LacewingSyncCompareExchange((volatile LONG *) &Internal.Readers, OldReaders + 1, OldReaders) == OldReaders)
            break;
    }
}

Lacewing::SpinSync::ReadLock::~ReadLock()
{
    Release();
}

void Lacewing::SpinSync::ReadLock::Release()
{
    if(!InternalTag)
        return;

    LacewingSyncDecrement((volatile LONG *) &((SpinSyncInternal *) InternalTag)->Readers);
    InternalTag = 0;
}

Lacewing::SpinSync::WriteLock::WriteLock(Lacewing::SpinSync &Object)
{
    if(!(InternalTag = Object.InternalTag))
        return;

    SpinSyncInternal &Internal = *((SpinSyncInternal *) InternalTag);

    LacewingSyncIncrement(&Internal.WritersWaiting);

    for(;;)
    {
        if(Internal.Readers != 0)
        {
            LacewingYield();
            continue;
        }

        if(LacewingSyncCompareExchange((volatile LONG *) &Internal.Readers, WriterPresent, 0) == 0)
            break;
    }

    LacewingSyncDecrement(&Internal.WritersWaiting);
}

Lacewing::SpinSync::WriteLock::~WriteLock()
{
    Release();
}

void Lacewing::SpinSync::WriteLock::Release()
{
    if(!InternalTag)
        return;

    LacewingSyncExchange(&((SpinSyncInternal *) InternalTag)->Readers, 0);
    InternalTag = 0;
}

