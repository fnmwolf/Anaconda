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

#define compress2 __pyx_f_10extensions_8Lacewing_7wrapper_compress2
#define uncompress __pyx_f_10extensions_8Lacewing_7wrapper_uncompress
#define generate_event __pyx_f_10extensions_8Lacewing_7wrapper_generate_event
#define get_condition_string __pyx_f_10extensions_8Lacewing_7wrapper_get_condition_string
#define get_condition_int __pyx_f_10extensions_8Lacewing_7wrapper_get_condition_int
#define get_action_string __pyx_f_10extensions_8Lacewing_7wrapper_get_action_string
#define get_action_float __pyx_f_10extensions_8Lacewing_7wrapper_get_action_float
#define get_expression_int __pyx_f_10extensions_8Lacewing_7wrapper_get_expression_int
#define get_action_int __pyx_f_10extensions_8Lacewing_7wrapper_get_action_int
#define return_int __pyx_f_10extensions_8Lacewing_7wrapper_return_int
#define return_string __pyx_f_10extensions_8Lacewing_7wrapper_return_string
#define return_float __pyx_f_10extensions_8Lacewing_7wrapper_return_float

static int compress2(unsigned char *, unsigned long *, unsigned char *, unsigned long);
static int uncompress(unsigned char *, unsigned long *, unsigned char *, unsigned long);
static void generate_event(PyObject *, int);
static char *get_condition_string(void);
static int get_condition_int(void);
static char *get_action_string(void);
static float get_action_float(void);
static int get_expression_int(void);
static int get_action_int(void);
static void return_int(int);
static void return_string(char *);
static void return_float(float);

#include "LacewingMMF.cc"
#include "Actions.cc"
#include "Expressions.cc"
#include "Conditions.cc"

void * create_object_c(void * instance, void * data)
{
    LacewingMMF * lacewing = LacewingMMF::create(instance,
        (LacewingMMF::EditData *)data);
    return (void*)lacewing;
}

void update_object_c(void * instance, void * handle)
{
    LacewingMMF * lacewing = (LacewingMMF*)handle;
    lacewing->handleRunObject(instance);
}

void remove_object_c(void * instance, void * handle)
{
    LacewingMMF * lacewing = (LacewingMMF*)handle;
    lacewing->removeExt(instance);
}

void call_action_c(void * instance, void * handle, int key)
{
    LacewingMMF * lacewing = (LacewingMMF*)handle;
    lacewing->action(instance, key);
}

int call_condition_c(void * instance, void * handle, int key)
{
    LacewingMMF * lacewing = (LacewingMMF*)handle;
    return lacewing->condition (instance, key);
}

void call_expression_c(void * instance, void * handle, int key)
{
    LacewingMMF * lacewing = (LacewingMMF*)handle;
    lacewing->expression(instance, key);
}

void LacewingMMF::generateEvent (void * instance, int ID)
{
    generate_event((PyObject *)instance, ID);
}

const char * LacewingMMF::getCndParamString ()
{
    return get_condition_string();
}

int LacewingMMF::getCndParamInt ()
{
    return get_condition_int();
}

const char * LacewingMMF::getActParamString ()
{
    return get_action_string();
}

float LacewingMMF::getActParamFloat ()
{
    return get_action_float();
}

int LacewingMMF::getExpParamInt ()
{
    return get_expression_int();
}

int LacewingMMF::getActParamInt ()
{
    return get_action_int();
}

void LacewingMMF::setExpReturnInt (int value)
{
    return_int(value);
}

void LacewingMMF::setExpReturnString (const char * value)
{
    return_string((char*)value);
}

void LacewingMMF::setExpReturnFloat (float value)
{
    return_float(value);
}