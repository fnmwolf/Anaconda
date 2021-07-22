
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

HTTPClient::MultipartProcessor::MultipartProcessor
    (HTTPClient &_Client, const char * ContentType) : Client(_Client)
{
    State = Continue;

    const char * Boundary = strstr(ContentType, "boundary=");

    if(!Boundary)
    {
        State = Error;
        return;
    }

    Boundary += 9;

    {   char * End = (char *) strchr(Boundary, ';');
        
        if(End)
            *End = 0;
    }

    if(strlen(Boundary) > (sizeof(this->Boundary) - 8))
    {
        State = Error;
        return;
    }

    sprintf(this->Boundary,           "--%s",     Boundary);
    sprintf(this->CRLFThenBoundary,   "\r\n--%s", Boundary);
    sprintf(this->FinalBoundary,      "%s--",     this->Boundary);

    InHeaders = InFile = false;
    Child     = Parent = 0;

    CurrentUpload = 0;
}

HTTPClient::MultipartProcessor::~MultipartProcessor()
{
    for (int i = 0; i < Uploads.Size; ++ i)
        delete (UploadInternal *) Uploads [i]->InternalTag;
    
    Uploads.Clear ();
}

int HTTPClient::MultipartProcessor::Process(char * Data, size_t Size)
{
    if(InFile && Child)
    {
        int SizeLeft = Child->Process(Data, Size);

        Data += Size - SizeLeft;
        Size = SizeLeft;

        if(Child->State == Done)
        {
            delete Child;
            Child = 0;
        }
    }

    int InFileOffset = InFile ? 0 : -1;

    for(size_t i = 0; i < Size; ++ i)
    {
        if(!InFile)
        {
            if(!InHeaders)
            {
                /* Before headers */

                if(BeginsWith(Data + i, this->Boundary))
                {
                    InHeaders = true;
                    i += strlen(this->Boundary);

                    while(i < Size && (Data[i] == '\r' || Data[i] == '\n'))
                        ++ i;

                    Header = Data + i --;
                }

                continue;
            }

            /* In headers */

            if(Data[i] == '\r' && Data[i + 1] == '\n')
            {
                Data[i] = 0;

                ProcessHeader();
                Header = Data + i + 2;

                if(InFile)
                    InFileOffset = i + 2;

                if(Child)
                    return Process(Data + i, Size - i);

                ++ i;
            }

            continue;
        }

        /* In file */

        if(BeginsWith(Data + i, this->CRLFThenBoundary))
        {
            ToFile(Data + InFileOffset, i - InFileOffset);

            if(CurrentUpload)
            {
                if(Parent)
                    Parent->Uploads.Push (&CurrentUpload->Upload);
                else
                    Uploads.Push (&CurrentUpload->Upload);

                if(!CurrentUpload->AutoSaveFile)
                {
                    /* Manual save */

                    if(Client.Server.HandlerUploadDone)
                        Client.Server.HandlerUploadDone(Client.Server.Webserver, Client.Request.Public, CurrentUpload->Upload);
                }
                else
                {
                    /* Auto save */

                    DebugOut("Closing auto save file");

                    fclose(CurrentUpload->AutoSaveFile);
                    CurrentUpload->AutoSaveFile = 0;
                }

                CurrentUpload = 0;
            }
            else
            {
                Buffer.Add <char> (0);

                Client.Request.PostItems.Set(Disposition.Get("name"), Buffer.Buffer);
                Buffer.Reset();
            }

            Disposition.Clear();

            if(BeginsWith(Data + i + 2, this->FinalBoundary))
            {
                State = Done;
                return Size - i - (strlen(this->FinalBoundary) + 2);
            }

            InFile = false;
            -- i;

            continue;
        }
    }

    ToFile(Data + InFileOffset, Size - InFileOffset);

    return 0;
}

void HTTPClient::MultipartProcessor::ProcessHeader()
{
    if(!*Header)
    {
        /* Blank line marks end of headers */

        InHeaders = false;
        InFile    = true;

        /* If a filename is specified, the data is handled by the PreFilePost/FilePostChunk/FilePostComplete
           handlers, and is assigned an Upload structure. */

        if(*Disposition.Get("filename"))
        {
            CurrentUpload = new HTTPUpload;
            
            CurrentUpload->Filename =
                CurrentUpload->Copier.Set("filename", Disposition.Get("filename"))->Value;


            /* If this is a child multipart, the upload takes the form element name from the parent disposition */

            CurrentUpload->FormElement = CurrentUpload->Copier.Set("name",
                                                    Parent ? Parent->Disposition.Get("name") : Disposition.Get("name"))->Value;
        
            if(Client.Server.HandlerUploadStart)
                Client.Server.HandlerUploadStart(Client.Server.Webserver, Client.Request.Public, CurrentUpload->Upload);

            return;
        }

        /* If a filename is not specified, the data is retrieved like a normal form post item via POST(). */
        
        return;
    }

    char * Name = Header;

    if(!(Header = strchr(Header, ':')))
    {
        State = Error;
        return;
    }

    *(Header ++) = 0;

    while(*Header == ' ')
        ++ Header;

    Headers.Set(Name, Header);

    if(!strcasecmp(Name, "Content-Disposition"))
    {
        char * i;

        if((!(i = strchr(Header, ';'))) && (!(i = strchr(Header, '\r'))))
        {
            State = Error;
            return;
        }

        *(i ++) = 0;
        
        Disposition.Set("Type", Header);

        char * Begin = i;

        for(;;)
        {
            if(i = strchr(Begin, ';'))
                *(i ++) = 0;
            else
                i = Begin + strlen(Begin) - 1;
            
            if(Begin >= i)
                break;

            while(*Begin == ' ')
                ++ Begin;

            ProcessDispositionPair(Begin);
            Begin = i + 1;
        }
    }

    if(!strcasecmp(Name, "Content-Type"))
    {
        if(BeginsWith(Header, "multipart"))
        {
            if(Parent)
            {
                /* A child processor can't have children */

                State = Error;
                return;
            }

            Child = new MultipartProcessor (Client, Header);
            Child->Parent = this;
        }
    }
}

void HTTPClient::MultipartProcessor::ProcessDispositionPair(char * Pair)
{
    char * Name = Pair;

    if(!(Pair = strchr(Pair, '=')))
    {
        State = Error;
        return;
    }

    *(Pair ++) = 0;

    while(*Pair == ' ')
        ++ Pair;

    if(*Pair == '"')
        ++ Pair;

    if(Pair[strlen(Pair) - 1] == '"')
        Pair[strlen(Pair) - 1] = 0;

    if((!strcasecmp(Name, "filename")) && strchr(Pair, '\\'))
    {
        /* Old versions of IE send the absolute path (!) */

        Pair += strlen(Pair) - 1;
        
        while(*Pair != '\\')
            -- Pair;

        ++ Pair;
    }
    
    Disposition.Set(Name, Pair);
}

void HTTPClient::MultipartProcessor::ToFile(const char * Data, size_t Size)
{
    LacewingAssert(Size >= 0);

    if(!Size)
        return;

    if(CurrentUpload)
    {
        if(!CurrentUpload->AutoSaveFile)
        {
            /* Manual save */

            if(Client.Server.HandlerUploadChunk)
                Client.Server.HandlerUploadChunk(Client.Server.Webserver,
                        Client.Request.Public, CurrentUpload->Upload, Data, Size);

            return;
        }

        /* Auto save */

        if(fwrite(Data, 1, Size, CurrentUpload->AutoSaveFile) != Size)
        {
            CurrentUpload->AutoSaveFile = 0;
            Client.Socket.Disconnect ();
        }

        return;
    }

    Buffer.Add(Data, Size);
}

void HTTPClient::MultipartProcessor::CallRequestHandler()
{
    Client.Request.BeforeHandler ();

    if(Client.Server.HandlerUploadPost)
        Client.Server.HandlerUploadPost (Client.Server.Webserver, Client.Request.Public,
                                                Uploads.Items, Uploads.Size);

    Client.Request.AfterHandler ();
}

const char * HTTPClient::HTTPUpload::Header (const char * Name)
{
    return Headers.Get (Name);
}

