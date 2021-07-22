// Copyright (c) Mathias Kaerlev 2012.

// This file is part of Anaconda.

// Anaconda is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Anaconda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>

#define ALURE_STATIC_LIBRARY

#include "AL/alure.h"
#include <math.h>

#if defined(__APPLE__)
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#define NUM_BUFS 3
#define CHUNK_LENGTH 250000
// #define DEBUG

#ifdef DEBUG
#define check_debug(error) check_debug_func(error)
#else
#define check_debug(error)
#endif

int make_alure_error(char * error)
{
    const ALchar * alure_error = alureGetErrorString();
#ifdef DEBUG
    if (strcmp(alure_error, "No error") != 0)
#endif
    {
        printf("ALURE: %s: %s\n", error, alure_error);
        return 1;
    }
    return 0;
}

int make_al_error(char * error)
{
    ALenum result = alGetError();
    if (result != AL_NO_ERROR)
    {
        printf("OpenAL: %s: %d\n", error, result);
        return 1;
    }
    return 0;
}

int inline check_debug_func(char * error)
{
    return make_alure_error(error) || make_al_error(error);
}

void snake_deinit_sound()
{
    alureUpdateInterval(0);
    alureShutdownDevice();
}

void snake_init_sound()
{
    if(!alureInitDevice(NULL, NULL))
    {
        printf("Failed to open OpenAL device: %s\n", alureGetErrorString());
    }
    alureStreamSizeIsMicroSec(AL_TRUE);
    alureUpdateInterval(0.125);
}

struct SnakeSound
{
    ALuint src;
    alureStream * stream;
    void (*callback)(SnakeSound *, void *);
    void * userdata;
};

void snake_close_sound(SnakeSound * sound)
{
    check_debug("Closing sound.");
    if (sound->stream != NULL) {
        alureDestroyStream(sound->stream, 0, NULL);
    }
    check_debug("Destroying stream");
    if (sound->src != NULL) {
        alDeleteSources(1, &sound->src);
    }
    check_debug("Delete source");
    delete sound;
}

static void eos_callback(void * userdata, ALuint source)
{
    SnakeSound * sound = (SnakeSound *)userdata;
    if (sound->callback != NULL)
    {
        sound->callback(sound, sound->userdata);
    }
}

bool snake_play_sound(SnakeSound * sound, int loops)
{
    loops--;
    if (alurePlaySourceStream(sound->src, sound->stream, NUM_BUFS, loops, 
    eos_callback, (void*)sound) == AL_FALSE)
    {
        make_alure_error("Error starting sound");
        return false;
    }
    return true;
}

void snake_resume_sound(SnakeSound * sound)
{
    alureResumeSource(sound->src);
    check_debug("Resuming source");
}

void snake_pause_sound(SnakeSound * sound)
{
    alurePauseSource(sound->src);
    check_debug("Pausing source");
}

void snake_stop_sound(SnakeSound * sound)
{
    alureStopSource(sound->src, AL_FALSE);
    check_debug("Stopping source");
}

// get/set for listener

float snake_get_main_volume()
{
    ALfloat volume;
    alGetListenerf(AL_GAIN, &volume);
    check_debug("Get main volume");
    return volume;
}

void snake_set_main_volume(float value)
{
    alListenerf(AL_GAIN, value);
    check_debug("Set main volume");
}

// get/set for SnakeSound

// pitch

float snake_get_pitch(SnakeSound * sound)
{
    ALfloat pitch;
    alGetSourcef(sound->src, AL_PITCH, &pitch);
    check_debug("Get pitch");
    return pitch;
}

void snake_set_pitch(SnakeSound * sound, float value)
{
    alSourcef(sound->src, AL_PITCH, value);
    check_debug("Set pitch");
}

// samplerate

int snake_get_rate(SnakeSound * sound)
{
    int rate = alureGetStreamFrequency(sound->stream);
    check_debug("Get frequency");
    return rate;
}

int snake_get_frequency(SnakeSound * sound)
{
    return snake_get_pitch(sound) * alureGetStreamFrequency(sound->stream);
}

void snake_set_frequency(SnakeSound * sound, int frequency)
{
    return snake_set_pitch(sound, 
        (float)frequency / alureGetStreamFrequency(sound->stream));
}

// length

long long snake_get_length(SnakeSound * sound)
{
    return alureGetStreamLength(sound->stream);
}

// volume

float snake_get_volume(SnakeSound * sound)
{
    ALfloat volume;
    alGetSourcef(sound->src, AL_GAIN, &volume);
    check_debug("Get volume");
    return volume;
}

void snake_set_volume(SnakeSound * sound, float value)
{
    alSourcef(sound->src, AL_GAIN, value);
#ifdef DEBUG
    if (check_debug("Set volume"))
    {
        printf("Source of call: %d\n", sound->src);
    }
#endif
}

// pan

float snake_get_pan(SnakeSound * sound)
{
    ALfloat x, y, z;
    alGetSource3f(sound->src, AL_POSITION, &x, &y, &z);
    check_debug("Get pan");
    return x * 100.0;
}

void snake_set_pan(SnakeSound * sound, float pan)
{
    pan = pan / 100.0;
    if (pan > 1.0)
        pan = 1.0;
    else if (pan < -1.0)
        pan = -1.0;
    alSource3f(sound->src, AL_POSITION, pan, -sqrt(1.0 - pan * pan), 0);
    check_debug("Set pan");
}

void snake_get_position(SnakeSound * sound, float * x, float * y, float * z)
{
    alGetSource3f(sound->src, AL_POSITION, x, y, z);
    check_debug("Get position");
}

void snake_set_position(SnakeSound * sound, float x, float y, float z)
{
    alSource3f(sound->src, AL_POSITION, x, y, z);
    check_debug("Set position");
}

long long snake_tell(SnakeSound * sound)
{
    long long pos = alureGetStreamPos(sound->stream);
    check_debug("Get stream position");
    return pos;
}

void snake_seek(SnakeSound * sound, long long pos)
{
    alureSeekStream(sound->stream, pos);
    check_debug("Seeking");
}

void snake_set_soundfont(SnakeSound * sound, char * font)
{
    alureSetStreamPatchset(sound->stream, font);
    check_debug("Set soundfont");
}

void snake_set_callback(SnakeSound * sound, 
                        void (*callback)(SnakeSound*, void *), 
                        void * userdata)
{
    sound->callback = callback;
    sound->userdata = userdata;
}

SnakeSound * make_sound()
{
    SnakeSound * sound = new SnakeSound;
    sound->callback = NULL;
    sound->userdata = NULL;
    sound->src = NULL;
    sound->stream = NULL;
    alGenSources(1, &sound->src);
    if (make_al_error("Could not create source")) {
        snake_close_sound(sound);
        return NULL;
    }
    return sound;
}

SnakeSound * snake_load_file(char * name)
{
    SnakeSound * sound = make_sound();
    if (sound == NULL) {
        return NULL;
    }
    sound->stream = alureCreateStreamFromFile(name, CHUNK_LENGTH, 0, NULL);
    if (!sound->stream) {
        make_alure_error("Could not load file");
        snake_close_sound(sound);
        return NULL;
    }
    return sound;
}

SnakeSound * snake_load_data(char * data, size_t size)
{
    SnakeSound * sound = make_sound();
    if (sound == NULL) {
        return NULL;
    }
    sound->stream = alureCreateStreamFromStaticMemory(
        (const unsigned char *)data, size, CHUNK_LENGTH, 0, NULL);
    if (!sound->stream)
    {
        make_alure_error("Could not load data");
        snake_close_sound(sound);
        return NULL;
    }
    return sound;
}