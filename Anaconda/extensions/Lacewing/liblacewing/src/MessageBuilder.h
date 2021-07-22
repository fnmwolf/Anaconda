
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

#ifndef LacewingMessageBuilder
#define LacewingMessageBuilder

class MessageBuilder
{
    
protected:

    unsigned int Allocated;

public:
    
    char * Buffer;
    unsigned int Size;

    inline MessageBuilder()
    {
        Size   = Allocated = 0;
        Buffer = 0;
    }

    inline ~ MessageBuilder()
    {
        free(Buffer);
    }

    inline void Add(const char * Buffer, int Size)
    {
        if(Size == -1)
            Size = strlen(Buffer);

        if(this->Size + Size > Allocated)
        {
            if(!Allocated)
                Allocated = 1024 * 4;
            else
                Allocated *= 3;

            if(this->Size + Size > Allocated)
                Allocated += Size;

            this->Buffer = (char *) realloc(this->Buffer, Allocated);
        }

        memcpy(this->Buffer + this->Size, Buffer, Size);
        this->Size += Size;
    }

    inline void Add(const String &Text)
    {
        Add(Text.Buffer, Text.Length);
    }

    template<class T> inline void Add (T Value)
    {
        Add((const char *) &Value, sizeof(T));
    }

    inline void AddNetwork16Bit (short Value)
    {
        Value = htons (Value);
        Add ((char *) &Value, sizeof(Value));
    }

    inline void AddNetwork24Bit (int Value)
    {
        Value = htonl (Value);
        Add (((char *) &Value) + 1, 3);
    }

    inline void AddNetwork32Bit (int Value)
    {
        Value = htonl (Value);
        Add ((char *) &Value, sizeof(Value));
    }

    inline void AddNetworkX31Bit (unsigned int Value)
    {
        Value = htonl (Value);

        *(char *) &Value &= 0x7F; /* 0 first bit */
        
        Add ((char *) &Value, sizeof(Value));
    }

    inline void Reset()
    {
        Size = 0;
    }

    inline void Send(Lacewing::Client &Socket, int Offset = 0)
    {
        Socket.Send(Buffer + Offset, Size - Offset);
    }

    inline void Send(Lacewing::Server::Client &Socket, int Offset = 0)
    {
        Socket.Send(Buffer + Offset, Size - Offset);
    }

    inline void Send(Lacewing::UDP &UDP, Lacewing::Address &Address, int Offset = 0)
    {
        UDP.Send(Address, Buffer + Offset, Size - Offset);
    }

};

#endif

