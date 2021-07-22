
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

RequestInternal::RequestInternal (WebserverInternal &_Server, WebserverClient &_Client)
    : Server(_Server), Client (_Client)
{
    Public.Tag         = 0;
    Public.InternalTag = this;

    FirstFile = 0;
}

RequestInternal::~RequestInternal ()
{
}

void RequestInternal::Clean ()
{
    /* HTTP clients reuse the same request object, so this should clear
       everything ready for a new request. */
    
    Responded = true;
   
    InHeaders   .Clear();
    InCookies   .Clear();
    GetItems    .Clear();
    PostItems   .Clear();

    *Method   = 0;
    *Version  = 0;
    *URL      = 0;
    *Hostname = 0;
    
    /* The output cookies are cleared here rather than in BeforeHandler() because
       the received cookies will be written to OutCookies as well as InCookies.  When
       it comes to sending the response, OutCookies will be compared to InCookies to
       see which cookies have changed and should be sent as Set-Cookie headers. */

    OutCookies.Clear();
}

void RequestInternal::BeforeHandler ()
{
    /* Any preparation to be done immediately before calling the handler should be in this function */

    strcpy(Status, "200 OK");

    Response.Reset();     
    OutHeaders.Clear();
    
    TotalFileSize = TotalNonFileSize = 0;

    OutHeaders.Set ("Server", Lacewing::Version ());
    OutHeaders.Set ("Content-Type", "text/html; charset=UTF-8");

    if(Client.Secure)
    {
        /* When the request is secure, add the "Cache-Control: public" header by default.
           DisableCache() called in the handler will remove this, and should be used for any
           pages containing sensitive data that shouldn't be cached.  */

        OutHeaders.Set("Cache-Control", "public");
    }   

    LacewingAssert (Responded);

    Responded = false;
}

void RequestInternal::AfterHandler ()
{
    /* Anything to be done immediately after the handler has returned should be in this function */
    
    if((!Responded) && Server.AutoFinish)
        Respond ();
}

void RequestInternal::RunStandardHandler ()
{
    /* If the protocol doesn't want to call the handler itself (ie. it's a standard GET/POST/HEAD
       request), it will call this function to invoke the appropriate handler automatically. */

    BeforeHandler ();

    do
    {
        /* The BodyProcessor might want to call its own handler (eg for multipart file upload) */
        
        if(!strcmp(Method, "GET"))
        {
            if(Server.HandlerGet)
                Server.HandlerGet(Server.Webserver, Public);

            break;
        }

        if(!strcmp(Method, "POST"))
        {
            if(Server.HandlerPost)
                Server.HandlerPost(Server.Webserver, Public);

            break;
        }

        if(!strcmp(Method, "HEAD"))
        {
            if(Server.HandlerHead)
                Server.HandlerHead(Server.Webserver, Public);

            break;
        }

        Public.Status (501, "Not Implemented");
        Public.Finish ();

        return;

    } while(0);

    AfterHandler ();
}

void RequestInternal::ProcessHeader (const char * Name, char * Value)
{
    if(!strcasecmp(Name, "Cookie"))
    {
        for(;;)
        {
            char * CookieName  = Value;
            char * CookieValue = Value;

            if(!(CookieValue = strchr(CookieValue, '=')))
                break; /* invalid cookie */
            
            *(CookieValue ++) = 0;

            char * Next = strchr(CookieValue, ';');

            if(Next)
                *(Next ++) = 0;

            while(*CookieName == ' ')
                ++ CookieName;

            while(*CookieName && CookieName[strlen(CookieName - 1)] == ' ')
                CookieName [strlen(CookieName - 1)] = 0;

            /* The copy in InCookies doesn't get modified, so the response generator
               can determine which cookies have been changed. */

            InCookies.Set (CookieName, CookieValue);
            OutCookies.Set (CookieName, CookieValue);
            
            if(!(Value = Next))
                break;
        }

        return;
    }

    if(!strcasecmp(Name, "Host"))
    {
        /* The hostname gets stored separately with the port removed for
           the Request.Hostname() function (the raw header is still saved) */

        strncpy(this->Hostname, Value, sizeof (this->Hostname));

        for(char * i = this->Hostname; *i; ++ i)
        {
            if(*i == ':')
            {
                *i = 0;
                break;
            }
        }
    }   
    
    InHeaders.Set (Name, Value);
}

bool RequestInternal::ProcessURL (char * URL)
{
    /* Must be able to process both absolute and relative URLs (which may come from either SPDY or HTTP) */

    char * ProtocolEnd = strstr (URL, "://");

    if (ProtocolEnd)
    {
        /* Absolute URL */

        URL += 3;

        char * HostnameEnd = strchr (URL, '/');

        if (!HostnameEnd)
            return false;

        *HostnameEnd ++ = 0;

        ProcessHeader ("Host", URL);

        if (!*HostnameEnd)
            return false;
        
        URL = HostnameEnd;
    }
    else
    {
        /* Relative URL */
           
        if (*URL == '/')
            ++ URL;
    }

    /* Strip the GET data from the URL and add it to GetItems, decoded */

    char * GetData = strchr(URL, '?');

    if(GetData)
    {
        *(GetData ++) = 0;

        for(;;)
        {
            char * Name = GetData;
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
                GetItems.Set (NameDecoded, ValueDecoded, false);

            if(!Next)
                break;

            GetData = Next;
        }
    }

    /* Make an URL decoded copy of the URL with the GET data stripped */
    
    if(!URLDecode(URL, this->URL, sizeof(this->URL)))
        return false;

    /* Check for any directory traversal in the URL, and replace any backward
       slashes with forward slashes. */

    for(char * i = this->URL; *i; ++ i)
    {
        if(i[0] == '.' && i[1] == '.')
            return false;

        if(*i == '\\')
            *i = '/';
    }

    return true;
}

void RequestInternal::AddFileSend(const char * Filename, lw_i64 FileOffset, lw_i64 FileSize)
{
    RequestInternal::File * File = FirstFile;

    if(!File)
    {
        File = FirstFile = new RequestInternal::File;
    }
    else
    {
        while(File->Next)
            File = File->Next;

        File->Next = new RequestInternal::File;
        File = File->Next;
    }

    File->Next       = 0;
    File->Offset     = Response.Size;
    File->FileOffset = FileOffset;
    File->FileSize   = FileSize;

    strncpy (File->Filename, Filename, sizeof (File->Filename));
}

void RequestInternal::File::Send(Lacewing::Server::Client &Socket, int ToSend, bool &Flushed)
{
    if (ToSend != -1)
    {
        if(*Filename)
        {
            /* TODO: Hmm, it's a shame it has to reopen the file every time the SPDY
               window is filled */

            Socket.SendFile (Filename, FileOffset, ToSend);
            Flushed = true;
            
            FileOffset += ToSend;
            FileSize   -= ToSend;
            
            return;
        }

        Socket.Send ((char *) FileOffset, ToSend);
            
        FileOffset += ToSend;
        FileSize   -= ToSend;

        return;
    }

    if(*Filename)
    {
        Socket.SendFile (Filename, FileOffset, FileSize);
        Flushed = true;

        return;
    }

    Socket.Send ((char *) FileOffset, (int) FileSize);
}

void RequestInternal::Respond ()
{
    LacewingAssert (!Responded);

    Client.Respond (*this);
    Responded = true;
}

Lacewing::Address &Lacewing::Webserver::Request::GetAddress()
{
    return ((RequestInternal *) InternalTag)->Client.Socket.GetAddress();
}

void Lacewing::Webserver::Request::Disconnect()
{
    ((RequestInternal *) InternalTag)->Client.Socket.Disconnect();
}

void Lacewing::Webserver::Request::Send(const char * Data, int Size)
{
    RequestInternal &Internal = *(RequestInternal *) InternalTag;

    if(Size == -1)
        Size = strlen(Data);

    if (Size <= 0)
        return;

    Internal.TotalNonFileSize += Size;
    Internal.Response.Add (Data, Size);
}

void Lacewing::Webserver::Request::SendConstant(const char * Data, int Size)
{
    RequestInternal &Internal = *(RequestInternal *) InternalTag;

    if(Size == -1)
        Size = strlen(Data);

    if (Size <= 0)
        return;

    Internal.TotalNonFileSize += Size;
    Internal.AddFileSend("", (lw_i64) Data, Size);
}

void Lacewing::Webserver::Request::SendFile(const char * Filename, lw_i64 Offset, lw_i64 Size)
{
    RequestInternal &Internal = *(RequestInternal *) InternalTag;

    if (!*Filename)
        return;

    if (Size == -1)
        Size = Lacewing::FileSize (Filename);

    if (Size <= 0)
        return;

    Internal.TotalFileSize += Size;
    Internal.AddFileSend (Filename, Offset, Size);
}

void Lacewing::Webserver::Request::Reset()
{
    RequestInternal &Internal = *(RequestInternal *) InternalTag;

    Internal.Response.Reset ();
    
    {   RequestInternal::File * File = Internal.FirstFile;

        while(File)
        {
            delete File;
            File = File->Next;
        }

        Internal.FirstFile = 0;
    }

    Internal.TotalFileSize = 0;
    Internal.TotalNonFileSize = 0;
}

void Lacewing::Webserver::Request::GuessMimeType(const char * Filename)
{
    SetMimeType(Lacewing::GuessMimeType(Filename));
}

void Lacewing::Webserver::Request::SetMimeType(const char * MimeType, const char * Charset)
{
     if (!*Charset)
     {
         Header ("Content-Type", MimeType);
         return;
     }

     char Type [256];

     lw_snprintf (Type, sizeof (Type), "%s; charset=%s", MimeType, Charset);
     Header ("Content-Type", Type);
}

void Lacewing::Webserver::Request::SetRedirect(const char * URL)
{
    Status (303, "See Other");
    Header ("Location", URL);
}

void Lacewing::Webserver::Request::DisableCache()
{
    Header("Cache-Control", "no-cache");
}

void Lacewing::Webserver::Request::Header(const char * Name, const char * Value)
{
    ((RequestInternal *) InternalTag)->OutHeaders.Set(Name, Value);
}

void Lacewing::Webserver::Request::Cookie(const char * Name, const char * Value)
{
    Cookie (Name, Value, Secure () ? "Secure; HttpOnly" : "HttpOnly");
}

void Lacewing::Webserver::Request::Cookie(const char * Name, const char * Value, const char * Attributes)
{
    if (!*Attributes)
    {
        ((RequestInternal *) InternalTag)->OutCookies.Set (Name, Value);
        return;
    }

    char * Buffer = (char *) malloc (strlen (Value) + strlen (Attributes) + 4);

    strcpy (Buffer, Value);
    strcat (Buffer, "; ");
    strcat (Buffer, Attributes);

    ((RequestInternal *) InternalTag)->OutCookies.Set (strdup (Name), Buffer, false);
}

void Lacewing::Webserver::Request::Status(int Code, const char * Message)
{
    sprintf(((RequestInternal *) InternalTag)->Status, "%d %s", Code, Message);
}

void Lacewing::Webserver::Request::SetUnmodified()
{
    Status (304, "Not Modified");
}

void Lacewing::Webserver::Request::LastModified(lw_i64 Time)
{
    tm TM;

    time_t TimeT = (time_t) Time;
    gmtime_r(&TimeT, &TM);

    char LastModified[128];
    sprintf(LastModified, "%s, %02d %s %d %02d:%02d:%02d GMT", Weekdays[TM.tm_wday], TM.tm_mday,
                            Months[TM.tm_mon], TM.tm_year + 1900, TM.tm_hour, TM.tm_min, TM.tm_sec);

    Header("Last-Modified", LastModified);
}

void Lacewing::Webserver::Request::Finish()
{
    ((RequestInternal *) InternalTag)->Respond ();
}

const char * Lacewing::Webserver::Request::Header(const char * Name)
{
    return ((RequestInternal *) InternalTag)->InHeaders.Get(Name);
}

struct Lacewing::Webserver::Request::Header * Lacewing::Webserver::Request::FirstHeader ()
{
    return (struct Lacewing::Webserver::Request::Header *)
                ((RequestInternal *) InternalTag)->InHeaders.First;
}

const char * Lacewing::Webserver::Request::Header::Name ()
{
    return ((Map::Item *) this)->Key;
}

const char * Lacewing::Webserver::Request::Header::Value ()
{
    return ((Map::Item *) this)->Value;
}

struct Lacewing::Webserver::Request::Header * Lacewing::Webserver::Request::Header::Next ()
{
    return (struct Lacewing::Webserver::Request::Header *) ((Map::Item *) this)->Next;
}

const char * Lacewing::Webserver::Request::Cookie(const char * Name)
{
    return ((RequestInternal *) InternalTag)->InCookies.Get(Name);
}

struct Lacewing::Webserver::Request::Cookie * Lacewing::Webserver::Request::FirstCookie ()
{
    return (struct Lacewing::Webserver::Request::Cookie *)
                ((RequestInternal *) InternalTag)->InCookies.First;
}

struct Lacewing::Webserver::Request::Cookie * Lacewing::Webserver::Request::Cookie::Next ()
{
    return (struct Lacewing::Webserver::Request::Cookie *) ((Map::Item *) this)->Next;
}

const char * Lacewing::Webserver::Request::Cookie::Name ()
{
    return ((Map::Item *) this)->Key;
}

const char * Lacewing::Webserver::Request::Cookie::Value ()
{
    return ((Map::Item *) this)->Value;
}

const char * Lacewing::Webserver::Request::GET(const char * Name)
{
    return ((RequestInternal *) InternalTag)->GetItems.Get(Name);
}

const char * Lacewing::Webserver::Request::POST(const char * Name)
{
    return ((RequestInternal *) InternalTag)->PostItems.Get(Name);
}

Lacewing::Webserver::Request::Parameter * Lacewing::Webserver::Request::GET ()
{
    return (Lacewing::Webserver::Request::Parameter *)
                ((RequestInternal *) InternalTag)->GetItems.First;
}

Lacewing::Webserver::Request::Parameter * Lacewing::Webserver::Request::POST ()
{
    return (Lacewing::Webserver::Request::Parameter *)
                ((RequestInternal *) InternalTag)->PostItems.First;
}

Lacewing::Webserver::Request::Parameter *
        Lacewing::Webserver::Request::Parameter::Next ()
{
    return (Lacewing::Webserver::Request::Parameter *) ((Map::Item *) this)->Next;
}

const char * Lacewing::Webserver::Request::Parameter::Name ()
{
    return ((Map::Item *) this)->Key;
}

const char * Lacewing::Webserver::Request::Parameter::Value ()
{
    return ((Map::Item *) this)->Value;
}

lw_i64 Lacewing::Webserver::Request::LastModified()
{
    const char * LastModified = Header("If-Modified-Since");

    if(*LastModified)
        return ParseTimeString(LastModified);

    return 0;
}

bool Lacewing::Webserver::Request::Secure()
{
    return ((RequestInternal *) InternalTag)->Client.Secure;
}

const char * Lacewing::Webserver::Request::Hostname()
{
    return ((RequestInternal *) InternalTag)->Hostname;
}

const char * Lacewing::Webserver::Request::URL()
{
    return ((RequestInternal *) InternalTag)->URL;
}

int Lacewing::Webserver::Request::IdleTimeout ()
{
    return ((RequestInternal *) InternalTag)->Client.Timeout;
}

void Lacewing::Webserver::Request::IdleTimeout (int Seconds)
{
    ((RequestInternal *) InternalTag)->Client.Timeout = Seconds;
}
