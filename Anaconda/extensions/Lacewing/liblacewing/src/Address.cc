
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

#ifdef HAVE_NETDB_H
    #include <netdb.h>
#endif

lw_iptr Ports [] =
{
    (lw_iptr) "http://",       80,
    (lw_iptr) "ftp://",        21,

    0
};

struct AddressInternal;

void Resolver (AddressInternal &);

struct AddressInternal
{
    Lacewing::Thread ResolverThread;

    char * Hostname;
    int Port;
    
    union
    {
        int IP;
        unsigned char Bytes[4];
    };

    AddressInternal (int _Port)
        : Port (_Port), ResolverThread ("Resolver", (void *) Resolver)
    {
        *StringIP = 0;
        IP = 0;
    }

    ~AddressInternal()
    {
        ResolverThread.Join();
    }

    char StringIP[32];

    void MakeStringIP()
    {
        if(!IP)
        {
            *StringIP = 0;
            return;
        }

        sprintf(StringIP, "%d.%d.%d.%d", Bytes[0], Bytes[1], Bytes[2], Bytes[3]);
    }

};

void Resolver (AddressInternal &Internal)
{
    hostent * Host;

    if(!(Host = gethostbyname(Internal.Hostname)))
    {
        Internal.IP = 0;
        free(Internal.Hostname);

        return;
    }

    Internal.IP = ((in_addr *) (Host->h_addr))->s_addr;
    Internal.MakeStringIP();
}

Lacewing::Address::Address()
{
    InternalTag = new AddressInternal(0);
    Tag = 0;

    AddressInternal &Internal = *(AddressInternal *) InternalTag;

    Internal.IP = 0;
    Internal.MakeStringIP();
}

Lacewing::Address::Address(const char * Hostname, int Port, bool Blocking)
{
    if(!*Hostname)
        Blocking = true;

    InternalTag = new AddressInternal(Port);
    Tag = 0;

    AddressInternal &Internal = *((AddressInternal *) InternalTag);

    char * Trimmed;
    Trim(Internal.Hostname = strdup(Hostname), Trimmed);

    if(!*Trimmed)
    {
        free(Internal.Hostname);
        return;
    }

    for(lw_iptr * i = Ports; *i; i += 2)
    {
        if(BeginsWith(Trimmed, (const char *) *i))
        {
            Internal.Port = i[1];
            Trimmed += strlen((const char *) *i);

            break;
        }
    }

    for(char * Iterator = Trimmed; *Iterator; ++ Iterator)
    {
        if(*Iterator != ':')
            continue;

        Internal.Port = atoi(Iterator + 1);
        *Iterator = 0;

        break;
    }

    if((Internal.IP = inet_addr(Trimmed)) != INADDR_NONE)
    {   
        Internal.MakeStringIP();
        free(Internal.Hostname);
    }
    else
    {
        Internal.IP = 0;

        if(Blocking)
        {
            Resolver (Internal);
            return;
        }

        Internal.ResolverThread.Start (&Internal);
    }
}

Lacewing::Address::Address(unsigned int IP, int Port)
{
    InternalTag = new AddressInternal(Port);
    Tag = 0;

    AddressInternal &Internal = *(AddressInternal *) InternalTag;

    Internal.IP = IP;
    Internal.MakeStringIP();
}

Lacewing::Address::Address(unsigned char IP_Byte1, unsigned char IP_Byte2,
                            unsigned char IP_Byte3, unsigned char IP_Byte4, int Port)
{
    InternalTag = new AddressInternal(Port);
    Tag = 0;

    AddressInternal &Internal = *(AddressInternal *) InternalTag;

    Internal.Bytes[0] = IP_Byte1;
    Internal.Bytes[1] = IP_Byte2;
    Internal.Bytes[2] = IP_Byte3;
    Internal.Bytes[3] = IP_Byte4;
    
    Internal.MakeStringIP();
}

Lacewing::Address::Address(const Lacewing::Address &Address)
{
    while(!Address.Ready())
        LacewingYield();

    InternalTag = new AddressInternal(Address.Port());
    Tag = 0;

    AddressInternal &Internal = *(AddressInternal *) InternalTag;

    Internal.IP = Address.IP();
    Internal.MakeStringIP();
}

Lacewing::Address::~Address()
{
    delete ((AddressInternal *) InternalTag);
}

int Lacewing::Address::Port() const
{
    if(!Ready())
        return -1;

    return ((AddressInternal *) InternalTag)->Port;
}

void Lacewing::Address::Port(int Port)
{
    if(!Ready())
        return;

    ((AddressInternal *) InternalTag)->Port = Port;
}

bool Lacewing::Address::Ready() const
{
    return !((AddressInternal *) InternalTag)->ResolverThread.Started ();
}

unsigned int Lacewing::Address::IP() const
{
    if(!Ready())
        return 0;

    return ((AddressInternal *) InternalTag)->IP;
}

unsigned char Lacewing::Address::IP_Byte(int Index) const
{
    if((!Ready()) || Index > 3 || Index < 0)
        return 0;

    return ((AddressInternal *) InternalTag)->Bytes[Index];
}

const char * Lacewing::Address::ToString() const
{
    if(!Ready())
        return "";

    return ((AddressInternal *) InternalTag)->StringIP;
}

Lacewing::Address::operator const char * () const
{
    return ToString();
}

