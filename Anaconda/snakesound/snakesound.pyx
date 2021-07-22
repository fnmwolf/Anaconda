# Copyright (c) Mathias Kaerlev 2012.

# This file is part of Anaconda.

# Anaconda is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Anaconda is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

cimport cython

import os

cdef extern from "Python.h":
    void PyEval_InitThreads()

cdef extern from "_snakesound.h":
    void snake_init_sound() nogil
    void snake_deinit_sound() nogil
    void make_error(char * context) nogil
    struct SnakeSound:
        pass
    SnakeSound * snake_load_file(char * name) nogil
    SnakeSound * snake_load_data(char * data, size_t size) nogil
    bint snake_play_sound(SnakeSound * sound, int loops) nogil
    void snake_pause_sound(SnakeSound * sound) nogil
    void snake_resume_sound(SnakeSound * sound) nogil
    void snake_stop_sound(SnakeSound * sound) nogil
    void snake_close_sound(SnakeSound * sound) nogil
    void snake_set_callback(SnakeSound * sound, 
        void (*callback)(SnakeSound *, void*), void * userdata) nogil
    # get/set for listener
    float snake_get_main_volume() nogil
    void snake_set_main_volume(float value) nogil
    # get/set for SnakeSound
    float snake_get_volume(SnakeSound * sound) nogil
    void snake_set_volume(SnakeSound * sound, float value) nogil
    float snake_get_pitch(SnakeSound * sound) nogil
    void snake_set_pitch(SnakeSound * sound, float value) nogil
    float snake_get_pan(SnakeSound * sound) nogil
    void snake_set_pan(SnakeSound * sound, float value) nogil
    void snake_get_position(SnakeSound * sound, float * x, float * y, float * z
        ) nogil
    void snake_set_position(SnakeSound * sound, float x, float y, float z) nogil
    int snake_get_rate(SnakeSound * sound) nogil
    int snake_get_frequency(SnakeSound * sound) nogil
    void snake_set_frequency(SnakeSound * sound, int frequency) nogil
    long long snake_get_length(SnakeSound * sound) nogil
    long long snake_tell(SnakeSound * sound) nogil
    void snake_seek(SnakeSound * sound, long long pos) nogil
    void snake_set_soundfont(SnakeSound * sound, char * font) nogil

cdef bint opened = False

def open():
    global opened
    if opened:
        return
    opened = True
    PyEval_InitThreads()
    with nogil:
        snake_init_sound()

cdef void on_eos(SnakeSound * sound, void * userdata) with gil:
    cdef Sound new_sound = <Sound>userdata
    if new_sound.callback is not None and not new_sound.closed:
        func, arg, kw = new_sound.callback
        func(*arg, **kw)

cdef list sounds = []

class AudioError(Exception):
    pass

cdef class Sound:
    cdef SnakeSound * sound
    cdef object callback, data
    cdef bint _paused
    cdef object __weakref__
    cdef public:
        bint closed
    
    def __init__(self, data = None, filename = None):
        if not opened:
            raise AudioError('audio device not initialized')
        cdef SnakeSound * sound
        cdef size_t length
        cdef char * c_data
        if data is not None:
            self.data = data
            length = len(data)
            c_data = data
            with nogil:
                sound = snake_load_data(c_data, length)
        elif filename is not None:
            c_data = filename
            with nogil:
                sound = snake_load_file(c_data)
        else:
            raise AudioError('No input specified')
        if sound == NULL:
            self.closed = True
            raise AudioError('Could not load sound')
        sounds.append(self)
        self.closed = False
        self.sound = sound
    
    def __cinit__(self):
        self._paused = False
    
    property volume:
        def __get__(self):
            cdef float volume
            with nogil:
                volume = snake_get_volume(self.sound)
            return volume
            
        def __set__(self, float value):
            with nogil:
                snake_set_volume(self.sound, value)
    
    property pitch:
        def __get__(self):
            cdef float value
            with nogil:
                value = snake_get_pitch(self.sound)
            return value
            
        def __set__(self, float value):
            with nogil:
                snake_set_pitch(self.sound, value)
    
    property pan:
        def __get__(self):
            cdef float value
            with nogil:
                value = snake_get_pan(self.sound)
            return value
        
        def __set__(self, float value):
            with nogil:
                snake_set_pan(self.sound, value)
    
    property position:
        def __get__(self):
            cdef float x, y, z
            with nogil:
                snake_get_position(self.sound, &x, &y, &z)
            return x, y, z
        
        def __set__(self, position):
            cdef float x, y, z
            x, y, z = position
            with nogil:
                snake_set_position(self.sound, x, y, z)
    
    property time:
        @cython.cdivision(True)
        def __get__(self):
            cdef float value
            with nogil:
                value = <float>snake_tell(self.sound) / snake_get_rate(
                    self.sound)
            return value
        
        def __set__(self, float position):
            with nogil:
                snake_seek(self.sound, <long long>(
                    position * snake_get_rate(self.sound)))
    
    property duration:
        @cython.cdivision(True)
        def __get__(self):
            cdef float value
            with nogil:
                value = <float>snake_get_length(self.sound) / snake_get_rate(
                    self.sound)
            return value
    
    property frequency:
        def __get__(self):
            cdef int value
            with nogil:
                value = snake_get_frequency(self.sound)
            return value
        
        def __set__(self, int value):
            with nogil:
                snake_set_frequency(self.sound, value)
    
    property paused:
        def __get__(self):
            return self._paused
        
        def __set__(self, bint value):
            if value == self._paused:
                return
            with nogil:
                if value:
                    snake_pause_sound(self.sound)
                else:
                    snake_resume_sound(self.sound)
            self._paused = value
                
    def play(self, int loops = 1):
        cdef bint value
        with nogil:
            value = snake_play_sound(self.sound, loops)
        if not value:
            raise AudioError('Could not play sound')
    
    def stop(self):
        with nogil:
            snake_stop_sound(self.sound)
    
    cdef void _close(self):
        if self.closed:
            return
        self.closed = True
        self.callback = None
        sounds.remove(self)
        with nogil:
            snake_close_sound(self.sound)
    
    def set_soundfont(self, char * value):
        with nogil:
            snake_set_soundfont(self.sound, value)
    
    def close(self):
        self._close()
    
    def __del__(self):
        self._close()
    
    def set_callback(self, callback, *arg, **kw):
        self.callback = (callback, arg, kw)
        with nogil:
            snake_set_callback(self.sound, on_eos, <void*>self)

cdef class Listener:
    property volume:
        def __get__(self):
            cdef float value
            with nogil:
                value = snake_get_main_volume()
            return value
        
        def __set__(self, float value):
            with nogil:
                snake_set_main_volume(value)

listener = Listener()

import atexit
@atexit.register
def close():
    global opened
    if not opened:
        return
    opened = False
    cdef Sound sound
    for sound in sounds[:]:
        sound._close()
    del sounds[:]
    with nogil:
        snake_deinit_sound()