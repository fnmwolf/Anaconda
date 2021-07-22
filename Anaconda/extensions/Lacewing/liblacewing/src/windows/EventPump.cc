
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

#include "../Common.h"

Lacewing::EventPump::EventPump(int)
{
    InternalTag = new EventPumpInternal(*this);
    Tag         = 0;
}

Lacewing::EventPump::~EventPump()
{
    EventPumpInternal &Internal = *((EventPumpInternal *) InternalTag);

    if(Internal.WatcherThread.Started())
    {
        Post (SigEndWatcherThread, 0);

        Internal.WatcherResumeEvent.Signal ();
        Internal.WatcherThread.Join ();
    }

    delete ((EventPumpInternal *) InternalTag);
}

inline void Process(EventPumpInternal &Internal, OVERLAPPED * Overlapped, unsigned int BytesTransferred, EventPumpInternal::Event &Event, int Error)
{
    if(Event.Callback == SigRemoveClient)
        Internal.EventBacklog.Return(*(EventPumpInternal::Event *) Event.Tag);
    else
    {
        if(!Event.Removing)
            ((void (*) (void *, OVERLAPPED *, unsigned int, int)) Event.Callback) (Event.Tag, Overlapped, BytesTransferred, Error);
    }

    if(Overlapped == (OVERLAPPED *) 1)
    {
        /* Event was created by Post() */

        Internal.EventBacklog.Return(Event);
    }
}

Lacewing::Error * Lacewing::EventPump::Tick()
{
    EventPumpInternal &Internal = *((EventPumpInternal *) InternalTag);

    if(Internal.HandlerTickNeeded)
    {
        /* Process whatever the watcher thread dequeued before telling the caller to tick */

        Process(Internal, Internal.WatcherOverlapped, Internal.WatcherBytesTransferred,
                    *Internal.WatcherEvent, Internal.WatcherError); 
    }

    OVERLAPPED * Overlapped;
    unsigned int BytesTransferred;
    
    EventPumpInternal::Event * Event;

    for(;;)
    {
        int Error = 0;

        if(!GetQueuedCompletionStatus(Internal.CompletionPort, (DWORD *) &BytesTransferred,
                 (PULONG_PTR) &Event, &Overlapped, 0))
        {
            Error = GetLastError();

            if(Error == WAIT_TIMEOUT)
                break;

            if(!Overlapped)
                break;
        }

        Process(Internal, Overlapped, BytesTransferred, *Event, Error);
    }
 
    if(Internal.HandlerTickNeeded)
        Internal.WatcherResumeEvent.Signal();

    return 0;
}

Lacewing::Error * Lacewing::EventPump::StartEventLoop()
{
    EventPumpInternal &Internal = *((EventPumpInternal *) InternalTag);

    OVERLAPPED * Overlapped;
    unsigned int BytesTransferred;
    
    EventPumpInternal::Event * Event;
    bool Exit = false;

    for(;;)
    {
        /* TODO : Use GetQueuedCompletionStatusEx where available */

        int Error = 0;

        if(!GetQueuedCompletionStatus(Internal.CompletionPort, (DWORD *) &BytesTransferred,
                 (PULONG_PTR) &Event, &Overlapped, INFINITE))
        {
            Error = GetLastError();

            if(!Overlapped)
                continue;
        }

        if(Event->Callback == SigExitEventLoop)
        {
            Internal.EventBacklog.Return(*Event);
            break;
        }
 
        Process(Internal, Overlapped, BytesTransferred, *Event, Error);
    }
        
    return 0;
}

void Lacewing::EventPump::Post(void * Function, void * Parameter)
{
    EventPumpInternal &Internal = *((EventPumpInternal *) InternalTag);

    EventPumpInternal::Event &Event = Internal.EventBacklog.Borrow(Internal);

    Event.Callback  = Function;
    Event.Tag       = Parameter;
    Event.Removing  = false;

    PostQueuedCompletionStatus(Internal.CompletionPort, 0, (ULONG_PTR) &Event, (OVERLAPPED *) 1);
}

void Lacewing::EventPump::PostEventLoopExit ()
{
    Post (SigExitEventLoop, 0);
}

void Watcher (EventPumpInternal &Internal)
{
    for(;;)
    {
        Internal.WatcherError = 0;

        if(!GetQueuedCompletionStatus(Internal.CompletionPort, (DWORD *) &Internal.WatcherBytesTransferred,
                 (PULONG_PTR) &Internal.WatcherEvent, &Internal.WatcherOverlapped, INFINITE))
        {
            if((Internal.WatcherError = GetLastError()) == WAIT_TIMEOUT)
                break;

            if(!Internal.WatcherOverlapped)
                break;

            Internal.WatcherBytesTransferred = 0;
        }

        if(Internal.WatcherEvent->Callback == SigEndWatcherThread)
            break;

        Internal.HandlerTickNeeded(Internal.Pump);

        Internal.WatcherResumeEvent.Wait();
        Internal.WatcherResumeEvent.Unsignal();
    }
}

Lacewing::Error * Lacewing::EventPump::StartSleepyTicking(void (LacewingHandler * onTickNeeded) (Lacewing::EventPump &EventPump))
{
    EventPumpInternal &Internal = *((EventPumpInternal *) InternalTag);

    Internal.HandlerTickNeeded = onTickNeeded;    
    Internal.WatcherThread.Start(&Internal);

    return 0;
}

bool Lacewing::EventPump::InUse ()
{
    return ((EventPumpInternal *) InternalTag)->InUse;
}

void Lacewing::EventPump::InUse (bool InUse)
{
    ((EventPumpInternal *) InternalTag)->InUse = InUse;
}

