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
KcArray.mfx
Array object - ClickTeam (http://www.clickteam.com)
Copyright 1996-2006 Clickteam

Numeric or text array (up to 3 dimensions).

Ported to Python by Mathias Kaerlev
"""

from mmfparser.player.common import convert_path
from mmfparser.player.extensions.common import UserExtension
from mmfparser.player.objects.common cimport ObjectPlayer
from mmfparser.player.event.actions.common cimport Action
from mmfparser.player.event.conditions.common cimport Condition
from mmfparser.player.event.expressions.common cimport Expression
from mmfparser.player.instance cimport Instance

from mmfparser.bitdict import BitDict
from mmfparser.bytereader cimport ByteReader
from mmfparser.exceptions import InvalidData
from mmfparser.loader cimport DataLoader

from cpython.ref cimport PyObject
from libc.stdlib cimport malloc, free

try:
    from mmfparser.player.dialog import open_file_selector, save_file_selector
except ImportError:
    pass

MAGIC = 'CNC ARRAY'

MAJOR_VERSION = 2
MINOR_VERSION = 0

cdef class MMFArray(DataLoader):
    cdef:
        int index_offset
        unsigned int x_size, y_size, z_size
        object flags
        object default
        list array

    cpdef initialize(self):
        self.flags = BitDict('Numeric', 'Text', 'Base1', 'Global')
        self.flags.setFlags(0) # to prevent BitDict from complaining :)
        self.flags['Base1'] = True # default
        self.flags['Text'] = True # default
        self.default = ''
        self.index_offset = <int>self.flags['Base1']
        
    cpdef read(self, ByteReader reader):
        if reader.readString() != MAGIC:
            raise InvalidData('data is invalid')
        if reader.readShort() != MAJOR_VERSION:
            raise InvalidData('major version incompatibility')
        if reader.readShort() != MINOR_VERSION:
            raise InvalidData('minor version incompatibility')
        self.x_size = reader.readInt(True)
        self.y_size = reader.readInt(True)
        self.z_size = reader.readInt(True)
        self.flags.setFlags(reader.readInt())
        if self.flags['Numeric']:
            self.default = 0
        elif self.flags['Text']:
            self.default = ''
        else:
            raise NotImplementedError(
                'invalid array type (should be "Text" or "Numeric")')
        self.clear()
        cdef int x, y, z, i
        i = 0
        for z in range(self.z_size):
            for y in range(self.y_size):
                for x in range(self.x_size):
                    if self.flags['Numeric']:
                        item = reader.readInt(True)
                    elif self.flags['Text']:
                        item = reader.read(reader.readInt(True))
                    else:
                        raise InvalidData('invalid flags')
                    self.array[i] = item
                    i += 1

    cdef inline int get_index(self, int x, int y, int z):
        return x + y * self.x_size + z * self.x_size * self.y_size
    
    def write(self, ByteReader reader):
        reader.writeString(MAGIC)
        reader.writeShort(MAJOR_VERSION)
        reader.writeShort(MINOR_VERSION)
        reader.writeInt(self.x_size)
        reader.writeInt(self.y_size)
        reader.writeInt(self.z_size)
        reader.writeInt(self.flags.getFlags())
        cdef int x, y, z
        cdef bint numeric = self.flags['Numeric']
        for z in range(self.z_size):
            for y in range(self.y_size):
                for x in range(self.x_size):
                    item = self.array[self.get_index(x, y, z)]
                    if numeric:
                        reader.writeInt(item)
                    else:
                        item = str(item)
                        reader.writeInt(len(item))
                        reader.write(item)

    cdef object get(self, int x, int y, int z):
        x -= self.index_offset
        y -= self.index_offset
        z -= self.index_offset
        try:
            return self.array[self.get_index(x, y, z)]
        except IndexError:
            return self.default
    
    cdef void set(self, int x, int y, int z, value):
        x -= self.index_offset
        y -= self.index_offset
        z -= self.index_offset
        try:
            self.array[self.get_index(x, y, z)] = value
        except IndexError:
            return
            
    cdef void setup(self, xDimension, yDimension, zDimension, arrayType):
        if arrayType == 'Text':
            self.flags['Numeric'] = False
            self.flags['Text'] = True
            self.default = ''
        elif arrayType == 'Numeric':
            self.flags['Numeric'] = True
            self.flags['Text'] = False
            self.default = 0
        else:
            raise NotImplementedError(
                'invalid array type (should be "Text" or "Numeric")')
        self.x_size = xDimension
        self.y_size = yDimension
        self.z_size = zDimension
        self.clear()
    
    cdef void clear(self):
        self.array = [self.default] * (self.x_size * self.y_size * self.z_size)


# Actions

# cdef inline DefaultObject get_object(Instance instance):
#     return (<DefaultObject>instance.objectPlayer)

cdef class SetXDimension(Action):
    """
    Change Current Position->Set X dimension index to...

    Parameters:
    0: Set X dimension index to... (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).set_x(self.evaluate_index(0))

cdef class SetYDimension(Action):
    """
    Change Current Position->Set Y dimension index to...

    Parameters:
    0: Set Y dimension index to... (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).set_y(self.evaluate_index(0))

cdef class SetZDimension(Action):
    """
    Change Current Position->Set Z dimension index to...

    Parameters:
    0: Set Z dimension index to... (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).set_z(self.evaluate_index(0))

cdef class AddOneX(Action):
    """
    Change Current Position->Add 1 to X dimension index
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).array_x += 1

cdef class AddOneY(Action):
    """
    Change Current Position->Add 1 to Y dimension index
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).array_y += 1

cdef class AddOneZ(Action):
    """
    Change Current Position->Add 1 to Z dimension index
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).array_z += 1

cdef class WriteValue(Action):
    """
    Write->Write Value to current position

    Parameters:
    0: Write Value to current position (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).write_value(self.evaluate_index(0), -1, -1, -1)

cdef class WriteString(Action):
    """
    Write->Write String to current position

    Parameters:
    0: Write String to current position (EXPSTRING, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).write_value(self.evaluate_index(0), -1, -1, -1)

cdef class Clear(Action):
    """
    Clear array
    """
    cdef void execute_instance(self, Instance instance):
        (<DefaultObject>instance.objectPlayer).array.clear()

cdef class Load(Action):
    """
    Files->Load array from file

    Parameters:
    0: Please select an array file (FILENAME, String)
    """
    cdef void execute_instance(self, Instance instance):
        filename = convert_path(
            self.get_filename(self.get_parameter(0)))
        try:
            reader = ByteReader(open(filename, 'rb'))
            (<DefaultObject>instance.objectPlayer).array.read(reader)
        except IOError:
            pass

cdef class LoadWithSelector(Action):
    """
    Files->Load array from file via a file selector
    """
    cdef void execute_instance(self, Instance instance):
        try:
            filename = open_file_selector()
            reader = ByteReader(open(filename, 'rb'))
            (<DefaultObject>instance.objectPlayer).array.read(reader)
            file.close()
        except IOError:
            pass

cdef class Save(Action):
    """
    Files->Save array to file

    Parameters:
    0: Please select an array file (FILENAME, String)
    """
    cdef void execute_instance(self, Instance instance):
        filename = convert_path(
            self.get_filename(self.get_parameter(0)))
        try:
            open(filename, 'wb').write(
                (<DefaultObject>instance.objectPlayer).array.generate().data())
        except IOError:
            pass

cdef class SaveWithSelector(Action):
    """
    Files->Save array to file via a file selector
    """
    cdef void execute_instance(self, Instance instance):
        try:
            filename = save_file_selector()
            file = open(filename, 'wb')
            file.write(
                (<DefaultObject>instance.objectPlayer).array.generate().data())
            file.close()
        except IOError:
            pass

cdef class WriteX(Action):
    """
    Write->Write Value to X

    Parameters:
    0: Enter value to write (EXPRESSION, ExpressionParameter)
    1: Enter X index (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        value = self.evaluate_index(0)
        x = self.evaluate_index(1)
        (<DefaultObject>instance.objectPlayer).write_value(value, x, -1, -1)

cdef class WriteXY(Action):
    """
    Write->Write Value to XY

    Parameters:
    0: Enter value to write (EXPRESSION, ExpressionParameter)
    1: Enter X index (EXPRESSION, ExpressionParameter)
    2: Enter Y index (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        value = self.evaluate_index(0)
        cdef int x = self.evaluate_index(1)
        cdef int y = self.evaluate_index(2)
        (<DefaultObject>instance.objectPlayer).write_value(value, x, y, -1)

cdef class WriteXYZ(Action):
    """
    Write->Write Value to XYZ

    Parameters:
    0: Enter value to write (EXPRESSION, ExpressionParameter)
    1: Enter X index (EXPRESSION, ExpressionParameter)
    2: Enter Y index (EXPRESSION, ExpressionParameter)
    3: Enter Z index (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        value = self.evaluate_index(0)
        cdef int x = self.evaluate_index(1)
        cdef int y = self.evaluate_index(2)
        cdef int z = self.evaluate_index(3)
        (<DefaultObject>instance.objectPlayer).write_value(value, x, y, z)

cdef class WriteStringX(Action):
    """
    Write->Write String to X

    Parameters:
    0: Enter string to write (EXPSTRING, ExpressionParameter)
    1: Enter X index (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        value = self.evaluate_index(0)
        cdef int x = self.evaluate_index(1)
        (<DefaultObject>instance.objectPlayer).write_value(value, x, -1, -1)

cdef class WriteStringXY(Action):
    """
    Write->Write String to XY

    Parameters:
    0: Enter string to write (EXPSTRING, ExpressionParameter)
    1: Enter X index (EXPRESSION, ExpressionParameter)
    2: Enter Y index (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        value = self.evaluate_index(0)
        cdef int x = self.evaluate_index(1)
        cdef int y = self.evaluate_index(2)
        (<DefaultObject>instance.objectPlayer).write_value(value, x, y, -1)

cdef class WriteStringXYZ(Action):
    """
    Write->Write String to XYZ

    Parameters:
    0: Enter string to write (EXPSTRING, ExpressionParameter)
    1: Enter X index (EXPRESSION, ExpressionParameter)
    2: Enter Y index (EXPRESSION, ExpressionParameter)
    3: Enter Z index (EXPRESSION, ExpressionParameter)
    """
    cdef void execute_instance(self, Instance instance):
        value = self.evaluate_index(0)
        cdef int x = self.evaluate_index(1)
        cdef int y = self.evaluate_index(2)
        cdef int z = self.evaluate_index(3)
        (<DefaultObject>instance.objectPlayer).write_value(value, x, y, z)

# Conditions

cdef class AtEndX(Condition):
    """
    Is the index to the X dimension at end?
    """
    cdef bint check_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array_x == (<DefaultObject>instance.objectPlayer).array.x_size-1

cdef class AtEndY(Condition):
    """
    Is the index to the Y dimension at end?
    """
    cdef bint check_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array_x == (<DefaultObject>instance.objectPlayer).array.y_size-1


cdef class AtEndZ(Condition):
    """
    Is the index to the Z dimension at end?
    """
    cdef bint check_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array_x == (<DefaultObject>instance.objectPlayer).array.z_size-1

# Expressions

cdef class PositionX(Expression):
    """
    Current position of X index
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array_x

cdef class PositionY(Expression):
    """
    Current position of Y index
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array_y

cdef class PositionZ(Expression):
    """
    Current position of Z index
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array_z

cdef class ReadValue(Expression):
    """
    Read Value from current position
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).read_value(-1, -1, -1)

cdef class ReadValueX(Expression):
    """
    Read Value from X position

    Parameters:
    0: Enter X offset (Int)
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        cdef int x = self.next_argument()
        return (<DefaultObject>instance.objectPlayer).read_value(x, -1, -1)

cdef class ReadValueXY(Expression):
    """
    Read Value from XY position

    Parameters:
    0: Enter X offset (Int)
    1: Enter Y offset (Int)
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        cdef int x = self.next_argument()
        cdef int y = self.next_argument()
        return (<DefaultObject>instance.objectPlayer).read_value(x, y, -1)

cdef class ReadValueXYZ(Expression):
    """
    Read Value from XYZ position

    Parameters:
    0: Enter X offset (Int)
    1: Enter Y offset (Int)
    2: Enter Z offset (Int)
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        cdef int x = self.next_argument()
        cdef int y = self.next_argument()
        cdef int z = self.next_argument()
        return (<DefaultObject>instance.objectPlayer).read_value(x, y, z)

cdef class ReadString(Expression):
    """
    Read String from current position
    Return type: String
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).read_value(-1, -1, -1)

cdef class ReadStringX(Expression):
    """
    Read String from X position

    Parameters:
    0: Enter X offset (Int)
    Return type: String
    """
    cdef object evaluate_instance(self, Instance instance):
        cdef int x = self.next_argument()
        return (<DefaultObject>instance.objectPlayer).read_value(x, -1, -1)

cdef class ReadStringXY(Expression):
    """
    Read String from XY position

    Parameters:
    0: Enter X offset (Int)
    1: Enter Y offset (Int)
    Return type: String
    """
    cdef object evaluate_instance(self, Instance instance):
        cdef int x = self.next_argument()
        cdef int y = self.next_argument()
        return (<DefaultObject>instance.objectPlayer).read_value(x, y, -1)

cdef class ReadStringXYZ(Expression):
    """
    Read String from XYZ position

    Parameters:
    0: Enter X offset (Int)
    1: Enter Y offset (Int)
    2: Enter Z offset (Int)
    Return type: String
    """
    cdef object evaluate_instance(self, Instance instance):
        cdef int x = self.next_argument()
        cdef int y = self.next_argument()
        cdef int z = self.next_argument()
        return (<DefaultObject>instance.objectPlayer).read_value(x, y, z)

cdef class DimensionX(Expression):
    """
    X dimension
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array.x_size

cdef class DimensionY(Expression):
    """
    Y dimension
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array.y_size
    
cdef class DimensionZ(Expression):
    """
    Z dimension
    Return type: Int
    """
    cdef object evaluate_instance(self, Instance instance):
        return (<DefaultObject>instance.objectPlayer).array.z_size

cdef class DefaultObject(ObjectPlayer):
    cdef:
        int array_x, array_y, array_z
        MMFArray array

    def created(self, data):
        cdef dict storage = self.get_storage()
        self.array_x = self.array_y = self.array_z = 0
        if storage:
            self.array = storage['value']
            return
        cdef int x_size = data.readInt()
        cdef int y_size = data.readInt()
        cdef int z_size = data.readInt()
        flags = BitDict('Numeric', 'Text', 'Base1', 'Global')
        flags.setFlags(data.readInt())
        if flags['Base1']:
            self.array_x = self.array_y = self.array_z = 1
        self.array = MMFArray()
        if flags['Numeric']:
            array_type = 'Numeric'
        else:
            array_type = 'Text'
        self.array.flags = flags
        self.array.setup(x_size, y_size, z_size, array_type)
        storage['value'] = self.array
    
    cdef void set_x(self, int x):
        self.array_x = x

    cdef void set_y(self, int y):
        self.array_y = y

    cdef void set_z(self, int z):
        self.array_z = z

    cdef void write_value(self, value, int x, int y, int z):
        if x == -1:
            x = self.array_x
        if y == -1:
            y = self.array_y
        if z == -1:
            z = self.array_z
        self.array.set(x, y, z, value)
    
    cdef object read_value(self, x, y, z):
        if x == -1:
            x = self.array_x
        if y == -1:
            y = self.array_y
        if z == -1:
            z = self.array_z
        return self.array.get(x, y, z)
    
    cdef void set_index(self, x, y, z):
        if x != -1:
            self.array_x = x
        if y != -1:
            self.array_y = y
        if z != -1:
            self.array_z = z

class KcArray(UserExtension):
    objectPlayer = DefaultObject
    
    actions = {
        0 : SetXDimension,
        1 : SetYDimension,
        2 : SetZDimension,
        3 : AddOneX,
        4 : AddOneY,
        5 : AddOneZ,
        6 : WriteValue,
        7 : WriteString,
        8 : Clear,
        9 : Load,
        10 : LoadWithSelector,
        11 : Save,
        12 : SaveWithSelector,
        13 : WriteX,
        14 : WriteXY,
        15 : WriteXYZ,
        16 : WriteStringX,
        17 : WriteStringXY,
        18 : WriteStringXYZ
    }
    
    conditions = {
        0 : AtEndX,
        1 : AtEndY,
        2 : AtEndZ
    }
    
    expressions = {
        0 : PositionX,
        1 : PositionY,
        2 : PositionZ,
        3 : ReadValue,
        4 : ReadString,
        5 : ReadValueX,
        6 : ReadValueXY,
        7 : ReadValueXYZ,
        8 : ReadStringX,
        9 : ReadStringXY,
        10 : ReadStringXYZ,
        11 : DimensionX,
        12 : DimensionY,
        13 : DimensionZ
    }

extension = KcArray()

def get_extension():
    return extension
