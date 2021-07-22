
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

struct TimerInternal;

void TimerThread (TimerInternal &Internal);

struct TimerInternal
{
    EventPumpInternal &EventPump;
    Lacewing::Timer   &Timer;
    
    Lacewing::Thread TimerThread;

    HANDLE TimerHandle;
    HANDLE ShutdownEvent;

    bool Started;

    Lacewing::Timer::HandlerTick HandlerTick;

    TimerInternal(Lacewing::Timer &_Timer, EventPumpInternal &_EventPump)
                : Timer(_Timer), EventPump(_EventPump),
                    TimerThread ("Timer", (void *) ::TimerThread)
    {
        ShutdownEvent = CreateEvent         (0, TRUE, FALSE, 0);
        TimerHandle   = CreateWaitableTimer (0, FALSE, 0);

        HandlerTick = 0;

        Started = false;
    }
};

void TimerCompletion(TimerInternal &Internal)
{
    if(Internal.HandlerTick)
        Internal.HandlerTick(Internal.Timer);
}

void TimerThread (TimerInternal &Internal)
{
    HANDLE Events[2] = { Internal.TimerHandle, Internal.ShutdownEvent };

    for(;;)
    {
        int Result = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

        if(Result != WAIT_OBJECT_0)
        {
            DebugOut ("Got result %d", Result);
            break;
        }

        Internal.EventPump.Pump.Post((void *) TimerCompletion, &Internal);
    }
}

Lacewing::Timer::Timer(Lacewing::EventPump &EventPump)
{
    InternalTag = new TimerInternal(*this, *(EventPumpInternal *) EventPump.InternalTag);
    Tag         = 0;

    EventPump.InUse (true);

    ((TimerInternal *) InternalTag)->TimerThread.Start(InternalTag);
}

Lacewing::Timer::~Timer()
{
    TimerInternal &Internal = *((TimerInternal *) InternalTag);
     
    SetEvent (Internal.ShutdownEvent);
    Internal.TimerThread.Join ();

    delete &Internal;
}

void Lacewing::Timer::Start(int Interval)
{
    Stop();

    TimerInternal &Internal = *((TimerInternal *) InternalTag);

    LARGE_INTEGER DueTime;
    DueTime.QuadPart = 0 - (Interval * 1000 * 10);

    if(!SetWaitableTimer(Internal.TimerHandle, &DueTime, Interval, 0, 0, 0))
    {
        LacewingAssert(false);
    }

    Internal.Started = true;
}

void Lacewing::Timer::Stop()
{
    TimerInternal &Internal = *((TimerInternal *) InternalTag);

    CancelWaitableTimer(Internal.TimerHandle);
    Internal.Started = false;
}

bool Lacewing::Timer::Started ()
{
    return ((TimerInternal *) InternalTag)->Started;
}

void Lacewing::Timer::ForceTick()
{
    TimerInternal &Internal = *((TimerInternal *) InternalTag);

    if(Internal.HandlerTick)
        Internal.HandlerTick(*this);
}

AutoHandlerFunctions(Lacewing::Timer, TimerInternal, Tick)

