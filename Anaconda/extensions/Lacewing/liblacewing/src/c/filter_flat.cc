
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

lw_filter * lw_filter_new ()
    { return (lw_filter *) new Lacewing::Filter;
    }
void lw_filter_delete (lw_filter * filter)
    { delete (Lacewing::Filter *) filter;
    }
void lw_filter_set_local_ip (lw_filter * filter, long ip)
    { ((Lacewing::Filter *) filter)->LocalIP(ip);
    }
long lw_filter_get_local_ip (lw_filter * filter)
    { return ((Lacewing::Filter *) filter)->LocalIP();
    }
void lw_filter_set_remote_addr (lw_filter * filter, lw_addr * addr)
    { ((Lacewing::Filter *) filter)->Remote(*(Lacewing::Address *) addr);
    }
lw_addr * lw_filter_get_remote_addr (lw_filter * filter)
    { return (lw_addr *) &((Lacewing::Filter *) filter)->Remote();
    }
void lw_filter_set_local_port (lw_filter * filter, long port)
    { ((Lacewing::Filter *) filter)->LocalPort(port);
    }
long lw_filter_get_local_port (lw_filter * filter)
    { return ((Lacewing::Filter *) filter)->LocalPort();
    }
void lw_filter_set_reuse (lw_filter * filter, lw_bool reuse)
    { ((Lacewing::Filter *) filter)->Reuse(reuse != 0);
    }
lw_bool lw_filter_is_reuse_set (lw_filter * filter)
    { return ((Lacewing::Filter *) filter)->Reuse();
    }
void lw_filter_set_local (lw_filter * filter, const char * name)
    { ((Lacewing::Filter *) filter)->Local(name);
    }
void lw_filter_set_remote (lw_filter * filter, const char * name)
    { ((Lacewing::Filter *) filter)->Remote(name);
    }



