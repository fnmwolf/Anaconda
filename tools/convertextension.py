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

import sys
sys.path.append('..')
import string
import textwrap
from mmfparser.extension import loadLibrary, LoadedExtension
from mmfparser.data.chunkloaders.parameters import loaders as parameters

if len(sys.argv) < 2:
    print 'Usage: %s <filename>' % sys.argv[0]
    exit()

filename = sys.argv[1]

import os

cwd = os.getcwd()
path = 'C:\\Program Files (x86)\\Multimedia Fusion Developer 2\\Extensions\\%s' % filename
os.chdir('C:\\Program Files (x86)\\Multimedia Fusion Developer 2\\Data\\Runtime')
loadedExtension = LoadedExtension(loadLibrary(path))
os.chdir(cwd)

extension_name = filename.split('.')[0]

template = """\
# %(extension_name)s.mfx
# %(name)s - %(author)s (%(site)s)
# %(copyright)s

%(description)s
# Ported to Python by <insert name here>

from mmfparser.player.extensions.common import UserExtension, HiddenObject
from mmfparser.player.event.actions.common import Action
from mmfparser.player.event.conditions.common import Condition
from mmfparser.player.event.expressions.common import Expression

%(actions)s

%(conditions)s

%(expressions)s

class DefaultObject(HiddenObject):
    def created(self, data):
        # you should probably make your object ready here.
        # data is your property data
        pass

class %(extension_name)s(UserExtension):
    objectPlayer = DefaultObject
    
    actions = {
%(action_dict)s\
    }
    
    conditions = {
%(condition_dict)s\
    }
    
    expressions = {
%(expression_dict)s\
    }

extension = %(extension_name)s()

def get_extension():
    return extension
"""

ace_template = """
class %(name)s(%(class_type)s):
    \"\"\"
    %(doc)s
    \"\"\"

    def %(method_name)s(self, instance):
        raise NotImplementedError('%%s not implemented' %% (
            self.__class__.__name__))
"""

dict_entry_template = """\
        %(key)s : %(value)s,
"""

method_names = {
    'Action' : 'execute',
    'Condition' : 'check',
    'Expression' : 'get'
}

EXPRESSION_PARAMETERS = {
    1 : 'Int',
    2 : 'Global',
    3 : 'String',
    4 : 'AlterableValue',
    5 : 'Flag'
}

def make_classes(aceList, class_type):
    data = ''
    dict_data = ''
    method_name = method_names[class_type]
    for index, item in enumerate(aceList):
        doc = """%s""" % '->'.join(item.menu)
        
        if item.parameters:
            doc += """

    Parameters:"""

        for parameterIndex, parameter in enumerate(item.parameters):
            try:
                if class_type == 'Expression':
                    parameterDescription = EXPRESSION_PARAMETERS[parameter.type]
                else:
                    parameterName = parameters.getName(parameter.type)
                    parameterLoader = parameters.parameterLoaders[parameter.type
                        ].__name__
                    parameterDescription = '%s, %s' % (parameterName, 
                        parameterLoader)
            except (KeyError, IndexError):
                parameterDescription = '(unknown %s)' % parameter.type
            
            doc += """
    %s: %s (%s)""" % (
                parameterIndex, parameter.description, parameterDescription)
        
        if class_type == 'Expression':
            doc += """
    Return type: %s""" % item.getReturnType()
        
        data += ace_template % {
            'name' : class_type + str(index),
            'class_type' : class_type,
            'doc' : doc,
            'method_name' : method_name}
            
        dict_data += dict_entry_template % {
            'key' : item.num,
            'value' : class_type + str(index) 
        }
    return data, dict_data

actions = """\
# Actions
"""
action_data, action_dict = make_classes(loadedExtension.actions, 'Action')
actions += action_data

conditions = """\
# Conditions
"""
condition_data, condition_dict = make_classes(loadedExtension.conditions, 
    'Condition')
conditions += condition_data

expressions = """\
# Expressions
"""
expression_data, expression_dict = make_classes(loadedExtension.expressions, 
    'Expression')
expressions += expression_data

description = ''
for line in textwrap.wrap(loadedExtension.description[3]):
    description += """# %s
""" % line

open('%s.py' % extension_name, 'wb').write(template % {
    'name' : loadedExtension.description[0],
    'author' : loadedExtension.description[1],
    'copyright' : loadedExtension.description[2],
    'description' : description,
    'site' : loadedExtension.description[4],

    'extension_name' : extension_name,
    
    'actions' : actions,
    'conditions' : conditions,
    'expressions' : expressions,
    
    'action_dict' : action_dict,
    'condition_dict' : condition_dict,
    'expression_dict' : expression_dict
}
)