
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

#ifndef LacewingFrameBuilder
#define LacewingFrameBuilder

class FrameBuilder : public MessageBuilder
{
protected:

    void PrepareForTransmission()
    {
        if(ToSend)
            return;

        int Type = *(unsigned int *) Buffer;
        int MessageSize = Size - 8;

        int HeaderSize;

        if(MessageSize < 254)
        {
            Buffer[6] = Type;
            Buffer[7] = MessageSize;

            HeaderSize = 2;
        }
        else if(MessageSize < 0xFFFF)
        {
            Buffer[4] = Type;

            (*(unsigned char  *) (Buffer + 5)) = 254;
            (*(unsigned short *) (Buffer + 6)) = MessageSize;

            HeaderSize = 4;
        }
        else if(MessageSize < 0xFFFFFFFF)
        {
            Buffer[2] = Type;

            (*(unsigned char  *) (Buffer + 3)) = 255;
            (*(unsigned int   *) (Buffer + 4)) = MessageSize;

            HeaderSize = 6;
        }

        ToSend     = (Buffer + 8) - HeaderSize;
        ToSendSize =  MessageSize + HeaderSize;
    }

    bool IsUDPClient;

    char * ToSend;
    int ToSendSize;

public:

    FrameBuilder(bool IsUDPClient)
    {
        this->IsUDPClient = IsUDPClient;
        ToSend = 0;
    }

    inline void AddHeader(unsigned char Type, unsigned char Variant, bool ForUDP = false, int UDPClientID = -1)
    {
        LacewingAssert(Size == 0);

        if(!ForUDP)
        {
            Add <unsigned int> ((Type << 4) | Variant);
            Add <unsigned int> (0);

            return;
        }

        Add <unsigned char>  ((Type << 4) | Variant);

        if(IsUDPClient)
            Add <unsigned short> (UDPClientID);
    }

    inline void Send(Lacewing::Server::Client &Client, bool Clear = true)
    {
        PrepareForTransmission ();
        Client.Send            (ToSend, ToSendSize);

        if(Clear)
            FrameReset();
    }

    inline void Send(Lacewing::Client &Client, bool Clear = true)
    {
        PrepareForTransmission ();
        Client.Send            (ToSend, ToSendSize);

        if(Clear)
            FrameReset();
    }

    inline void Send(Lacewing::UDP &UDP, Lacewing::Address &Address, bool Clear = true)
    {
        UDP.Send (Address, Buffer, Size);
 
        if(Clear)
            FrameReset();
    }

    inline void FrameReset()
    {
        Reset();
        ToSend = 0;
    }

};

#endif

