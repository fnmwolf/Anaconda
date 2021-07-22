
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

#ifndef LacewingFrameReader
#define LacewingFrameReader

class FrameReader
{
    
protected:

    MessageBuilder Buffer;
        
    int            State;
    int            SizeBytesLeft;
    unsigned int   MessageSize;
    unsigned char  MessageType; 

public:

    void  * Tag;
    void (* MessageHandler) (void * Tag, unsigned char Type, char * Message, int Size);

    FrameReader()
    {
        MessageHandler = 0;
        State          = 0;
    }

    inline void Process(char * Data, unsigned int Size)
    {
        while(State < 3 && Size -- > 0)
        {
            unsigned char Byte = *(Data ++);

            switch(State)
            {
                case 0: /* Haven't yet got type */
                    
                    MessageType = Byte;
                    State = 1;
                    SizeBytesLeft = 0;

                    break;
            
                case 1: /* Have type, but no size */
                {
                    if(SizeBytesLeft > 0)
                    {
                        Buffer.Add <char> (Byte);
                        
                        if((-- SizeBytesLeft) == 0)
                        {
                            switch(Buffer.Size)
                            {
                            case 2:

                                MessageSize = *(lw_i16 *) Buffer.Buffer;
                                break;

                            case 4:

                                MessageSize = *(lw_i32 *) Buffer.Buffer;
                                break;
                            }

                            Buffer.Reset();

                            State = 3;
                            break;
                        }

                        break;
                    }

                    /* Byte is the first byte of the size */

                    if(Byte == 254)
                    {
                        /* 16 bit message size to follow */

                        SizeBytesLeft = 2;
                        break;
                    }

                    if(Byte == 255)
                    {
                        /* 32 bit message size to follow */

                        SizeBytesLeft = 4;
                        break;
                    }

                    /* 8 bit message size */

                    MessageSize = Byte;
                    State = 3;

                    break;
                }
            };
        }

        if(State < 3) /* Header not complete yet */
            return;

        if(Buffer.Size == 0)
        {
            if(Size == MessageSize)
            {
                /* The message isn't fragmented, and it's the only message. */

                MessageHandler(Tag, MessageType, Data, MessageSize);
                State = 0;

                return;
            }

            if(Size > MessageSize)
            {
                /* There message isn't fragmented, but there are more messages than
                   this one.  Lovely hack to give it a null terminator without copying
                   the message..!  */

                char NextByte = Data[MessageSize];
                Data[MessageSize] = 0;

                MessageHandler(Tag, MessageType, Data, MessageSize); 
                Data[MessageSize] = NextByte;

                State = 0;
                Process(Data + MessageSize, Size - MessageSize);

                return;
            }
        }

        unsigned int ThisMessageBytes = MessageSize - Buffer.Size;

        if(Size < ThisMessageBytes)
            ThisMessageBytes = Size;

        Buffer.Add(Data, ThisMessageBytes);
        
        Size -= ThisMessageBytes;
        Data += ThisMessageBytes;

        if(Buffer.Size == MessageSize)
        {
            Buffer.Add <char> (0);

            MessageHandler(Tag, MessageType, Buffer.Buffer, MessageSize);
            Buffer.Reset();

            State = 0;

            if(Size > 0)
                Process(Data, Size);
        }
    }


};

#endif

