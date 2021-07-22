
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

struct TimerInternal;

void LegacyTimer (TimerInternal &Internal);
void TimerTick (TimerInternal &Internal);

struct TimerInternal
{
    Lacewing::Timer &Timer;
    PumpInternal &EventPump;

    Lacewing::Timer::HandlerTick HandlerTick;

    bool Started;

    #ifdef LacewingUseTimerFD
        int FD;
    #endif

    Lacewing::Event StopEvent;
    int Interval;

    Lacewing::Thread LegacyTimerThread;
    
    TimerInternal(Lacewing::Timer &_Timer, PumpInternal &_EventPump)
                    : Timer(_Timer), EventPump(_EventPump),
                      LegacyTimerThread ("LegacyTimer", (void *) LegacyTimer)
    {
        HandlerTick = 0;
        Started = false;
        
        #ifdef LacewingUseTimerFD
            FD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
            EventPump.AddRead(FD, this, (void *) TimerTick);
        #endif
    }

    ~TimerInternal()
    {
        #ifdef LacewingUseTimerFD
            close(FD);
        #endif
    }
};

void TimerTick(TimerInternal &Internal)
{
    if(Internal.HandlerTick)
        Internal.HandlerTick(Internal.Timer);
}

void LegacyTimer (TimerInternal &Internal)
{
    for(;;)
    {
        Internal.StopEvent.Wait(Internal.Interval);

        if(Internal.StopEvent.Signalled())
            break;
        
        Internal.EventPump.Pump.Post((void *) TimerTick, &Internal);
    }
}

Lacewing::Timer::Timer(Lacewing::Pump &Pump)
{
    InternalTag = new TimerInternal(*this, *(PumpInternal *) Pump.InternalTag);
    Tag         = 0;
    
    Pump.InUse (true);
}

Lacewing::Timer::~Timer()
{
    Stop ();

    delete ((TimerInternal *) InternalTag);
}

void Lacewing::Timer::Start(int Interval)
{
    Stop();

    TimerInternal &Internal = *((TimerInternal *) InternalTag);
    
    Internal.Started = true;

    #ifdef LacewingUseKQueue
    
        if(Internal.EventPump.Pump.IsEventPump())
        {
            EventPumpInternal &EventPump = *(EventPumpInternal *)
                    ((Lacewing::EventPump *) &Internal.EventPump.Pump)->EPInternalTag;

            struct kevent Change;
            EV_SET(&Change, 0, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_CLEAR, 0, Interval, this);
            
            if(kevent(EventPump.Queue, &Change, 1, 0, 0, 0) == -1)
            {
                DebugOut("Timer: Failed to add timer to kqueue: %s", strerror(errno));
                return;
            }
        }
        else
        {
            Internal.Interval = Interval;
            Internal.LegacyTimerThread.Start (&Internal);
        }

    #else
        #ifdef LacewingUseTimerFD
        
            itimerspec spec;

            spec.it_interval.tv_sec  = Interval / 1000;
            spec.it_interval.tv_nsec = (Interval % 1000) * 1000000;

            memcpy(&spec.it_value, &spec.it_interval, sizeof(spec.it_interval));
            
            timerfd_settime(Internal.FD, 0, &spec, 0);
            
        #else

            Internal.Interval = Interval;
            Internal.LegacyTimerThread.Start (&Internal);

        #endif
    #endif
}

void Lacewing::Timer::Stop()
{
    /* TODO: What if a tick has been posted using internal and this gets destructed? */

    TimerInternal &Internal = *((TimerInternal *) InternalTag);

    #ifndef LacewingUseTimerFD

        Internal.StopEvent.Signal ();
        Internal.LegacyTimerThread.Join ();
        Internal.StopEvent.Unsignal ();

    #endif

    #ifdef LacewingUseKQueue

        /* TODO */

    #else
        #ifdef LacewingUseTimerFD
            itimerspec spec;
            memset(&spec, 0, sizeof(itimerspec));
            timerfd_settime(Internal.FD, 0, &spec, 0);
        #endif
    #endif

    Internal.Started = false;
}

void Lacewing::Timer::ForceTick()
{
    TimerInternal &Internal = *((TimerInternal *) InternalTag);
    
    if(Internal.HandlerTick)
        Internal.HandlerTick(*this);
}

bool Lacewing::Timer::Started ()
{
    return ((TimerInternal *) InternalTag)->Started;
}

AutoHandlerFunctions(Lacewing::Timer, TimerInternal, Tick)

