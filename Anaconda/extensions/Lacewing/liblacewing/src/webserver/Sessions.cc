
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

const char * const SessionCookie = "LacewingSession";

void Lacewing::Webserver::Request::Session(const char * Key, const char * Value)
{
    RequestInternal &Internal = *((RequestInternal *) InternalTag);

    WebserverInternal::Session * Session = Internal.Server.FindSession (Cookie (SessionCookie));

    if (!Session)
    {
        char SessionID [128];

        sprintf(SessionID, "Session-%s-%d%d%d",
            Internal.Client.Socket.GetAddress().ToString(), (int) time(0),
                rand(), (int) (lw_iptr) this);

        Lacewing::MD5 (SessionID, SessionID);

        {   char SessionID_Hex [40];
            
            for(int i = 0; i < 16; ++ i)
                sprintf (SessionID_Hex + (i * 2), "%02x", ((unsigned char *) SessionID) [i]);

            Cookie (SessionCookie, SessionID_Hex);
        }

        Session = new WebserverInternal::Session;

        Session->ID_Part1 = ((lw_i64 *) SessionID) [0];
        Session->ID_Part2 = ((lw_i64 *) SessionID) [1];

        if (Internal.Server.FirstSession)
        {
            Session->Next = Internal.Server.FirstSession;
            Internal.Server.FirstSession = Session;
        }
        else
        {
            Session->Next = 0;
            Internal.Server.FirstSession = Session;
        }
    }

    Session->Data.Set (Key, Value);
}

const char * Lacewing::Webserver::Request::Session(const char * Key)
{
    WebserverInternal::Session * Session = ((RequestInternal *) InternalTag)->Server
                                                    .FindSession (Cookie (SessionCookie));
    if (!Session)
        return "";

    return Session->Data.Get (Key);
}

void Lacewing::Webserver::CloseSession (const char * ID)
{
    RequestInternal &Internal = *((RequestInternal *) InternalTag);

    WebserverInternal::Session * Session = Internal.Server.FindSession (ID);

    if (Session == Internal.Server.FirstSession)
        Internal.Server.FirstSession = Session->Next;
    else
    {
        for (WebserverInternal::Session * S = Internal.Server.FirstSession; S; S = S->Next)
        {
            if (S->Next == Session)
            {
                S->Next = Session->Next;
                break;
            }
        }
    }

    delete Session;
}

void Lacewing::Webserver::Request::CloseSession()
{
    ((RequestInternal *) InternalTag)->Server.Webserver.CloseSession(Session ());
}

const char * Lacewing::Webserver::Request::Session()
{
    return Cookie (SessionCookie);
}

WebserverInternal::Session * WebserverInternal::FindSession (const char * SessionID_Hex)
{
    if (strlen (SessionID_Hex) != 32)
        return 0;

    union
    {
        char SessionID_Bytes [16];
        
        struct
        {
            lw_i64 Part1;
            lw_i64 Part2;

        } SessionID;
    };

    char hex [3];
    hex [2] = 0;

    for (int i = 0, c = 0; i < 16; ++ i, c += 2)
    {
        hex [0] = SessionID_Hex [c];
        hex [1] = SessionID_Hex [c + 1];

        SessionID_Bytes [i] = (char) strtol (hex, 0, 16);
    }

    WebserverInternal::Session * Session;

    for (Session = FirstSession; Session; Session = Session->Next)
    {
        if (Session->ID_Part1 == SessionID.Part1 &&
                Session->ID_Part2 == SessionID.Part2)
        {
            break;
        }
    }

    return Session;
}


Lacewing::Webserver::Request::SessionItem * Lacewing::Webserver::Request::FirstSessionItem ()
{
    RequestInternal &Internal = *(RequestInternal *) InternalTag;

    WebserverInternal::Session * Session = Internal.Server.FindSession (Cookie (SessionCookie));

    if (!Session)
        return 0;

    return (Lacewing::Webserver::Request::SessionItem *) Session->Data.First;
}

Lacewing::Webserver::Request::SessionItem *
        Lacewing::Webserver::Request::SessionItem::Next ()
{
    return (Lacewing::Webserver::Request::SessionItem *) ((Map::Item *) this)->Next;
}

const char * Lacewing::Webserver::Request::SessionItem::Name ()
{
    return ((Map::Item *) this)->Key;
}

const char * Lacewing::Webserver::Request::SessionItem::Value ()
{
    return ((Map::Item *) this)->Value;
}

