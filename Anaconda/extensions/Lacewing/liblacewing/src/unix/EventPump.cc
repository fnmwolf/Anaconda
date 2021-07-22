
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
#include "EventPump.h"

Lacewing::EventPump::EventPump(int MaxHint)
{
    EPInternalTag = new EventPumpInternal(*this, MaxHint);
    EPTag         = 0;
}

Lacewing::EventPump::~EventPump()
{
    delete ((EventPumpInternal *) EPInternalTag);
}

const int MaxEvents = 32;

Lacewing::Error * Lacewing::EventPump::Tick()
{
    EventPumpInternal &Internal = *((EventPumpInternal *) EPInternalTag);

    #ifdef LacewingUseEPoll
    
        epoll_event EPollEvents [MaxEvents];
        int Count = epoll_wait(Internal.Queue, EPollEvents, MaxEvents, 0);
    
        for(int i = 0; i < Count; ++ i)
        {
            epoll_event &EPollEvent = EPollEvents[i];

            Ready (EPollEvent.data.ptr, (EPollEvent.events & EPOLLIN) != 0
                        || (EPollEvent.events & EPOLLHUP) != 0 ||
                           (EPollEvent.events & EPOLLRDHUP) != 0,
                           (EPollEvent.events & EPOLLOUT) != 0);
        }
   
    #endif
    
    #ifdef LacewingUseKQueue
    
        struct kevent KEvents [MaxEvents];

        timespec Zero;
        memset(&Zero, 0, sizeof(Zero));
        
        int Count = kevent(Internal.Queue, 0, 0, KEvents, MaxEvents, &Zero);

        for(int i = 0; i < Count; ++ i)
        {
            struct kevent &KEvent = KEvents[i];

            Ready (KEvent.udata, KEvent.filter == EVFILT_READ || (KEvent.flags & EV_EOF),
                                    KEvent.filter == EVFILT_WRITE);
        }
        
    #endif

    return 0;
}

Lacewing::Error * Lacewing::EventPump::StartEventLoop()
{
    EventPumpInternal &Internal = *((EventPumpInternal *) EPInternalTag);

    for(;;)
    {
        bool Continue = true;

        #ifdef LacewingUseEPoll
        
            epoll_event EPollEvents [MaxEvents];
            int Count = epoll_wait(Internal.Queue, EPollEvents, MaxEvents, -1);
        
            for(int i = 0; i < Count; ++ i)
            {
                epoll_event &EPollEvent = EPollEvents[i];

                Continue = Ready (EPollEvent.data.ptr, (EPollEvent.events & EPOLLIN) != 0
                                        || (EPollEvent.events & EPOLLHUP) != 0 ||
                                        (EPollEvent.events & EPOLLRDHUP) != 0,
                                        (EPollEvent.events & EPOLLOUT) != 0);
            }
       
        #endif
        
        #ifdef LacewingUseKQueue
        
            struct kevent KEvents [MaxEvents];
            int Count = kevent(Internal.Queue, 0, 0, KEvents, MaxEvents, 0);

            for(int i = 0; i < Count; ++ i)
            {
                struct kevent &KEvent = KEvents[i];
                    
                if(KEvent.filter == EVFILT_TIMER)
                {
                    ((Lacewing::Timer *) KEvent.udata)->ForceTick();
                }
                else
                {
                    Continue = Ready (KEvent.udata, KEvent.filter == EVFILT_READ ||
                                (KEvent.flags & EV_EOF), KEvent.filter == EVFILT_WRITE);
                }
            }
            
        #endif

        if(!Continue)
            break;
    }
        
    return 0;
}

Lacewing::Error * Lacewing::EventPump::StartSleepyTicking(void (LacewingHandler * onTickNeeded) (Lacewing::EventPump &EventPump))
{
    EventPumpInternal &Internal = *((EventPumpInternal *) EPInternalTag);

    /* TODO */

    return 0;
}

void * Lacewing::EventPump::AddRead(int FD, void * Tag)
{
    EventPumpInternal &Internal = *((EventPumpInternal *) EPInternalTag);

    #ifdef LacewingUseEPoll
    
        epoll_event Event;
        memset(&Event, 0, sizeof(epoll_event));

        Event.events = EPOLLIN | EPOLLET;
        Event.data.ptr = Tag;

        if(epoll_ctl(Internal.Queue, EPOLL_CTL_ADD, FD, &Event) == -1)
        {
            DebugOut("EventPump: Failed to add FD: %s", strerror(errno));
            return 0;
        }
            
    #endif
    
    #ifdef LacewingUseKQueue
    
        struct kevent Change;
        EV_SET(&Change, FD, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, Tag); /* EV_CLEAR = edge triggered */
        
        if(kevent(Internal.Queue, &Change, 1, 0, 0, 0) == -1)
        {
            DebugOut("EventPump: Failed to add FD: %s", strerror(errno));
            return 0;
        }
        
    #endif

    return 0;
}

void * Lacewing::EventPump::AddReadWrite(int FD, void * Tag)
{
    EventPumpInternal &Internal = *((EventPumpInternal *) EPInternalTag);

    #ifdef LacewingUseEPoll
    
        epoll_event Event;
        memset(&Event, 0, sizeof(epoll_event));

        Event.events = EPOLLIN | EPOLLOUT | EPOLLET;
        Event.data.ptr = Tag;

        if(epoll_ctl(Internal.Queue, EPOLL_CTL_ADD, FD, &Event) == -1)
        {
            DebugOut("EventPump: Failed to add FD: %s", strerror(errno));
            return 0;
        }
            
    #endif
    
    #ifdef LacewingUseKQueue
    
        struct kevent Change;

        EV_SET(&Change, FD, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, Tag); /* EV_CLEAR = edge triggered */
        
        if(kevent(Internal.Queue, &Change, 1, 0, 0, 0) == -1)
        {
            DebugOut("EventPump: Failed to add FD: %s", strerror(errno));
            return 0;
        }

        EV_SET(&Change, FD, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, Tag);
        
        if(kevent(Internal.Queue, &Change, 1, 0, 0, 0) == -1)
        {
            DebugOut("EventPump: Failed to add FD: %s", strerror(errno));
            return 0;
        }

    #endif

    return 0;
}

bool Lacewing::EventPump::IsEventPump ()
{
    return true;
}

