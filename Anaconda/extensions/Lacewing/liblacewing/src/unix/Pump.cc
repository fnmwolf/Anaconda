
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

Lacewing::Pump::Pump()
{
    InternalTag = new PumpInternal(*this);
    Tag         = 0;
}

Lacewing::Pump::~Pump()
{
    delete ((PumpInternal *) InternalTag);
}

bool Lacewing::Pump::Ready (void * Tag, bool CanRead, bool CanWrite)
{
    PumpInternal &Internal = *((PumpInternal *) InternalTag);

    PumpInternal::Event * Event = (PumpInternal::Event *) Tag;

    if((!Event->ReadCallback) && (!Event->WriteCallback))
    {
        /* Post() */

        {   Lacewing::Sync::Lock Lock(Internal.Sync_PostQueue);

            while(Internal.PostQueue.Size)
            {
                /* Each post = one byte written to the pipe */

                {   char b;
                    read (Internal.PostFD_Read, &b, 1);
                }

                Event = Internal.PostQueue.PopFront ();
                
                if(Event->ReadCallback == SigExitEventLoop)
                    return false;

                if(Event->ReadCallback == SigRemoveClient)
                {
                    this->Gone (Event->GoneKey);
                    Internal.EventBacklog.Return(*(PumpInternal::Event *) Event->Tag);
                }
                else
                    ((void (*) (void *)) Event->ReadCallback) (Event->Tag);
        
                Internal.EventBacklog.Return(*Event);
            }
        }

        return true;
    }

    if(CanWrite)
    {
        if(Event->WriteCallback && !Event->Removing)
            ((void (*) (void *)) Event->WriteCallback) (Event->Tag);
    }
    
    if(CanRead)
    {
        if(!Event->Removing)
            ((void (*) (void *)) Event->ReadCallback) (Event->Tag);
    }

    return true;
}

void Lacewing::Pump::Post (void * Function, void * Parameter)
{
    PumpInternal &Internal = *((PumpInternal *) InternalTag);

    if(!Internal.PostFD_Added)
    {
        Internal.AddRead (Internal.PostFD_Read, 0, 0);
        Internal.PostFD_Added = true;
    }

    PumpInternal::Event &Event = Internal.EventBacklog.Borrow(Internal);

    LacewingAssert(Function != 0);

    Event.ReadCallback  = Function;
    Event.WriteCallback = 0;
    Event.Tag           = Parameter;
    Event.Removing      = false;

    {   Lacewing::Sync::Lock Lock(Internal.Sync_PostQueue);
   
        Internal.PostQueue.Push (&Event);
        write(Internal.PostFD_Write, "", 1);
    }
}

void * PumpInternal::AddRead (int FD, void * Tag, void * Callback)
{
    InUse = true;
    Event &E = EventBacklog.Borrow(*this);

    E.Tag           = Tag;
    E.ReadCallback  = Callback;
    E.WriteCallback = 0;
    E.Removing      = false;
    E.GoneKey       = Pump.AddRead(FD, &E);

    return &E;
}

void * PumpInternal::AddReadWrite (int FD, void * Tag, void * ReadCallback, void * WriteCallback)
{
    InUse = true;
    Event &E = EventBacklog.Borrow(*this);

    E.Tag           = Tag;
    E.ReadCallback  = ReadCallback;
    E.WriteCallback = WriteCallback;
    E.Removing      = false;
    E.GoneKey       = Pump.AddReadWrite(FD, &E);

    return &E;
}

PumpInternal::PumpInternal (Lacewing::Pump &_Pump) : Pump(_Pump)
{
    int PostPipe[2];
    pipe(PostPipe);
    
    PostFD_Read  = PostPipe[0];
    PostFD_Write = PostPipe[1];
    PostFD_Added = false;
   
    fcntl (PostFD_Read, F_SETFL, fcntl (PostFD_Read, F_GETFL, 0) | O_NONBLOCK);

    InUse = false;
}

bool Lacewing::Pump::IsEventPump ()
{
    return false;
}

void Lacewing::Pump::PostEventLoopExit ()
{
    Post (SigExitEventLoop, 0);
}

bool Lacewing::Pump::InUse()
{
    return ((PumpInternal *) InternalTag)->InUse;
}

void Lacewing::Pump::InUse(bool InUse)
{
    ((PumpInternal *) InternalTag)->InUse = InUse;
}

void Lacewing::Pump::Gone(void *)
{
}

