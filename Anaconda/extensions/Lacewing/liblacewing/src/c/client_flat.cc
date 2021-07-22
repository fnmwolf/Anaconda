
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

lw_client * lw_client_new (lw_eventpump * eventpump)
    { return (lw_client *) new Lacewing::Client(*(Lacewing::EventPump *) eventpump);
    }
void lw_client_delete (lw_client * client)
    { delete (Lacewing::Client *) client;
    }
void lw_client_connect (lw_client * client, const char * host, long port)
    { ((Lacewing::Client *) client)->Connect(host, port);
    }
void lw_client_connect_addr (lw_client * client, lw_addr * address)
    { ((Lacewing::Client *) client)->Connect(*(Lacewing::Address *) address);
    }
void lw_client_disconnect (lw_client * client)
    { ((Lacewing::Client *) client)->Disconnect();
    }
lw_bool lw_client_connected (lw_client * client)
    { return ((Lacewing::Client *) client)->Connected();
    }
lw_bool lw_client_connecting (lw_client * client)
    { return ((Lacewing::Client *) client)->Connecting();
    }
lw_addr * lw_client_server_addr (lw_client * client)
    { return (lw_addr *) &((Lacewing::Client *) client)->ServerAddress();
    }
void lw_client_send (lw_client * client, const char * data, long size)
    { ((Lacewing::Client *) client)->Send(data, size);
    }
void lw_client_send_text (lw_client * client, const char * text)
    { ((Lacewing::Client *) client)->Send(text);
    }
void lw_client_disable_nagling (lw_client * client)
    { ((Lacewing::Client *) client)->DisableNagling();
    }
lw_bool lw_client_cheap_buffering (lw_client * client)
    { return ((Lacewing::Client *) client)->CheapBuffering();
    }
void lw_client_start_buffering (lw_client * client)
    { ((Lacewing::Client *) client)->StartBuffering();
    }
void lw_client_flush (lw_client * client)
    { ((Lacewing::Client *) client)->Flush();
    }

void lw_client_sendf (lw_client * client, const char * format, ...)
{
    va_list args;
    va_start (args, format);
    
    char * data;
    int size = LacewingFormat (data, format, args);
    
    if(size > 0)
        ((Lacewing::Client *) client)->Send(data, size);

    va_end (args);
}

AutoHandlerFlat(Lacewing::Client, lw_client, Connect, connect)
AutoHandlerFlat(Lacewing::Client, lw_client, Disconnect, disconnect)
AutoHandlerFlat(Lacewing::Client, lw_client, Receive, receive)
AutoHandlerFlat(Lacewing::Client, lw_client, Error, error)





