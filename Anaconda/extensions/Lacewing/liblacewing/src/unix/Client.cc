
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

struct ClientInternal;

struct ClientInternal
{
    PumpInternal &EventPump;

    Lacewing::Client::HandlerConnect     HandlerConnect;
    Lacewing::Client::HandlerDisconnect  HandlerDisconnect;
    Lacewing::Client::HandlerReceive     HandlerReceive;
    Lacewing::Client::HandlerError       HandlerError;

    Lacewing::Client &Public;

    ClientInternal(Lacewing::Client &_Public, PumpInternal &_EventPump)
            : Public(_Public), EventPump(_EventPump)
    {
        Connected = Connecting = false;

        Address =  0;
        Socket  = -1;

        HandlerConnect     = 0;
        HandlerDisconnect  = 0;
        HandlerReceive     = 0;
        HandlerError       = 0;

        Nagle = true;
        Address = new Lacewing::Address();

        QueuedOffset = 0;
    }

    Lacewing::Address * Address;

    int Socket;
    bool Connected, Connecting;

    sockaddr_in HostStructure;

    ReceiveBuffer Buffer;

    void ReadReady();
    void WriteReady();

    bool Nagle;

    MessageBuilder Queued;
    int QueuedOffset;
};

Lacewing::Client::Client(Lacewing::Pump &EventPump)
{
    LacewingInitialise();
    InternalTag = new ClientInternal(*this, *(PumpInternal *) EventPump.InternalTag);
}

Lacewing::Client::~Client()
{
    Disconnect();

    delete ((ClientInternal *) InternalTag);
}

void Lacewing::Client::Connect(const char * Host, int Port)
{
    Lacewing::Address Address(Host, Port, true);
    Connect(Address);
}

void WriteReady(ClientInternal &Internal)
{
    Internal.WriteReady();
}

void ClientInternal::WriteReady()
{
    if(Connecting)
    {
        int Error;
        
        {   socklen_t ErrorLength = sizeof(Error);
            getsockopt(Socket, SOL_SOCKET, SO_ERROR, &Error, &ErrorLength);
        }

        if(Error != 0)
        {
            /* Failed to connect */

            Connecting = false;

            Lacewing::Error Error;
            Error.Add("Error connecting");

            if(HandlerError)
                HandlerError(Public, Error);

            return;
        }

        Connected  = true;
        Connecting = false;

        if(HandlerConnect)
            HandlerConnect(Public);
    }

    if(Queued.Size > 0)
    {
        int Sent = send (Socket, Queued.Buffer + QueuedOffset, Queued.Size - QueuedOffset, LacewingNoSignal);
        
        if(Sent < (Queued.Size - QueuedOffset))
            QueuedOffset += Sent;
        else
        {
            Queued.Reset();
            QueuedOffset = 0;
        }
    }
}

void ReadReady(ClientInternal &Internal)
{
    Internal.ReadReady();
}

void ClientInternal::ReadReady()
{
    for(;;)
    {
        Buffer.Prepare();
        int Bytes = recv(Socket, Buffer.Buffer, Buffer.Size, MSG_DONTWAIT);

        if(Bytes == -1)
        {
            if(errno == EAGAIN)
                break;

            /* Assume some sort of disconnect */

            close(Socket);
            Bytes = 0;
        }

        if(Bytes == 0)
        {
            Socket = -1;
            Connected = false;

            if(HandlerDisconnect)
                HandlerDisconnect(Public);

            return;
        }

        Buffer.Received(Bytes);

        if(HandlerReceive)
            HandlerReceive(Public, Buffer.Buffer, Bytes);
            
        if(Socket == -1)
        {
            /* Disconnect called from within the receive handler */

            Connected = false;
            
            if(HandlerDisconnect)
                HandlerDisconnect(Public);

            return;
        }
    }
}

void Lacewing::Client::Connect(Lacewing::Address &Address)
{
    ClientInternal &Internal = *((ClientInternal *) InternalTag);
    
    if(Connected() || Connecting())
    {
        Lacewing::Error Error;
        Error.Add("Already connected to a server");
        
        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;
    }

    Internal.Connecting = true;

    while(!Address.Ready())
        LacewingYield();

    delete Internal.Address;
    Internal.Address = new Lacewing::Address(Address);

    GetSockaddr(*Internal.Address, Internal.HostStructure);

    if((Internal.Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        Lacewing::Error Error;
       
        Error.Add(LacewingGetSocketError());        
        Error.Add("Error creating socket");
        
        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;
    }

    DisableSigPipe (Internal.Socket);

    if(!Internal.Nagle)
        ::DisableNagling(Internal.Socket);

    fcntl(Internal.Socket, F_SETFL, fcntl(Internal.Socket, F_GETFL, 0) | O_NONBLOCK);

    Internal.EventPump.AddReadWrite(Internal.Socket, (void *) &Internal, (void *) ReadReady, (void *) WriteReady);

    if(connect(Internal.Socket, (sockaddr *) &Internal.HostStructure, sizeof(sockaddr)) == -1)
    {
        if(errno == EINPROGRESS)
            return;

        Internal.Connecting = false;

        Lacewing::Error Error;
       
        Error.Add(errno);        
        Error.Add("Error connecting");

        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);
    }
}

bool Lacewing::Client::Connected()
{
    return ((ClientInternal *) InternalTag)->Connected;
}

bool Lacewing::Client::Connecting()
{
    return ((ClientInternal *) InternalTag)->Connecting;
}

void Lacewing::Client::Disconnect()
{
    ClientInternal &Internal = *((ClientInternal *) InternalTag);

    LacewingCloseSocket(Internal.Socket);
    Internal.Socket = -1;
}

void Lacewing::Client::Send(const char * Data, int Size)
{
    if(!Connected())
        return;

    if(Size == -1)
        Size = strlen(Data);

    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    if(Internal.Queued.Size > 0)
    {
        Internal.Queued.Add(Data, Size);
        return;
    }

    int Sent = send (Internal.Socket, Data, Size, LacewingNoSignal);

    if(Sent < Size)
        Internal.Queued.Add(Data + Sent, Size - Sent);
}

Lacewing::Address &Lacewing::Client::ServerAddress()
{
    return *((ClientInternal *) InternalTag)->Address;
}

void Lacewing::Client::DisableNagling()
{
    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    Internal.Nagle = false;

    if(Internal.Socket != -1)
        ::DisableNagling(Internal.Socket);
}

bool Lacewing::Client::CheapBuffering ()
{
    /* TODO : Userland buffering support for Unix client? */

    return true;
}

void Lacewing::Client::StartBuffering()
{
    if(!Connected())
        return;

    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    #ifdef LacewingAllowCork
        int Enabled = 1;
        setsockopt(Internal.Socket, IPPROTO_TCP, LacewingCork, &Enabled, sizeof(Enabled));
    #endif
}

void Lacewing::Client::Flush()
{
    if(!Connected())
        return;

    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    #ifdef LacewingAllowCork    
        int Enabled = 0;
        setsockopt(Internal.Socket, IPPROTO_TCP, LacewingCork, &Enabled, sizeof(Enabled));
    #endif
}


AutoHandlerFunctions(Lacewing::Client, ClientInternal, Connect)
AutoHandlerFunctions(Lacewing::Client, ClientInternal, Disconnect)
AutoHandlerFunctions(Lacewing::Client, ClientInternal, Receive)
AutoHandlerFunctions(Lacewing::Client, ClientInternal, Error)

