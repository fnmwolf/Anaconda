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

import zlib

cdef class State:
    cdef public:
        object action, condition, expression
        object expression_return
        object action_index, condition_index
        object string_save

cdef State state = State()

cdef extern from "wrapper_c.cc":
    void * create_object_c(void * instance, void * data)
    void * update_object_c(void * instance, void * handle)
    void * remove_object_c(void * instance, void * handle)
    void call_action_c(void * instance, void * handle, int key)
    int call_condition_c(void * instance, void * handle, int key)
    void call_expression_c(void * instance, void * handle, int key)

cdef int compress2(unsigned char * dest, unsigned long * destLen, 
    unsigned char * source, unsigned long sourceLen):
    return 0

cdef int uncompress(unsigned char * dest, unsigned long * destLen, 
    unsigned char * source, unsigned long sourceLen):
    return 0
    
cdef void generate_event(object instance, int id):
    instance.objectPlayer.generate(id)

cdef inline object get_condition_parameter():
    condition = state.condition
    ret = condition.evaluate_expression(condition.get_parameter(
        state.condition_index))
    state.condition_index += 1
    return ret

cdef inline object get_action_parameter():
    action = state.action
    ret = action.evaluate_expression(action.get_parameter(
        state.action_index))
    state.action_index += 1
    return ret

cdef inline object get_expression_parameter():
    return state.expression.next_argument()

cdef char * get_condition_string():
    state.string_save = get_condition_parameter()
    return <char*>(state.string_save)

cdef int get_condition_int():
    return <int>get_condition_parameter()

cdef char * get_action_string():
    state.string_save = get_action_parameter()
    return <char*>(state.string_save)

cdef float get_action_float():
    return <float>get_action_parameter()

cdef int get_expression_int():
    return <int>get_expression_parameter()

cdef int get_action_int():
    return <int>get_action_parameter()

cdef void return_int(int value):
    state.expression_return = value

cdef void return_string(char * value):
    state.expression_return = value

cdef void return_float(float value):
    state.expression_return = value
    
cdef class Client:
    cdef void * handle
    cdef object instance
    def __init__(self, instance, data):
        self.instance = instance
        extension_data = data.read()
        self.handle = create_object_c(<void*>instance, 
            <void*>(<char*>extension_data))
    
    def update(self):
        update_object_c(<void*>self.instance, self.handle)
    
    def on_detach(self):
        remove_object_c(<void*>self.instance, self.handle)

def create_object(instance, data):
    return Client(instance, data)

cdef inline Client get_client(instance):
    return <Client>instance.objectPlayer.client

def call_action(instance, action, key):
    old_action = state.action
    old_index = state.action_index
    state.action = action
    state.action_index = 0
    call_action_c(<void*>instance, get_client(instance).handle, key)
    state.action = old_action
    state.action_index = old_index

def call_condition(instance, condition, key):
    old_condition = state.condition
    old_index = state.condition_index
    state.condition = condition
    state.condition_index = 0
    ret = call_condition_c(<void*>instance, get_client(instance).handle, key)
    state.condition = old_condition
    state.condition_index = old_index
    return ret

def call_expression(instance, expression, key):
    old_expression = state.expression
    state.expression = expression
    call_expression_c(<void*>instance, get_client(instance).handle, key)
    state.expression = old_expression
    return state.expression_return