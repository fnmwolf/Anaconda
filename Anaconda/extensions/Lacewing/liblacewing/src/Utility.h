
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

template <class T> struct List
{   
    struct Element
    {
        Element * Next, * Prev;
        T Value;
        
        inline T operator * ()
        {
            return Value;
        }
        
    } * First, * Last;
    
    int Size;

    inline List ()
    {
        First = Last = 0;
        Size = 0;
    }
    
    inline ~ List ()
    {
        Clear ();
    }

    inline Element * Push (T What)
    {
        Element * E = new Element;
        
        E->Value = What;
        E->Next  = 0;
        E->Prev  = Last;
        
        ++ Size;
        
        if (Last)
        {
            Last->Next = E;
            Last = E;
        }
        else
        {
            First = Last = E;
        }

        return E;
    }

    inline Element * InsertBefore (Element * Before, T What)
    {
        Element * E = new Element;

        if (Before->Prev)
            Before->Prev->Next = E;

        E->Prev = Before->Prev;
        Before->Prev = E;

        E->Value = What;
        E->Next  = Before;

        if (Before == First)
            First = E;

        ++ Size;

        return E;
    }

    inline T Pop ()
    {
        T Value = ** Last;
        Erase (Last);
        return Value;
    }

    inline T PopFront ()
    {
        T Value = ** First;
        Erase (First);
        return Value;
    }

    inline void Erase (Element * E)
    {
        if (E == First)
            First = E->Next;

        if (E == Last)
            Last = E->Prev;

        if (E->Next)
            E->Next->Prev = E->Prev;

        if (E->Prev)
            E->Prev->Next = E->Next;
    
        delete E;

        -- Size;
    }
    
    inline void Clear ()
    {
        while (First)
        {
            Element * Next = First->Next;
            delete First;
            First = Next;
        }

        Last = 0;
        Size = 0;
    }
};

template <class T> struct Array
{       
    T * Items;
    
    int Allocated, Size;

    inline Array ()
    {
        Items = (T *) malloc (sizeof(T) * (Allocated = 16));
        Size = 0;
    }
    
    inline ~Array ()
    {
        free (Items);
    }

    inline void Push (T What)
    {
        if (Size + 1 >= Allocated)
            Items = (T *) realloc (Items, sizeof(T) * (Allocated *= 3));

        Items [Size ++] = What;
    }

    inline T Pop ()
    {
        return Items [-- Size];
    }

    inline T operator [] (int Index)
    {
        return Items [Index];
    }

    inline operator T * ()
    {
        return Items;
    }

    inline void Clear ()
    {
        Size = 0;
    }
};

struct String
{
    const static int MaxPrealloc = 4096;

    int Length, AllocatedFor, MaxLength;
    char * Buffer;
 
    inline String (int _MaxLength = 512) : MaxLength (_MaxLength)
    {
        *(Buffer = (char *) malloc (AllocatedFor = (MaxLength
                        <= MaxPrealloc ? MaxLength : MaxPrealloc))) = 0;

        Length = 0;
    }

    inline String (const char * Source)
    {
        AllocatedFor = Length = strlen (Source);
        Buffer = strdup (Source);
    }

    inline String (const String &Source)
    {
        AllocatedFor = Length = strlen (Source.Buffer);
        Buffer = strdup (Source.Buffer);

        MaxLength = Source.MaxLength;
    }

    inline ~ String ()
    {
        free (Buffer);
    }   

    inline int Prepare (int Size)
    {
        if (MaxLength && Size > MaxLength)
            Size = MaxLength;

        if (Size < AllocatedFor)
            return Size;

        char * New = (char *) realloc (Buffer, AllocatedFor *= 3);

        if (New)
        {
            Buffer = New;
            return Size;
        }

        return 0;
    }

    inline String &operator = (const char * S)
    {
        memcpy (Buffer, S, Length = (Prepare (strlen (S) + 1) - 1));
        Buffer [Length] = 0;

        return *this;
    }

    inline String &operator = (const String &S)
    {
        memcpy (Buffer, S.Buffer, Length = (Prepare (strlen (S.Buffer) + 1) - 1));
        Buffer [Length] = 0;

        return *this;
    }

    inline operator const char * ()
    {
        return Buffer ? Buffer : "";
    }

    inline int CompareCaseSensitive (String &B)
    {
        return strcmp ((const char *) *this, (const char *) B);
    }
    
    inline int Compare (String &B)
    {
        return strcasecmp ((const char *) *this, (const char *) B);
    }
};

