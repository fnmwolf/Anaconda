
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

#ifndef LacewingEventPump
#define LacewingEventPump

#define SigRemoveClient        (void *) 0
#define SigEndWatcherThread    (void *) 1
#define SigExitEventLoop       (void *) 2

struct EventPumpInternal;

void Watcher (EventPumpInternal &);

struct EventPumpInternal
{  
    Lacewing::EventPump &Pump;
    Lacewing::Thread WatcherThread;

    HANDLE CompletionPort;

    bool InUse;

    EventPumpInternal(Lacewing::EventPump &_Pump)
        : Pump(_Pump), WatcherThread ("Watcher", (void *) Watcher)
    {
        CompletionPort     = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 3, 0);
        HandlerTickNeeded  = 0;

        InUse = false;
    }

    inline void * Add (HANDLE Handle, void * Tag, void * Callback)
    {
        InUse = true;

        Event &E = EventBacklog.Borrow(*this);

        E.Tag      = Tag;
        E.Callback = Callback;
        E.Removing = false;

        HANDLE Result = CreateIoCompletionPort(Handle, CompletionPort, (ULONG_PTR) &E, 0);

        LacewingAssert(Result != 0);

        return &E;
    }

    inline void Remove (void * RemoveKey)
    {
        ((Event *) RemoveKey)->Removing = true;
        Pump.Post(SigRemoveClient, RemoveKey);
    }

    struct Event
    {
        Event(EventPumpInternal &)
        {
        }

        void * Tag;
        void * Callback;
        
        bool Removing;
    };

    Backlog<EventPumpInternal, Event> EventBacklog;

    void (LacewingHandler * HandlerTickNeeded) (Lacewing::EventPump &EventPump);

    OVERLAPPED * WatcherOverlapped;
    unsigned int WatcherBytesTransferred;
    EventPumpInternal::Event * WatcherEvent;
    Lacewing::Event WatcherResumeEvent;
    int WatcherError;
};

typedef EventPumpInternal PumpInternal;

#endif


