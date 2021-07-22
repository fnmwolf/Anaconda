
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

lw_udp* lw_udp_new (lw_eventpump * eventpump)
    { return (lw_udp *) new Lacewing::UDP(*(Lacewing::EventPump *) eventpump);
    }
void lw_udp_delete (lw_udp * udp)
    { delete (Lacewing::UDP *) udp;
    }
void lw_udp_host (lw_udp * udp, long port)
    { ((Lacewing::UDP *) udp)->Host(port);
    }
void lw_udp_host_filter (lw_udp * udp, lw_filter * filter)
    { ((Lacewing::UDP *) udp)->Host(*(Lacewing::Filter *) filter);
    }
void lw_udp_host_addr (lw_udp * udp, lw_addr * addr)
    { ((Lacewing::UDP *) udp)->Host(*(Lacewing::Address *) addr);
    }
lw_bool lw_udp_hosting (lw_udp * udp)
    { return ((Lacewing::UDP *) udp)->Hosting ();
    }
void lw_udp_unhost (lw_udp * udp)
    { ((Lacewing::UDP *) udp)->Unhost();
    }
long lw_udp_port (lw_udp * udp)
    { return ((Lacewing::UDP *) udp)->Port();
    }
lw_i64 lw_udp_bytes_sent (lw_udp * udp)
    { return ((Lacewing::UDP *) udp)->BytesSent();
    }
lw_i64 lw_udp_bytes_received (lw_udp * udp)
    { return ((Lacewing::UDP *) udp)->BytesReceived();
    }
void lw_udp_send (lw_udp * udp, lw_addr * addr, const char * data, long size)
    { ((Lacewing::UDP *) udp)->Send(*(Lacewing::Address *) addr, data, size);
    }

AutoHandlerFlat(Lacewing::UDP, lw_udp, Receive, receive)
AutoHandlerFlat(Lacewing::UDP, lw_udp, Error, error)

