
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

struct UDPInternal
{
    PumpInternal &EventPump;

    Lacewing::UDP &Public;
    
    UDPInternal(Lacewing::UDP &_Public, PumpInternal &_EventPump)
            : Public(_Public), EventPump(_EventPump)
    {
        RemoteIP       = 0;

        HandlerReceive = 0;
        HandlerError   = 0;

        Socket = -1;
    }

    Lacewing::UDP::HandlerReceive  HandlerReceive;
    Lacewing::UDP::HandlerError    HandlerError;

    int RemoteIP;
    int Port;

    int Socket;

    lw_i64 BytesSent;
    lw_i64 BytesReceived;
};

void UDPSocketCompletion(UDPInternal &Internal, bool)
{
    sockaddr_in From;
    socklen_t FromSize = sizeof(From);

    char Buffer[256 * 1024];
    
    for(;;)
    {
        int Bytes = recvfrom(Internal.Socket, Buffer, sizeof(Buffer), 0, (sockaddr *) &From, &FromSize);

        if(Bytes == -1)
            break;

        if(Internal.RemoteIP && From.sin_addr.s_addr != Internal.RemoteIP)
            break;

        Lacewing::Address Address(From.sin_addr.s_addr, ntohs(From.sin_port));
        Buffer[Bytes] = 0;

        if(Internal.HandlerReceive)
            Internal.HandlerReceive(Internal.Public, Address, Buffer, Bytes);
    }
}

void Lacewing::UDP::Host(int Port)
{
    Lacewing::Filter Filter;
    Filter.LocalPort(Port);

    Host(Filter);
}

void Lacewing::UDP::Host(Lacewing::Address &Address)
{
    Lacewing::Filter Filter;
    Filter.Remote(Address);

    Host(Filter);
}

void Lacewing::UDP::Host(Lacewing::Filter &Filter)
{
    Unhost();

    UDPInternal &Internal = *((UDPInternal *) InternalTag);

    if(Internal.Socket != -1)
    {
        Lacewing::Error Error;
        Error.Add("Already hosting");
        
        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;    
    }

    Internal.Socket   = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    Internal.RemoteIP = Filter.Remote().IP();

    DisableSigPipe (Internal.Socket);

    fcntl(Internal.Socket, F_SETFL, fcntl(Internal.Socket, F_GETFL, 0) | O_NONBLOCK);

    Internal.EventPump.AddRead(Internal.Socket, &Internal, (void *) UDPSocketCompletion);

    sockaddr_in SocketAddress;

    memset(&SocketAddress, 0, sizeof(Address));

    SocketAddress.sin_family = AF_INET;
    SocketAddress.sin_port = htons(Filter.LocalPort() ? Filter.LocalPort() : 0);
    SocketAddress.sin_addr.s_addr = Filter.LocalIP() ? Filter.LocalIP() : INADDR_ANY;

    if(bind(Internal.Socket, (sockaddr *) &SocketAddress, sizeof(sockaddr_in)) == -1)
    {
        close (Internal.Socket);
        Internal.Socket = -1;

        Lacewing::Error Error;
        
        Error.Add(errno);
        Error.Add("Error binding port");

        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;
    }

    socklen_t AddressLength = sizeof(sockaddr_in);
    getsockname(Internal.Socket, (sockaddr *) &SocketAddress, &AddressLength);

    Internal.Port = ntohs(SocketAddress.sin_port);
}

bool Lacewing::UDP::Hosting ()
{
    return ((UDPInternal *) InternalTag)->Socket != -1;
}

int Lacewing::UDP::Port()
{
    return ((UDPInternal *) InternalTag)->Port;
}

void Lacewing::UDP::Unhost()
{
    UDPInternal &Internal = *((UDPInternal *) InternalTag);

    LacewingCloseSocket(Internal.Socket);
    Internal.Socket = -1;
}

Lacewing::UDP::UDP(Lacewing::Pump &Pump)
{
    LacewingInitialise();  
    InternalTag = new UDPInternal(*this, *(PumpInternal *) Pump.InternalTag);
}

Lacewing::UDP::~UDP()
{
    delete ((UDPInternal *) InternalTag);
}

void Lacewing::UDP::Send(Lacewing::Address &Address, const char * Data, int Size)
{
    UDPInternal &Internal = *(UDPInternal *) InternalTag;

    if(!Address.Ready())
    {
        Lacewing::Error Error;

        Error.Add("The address object passed to Send() wasn't ready");
        Error.Add("Error sending");

        if(Internal.HandlerError)
            Internal.HandlerError(Internal.Public, Error);

        return;
    }

    if(Size == -1)
        Size = strlen(Data);

    sockaddr_in To;
    GetSockaddr(Address, To);

    if(sendto(Internal.Socket, Data, Size, 0, (sockaddr *) &To, sizeof(To)) == -1)
    {
        Lacewing::Error Error;

        Error.Add(errno);            
        Error.Add("Error sending");

        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;
    }
}

lw_i64 Lacewing::UDP::BytesReceived()
{
    return ((UDPInternal *) InternalTag)->BytesReceived;
}

lw_i64 Lacewing::UDP::BytesSent()
{
    return ((UDPInternal *) InternalTag)->BytesSent;
}

AutoHandlerFunctions(Lacewing::UDP, UDPInternal, Error)
AutoHandlerFunctions(Lacewing::UDP, UDPInternal, Receive)

