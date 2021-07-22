
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

lw_timer * lw_timer_new (lw_eventpump * eventpump)
    { return (lw_timer *) new Lacewing::Timer(*(Lacewing::EventPump *) eventpump);
    }
void lw_timer_delete (lw_timer * timer)
    { delete (Lacewing::Timer *) timer;
    }
void lw_timer_start (lw_timer * timer, long milliseconds)
    { ((Lacewing::Timer *) timer)->Start(milliseconds);
    }
void lw_timer_stop (lw_timer * timer)
    { ((Lacewing::Timer *) timer)->Stop();
    }
void lw_timer_force_tick (lw_timer * timer)
    { ((Lacewing::Timer *) timer)->ForceTick();
    }
void lw_timer_ontick (lw_timer * timer, lw_timer_handler_tick ontick)
    { ((Lacewing::Timer *) timer)->onTick((Lacewing::Timer::HandlerTick) ontick);
    }




