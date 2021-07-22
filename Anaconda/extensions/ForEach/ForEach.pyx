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

"""
ForEach.mfx
ForEach - Ross Pickett (http://rossp.webs.com)

Iterates through and helps manage instances of objects.

Ported to Python by Mathias Kaerlev
"""

from mmfparser.player.extensions.common import UserExtension
from mmfparser.player.objects.common cimport ObjectPlayer
from mmfparser.player.event.actions.common cimport Action
from mmfparser.player.event.conditions.common cimport Condition, TrueCondition
from mmfparser.player.event.expressions.common cimport Expression
from mmfparser.player.instance cimport Instance
import time

# Actions

cdef class Action0(Action):
    """
    Start ForEach loop for object...

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    1: Select object... (OBJECT, Object)
    """
    cdef:
        str name
        object loop_object_info
        bint initialized

    cdef void created(self):
        self.initialized = False

    cdef void execute_instance(self, Instance instance):
        if not self.initialized:
            self.name = self.evaluate_index(0)
            self.loop_object_info = self.get_parameter(1).objectInfo
            self.initialized = True
        objects = self.get_instances(self.loop_object_info)
        (<DefaultObject>instance.objectPlayer).start_loop(self.name, objects)

cdef class Action1(Action):
    """
    Pause ForEach loop...

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).pause_loop(self.evaluate_expression(
            self.get_parameter(0)))

cdef class Action2(Action):
    """
    Resume ForEach loop...

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).resume_loop(self.evaluate_expression(
            self.get_parameter(0)))

cdef class Action3(Action):
    """
    Set ForEach loop's iteration...

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    1: Enter iteration index of loop (EXPRESSION, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).set_loop_index(
            self.evaluate_index(0),
            self.evaluate_index(1))

cdef class Action4(Action):
    """
    Groups->Start ForEach loop for group...

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    1: Enter group's name (EXPSTRING, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        name = self.evaluate_index(0)
        group = self.evaluate_index(1)
        (<DefaultObject>instance.objectPlayer).start_group_loop(name, group)

cdef class Action5(Action):
    """
    Groups->Add object(s) to group...

    Parameters:
    0: Select object... (OBJECT, Object)
    1: Enter group's name (EXPSTRING, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        objects = self.get_instances(self.get_parameter(0).objectInfo)
        group = self.evaluate_index(1)
        (<DefaultObject>instance.objectPlayer).add_objects(objects, group)

cdef class Action6(Action):
    """
    Groups->Add object to group by Fixed Value...

    Parameters:
    0: Enter an object's Fixed Value (EXPRESSION, ExpressionParameter)
    1: Enter group's name (EXPSTRING, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        fixed = self.evaluate_index(0)
        fixed_object = self.player.frame.get_fixed_object(fixed)
        if fixed_object is None:
            return
        group = self.evaluate_index(1)
        (<DefaultObject>instance.objectPlayer).add_objects([fixed_object], group)

cdef class Action7(Action):
    """
    Groups->Remove object(s) from group...

    Parameters:
    0: Select object... (OBJECT, Object)
    1: Enter group's name (EXPSTRING, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        objects = self.get_instances(self.get_parameter(0).objectInfo)
        group = self.evaluate_index(1)
        (<DefaultObject>instance.objectPlayer).remove_objects(objects, group)

cdef class Action8(Action):
    """
    Groups->Remove object from group by Fixed Value...

    Parameters:
    0: Enter an object's Fixed Value (EXPRESSION, ExpressionParameter)
    1: Enter group's name (EXPSTRING, ExpressionParameter)
    """

    cdef void execute_instance(self, Instance instance):
        fixed = self.evaluate_index(0)
        fixed_object = self.player.frame.get_fixed_object(fixed)
        if fixed_object is None:
            return
        group = self.evaluate_index(1)
        (<DefaultObject>instance.objectPlayer).remove_objects([fixed_object], group)

# Conditions

cdef class Condition0(TrueCondition):
    """
    On ForEach loop

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    """
    cdef:
        str name
    
    cdef void created(self):
        self.add_handlers(object_loop = self.object_loop)
    
    def object_loop(self, currentObject):
        self.generate()
    
    cdef str get_name(self):
        if self.name is None:
            self.name = self.evaluate_index(0)
        return self.name

cdef class Condition1(Condition):
    """
    ForEach loop is paused

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    """

    cdef bint check_instance(self, Instance instance):
        try:
            loop = (<DefaultObject>instance.objectPlayer).loops[
                self.evaluate_index(0)]
            return loop.paused
        except KeyError:
            return False

cdef class Condition2(Condition):
    """
    Object is part of ForEach loop

    Parameters:
    0: Select object... (OBJECT, Object)
    1: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    """

    cdef bint check_instance(self, Instance instance):
        try:
            loop = (<DefaultObject>instance.objectPlayer).loops[
                self.evaluate_index(1)]
        except KeyError:
            return False
        objects = self.get_instances(self.get_parameter(0).objectInfo)
        return set(objects) <= set(loop.objects)

cdef class Condition3(Condition):
    """
    Groups->Object is part of group

    Parameters:
    0: Select object... (OBJECT, Object)
    1: Enter group's name (EXPSTRING, ExpressionParameter)
    """

    cdef bint check_instance(self, Instance instance):
        try:
            groupObjects = (<DefaultObject>instance.objectPlayer).groups[
                self.evaluate_index(1)]
        except KeyError:
            return False
        objects = self.get_instances(self.get_parameter(0).objectInfo)
        return set(objects) <= set(groupObjects)

cdef class Condition4(Condition):
    """
    On ForEach loop for object

    Parameters:
    0: Enter ForEach loop's name (EXPSTRING, ExpressionParameter)
    1: Select object... (OBJECT, Object)
    """
    cdef:
        str name
        object currentObject, objectHandle
        set handles

    cdef void created(self):
        if self.isTriggered:
            self.add_handlers(object_loop = self.object_loop)
        self.objectHandle = self.get_parameter(1).objectInfo
        self.handles = set(self.resolve_objects(self.objectHandle))
        
    def object_loop(self, currentObject):
        self.currentObject = currentObject
        self.generate()
    
    cdef str get_name(self):
        if self.name is None:
            self.name = self.evaluate_index(0)
        return self.name
    
    def get_handles(self):
        return self.handles
    
    cdef bint check_instance(self, Instance instance):
        if self.currentObject is None:
            return False
        self.select_instances([self.currentObject], self.objectHandle)
        return True

# Expressions

cdef class Expression0(Expression):
    """
    Get ForEach loop's current iterating Fixed Value

    Parameters:
    0: Enter ForEach loop's name (String)
    Return type: Int
    """

    cdef object evaluate_instance(self, Instance instance):
        try:
            loop = (<DefaultObject>instance.objectPlayer).loops[self.next_argument()]
            return id(loop.objects[loop.index])
        except (KeyError, IndexError):
            return -1

cdef class Expression1(Expression):
    """
    Get ForEach's current iteration

    Parameters:
    0: Enter ForEach loop's name (String)
    Return type: Int
    """

    cdef object evaluate_instance(self, Instance instance):
        try:
            loop = (<DefaultObject>instance.objectPlayer).loops[self.next_argument()]
            return loop.index
        except KeyError:
            return -1

cdef class Expression2(Expression):
    """
    Get ForEach's maximum iteration

    Parameters:
    0: Enter ForEach loop's name (String)
    Return type: Int
    """

    cdef object evaluate_instance(self, Instance instance):
        try:
            loop = (<DefaultObject>instance.objectPlayer).loops[self.next_argument()]
            return len(loop.objects) - 1
        except KeyError:
            return -1

cdef class Expression3(Expression):
    """
    Groups->Get number of objects in group

    Parameters:
    0: Enter group's name (String)
    Return type: Int
    """

    cdef object evaluate_instance(self, Instance instance):
        try:
            return len((<DefaultObject>instance.objectPlayer).groups[self.next_argument()])
        except KeyError:
            return 0

cdef class Loop:
    cdef public:
        int index
        bint paused
        str name
        list objects

    def __init__(self, name, objects):
        self.name = name
        self.objects = objects
        self.index = 0
        self.paused = False

cdef class DefaultObject(ObjectPlayer):
    cdef public:
        dict groups, loops, conditions, object_conditions
        list objects
        bint initialized

    def created(self):
        self.groups = {}
        self.loops = {}
        self.conditions = {}
        self.object_conditions = {}
        self.objects = None
        self.initialized = False
    
    cdef void start_loop(self, name, objects):
        # if name == 'Apply Modifiers':
        #     print len(objects)
        #     import code
        #     code.interact(local = locals())
        cdef Loop loop
        try:
            loop = self.loops[name]
            loop.index = 0
            loop.objects = objects
        except KeyError:
            loop = Loop(name, objects)
            self.loops[name] = loop
        # current_time = time.clock()
        self.run(loop)
        # dt = time.clock() - current_time
        # if dt > 0.005:
        # print name, dt, len(objects)

    cdef void start_group_loop(self, name, group):
        try:
            objects = self.groups[group]
        except KeyError:
            return
        self.start_loop(name, objects)
    
    cdef void add_objects(self, objects, group):
        try:
            self.groups[group].extend(objects)
        except KeyError:
            self.groups[group] = objects
    
    cdef void remove_objects(self, objects, group):
        try:
            groupObjects = self.groups[group]
            for item in objects:
                groupObjects.remove(item)
        except KeyError:
            pass
        
    cdef void run(self, loop):
        if not self.initialized:
            self.initialized = True
            try:
                all_conditions = self.player.eventPlayer.conditions[
                    Condition0]
            except KeyError:
                all_conditions = []
            try:
                all_object_conditions = self.player.eventPlayer.conditions[
                    Condition4]
            except KeyError:
                all_object_conditions = []
            
            for condition in all_conditions:
                name = (<Condition0>condition).get_name()
                if name not in self.conditions:
                    self.conditions[name] = [condition]
                else:
                    self.conditions[name].append(condition)
            for condition in all_object_conditions:
                name = (<Condition4>condition).get_name()
                handles = condition.get_handles()
                if name not in self.object_conditions:
                    self.object_conditions[name] = {}
                object_dict = self.object_conditions[name]
                for handle in handles:
                    if handle not in object_dict:
                        object_dict[handle] = [condition]
                    else:
                        object_dict[handle].append(condition)

        name = loop.name
        try:
            conditions = self.conditions[name]
        except KeyError:
            conditions = []
        try:
            object_conditions = self.object_conditions[name]
        except KeyError:
            object_conditions = []

        while not loop.paused and loop.index < len(loop.objects):
            currentObject = loop.objects[loop.index]
            if not currentObject.destroyed:
                for condition in conditions:
                    condition.object_loop(currentObject)
                try:
                    for condition in object_conditions[currentObject.handle]:
                        condition.object_loop(currentObject)
                except KeyError:
                    pass
            loop.index += 1
    
    cdef void set_loop_iteration(self, name, index):
        try:
            self.loops[name].index = index
        except KeyError:
            return
    
    cdef void pause_loop(self, name):
        try:
            self.loops[name].paused = True
        except KeyError:
            return

    cdef void resume_loop(self, name):
        try:
            self.loops[name].paused = False
        except KeyError:
            return

class ForEach(UserExtension):
    objectPlayer = DefaultObject
    
    actions = {
        0 : Action0,
        1 : Action1,
        2 : Action2,
        3 : Action3,
        4 : Action4,
        5 : Action5,
        6 : Action6,
        7 : Action7,
        8 : Action8,
    }
    
    conditions = {
        0 : Condition0,
        1 : Condition1,
        2 : Condition2,
        3 : Condition3,
        4 : Condition4,
    }
    
    expressions = {
        0 : Expression0,
        1 : Expression1,
        2 : Expression2,
        3 : Expression3,
    }

extension = ForEach()

def get_extension():
    return extension
