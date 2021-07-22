
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

#ifndef LacewingIDPool
#define LacewingIDPool

class IDPool
{
    
protected:

    List <unsigned short> IDs;

    unsigned short NextID;
    int BorrowedCount;

public:

    inline IDPool()
    {
        NextID        = 0;
        BorrowedCount = 0;
    }
    
    inline unsigned short Borrow()
    {
        ++ BorrowedCount;

        return IDs.Size ? IDs.PopFront () : NextID ++;
    }

    inline void Return(unsigned short ID)
    {
        if((-- BorrowedCount) == 0)
        {
            IDs.Clear ();
            NextID = 0;
        }
        else
        {
            for (List <unsigned short>::Element * E = IDs.First; E; E = E->Next)
            {
                if (** E > ID)
                {
                    IDs.InsertBefore (E, ID);
                    return;
                }
            }

            IDs.Push (ID);
        }
    }
};

#endif

