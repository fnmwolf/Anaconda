
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

lw_eventpump * lw_eventpump_new ()
    { return (lw_eventpump *) new Lacewing::EventPump;
    }
void lw_eventpump_delete (lw_eventpump * eventpump)
    { delete (Lacewing::EventPump *) eventpump;
    }
void lw_eventpump_tick (lw_eventpump * eventpump)
    { ((Lacewing::EventPump *) eventpump)->Tick();
    }
void lw_eventpump_start_event_loop (lw_eventpump * eventpump)
    { ((Lacewing::EventPump *) eventpump)->StartEventLoop();
    }
void lw_eventpump_start_sleepy_ticking (lw_eventpump * eventpump, void (LacewingHandler * on_tick_needed) (lw_eventpump *))
    { ((Lacewing::EventPump *) eventpump)->StartSleepyTicking((void (LacewingHandler *) (Lacewing::EventPump &)) on_tick_needed);
    }
void lw_eventpump_post_eventloop_exit (lw_eventpump * eventpump)
    { ((Lacewing::EventPump *) eventpump)->PostEventLoopExit();
    }
lw_bool lw_eventpump_in_use (lw_eventpump * eventpump)
    { return ((Lacewing::EventPump *) eventpump)->InUse();
    }
void lw_eventpump_set_in_use (lw_eventpump * eventpump, lw_bool in_use)
    { ((Lacewing::EventPump *) eventpump)->InUse(in_use != 0);
    }

