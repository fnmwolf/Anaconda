
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

HTTPClient::HTTPClient (WebserverInternal &_Server, Lacewing::Server::Client &_Socket, bool Secure)
    : Request (_Server, *this), WebserverClient (_Server, _Socket, Secure)
{
    Multipart = 0;
    Reset ();
}

HTTPClient::~HTTPClient ()
{
    delete Multipart;
}

/* Called upon initialisation, and at the end of each request to prepare for a new one */

void HTTPClient::Reset()
{
    Request.Clean ();
    
    State = 0;
    BodyRemaining = 0;
}

void HTTPClient::Process (char * Buffer, int Size)
{
    if(!Request.Responded)
    {
        /* The application hasn't yet called Finish() for the last request, so this data
           can't be processed.  Buffer it to process when Finish() is called. */

        this->Buffer.Add(Buffer, Size);
        return;
    }

    /* TODO: A naughty client could keep the connection open by sending 1 byte every 5 seconds */

    LastActivityTime = time (0);


    /* State 0 : Line content
       State 1 : Got CR, need LF
       State 2 : Message body */

    if(State < 2)
    {
        for(char * i = Buffer; *i; )
        {
            if(State == 0 && *i == '\r')
            {
                State = 1;
            }
            else if(*i == '\n')
            {
                i [State == 1 ? -1 : 0] = 0;

                if(this->Buffer.Size)
                {
                    this->Buffer.Add (Buffer, -1);
                    this->Buffer.Add <char> (0);

                    ProcessLine(this->Buffer.Buffer);

                    this->Buffer.Reset();
                }
                else
                {
                    ProcessLine(Buffer);
                }

                Size -= (++ i) - Buffer;
                Buffer = i;

                if(State == -1 || State >= 2)
                    break;

                State = 0;
                continue;
            }
            else if(State == 1)
            {
                /* The only thing valid after \r is \n */

                State = -1;
                break;
            }
                
            ++ i;
        }
            
        if(State == -1)
        {
            Socket.Disconnect();
            return;
        }

        if(State < 2)
        {
            this->Buffer.Add(Buffer, Size);
            return;
        }
    }

    /* State >= 2 is the request body */

    if(Multipart)
    {
        Multipart->Process(Buffer, Size);
     
        if(Multipart->State == MultipartProcessor::Done)
        {
            Multipart->CallRequestHandler ();
            
            delete Multipart;
            Multipart = 0;

            return;
        }

        if(Multipart->State == MultipartProcessor::Error)
        {       
            DebugOut("Multipart reported error - killing client");
            Request.Public.Disconnect();

            return;
        }

        return;
    }

    /* No multipart = standard form post data */

    int ToRead = (int) (BodyRemaining < Size ? BodyRemaining : Size);
    this->Buffer.Add(Buffer, ToRead);

    BodyRemaining -= ToRead;
    Size -= ToRead;
    
    if(BodyRemaining == 0)
    {
        this->Buffer.Add<char>(0);
        char * PostData = this->Buffer.Buffer;

        /* Parse the POST data into name/value pairs.  Using BeginsWith to check the Content-Type to
           exclude the charset if specified (eg. application/x-www-form-urlencoded; charset=UTF-8) */

        if(*PostData)
        {
            for(;;)
            {
                char * Name = PostData;
                char * Value = strchr(Name, '=');

                if(!Value)
                    break;

                *(Value ++) = 0;

                char * Next = strchr(Value, '&');
                
                if(Next)
                    *(Next ++) = 0;

                char * NameDecoded = (char *) malloc(strlen(Name) + 1);
                char * ValueDecoded = (char *) malloc(strlen(Value) + 1);

                if(!URLDecode(Name, NameDecoded, strlen(Name) + 1) || !URLDecode(Value, ValueDecoded, strlen(Value) + 1))
                {
                    free(NameDecoded);
                    free(ValueDecoded);
                }
                else
                    Request.PostItems.Set (NameDecoded, ValueDecoded, false);

                if(!Next)
                    break;

                PostData = Next;
            }
        }

        this->Buffer.Reset();
        
        Request.RunStandardHandler();

        /* Now we wait for Finish() to call Respond().  It may have done so already, or the application
           may call Finish() later. */
    }

    /* If Size is > 0 here, the client must be pipelining requests. */

    if(Size > 0)
        Process(Buffer, Size);
}

void HTTPClient::ProcessLine(char * Line)
{
    do
    {   if(!*Request.Method)
        {
            ProcessFirstLine(Line);
            break;
        }

        if(*Line)
        {
            ProcessHeader(Line);
            break;
        }

        /* Blank line marks end of headers */

        if(!strcmp (Request.Method, "GET") || !strcmp (Request.Method, "HEAD") ||
                (BodyRemaining = _atoi64(Request.InHeaders.Get ("Content-Length"))) <= 0)
        {
            /* No body - this is a complete request done */
        
            Request.RunStandardHandler();
            break;
        }


        /* The request has a body.  BodyRemaining has been set, and definitely isn't 0. */

        const char * ContentType = Request.InHeaders.Get ("Content-Type");

        if(BeginsWith(ContentType, "application/x-www-form-urlencoded"))
        {
            State = 2;
            break;
        }

        if(BeginsWith(ContentType, "multipart"))
        {
            Multipart = new MultipartProcessor (*this, ContentType);
            
            State = 3;
            break;
        }

        /* Unknown content type */

        State = -1;
    }
    while(0);

    if(State == -1)
    {
        /* Parsing error */
        Request.Public.Disconnect();
    }
}

void HTTPClient::ProcessFirstLine(char * Line)
{
    /* Method (eg GET, POST, HEAD) */

    {   char * Method = Line;

        if(!(Line = strchr(Line, ' ')))
        {
            State = -1;
            return;
        }

        *(Line ++) = 0;

        strncpy(Request.Method, Method, sizeof(Request.Method));
    }

    /* URL */

    {   char * URL = Line;

        if(!(Line = strchr(Line, ' ')))
        {
            State = -1;
            return;
        }

        *(Line ++) = 0;

        if (!Request.ProcessURL (URL))
        {
            State = -1;
            return;
        }
    }

    /* HTTP version */

    if(strcasecmp(Line, "HTTP/1.1") && strcasecmp(Line, "HTTP/1.0"))
    {
        State = -1;
        return;
    }

    strcpy (Request.Version, Line);
}

void HTTPClient::ProcessHeader(char * Line)
{
    char * Name = Line;

    if(!(Line = strchr(Line, ':')))
    {
        State = -1;
        return;
    }

    *(Line ++) = 0;

    while(*Line == ' ')
        ++ Line;

    Request.ProcessHeader (Name, Line);
}

void HTTPClient::Respond(RequestInternal &) /* request parameter ignored - HTTP only ever has one request object per client */
{
    LastActivityTime = time (0);
    
    Socket.StartBuffering ();

    Socket << Request.Version << " " << Request.Status;
    
    for(Map::Item * Current = Request.OutHeaders.First; Current; Current = Current->Next)
        Socket << "\r\n" << Current->Key << ": " << Current->Value;

    for(Map::Item * Current = Request.OutCookies.First; Current; Current = Current->Next)
    {
        const char * OldValue = Request.InCookies.Get(Current->Key);
        
        int ValueSize = (int) (strchr (Current->Value, ';') - Current->Value);

        if (ValueSize < 0)
            ValueSize = strlen (Current->Value);

        if (ValueSize == strlen (OldValue) && memcmp (OldValue, Current->Value, ValueSize) == 0)
            continue;

        Socket << "\r\nSet-Cookie: " << Current->Key << "=" << Current->Value;
    }

    Socket << "\r\nContent-Length: " << (Request.TotalFileSize + Request.TotalNonFileSize);
    Socket << "\r\n\r\n";

    bool Flushed = false;

    if ((!Socket.CheapBuffering()) && Request.TotalNonFileSize > (1024 * 8))
    {
        Socket.Flush ();
        Flushed = true;
    }

    for (int Offset = 0 ;;)
    {
        RequestInternal::File * File = Request.FirstFile;

        if (File)
        {
            Socket.SendWritable (Request.Response.Buffer + Offset, File->Offset - Offset);
            
            File->Send (Socket, -1, Flushed);

            Offset = File->Offset;
            Request.FirstFile = File->Next;

            delete File;
                        
            if (Request.FirstFile)
                continue;
        }

        Socket.SendWritable (Request.Response.Buffer + Offset, Request.Response.Size - Offset);
        break;
    }

    Request.Response.Reset ();

    if (!Flushed)
        Socket.Flush ();

    /* Close the connection if this is HTTP/1.0 without a Connection: Keep-Alive header */

    if((!strcasecmp(Request.Version, "HTTP/1.0")) && strcasecmp(Request.InHeaders.Get("Connection"), "Keep-Alive"))
        Request.Public.Disconnect();
    
    Reset ();
}

void HTTPClient::Dead ()
{
    if (Request.Responded)
    {
        /* The request isn't unfinished - don't call the disconnect handler */
        return;
    }

    if (Server.HandlerDisconnect)
        Server.HandlerDisconnect (Server.Webserver, Request.Public);
}

bool HTTPClient::IsSPDY ()
{
    return false;
}

void HTTPClient::Tick ()
{
    if (Request.Responded && (time(0) - LastActivityTime) > Timeout)
    {
        DebugOut ("Dropping HTTP connection due to inactivity (%s/%d)", Socket.GetAddress().ToString(), &Socket.GetAddress());
        Socket.Disconnect ();
    }
}

