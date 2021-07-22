
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

struct FilterInternal
{
    FilterInternal() : RemoteAddress((unsigned int) 0, 0)
    {
        LocalIP = 0;
        LocalPort = 0;

        Reuse = false;
    }

    int LocalIP;
    int LocalPort;

    Lacewing::Address RemoteAddress;
    
    bool Reuse;
};

Lacewing::Filter::Filter()
{
    InternalTag = new FilterInternal;
    Tag         = 0;
}

Lacewing::Filter::Filter(const Filter &_Filter)
{
    InternalTag = new FilterInternal;
    Tag         = 0;

    Remote        (_Filter.Remote());
    LocalIP       (_Filter.LocalIP());
    LocalPort     (_Filter.LocalPort());
}

Lacewing::Filter::~Filter()
{
    delete ((FilterInternal *) InternalTag);
}

void Lacewing::Filter::Remote(const Lacewing::Address &Address)
{
    while(!Address.Ready())
        LacewingYield();

    ((FilterInternal *) InternalTag)->RemoteAddress.~Address();
    new (&((FilterInternal *) InternalTag)->RemoteAddress) Lacewing::Address (Address);
}

void Lacewing::Filter::LocalIP(int IP)
{
    ((FilterInternal *) InternalTag)->LocalIP = IP;
}

int Lacewing::Filter::LocalIP() const
{
    return ((FilterInternal *) InternalTag)->LocalIP;
}

void Lacewing::Filter::LocalPort(int Port)
{
    ((FilterInternal *) InternalTag)->LocalPort = Port;
}

int Lacewing::Filter::LocalPort() const
{
    return ((FilterInternal *) InternalTag)->LocalPort;
}

Lacewing::Address &Lacewing::Filter::Remote() const
{
    return ((FilterInternal *) InternalTag)->RemoteAddress;
}

void Lacewing::Filter::Reuse(bool Enabled)
{
    ((FilterInternal *) InternalTag)->Reuse = Enabled;
}

bool Lacewing::Filter::Reuse() const
{
    return ((FilterInternal *) InternalTag)->Reuse;
}

void Lacewing::Filter::Local (const char * Name)
{
    Lacewing::Address Address(Name, 0, true);
    
    LocalIP(Address.IP());
    LocalPort(Address.Port());
}

void Lacewing::Filter::Remote (const char * Name)
{
    Lacewing::Address Address(Name, 0, true);
    Remote (Address);
}


