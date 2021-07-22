/*
 * ALURE  OpenAL utility library
 * Copyright (c) 2009-2010 by Chris Robinson.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "config.h"

#include "main.h"

#include <string.h>
#include <assert.h>

#include <istream>

#include <mpg123.h>


#ifdef DYNLOAD
static void *mp123_handle;
#define MAKE_FUNC(x) static typeof(x)* p##x
MAKE_FUNC(mpg123_read);
MAKE_FUNC(mpg123_init);
MAKE_FUNC(mpg123_open_feed);
MAKE_FUNC(mpg123_new);
MAKE_FUNC(mpg123_delete);
MAKE_FUNC(mpg123_feed);
MAKE_FUNC(mpg123_exit);
MAKE_FUNC(mpg123_getformat);
MAKE_FUNC(mpg123_format_none);
MAKE_FUNC(mpg123_decode);
MAKE_FUNC(mpg123_format);
#undef MAKE_FUNC

#define mpg123_read pmpg123_read
#define mpg123_init pmpg123_init
#define mpg123_open_feed pmpg123_open_feed
#define mpg123_new pmpg123_new
#define mpg123_delete pmpg123_delete
#define mpg123_feed pmpg123_feed
#define mpg123_exit pmpg123_exit
#define mpg123_getformat pmpg123_getformat
#define mpg123_format_none pmpg123_format_none
#define mpg123_decode pmpg123_decode
#define mpg123_format pmpg123_format
#else
#define mp123_handle 1
#endif


struct mp3Stream : public alureStream {
private:
    mpg123_handle *mp3File;
    long samplerate;
    int channels;
    ALenum format;
    std::ios::pos_type dataStart;
    std::ios::pos_type dataEnd;

public:
#ifdef DYNLOAD
    static void Init()
    {
#ifdef _WIN32
#define MPG123_LIB "libmpg123.dll"
#elif defined(__APPLE__)
#define MPG123_LIB "libmpg123.0.dylib"
#else
#define MPG123_LIB "libmpg123.so.0"
#endif
        mp123_handle = OpenLib(MPG123_LIB);
        if(!mp123_handle) return;

        LOAD_FUNC(mp123_handle, mpg123_read);
        LOAD_FUNC(mp123_handle, mpg123_init);
        LOAD_FUNC(mp123_handle, mpg123_open_feed);
        LOAD_FUNC(mp123_handle, mpg123_new);
        LOAD_FUNC(mp123_handle, mpg123_delete);
        LOAD_FUNC(mp123_handle, mpg123_feed);
        LOAD_FUNC(mp123_handle, mpg123_exit);
        LOAD_FUNC(mp123_handle, mpg123_getformat);
        LOAD_FUNC(mp123_handle, mpg123_format_none);
        LOAD_FUNC(mp123_handle, mpg123_decode);
        LOAD_FUNC(mp123_handle, mpg123_format);
        pmpg123_init();
    }
    static void Deinit()
    {
        if(mp123_handle)
        {
            pmpg123_exit();
            CloseLib(mp123_handle);
        }
        mp123_handle = NULL;
    }
#else
    static void Init() { }
    static void Deinit() { }
#endif

    virtual bool IsValid()
    { return mp3File != NULL; }

    virtual bool GetFormat(ALenum *fmt, ALuint *frequency, ALuint *blockalign)
    {
        *fmt = format;
        *frequency = samplerate;
        *blockalign = channels*2;
        return true;
    }

    virtual ALuint GetData(ALubyte *data, ALuint bytes)
    {
        if(!mp3File)
            return 0;

        ALuint amt = 0;
        while(bytes > 0)
        {
            size_t got = 0;
            int ret = mpg123_read(mp3File, data, bytes, &got);

            bytes -= got;
            data += got;
            amt += got;

            if(ret == MPG123_NEW_FORMAT)
            {
                mpg123_delete(mp3File);
                mp3File = NULL;
                break;
            }
            if(ret == MPG123_NEED_MORE)
            {
                unsigned char data[4096];
                ALint insize = std::min<ALint>(sizeof(data),
                                               (dataEnd-fstream->tellg()));
                if(insize > 0)
                {
                    fstream->read((char*)data, insize);
                    insize = fstream->gcount();
                }
                if(insize > 0 && mpg123_feed(mp3File, data, insize) == MPG123_OK)
                    continue;
            }
            if(got == 0)
                break;
        }
        return amt;
    }

    virtual bool Rewind()
    {
        fstream->clear();
        std::ios::pos_type oldpos = fstream->tellg();
        fstream->seekg(dataStart);

        mpg123_handle *newFile = mpg123_new(NULL, NULL);
        if(mpg123_open_feed(newFile) == MPG123_OK)
        {
            unsigned char data[4096];
            long newrate;
            int newchans;
            int enc;

            ALuint amt, total = 0;
            int ret = MPG123_OK;
            do {
                amt = std::min<ALint>(sizeof(data),
                                      (dataEnd-fstream->tellg()));
                fstream->read((char*)data, amt);
                amt = fstream->gcount();
                if(amt == 0)  break;
                total += amt;
                ret = mpg123_decode(newFile, data, amt, NULL, 0, NULL);
            } while(ret == MPG123_NEED_MORE && total < 64*1024);

            if(ret == MPG123_NEW_FORMAT &&
               mpg123_getformat(newFile, &newrate, &newchans, &enc) == MPG123_OK)
            {
                if(mpg123_format_none(newFile) == MPG123_OK &&
                   mpg123_format(newFile, samplerate, channels, MPG123_ENC_SIGNED_16) == MPG123_OK)
                {
                    // All OK
                    if(mp3File)
                        mpg123_delete(mp3File);
                    mp3File = newFile;
                    return true;
                }
            }
            mpg123_delete(newFile);
        }

        fstream->seekg(oldpos);
        SetError("Restart failed");
        return false;
    }

    mp3Stream(std::istream *_fstream)
      : alureStream(_fstream), mp3File(NULL), format(AL_NONE),
        dataStart(0), dataEnd(0)
    {
        if(!mp123_handle) return;

        if(!FindDataChunk())
            return;

        mp3File = mpg123_new(NULL, NULL);
        if(mpg123_open_feed(mp3File) == MPG123_OK)
        {
            unsigned char data[4096];
            int enc;

            ALuint amt, total = 0;
            int ret = MPG123_OK;
            do {
                amt = std::min<ALint>(sizeof(data),
                                      (dataEnd-fstream->tellg()));
                fstream->read((char*)data, amt);
                amt = fstream->gcount();
                if(amt == 0)  break;
                total += amt;
                ret = mpg123_decode(mp3File, data, amt, NULL, 0, NULL);
            } while(ret == MPG123_NEED_MORE && total < 64*1024);

            if(ret == MPG123_NEW_FORMAT &&
               mpg123_getformat(mp3File, &samplerate, &channels, &enc) == MPG123_OK)
            {
                format = GetSampleFormat(channels, 16, false);
                if(mpg123_format_none(mp3File) == MPG123_OK &&
                   mpg123_format(mp3File, samplerate, channels, MPG123_ENC_SIGNED_16) == MPG123_OK)
                {
                    // All OK
                    return;
                }
            }
        }
        mpg123_delete(mp3File);
        mp3File = NULL;
    }

    virtual ~mp3Stream()
    {
        if(mp3File)
            mpg123_delete(mp3File);
        mp3File = NULL;
    }

private:
    bool FindDataChunk()
    {
        ALubyte buffer[25];
        int length;

        if(!fstream->read(reinterpret_cast<char*>(buffer), 12))
            return false;

        if(memcmp(buffer, "RIFF", 4) != 0 || memcmp(buffer+8, "WAVE", 4) != 0)
        {
            dataStart = 0;

            // Check for an ID3v2 tag, and skip it
            if(memcmp(buffer, "ID3", 3) == 0 &&
               buffer[3] <= 4 && buffer[4] != 0xff &&
               (buffer[5]&0x0f) == 0 && (buffer[6]&0x80) == 0 &&
               (buffer[7]&0x80) == 0 && (buffer[8]&0x80) == 0 &&
               (buffer[9]&0x80) == 0)
            {
                dataStart = (buffer[6]<<21) | (buffer[7]<<14) |
                            (buffer[8]<< 7) | (buffer[9]    );
                dataStart += ((buffer[5]&0x10) ? 20 : 10);
            }

            if(fstream->seekg(0, std::ios_base::end))
            {
                dataEnd = fstream->tellg();
                fstream->seekg(dataStart);
            }
            return fstream->good();
        }

        int type = 0;
        while(1)
        {
            char tag[4];
            if(!fstream->read(tag, 4))
                break;

            /* read chunk length */
            length = read_le32(fstream);

            if(memcmp(tag, "fmt ", 4) == 0 && length >= 16)
            {
                /* Data type (should be 0x0050 or 0x0055 for MP3 data) */
                type = read_le16(fstream);
                if(type != 0x0050 && type != 0x0055)
                    break;
                length -= 2;
                /* Ignore the rest of the chunk. Everything we need is in the
                 * data stream */
            }
            else if(memcmp(tag, "data", 4) == 0)
            {
                if(type == 0x0050 || type == 0x0055)
                {
                    dataStart = fstream->tellg();
                    dataEnd = dataStart;
                    dataEnd += length;
                    return fstream->good();
                }
            }

            fstream->seekg(length, std::ios_base::cur);
        }

        return false;
    }
};
// Priority = -2, because mp3 loading can find false-positives, and interferes
// with mod files
static DecoderDecl<mp3Stream,-2> mp3Stream_decoder;
Decoder &alure_init_mpg123(void)
{ return mp3Stream_decoder; }
