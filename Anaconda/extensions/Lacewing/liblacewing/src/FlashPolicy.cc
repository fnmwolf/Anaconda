
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

#include "Common.h"

struct FlashPolicyInternal
{
    char * Buffer;
    size_t Size;

    Lacewing::FlashPolicy &Public;
    Lacewing::Server Socket;

    Lacewing::FlashPolicy::HandlerError HandlerError;

    FlashPolicyInternal(Lacewing::FlashPolicy &_Public, Lacewing::Pump &_Pump)
            : Public(_Public), Socket(_Pump)
    {
        Socket.Tag = this;

        Buffer        = 0;
        HandlerError  = 0;
    }
};

void SocketReceive(Lacewing::Server &Socket, Lacewing::Server::Client &Client, char * Buffer, int Size)
{
    FlashPolicyInternal &Internal = *(FlashPolicyInternal *) Socket.Tag;

    for(int i = 0; i < Size; ++i)
    {
        if(!Buffer[i])
        {
            Client.Send(Internal.Buffer, Internal.Size);
            Client.Send("\0", 1);

            return;
        }
    }
}

void SocketError(Lacewing::Server &Socket, Lacewing::Error &Error)
{
    FlashPolicyInternal &Internal = *(FlashPolicyInternal *) Socket.Tag;

    Error.Add("Socket error");
    
    if(Internal.HandlerError)
        Internal.HandlerError(Internal.Public, Error);
}

Lacewing::FlashPolicy::FlashPolicy(Lacewing::Pump &Pump)
{
    FlashPolicyInternal &Internal = *(FlashPolicyInternal *)
            (InternalTag = new FlashPolicyInternal(*this, Pump));

    Internal.Socket.onError   (SocketError);
    Internal.Socket.onReceive (SocketReceive);
}

Lacewing::FlashPolicy::~FlashPolicy()
{
    Unhost();
    
    delete ((FlashPolicyInternal *) InternalTag);
}

void Lacewing::FlashPolicy::Host(const char * Filename, int Port)
{
    Lacewing::Filter Filter;
    Filter.LocalPort(Port);

    Host(Filename, Filter);
}

void Lacewing::FlashPolicy::Host(const char * Filename, Lacewing::Filter &Filter)
{
    Unhost();

    if(!Filter.LocalPort())
        Filter.LocalPort(843);
    
    FlashPolicyInternal &Internal = *(FlashPolicyInternal *) InternalTag;
    
    {   FILE * File = fopen(Filename, "r");

        if(!File)
        {
            Lacewing::Error Error;

            Error.Add (LacewingGetLastError());
            Error.Add ("Error opening file: %s", Filename);
                
            if(Internal.HandlerError)
                Internal.HandlerError(*this, Error);

            return;
        }

        fseek(File, 0, SEEK_END);

        Internal.Size = ftell(File);
        Internal.Buffer = (char *) malloc(Internal.Size);
        
        fseek(File, 0, SEEK_SET);

        int bytes = fread (Internal.Buffer, 1, Internal.Size, File);
        
        if (bytes != Internal.Size)
        {
            Internal.Size = bytes;

            if (ferror (File))
            {
                Lacewing::Error Error;
                
                Error.Add (LacewingGetLastError());
                Error.Add ("Error reading file: %s", Filename);

                if(Internal.HandlerError)
                    Internal.HandlerError(*this, Error);

                free (Internal.Buffer);
                Internal.Buffer = 0;
        
                fclose (File);
                
                return;
            }
        }

        fclose(File);
    }

    Internal.Socket.Host(Filter);
}

void Lacewing::FlashPolicy::Unhost()
{
    FlashPolicyInternal &Internal = *(FlashPolicyInternal *) InternalTag;
   
    Internal.Socket.Unhost ();

    free (Internal.Buffer);
    Internal.Buffer = 0;
}

bool Lacewing::FlashPolicy::Hosting()
{
    return ((FlashPolicyInternal *) InternalTag)->Socket.Hosting();
}

AutoHandlerFunctions(Lacewing::FlashPolicy, FlashPolicyInternal, Error);

