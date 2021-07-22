
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

WebserverClient::WebserverClient(WebserverInternal &_Server, Lacewing::Server::Client &_Socket, bool _Secure)
    : Server (_Server), Socket (_Socket), Secure (_Secure), Timeout (_Server.Timeout)
{
}

char * WebserverInternal::BorrowSendBuffer()
{
    if(!SendBuffers.First)
        for(int i = WebserverInternal::SendBufferBacklog; i; -- i)
            SendBuffers.Push (new char [WebserverInternal::SendBufferSize]);

    return SendBuffers.Pop ();
}

void WebserverInternal::ReturnSendBuffer(char * SendBuffer)
{
    SendBuffers.Push (SendBuffer);
}

void WebserverInternal::SocketConnect(Lacewing::Server &Server, Lacewing::Server::Client &Client)
{
}

void WebserverInternal::SocketDisconnect(Lacewing::Server &Server, Lacewing::Server::Client &Client)
{
    if (!Client.Tag)
        return;
    
    ((WebserverClient *) Client.Tag)->Dead ();

    delete ((WebserverClient *) Client.Tag);
}

void WebserverInternal::SocketReceive(Lacewing::Server &Server, Lacewing::Server::Client &Client, char * Buffer, int Size)
{
    WebserverInternal &Webserver = *(WebserverInternal *) Server.Tag;

    if (!Client.Tag)
        Client.Tag = new HTTPClient (Webserver, Client, &Server == Webserver.SecureSocket);

    ((WebserverClient *) Client.Tag)->Process(Buffer, Size);
}

void WebserverInternal::SocketError(Lacewing::Server &Server, Lacewing::Error &Error)
{
    Error.Add("Socket error");

    WebserverInternal &Webserver = *(WebserverInternal *) Server.Tag;

    if(Webserver.HandlerError)
        Webserver.HandlerError(Webserver.Webserver, Error);
}

void WebserverInternal::TimerTickStatic (Lacewing::Timer &Timer)
{
    ((WebserverInternal *) Timer.Tag)->TimerTick ();
}

void WebserverInternal::TimerTick ()
{
    if (Socket)
    {
        for (Lacewing::Server::Client * Client =
                Socket->FirstClient (); Client; Client = Client->Next())
        {
            if (!Client->Tag)
                continue;

            ((WebserverClient *) Client->Tag)->Tick ();
        }
    }

    if (SecureSocket)
    {
        for (Lacewing::Server::Client * Client =
                SecureSocket->FirstClient (); Client; Client = Client->Next())
        {
            if (!Client->Tag)
                continue;

            ((WebserverClient *) Client->Tag)->Tick ();
        }
    }
}

Lacewing::Webserver::Webserver(Lacewing::Pump &EventPump)
{
    LacewingInitialise();
    
    InternalTag = new WebserverInternal(*this, *(PumpInternal *) EventPump.InternalTag);
    Tag         = 0;
}

Lacewing::Webserver::~Webserver()
{
    Unhost();
    UnhostSecure();

    delete ((WebserverInternal *) InternalTag);
}

void Lacewing::Webserver::Host(int Port)
{
    Lacewing::Filter Filter;
    Filter.LocalPort(Port);

    Host(Filter);
}

void Lacewing::Webserver::Host(Lacewing::Filter &Filter)
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);
    Internal.PrepareSocket();

    if(!Filter.LocalPort())
        Filter.LocalPort(80);

    Internal.Socket->Host(Filter, true);
}

void Lacewing::Webserver::HostSecure(int Port)
{
    Lacewing::Filter Filter;
    Filter.LocalPort(Port);

    HostSecure(Filter);
}

void Lacewing::Webserver::HostSecure(Lacewing::Filter &Filter)
{
    if(!CertificateLoaded())
        return;

    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);
    Internal.PrepareSecureSocket();

    if(!Filter.LocalPort())
        Filter.LocalPort(443);

    Internal.SecureSocket->Host(Filter, true);
}

void Lacewing::Webserver::Unhost()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    if(Internal.Socket)
        Internal.Socket->Unhost();
}

void Lacewing::Webserver::UnhostSecure()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    if(Internal.SecureSocket)
        Internal.SecureSocket->Unhost();
}

bool Lacewing::Webserver::Hosting()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    if(!Internal.Socket)
        return false;

    return Internal.Socket->Hosting();
}

bool Lacewing::Webserver::HostingSecure()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    if(!Internal.SecureSocket)
        return false;

    return Internal.SecureSocket->Hosting();
}

int Lacewing::Webserver::Port()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    if(!Internal.Socket)
        return 0;

    return Internal.Socket->Port();
}

int Lacewing::Webserver::SecurePort()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    if(!Internal.SecureSocket)
        return 0;

    return Internal.SecureSocket->Port();
}

lw_i64 Lacewing::Webserver::BytesSent()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    return (Internal.Socket ? Internal.Socket->BytesSent() : 0)
        + (Internal.SecureSocket ? Internal.SecureSocket->BytesSent() : 0);
}

lw_i64 Lacewing::Webserver::BytesReceived()
{
   WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    return (Internal.Socket ? Internal.Socket->BytesReceived() : 0)
        + (Internal.SecureSocket ? Internal.SecureSocket->BytesReceived() : 0);
}

bool Lacewing::Webserver::LoadCertificateFile(const char * Filename, const char * CommonName)
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);
    Internal.PrepareSecureSocket();

    return Internal.SecureSocket->LoadCertificateFile(Filename, CommonName);
}

bool Lacewing::Webserver::LoadSystemCertificate(const char * StoreName, const char * CommonName, const char * Location)
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);
    Internal.PrepareSecureSocket();

    return Internal.SecureSocket->LoadSystemCertificate(StoreName, CommonName, Location);
}

bool Lacewing::Webserver::CertificateLoaded()
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);
    Internal.PrepareSecureSocket();

    return Internal.SecureSocket->CertificateLoaded();
}

void Lacewing::Webserver::EnableManualRequestFinish()
{
    ((WebserverInternal *) InternalTag)->AutoFinish = false;
}

void Lacewing::Webserver::IdleTimeout (int Seconds)
{
    WebserverInternal &Internal = *((WebserverInternal *) InternalTag);

    Internal.Timeout = Seconds;

    if (Internal.Timer.Started())
    {
        Internal.StopTimer ();
        Internal.StartTimer ();
    }
}

int Lacewing::Webserver::IdleTimeout ()
{
    return ((WebserverInternal *) InternalTag)->Timeout;
}

const char * Lacewing::Webserver::Upload::Filename()
{
    return ((UploadInternal *) InternalTag)->Filename;
}

const char * Lacewing::Webserver::Upload::FormElementName()
{
    return ((UploadInternal *) InternalTag)->FormElement;
}

const char * Lacewing::Webserver::Upload::Header(const char * Name)
{
    return ((UploadInternal *) InternalTag)->Header (Name);
}

struct Lacewing::Webserver::Upload::Header * Lacewing::Webserver::Upload::FirstHeader ()
{
    return (struct Lacewing::Webserver::Upload::Header *)
                ((UploadInternal *) InternalTag)->Headers.First;
}

const char * Lacewing::Webserver::Upload::Header::Name ()
{
    return ((Map::Item *) this)->Key;
}

const char * Lacewing::Webserver::Upload::Header::Value ()
{
    return ((Map::Item *) this)->Value;
}

struct Lacewing::Webserver::Upload::Header * Lacewing::Webserver::Upload::Header::Next ()
{
    return (struct Lacewing::Webserver::Upload::Header *) ((Map::Item *) this)->Next;
}

void Lacewing::Webserver::Upload::SetAutoSave()
{
    UploadInternal &Internal = *((UploadInternal *) InternalTag);

    if(*Internal.AutoSaveFilename)
        return;

    char Filename [lw_max_path];
    NewTempFile (Filename);

    Internal.AutoSaveFilename = Internal.Copier.Set("AutoSaveFilename", Filename)->Value;
    Internal.AutoSaveFile     = fopen(Filename, "wb");
}

const char * Lacewing::Webserver::Upload::GetAutoSaveFilename()
{
    return ((UploadInternal *) InternalTag)->AutoSaveFilename;
}

AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, Get)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, Post)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, Head)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, Error)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, UploadStart)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, UploadChunk)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, UploadDone)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, UploadPost)
AutoHandlerFunctions(Lacewing::Webserver, WebserverInternal, Disconnect)

