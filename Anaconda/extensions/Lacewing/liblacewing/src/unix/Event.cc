
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

struct EventInternal
{
    
    volatile long Signalled;
    
    #ifdef LacewingUseMPSemaphore

        MPSemaphoreID Semaphore;
        volatile long WaiterCount;
        
        EventInternal()
        {
            Signalled = 0;
            WaiterCount = 0;
            
            MPCreateSemaphore(1, 0, &Semaphore);
        }
    
        ~EventInternal()
        {
            MPDeleteSemaphore(Semaphore);
            Signalled = 1;
        }
        
    #else
        
        sem_t Semaphore;
        
        EventInternal()
        {
            Signalled = 0;
            sem_init(&Semaphore, 0, 0);
        }
        
        ~EventInternal()
        {
            sem_destroy(&Semaphore);
            Signalled = 1;
        }

    #endif

};

Lacewing::Event::Event()
{
    LacewingInitialise();

    Tag = 0;
    InternalTag = new EventInternal;
}

Lacewing::Event::~Event()
{
    delete ((EventInternal *) InternalTag);
}

bool Lacewing::Event::Signalled()
{
    return ((EventInternal *) InternalTag)->Signalled != 0;
}

void Lacewing::Event::Signal()
{
    LacewingSyncExchange(&((EventInternal *) InternalTag)->Signalled, 1);
    
    #ifdef LacewingUseMPSemaphore
    
        long CurrentWaiterCount = ((EventInternal *) InternalTag)->WaiterCount;
        
        for(int i = 0; i < CurrentWaiterCount; ++ i)
            MPSignalSemaphore(((EventInternal *) InternalTag)->Semaphore);
            
    #else
        sem_post(&((EventInternal *) InternalTag)->Semaphore);
    #endif
}

void Lacewing::Event::Unsignal()
{
    LacewingSyncExchange(&((EventInternal *) InternalTag)->Signalled, 0);
}

void Lacewing::Event::Wait(int Timeout)
{
    if(Signalled())
        return;
        
    #ifdef LacewingUseMPSemaphore
    
        LacewingSyncIncrement(&((EventInternal *) InternalTag)->WaiterCount);
    
        if(Timeout == -1)
        {
            while(((EventInternal *) InternalTag)->Signalled == 0)
                MPWaitOnSemaphore(((EventInternal *) InternalTag)->Semaphore, kDurationForever);
        }
        else
            MPWaitOnSemaphore(((EventInternal *) InternalTag)->Semaphore, kDurationMillisecond * Timeout);
        
        LacewingSyncDecrement(&((EventInternal *) InternalTag)->WaiterCount);
        
    #else

        if(Timeout == -1)
        {
            while(((EventInternal *) InternalTag)->Signalled == 0)
                sem_wait(&((EventInternal *) InternalTag)->Semaphore);
        
            return;
        }

        timespec Time;

        #ifdef HAVE_CLOCK_GETTIME
            clock_gettime(CLOCK_REALTIME, &Time);
        #else

        #endif

        Time.tv_sec += Timeout / 1000;
        Time.tv_nsec += (Timeout % 1000) * 1000000;

        sem_timedwait(&((EventInternal *) InternalTag)->Semaphore, &Time);
    
    #endif
}

