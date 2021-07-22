
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

class ReceiveBuffer
{
protected:

    size_t NextSize, MaxSize;

public:

    char * Buffer;
    size_t Size;

    inline ReceiveBuffer(int MaxKB = 1024)
    {
        Buffer   = 0;
        Size     = 0;
        NextSize = (MaxSize = MaxKB * 1024) / 8;
    }

    inline ~ReceiveBuffer()
    {
        free(Buffer);
    }
    
    inline operator char * ()
    {
        return Buffer;
    }

    inline void Prepare()
    {
        if(NextSize != 0)
        {
            char * NewBuffer = (char *) malloc(NextSize + 1);

            if(NewBuffer)
            {
                free (Buffer);

                Buffer = NewBuffer;    
                Size   = NextSize;

                NextSize = 0;
            }
            else
            {
                /* Cancel the upgrade, since malloc failing can't be a good sign */

                NextSize = 0;
            }
        }
    }

    inline void Received(size_t Size)
    {
        Buffer[Size] = 0;

        if(Size == this->Size && this->Size < MaxSize)
            NextSize = this->Size * 2;
    }

    /* TODO: A way for the buffer to be downsized again when the capacity
       isn't being reached anymore? */

};

