
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

#ifndef LacewingMessageReader
#define LacewingMessageReader

class MessageReader
{
protected:

    char * Buffer;
    
    unsigned int Size;

    List <char *> ToFree;

public:

    unsigned int Offset;

    bool Failed;

    inline MessageReader(char * Buffer, unsigned int Size)
    {
        Failed = false;

        this->Buffer = Buffer;
        this->Size = Size;

        this->Offset = 0;
    }

    inline ~MessageReader()
    {
        for (List <char *>::Element * e = ToFree.First; e; e = e->Next)
            delete [] (** e);
    }

    inline bool Check(unsigned int Size)
    {
        if(Failed)
            return false;

        if(Offset + Size > this->Size)
        {
            Failed = true;
            return false;
        }

        return true;
    }

    template<class T> inline T Get()
    {
        if(!Check(sizeof(T)))
            return 0;

        T Value = *(T *) (Buffer + Offset);

        Offset += sizeof(T);
        return Value;
    }

    char * Get (unsigned int Size)
    {
        if(!Check(Size))
            return 0;

        char * Output = (char *) malloc (Size + 1);

        if(!Output)
        {
            Failed = true;
            return 0;
        }

        ToFree.Push (Output);

        memcpy(Output, Buffer + Offset, Size);
        Output[Size] = 0;

        Offset += Size;

        return Output;
    }

    inline int BytesLeft ()
    {
        return Size - Offset;
    }

    inline char * Cursor ()
    {
        return Buffer + Offset;
    }

    inline char * GetRemaining(bool AllowEmpty = true)
    {
        if(Failed)
            return this->Buffer;

        char * Remaining = this->Buffer + Offset;
        Offset += Size;

        if(!AllowEmpty && !*Remaining)
            Failed = true;

        return Remaining;
    }

    inline void GetRemaining(char * &Buffer, unsigned int &Size, unsigned int MinimumLength = 0, unsigned int MaximumLength = 0xFFFFFFFF)
    {
        Buffer = this->Buffer + Offset;
        Size   = this->Size - Offset;

        if(Size > MaximumLength || Size < MinimumLength)
            Failed = true;

        Offset += Size;
    }

    inline short Network16Bit ()
    {
        return ntohs (Get <short> ());
    }
    
    inline int Network24Bit ()
    {
        if (!Check (3))
            return 0;
        
        return Read24Bit (Buffer + Offset);
    }

    inline int Network32Bit ()
    {
        return ntohl (Get <int> ());
    }
    
    inline int NetworkX31Bit ()
    {
        int value = Get <int> ();
   
        *(char *) &value &= 0x7F;
        
        return ntohl (value);
    }
};

#endif

