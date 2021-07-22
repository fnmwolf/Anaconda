
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

struct ClientOverlapped
{
    /* Constructor required by Backlog */

    ClientOverlapped (ClientInternal &)
    {
    }

    OVERLAPPED Overlapped;
    bool IsSend;
};

struct ClientInternal
{
    EventPumpInternal  &EventPump;

    ClientOverlapped Overlapped;

    Lacewing::Client::HandlerConnect     HandlerConnect;
    Lacewing::Client::HandlerDisconnect  HandlerDisconnect;
    Lacewing::Client::HandlerReceive     HandlerReceive;
    Lacewing::Client::HandlerError       HandlerError;

    Lacewing::Client &Public;

    ClientInternal(Lacewing::Client &_Public, EventPumpInternal &_EventPump)
            : Public(_Public), EventPump(_EventPump), Overlapped(*this)
    {
        Connected = Connecting = false;

        Address = 0;
        Socket  = SOCKET_ERROR;

        HandlerConnect     = 0;
        HandlerDisconnect  = 0;
        HandlerReceive     = 0;
        HandlerError       = 0;

        Nagle = true;
        BufferingOutput = false;

        Address = new Lacewing::Address();
    }

    Lacewing::Address * Address;

    SOCKET Socket;
    bool Connected, Connecting;

    sockaddr_in HostStructure;

    ReceiveBuffer Buffer;
    WSABUF WinsockBuffer;

    void Completion  (ClientOverlapped &Overlapped, unsigned int BytesTransferred, int Error);
    bool PostReceive ();

    bool Nagle;
    
    MessageBuilder OutputBuffer;
    bool BufferingOutput;

    Backlog <ClientInternal, ClientOverlapped>
        OverlappedBacklog;
};

Lacewing::Client::Client(Lacewing::EventPump &EventPump)
{
    LacewingInitialise();
    InternalTag = new ClientInternal(*this, *(EventPumpInternal *) EventPump.InternalTag);
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

void Completion(ClientInternal &Internal, ClientOverlapped &Overlapped, unsigned int BytesTransferred, int Error)
{
    Internal.Completion(Overlapped, BytesTransferred, Error);
}

void ClientInternal::Completion(ClientOverlapped &Overlapped, unsigned int BytesTransferred, int Error)
{
    if(Overlapped.IsSend)
    {
        OverlappedBacklog.Return(Overlapped);
        return;
    }

    if(Connecting)
    {
        if(!PostReceive())
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

        return;
    }

    if(!BytesTransferred)
    {
        Socket = SOCKET_ERROR;
        Connected = false;

        if(HandlerDisconnect)
            HandlerDisconnect(Public);

        return;
    }

    Buffer.Received(BytesTransferred);

    if(HandlerReceive)
        HandlerReceive(Public, Buffer.Buffer, BytesTransferred);
    
    if(Socket == SOCKET_ERROR)
    {
        /* Disconnect called from within the receive handler */

        Connected = false;
        
        if(HandlerDisconnect)
            HandlerDisconnect(Public);

        return;
    }

    PostReceive();
}

bool ClientInternal::PostReceive()
{
    memset(&Overlapped, 0, sizeof(ClientOverlapped));
    Overlapped.IsSend = false;

    Buffer.Prepare();

    WinsockBuffer.len = Buffer.Size;
    WinsockBuffer.buf = Buffer.Buffer;

    DWORD Flags = 0;

    if(WSARecv(Socket, &WinsockBuffer, 1, 0, &Flags, (OVERLAPPED *) &Overlapped, 0) == SOCKET_ERROR)
    {
        int Code = LacewingGetSocketError();

        return (Code == WSA_IO_PENDING);
    }

    return true;
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

    if((Internal.Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED)) == SOCKET_ERROR)
    {
        Lacewing::Error Error;
       
        Error.Add(LacewingGetSocketError());        
        Error.Add("Error creating socket");
        
        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;
    }

    if(!Internal.Nagle)
        ::DisableNagling(Internal.Socket);

    Internal.EventPump.Add((HANDLE) Internal.Socket, (void *) &Internal, (void *) Completion);

    memset(&Internal.Overlapped, 0, sizeof(OVERLAPPED));
    Internal.Overlapped.IsSend = false;

    /* LPFN_CONNECTEX and WSAID_CONNECTEX aren't defined w/ MinGW */

    static BOOL (PASCAL FAR * lw_ConnectEx)
    (   
        SOCKET s,
        const struct sockaddr FAR *name,
        int namelen,
        PVOID lpSendBuffer,
        DWORD dwSendDataLength,
        LPDWORD lpdwBytesSent,
        LPOVERLAPPED lpOverlapped

    ) = 0;

    if (!lw_ConnectEx)
    {   
        GUID  ID = {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}};
        DWORD Bytes = 0;

        WSAIoctl (Internal.Socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &ID,
                        sizeof(ID), &lw_ConnectEx, sizeof (lw_ConnectEx), &Bytes, 0, 0);
    }

    sockaddr_in LocalAddress;
    memset(&LocalAddress, 0, sizeof(sockaddr_in));

    LocalAddress.sin_family = AF_INET;
    LocalAddress.sin_addr.S_un.S_addr = INADDR_ANY;

    if(bind(Internal.Socket, (sockaddr *) &LocalAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        int Code = LacewingGetSocketError();

        LacewingAssert(false);
    }

    if(!lw_ConnectEx (Internal.Socket, (sockaddr *) &Internal.HostStructure, sizeof(sockaddr_in),
                0, 0, 0, (OVERLAPPED *) &Internal.Overlapped))
    {
        int Code = LacewingGetSocketError();

        LacewingAssert(Code == WSA_IO_PENDING);
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
    Internal.Socket = SOCKET_ERROR;
}

void Lacewing::Client::Send(const char * Data, int Size)
{
    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    if(!Internal.Connected)
        return;

    if(Size == -1)
        Size = strlen(Data);

    WSABUF SendBuffer = { Size, (CHAR *) Data };

    ClientOverlapped &Overlapped = Internal.OverlappedBacklog.Borrow(Internal);    

    memset(&Overlapped, 0, sizeof(OVERLAPPED));
    Overlapped.IsSend = true;

    if(WSASend(Internal.Socket, &SendBuffer, 1, 0, 0, (OVERLAPPED *) &Overlapped, 0) != 0)
    {
        int Error = WSAGetLastError();

        if(Error != WSA_IO_PENDING)
        {
            Internal.OverlappedBacklog.Return(Overlapped);
        }
    }
}

Lacewing::Address &Lacewing::Client::ServerAddress()
{
    return *((ClientInternal *) InternalTag)->Address;
}

void Lacewing::Client::DisableNagling()
{
    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    Internal.Nagle = false;

    if(Internal.Socket != SOCKET_ERROR)
        ::DisableNagling(Internal.Socket);
}

bool Lacewing::Client::CheapBuffering ()
{
    return false;
}

void Lacewing::Client::StartBuffering()
{
    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    if(Internal.BufferingOutput)
        return;

    Internal.BufferingOutput = true;
}

void Lacewing::Client::Flush()
{
    ClientInternal &Internal = *(ClientInternal *) InternalTag;

    if(!Internal.BufferingOutput)
        return;

    Internal.BufferingOutput = false;
    Send(Internal.OutputBuffer.Buffer, Internal.OutputBuffer.Size);

    Internal.OutputBuffer.Reset();
}

AutoHandlerFunctions(Lacewing::Client, ClientInternal, Connect)
AutoHandlerFunctions(Lacewing::Client, ClientInternal, Disconnect)
AutoHandlerFunctions(Lacewing::Client, ClientInternal, Receive)
AutoHandlerFunctions(Lacewing::Client, ClientInternal, Error)

