
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

lw_server * lw_server_new (lw_eventpump * eventpump)
    { return (lw_server *) new Lacewing::Server(*(Lacewing::EventPump *) eventpump);
    }
void lw_server_delete (lw_server * server)
    { delete (Lacewing::Server *) server;
    }
void lw_server_host (lw_server * server, long port)
    { ((Lacewing::Server *) server)->Host(port);
    }
void lw_server_host_ex (lw_server * server, long port, lw_bool client_speaks_first)
    { ((Lacewing::Server *) server)->Host(port, client_speaks_first != 0);
    }
void lw_server_host_filter (lw_server * server, lw_filter * filter)
    { ((Lacewing::Server *) server)->Host(*(Lacewing::Filter *) filter);
    }
void lw_server_host_filter_ex (lw_server * server, lw_filter * filter, lw_bool client_speaks_first)
    { ((Lacewing::Server *) server)->Host(*(Lacewing::Filter *) filter, client_speaks_first != 0);
    }
void lw_server_unhost (lw_server * server)
    { ((Lacewing::Server *) server)->Unhost();
    }
lw_bool lw_server_hosting (lw_server * server)
    { return ((Lacewing::Server *) server)->Hosting() ? 1 : 0;
    }
long lw_server_port (lw_server * server)
    { return ((Lacewing::Server *) server)->Port();
    }
lw_bool lw_server_load_cert_file (lw_server * server, const char * filename, const char * passphrase)
    { return ((Lacewing::Server *) server)->LoadCertificateFile(filename, passphrase);
    }
lw_bool lw_server_load_sys_cert (lw_server * server, const char * store_name, const char * common_name, const char * location)
    { return ((Lacewing::Server *) server)->LoadSystemCertificate(store_name, common_name, location);
    }
lw_bool lw_server_cert_loaded (lw_server * server)
    { return ((Lacewing::Server *) server)->CertificateLoaded();
    }
lw_i64 lw_server_bytes_sent (lw_server * server)
    { return ((Lacewing::Server *) server)->BytesSent();
    }
lw_i64 lw_server_bytes_received (lw_server * server)
    { return ((Lacewing::Server *) server)->BytesReceived();
    }
void lw_server_disable_nagling (lw_server * server)
    { ((Lacewing::Server *) server)->DisableNagling();
    }
lw_addr * lw_server_client_address (lw_server_client * client)
    { return (lw_addr *) &((Lacewing::Server::Client *) client)->GetAddress();
    }
void lw_server_client_send (lw_server_client * client, const char * data, long size)
    { ((Lacewing::Server::Client *) client)->Send(data, size);
    }
void lw_server_client_send_text (lw_server_client * client, const char * text)
    { ((Lacewing::Server::Client *) client)->Send(text);
    }
void lw_server_client_send_writable (lw_server_client * client, char * data, long size)
    { ((Lacewing::Server::Client *) client)->SendWritable(data, size);
    }
lw_bool lw_server_client_cheap_buffering (lw_server_client * client)
    { return ((Lacewing::Server::Client *) client)->CheapBuffering();
    }
void lw_server_client_start_buffering (lw_server_client * client)
    { ((Lacewing::Server::Client *) client)->StartBuffering();
    }
void lw_server_client_flush (lw_server_client * client)
    { ((Lacewing::Server::Client *) client)->Flush();
    }
void lw_server_client_disconnect (lw_server_client * client)
    { ((Lacewing::Server::Client *) client)->Disconnect();
    }
lw_server_client * lw_server_client_next (lw_server_client * client)
    { return (lw_server_client *) ((Lacewing::Server::Client *) client)->Next();
    }

void lw_server_client_sendf (lw_server_client * client, const char * format, ...)
{
    va_list args;
    va_start (args, format);
    
    char * data;
    int size = LacewingFormat (data, format, args);
    
    if(size > 0)
        ((Lacewing::Server::Client *) client)->SendWritable (data, size);

    va_end (args);
}

AutoHandlerFlat(Lacewing::Server, lw_server, Connect, connect)
AutoHandlerFlat(Lacewing::Server, lw_server, Disconnect, disconnect)
AutoHandlerFlat(Lacewing::Server, lw_server, Receive, receive)
AutoHandlerFlat(Lacewing::Server, lw_server, Error, error)




