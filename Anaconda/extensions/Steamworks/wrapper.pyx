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

cdef extern from "Main.cpp":
    void * c_create_object "create_object" ()
    void update_object()
    void * destroy_object(void * handle)
    int is_initialized(void * handle)
    void set_achievement(char * name)
    void request_data()
    void set_int_statistic(char * name, int value)
    void set_float_statistic(char * name, float value)
    void update_avgrate(char * statistic, float value, float duration)
    void store_data(void * handle)
    int get_app_id()
    char * get_player_name()
    char * get_achievement_api(void * handle)
    unsigned int get_current_progress(void * handle)
    unsigned int get_max_progress(void * handle)
    char * get_achievement_display(char * achievement)
    char * get_achievement_description(char * achievement)
    float get_statistics_int(char * statistic)
    float get_statistics_float(char * statistic)

cdef class Object:
    cdef void * handle
    cdef public:
        object parent
    
    def update(self):
        update_object()
    
    def close(self):
        destroy_object(self.handle)
    
    def is_initialized(self):
        return is_initialized(self.handle)
    
    def set_achievement(self, name):
        set_achievement(name)
    
    def request_data(self):
        request_data()
    
    def set_int_statistic(self, name, value):
        set_int_statistic(name, value)
    
    def set_float_statistic(self, name, value):
        set_float_statistic(name, value)
    
    def update_avgrate(self, statistic, value, duration):
        update_avgrate(statistic, value, duration)
    
    def store_data(self):
        store_data(self.handle)
    
    def get_app_id(self):
        return get_app_id()
    
    def get_player_name(self):
        return get_player_name()
    
    def get_achievement_api(self):
        return get_achievement_api(self.handle)
    
    def get_current_progress(self):
        return get_current_progress(self.handle)
    
    def get_max_progress(self):
        return get_max_progress(self.handle)
    
    def get_achievement_display(self, name):
        return get_achievement_display(name)
    
    def get_achievement_description(self, name):
        return get_achievement_description(name)
        
    def get_statistics_int(self, name):
        return get_statistics_int(name)
    
    def get_statistics_float(self, name):
        return get_statistics_float(name)

cdef Object global_object = None

cdef void generate_event(int id):
    if global_object is None:
        return
    global_object.parent.generate(id)

def create_object():
    global global_object
    if global_object is not None:
        return global_object
    cdef Object object = Object()
    object.handle = c_create_object()
    return object