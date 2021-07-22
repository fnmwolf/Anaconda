
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

#ifndef LacewingQueuedSend
#define LacewingQueuedSend

namespace QueuedSendType
{
    enum Type
    {
        /* Will be written when the last queue item has finished being written. */
          
          Data,
          File,

        /* Writing will be attempted again after the next SSL_read has been performed.
           Always gets pushed to the front of the queue.  Only one can exist in the queue. */
           
          SSLWriteWhenReadable,

        /* Added to the queue to disconnect after sending all pending data */

          Disconnect
    };
}

struct QueuedSend
{
    QueuedSendType::Type Type;

    MessageBuilder Data;
    int DataOffset;

    char * Filename;
    
    lw_i64 FileSize;
    lw_i64 FileOffset;

    QueuedSend(QueuedSendType::Type Type)
    {
        this->Type = Type;

        Filename   = 0;
        Next       = 0;
        DataOffset = 0;
    }

    ~QueuedSend()
    {
        free(Filename);
    }

    QueuedSend * Next;
};

struct QueuedSendManager
{
    Lacewing::Sync Sync;

    QueuedSend * First, * Last;
    
    inline QueuedSendManager()
    {
        First = Last = 0;
    }
    

    inline void Add(QueuedSend * Data)
    {
        Lacewing::Sync::Lock Lock(Sync);

        if(!First)
        {
            Last = First = Data;
            return;
        }
        
        Last->Next = Data;
        Last = Data;
    }

    inline void Add(const char * Buffer, int Size)
    {
        Lacewing::Sync::Lock Lock(Sync);

        if(Last && (Last->Type == QueuedSendType::Data || Last->Type == QueuedSendType::SSLWriteWhenReadable))
        {
            Last->Data.Add(Buffer, Size);
            return;
        }

        QueuedSend * New = new QueuedSend(QueuedSendType::Data);
        New->Data.Add(Buffer, Size);

        Add(New);
    }

    inline void Add(const char * Filename, lw_i64 Offset, lw_i64 Size)
    {
        Lacewing::Sync::Lock Lock(Sync);

        QueuedSend * New = new QueuedSend(QueuedSendType::File); 

        New->Filename   = strdup(Filename);
        New->FileOffset = Offset;
        New->FileSize   = Size;

        Add(New);
    }


    inline void AddFront(QueuedSend * Data)
    {
        Lacewing::Sync::Lock Lock(Sync);

        if(!First)
        {
            Last = First = Data;
            return;
        }
        
        Data->Next = First;
        First = Data;
    }

    inline void AddSSLWriteWhenReadable(const char * Buffer, int Size)
    {
        Lacewing::Sync::Lock Lock(Sync);

        QueuedSend * New = new QueuedSend(QueuedSendType::SSLWriteWhenReadable);
        New->Data.Add(Buffer, Size);

        AddFront(New);
    }
};

#endif

