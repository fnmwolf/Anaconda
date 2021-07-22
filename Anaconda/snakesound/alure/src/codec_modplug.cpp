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

#include <modplug.h>


#ifdef DYNLOAD
static void *mod_handle;
#define MAKE_FUNC(x) static typeof(x)* p##x
MAKE_FUNC(ModPlug_Load);
MAKE_FUNC(ModPlug_Unload);
MAKE_FUNC(ModPlug_Read);
MAKE_FUNC(ModPlug_SeekOrder);
#undef MAKE_FUNC

#define ModPlug_Load pModPlug_Load
#define ModPlug_Unload pModPlug_Unload
#define ModPlug_Read pModPlug_Read
#define ModPlug_SeekOrder pModPlug_SeekOrder
#else
#define mod_handle 1
#endif


struct modStream : public alureStream {
private:
    ModPlugFile *modFile;
    int lastOrder;

public:
#ifdef DYNLOAD
    static void Init()
    {
#ifdef _WIN32
#define MODPLUG_LIB "libmodplug.dll"
#elif defined(__APPLE__)
#define MODPLUG_LIB "libmodplug.1.dylib"
#else
#define MODPLUG_LIB "libmodplug.so.1"
#endif
        mod_handle = OpenLib(MODPLUG_LIB);
        if(!mod_handle) return;

        LOAD_FUNC(mod_handle, ModPlug_Load);
        LOAD_FUNC(mod_handle, ModPlug_Unload);
        LOAD_FUNC(mod_handle, ModPlug_Read);
        LOAD_FUNC(mod_handle, ModPlug_SeekOrder);
    }
    static void Deinit()
    {
        if(mod_handle)
            CloseLib(mod_handle);
        mod_handle = NULL;
    }
#else
    static void Init() { }
    static void Deinit() { }
#endif

    virtual bool IsValid()
    { return modFile != NULL; }

    virtual bool GetFormat(ALenum *fmt, ALuint *frequency, ALuint *blockalign)
    {
        *fmt = AL_FORMAT_STEREO16;
        *frequency = 44100;
        *blockalign = 2 * sizeof(ALshort);
        return true;
    }

    virtual ALuint GetData(ALubyte *data, ALuint bytes)
    {
        int ret = ModPlug_Read(modFile, data, bytes);
        if(ret < 0) return 0;
        return ret;
    }

    virtual bool Rewind()
    { return SetOrder(lastOrder); }

    virtual bool SetOrder(ALuint order)
    {
        std::vector<char> data(16384);
        ALuint total = 0;
        while(1)
        {
            fstream->read(&data[total], data.size()-total);
            if(fstream->gcount() == 0) break;
            total += fstream->gcount();
            data.resize(total*2);
        }
        data.resize(total);

        ModPlugFile *newMod = ModPlug_Load(&data[0], data.size());
        if(!newMod)
        {
            SetError("Could not reload data");
            return false;
        }
        ModPlug_Unload(modFile);
        modFile = newMod;

        // There seems to be no way to tell if the seek succeeds
        ModPlug_SeekOrder(modFile, order);
        lastOrder = order;

        return true;
    }

    modStream(std::istream *_fstream)
      : alureStream(_fstream), modFile(NULL), lastOrder(0)
    {
        if(!mod_handle) return;

        std::vector<char> data(1024);
        ALuint total = 0;

        fstream->read(&data[total], data.size()-total);
        total += fstream->gcount();
        if(total < 32) return;

        if(memcmp(&data[0], "Extended Module: ", 17) == 0 || /* XM */
           (data[28] == 0x1A && data[29] == 0x10) || /* S3M */
           memcmp(&data[0], "IMPM", 4) == 0) /* IT */
        {
            while(1)
            {
                data.resize(total*2);
                fstream->read(&data[total], data.size()-total);
                if(fstream->gcount() == 0) break;
                total += fstream->gcount();
            }
            data.resize(total);

            modFile = ModPlug_Load(&data[0], data.size());
        }
    }

    virtual ~modStream()
    {
        if(modFile)
            ModPlug_Unload(modFile);
        modFile = NULL;
    }
};
// Priority = -1, because mod loading can find false-positives
static DecoderDecl<modStream,-1> modStream_decoder;
Decoder &alure_init_modplug(void)
{ return modStream_decoder; }
