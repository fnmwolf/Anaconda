
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

lw_addr * lw_addr_new ()
    { return (lw_addr *) new Lacewing::Address();
    }
lw_addr * lw_addr_new_ip (long ip, long port)
    { return (lw_addr *) new Lacewing::Address ((unsigned int) ip, port);
    }
lw_addr * lw_addr_new_name (const char * hostname, long port, lw_bool blocking)
    { return (lw_addr *) new Lacewing::Address(hostname, port, blocking ? 1 : 0);
    }
lw_addr * lw_addr_copy (lw_addr * address)
    { return (lw_addr *) new Lacewing::Address(*(Lacewing::Address *) address);
    }
void lw_addr_delete (lw_addr * address)
    { delete (Lacewing::Address *) address;
    }
long lw_addr_port (lw_addr * address)
    { return ((Lacewing::Address *) address)->Port();
    }
void lw_addr_set_port (lw_addr * address, long port)
    { ((Lacewing::Address *) address)->Port(port);
    }
lw_bool lw_addr_is_ready (lw_addr * address)
    { return ((Lacewing::Address *) address)->Ready();
    }
long lw_addr_ip (lw_addr * address)
    { return ((Lacewing::Address *) address)->IP();
    }
unsigned char lw_addr_ip_byte (lw_addr * address, long index)
    { return ((Lacewing::Address *) address)->IP_Byte(index);
    }
const char * lw_addr_tostring (lw_addr * address)
    { return ((Lacewing::Address *) address)->ToString();
    }

