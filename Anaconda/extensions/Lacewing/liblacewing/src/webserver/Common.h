
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
#include "Map.h"

class HTTPClient;

struct UploadInternal
{
    Lacewing::Webserver::Upload Upload;

    const char * FormElement;
    const char * Filename;
    
    const char * AutoSaveFilename;
    FILE * AutoSaveFile;

    Map Headers, Copier;

    inline UploadInternal()
    {
        Upload.InternalTag = this;
        Upload.Tag         = 0;

        AutoSaveFilename = "";
        AutoSaveFile     = 0;
    }

    inline ~ UploadInternal()
    {
        DebugOut("Free upload!");
    }

    virtual const char * Header (const char * Name) = 0;
};

class WebserverInternal
{
public:

    PumpInternal &EventPump;

    const static size_t SendBufferSize    = 1024 * 32;  /* Maximum of 32 KB wasted per SendFile/SendConstant */
    const static size_t SendBufferBacklog = 32;         /* 256 KB allocated initially and when the server runs out */

    Lacewing::Server * Socket, * SecureSocket;
    Lacewing::Timer Timer;

    List <char *> SendBuffers;

    char * BorrowSendBuffer();
    void ReturnSendBuffer(char * SendBuffer);

    struct Session
    {
        lw_i64 ID_Part1;
        lw_i64 ID_Part2;

        Session * Next;

        Map Data;

    } * FirstSession;

    Session * FindSession (const char * SessionID_Hex);

    bool AutoFinish;

    static void SocketConnect(Lacewing::Server &, Lacewing::Server::Client &);
    static void SocketDisconnect(Lacewing::Server &, Lacewing::Server::Client &);
    static void SocketReceive(Lacewing::Server &, Lacewing::Server::Client &, char * Buffer, int Size);
    static void SocketError(Lacewing::Server &, Lacewing::Error &);

    inline void PrepareSocket()
    {
        if(!Socket)
        {
            Socket = new Lacewing::Server(EventPump.Pump);

            Socket->Tag = this;

            Socket->onConnect    (SocketConnect);
            Socket->onDisconnect (SocketDisconnect);
            Socket->onReceive    (SocketReceive);
            Socket->onError      (SocketError);
        }

        StartTimer ();
    }

    inline void PrepareSecureSocket()
    {
        if(!SecureSocket)
        {
            SecureSocket = new Lacewing::Server(EventPump.Pump);
    
            SecureSocket->Tag = this;

            SecureSocket->onConnect    (SocketConnect);
            SecureSocket->onDisconnect (SocketDisconnect);
            SecureSocket->onReceive    (SocketReceive);
            SecureSocket->onError      (SocketError);
        }
        
        StartTimer ();
    }

    int Timeout;

    inline void StartTimer ()
    {
        #ifdef LacewingTimeoutExperiment

            if (Timer.Started())
                return;

            Timer.Start (Timeout * 1000);
        
        #endif
    }

    inline void StopTimer ()
    {
        Timer.Stop ();
    }

    Lacewing::Webserver &Webserver;

    Lacewing::Webserver::HandlerError        HandlerError;
    Lacewing::Webserver::HandlerGet          HandlerGet;
    Lacewing::Webserver::HandlerPost         HandlerPost;
    Lacewing::Webserver::HandlerHead         HandlerHead;
    Lacewing::Webserver::HandlerUploadStart  HandlerUploadStart;
    Lacewing::Webserver::HandlerUploadChunk  HandlerUploadChunk;
    Lacewing::Webserver::HandlerUploadDone   HandlerUploadDone;
    Lacewing::Webserver::HandlerUploadPost   HandlerUploadPost;
    Lacewing::Webserver::HandlerDisconnect   HandlerDisconnect;

    inline WebserverInternal(Lacewing::Webserver &_Webserver, PumpInternal &_EventPump)
            : Webserver(_Webserver), EventPump(_EventPump), Timer (_EventPump.Pump)
    {
        Socket = SecureSocket = 0;

        HandlerError        = 0;
        HandlerGet          = 0;
        HandlerPost         = 0;
        HandlerHead         = 0;
        HandlerUploadStart  = 0;
        HandlerUploadChunk  = 0;
        HandlerUploadDone   = 0;
        HandlerUploadPost   = 0;
        HandlerDisconnect   = 0;

        AutoFinish = true;
        FirstSession = 0;

        Timeout = 5;

        Timer.Tag = this;
        Timer.onTick (TimerTickStatic);
    }

    static void TimerTickStatic (Lacewing::Timer &);
    void TimerTick ();
};

class WebserverClient;

struct RequestInternal
{
    Lacewing::Webserver::Request Public; 

    void * Tag;

    WebserverInternal &Server;
    WebserverClient   &Client;

    RequestInternal (WebserverInternal &_Server, WebserverClient &_Client);
    ~ RequestInternal ();

    void Clean ();
    

    /* Input */

    char Method     [8];
    char Version    [16];
    char URL        [4096];
    char Hostname   [128];
    
    Map InHeaders, InCookies, GetItems, PostItems;
    
    void ProcessHeader (const char * Name, char * Value);
    bool ProcessURL (char * URL);


    /* Output */

    char Status [64];

    Map OutHeaders, OutCookies;
    
    struct File
    {
        char Filename [lw_max_path];
        unsigned int Offset;

        File * Next;

        void Send (Lacewing::Server::Client &Socket, int MaxOutput, bool &Flushed);
        
        lw_i64 FileOffset, FileSize;
    };

    MessageBuilder Response;

    File * FirstFile;
    lw_i64 TotalFileSize, TotalNonFileSize;

    void BeforeHandler ();
    void AfterHandler ();

    void RunStandardHandler ();    

    void AddFileSend (const char * Filename, lw_i64 FileOffset, lw_i64 FileSize);

    bool Responded;
    void Respond ();
};

class WebserverClient
{
public:

    bool Secure;

    Lacewing::Server::Client &Socket;
    WebserverInternal &Server;

    int Timeout;

    WebserverClient (WebserverInternal &, Lacewing::Server::Client &, bool Secure);
    
    virtual void Tick () = 0;

    virtual void Process (char * Buffer, int Size) = 0;
    virtual void Respond (RequestInternal &Request) = 0;
    virtual void Dead () = 0;
    
    virtual bool IsSPDY () = 0;
};

#include "http/HTTP.h"


