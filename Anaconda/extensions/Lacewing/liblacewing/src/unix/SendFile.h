
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

#ifdef __linux__
    #define LacewingLinuxSendfile
#else
    #ifdef __APPLE__
        #define LacewingMacSendFile
    #else
        #ifdef __FreeBSD__
            #define LacewingFreeBSDSendFile
        #else
            #error "Don't know how to use sendfile() on this platform"
        #endif
    #endif
#endif


/* This is just a thin wrapper around the various sendfile() calls offered by different *nix-es.

   Sends a file from SourceFD to DestFD.  Returns true for success, false for failure.
   On success, the number of bytes sent will be added to Offset and subtracted from Size. */

bool LacewingSendFile(int SourceFD, int DestFD, off_t &Offset, off_t &Size)
{
    #ifdef LacewingLinuxSendfile
        
        int Sent = sendfile(DestFD, SourceFD, &Offset, Size);

        if(Sent == -1)
        {
            if(errno == EAGAIN)
                return true;

            return false;
        }

        /* Linux automatically adjusts the offset */

        Size -= Sent;        
        return true;

    #endif

    #ifdef LacewingFreeBSDSendFile

        off_t Sent;

        if(sendfile(SourceFD, DestFD, Offset, Size, 0, &Sent, 0) != 0)
        {
            /* EAGAIN might still have sent some bytes on BSD */

            if(errno != EAGAIN)
                return false;
        }
                                    
        Offset += Sent;
        Size   -= Sent;
    
        return true;

    #endif

    #ifdef LacewingMacSendFile

        off_t Sent = Size;

        if(sendfile(SourceFD, DestFD, Offset, &Sent, 0, 0) != 0)
        {
            /* EAGAIN might still have sent some bytes on OS X */

            if(errno != EAGAIN)
                return false;
        }
                  
        Offset += Sent;
        Size   -= Sent;
    
    #endif
}


/* And these are the high level file transfer classes used by Lacewing::Server */

class FileTransfer
{
public:

    /* These will return false when the file transfer is complete (or failed) */

    virtual bool WriteReady () = 0;
    virtual bool ReadReady () = 0;
};

class RawFileTransfer : public FileTransfer
{
protected:

    int FromFD;
    int ToFD;

    off_t Offset, Size;

public:

    RawFileTransfer (int FromFD, int ToFD, off_t Offset, off_t Size)
    {
        this->FromFD = FromFD;
        this->ToFD   = ToFD;
        this->Offset = Offset;
        this->Size   = Size;
    }

    ~ RawFileTransfer ()
    {
        close(FromFD);
    }

    bool WriteReady()
    {
        if(!LacewingSendFile(FromFD, ToFD, Offset, Size))
            return false;

        if(!Size)
            return false;

        return true;
    }

    bool ReadReady()
    {
        return true;
    }
};

/* With SSL, we can't use sendfile() at all, so Lacewing reads chunks of the file
   into userland with read() and sends them with SSL_write() */

class SSLFileTransfer : public FileTransfer
{
protected:

    char Chunk[1024 * 256];
    int ChunkSize;
    
    /* Returns true if there's a chunk loaded that needs sending, or
       false if there was an error (assume EOF) */

    bool Read()
    {
        if(ChunkSize != -1)
        {
            /* There's already a chunk to send */
            return true;
        }
        
        int BytesLeft = Bytes - BytesRead;
        int ToRead = BytesLeft > sizeof(Chunk) ? sizeof(Chunk) : BytesLeft;

        if(!ToRead)
            return false;
        
        if((ChunkSize = read(FromFD, Chunk, ToRead)) == -1)
            return false;

        BytesRead += ChunkSize;
        return true;
    }

    int FromFD;
    SSL * To;

    off_t Bytes, BytesRead;

    enum
    {
        SendOnReadReady,
        SendOnWriteReady

    } SendOn;

    bool Ready()
    {
        for(;;)
        {
            if(!Read())
                return false;

            int Bytes = SSL_write(To, Chunk, ChunkSize);

            if(Bytes == 0)
                return false;

            if(Bytes > 0)
            {
                /* Onwards to the next chunk */

                ChunkSize = -1;
                continue;
            }

            /* We're not ready to send any more (or there was an error) */

            switch(SSL_get_error(To, Bytes))
            {
                case SSL_ERROR_WANT_READ:

                    SendOn = SendOnReadReady;
                    return true;

                case SSL_ERROR_WANT_WRITE:

                    SendOn = SendOnWriteReady;
                    return true;

                default:
                    return false;
            };
        }

        return true;
    }

public:

    SSLFileTransfer (int FromFD, SSL * To, off_t Bytes)
    {
        this->FromFD    = FromFD;
        this->To        = To;
        this->Bytes     = Bytes;
        this->BytesRead = 0;

        SendOn = SendOnWriteReady;
        ChunkSize = -1;
    }

    ~ SSLFileTransfer ()
    {
        close(FromFD);
    }

    bool WriteReady()
    {
        if(SendOn != SendOnWriteReady)
            return true;

        return Ready();
    }

    bool ReadReady()
    {
        if(SendOn != SendOnReadReady)
            return true;

        return Ready();
    }
};





