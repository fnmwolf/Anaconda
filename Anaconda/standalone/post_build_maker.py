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
import os
import mmfparser.player.main
import mmfparser.player.extensions
import mmfparser.player.movements.all
import mmfparser.player.event.conditions.all
import mmfparser.player.event.expressions.all
import mmfparser.player.event.actions.all
import mmfparser.data.chunkloaders.all
import mmfparser.data.chunkloaders.onepointfive.all
import pyglet
sys.is_epydoc = True # no OpenGL thank you

path = os.path.dirname(mmfparser.player.extensions.__file__)

names = set([item.split('.')[0] for item in os.listdir(path)])
names -= set(['common', '__init__'])

def dummy_function(*arg, **kw):
    return None

extensions = []
movements = []

for item in names:
    if not item:
        continue
    extension = mmfparser.player.extensions.get_extension(item)
    movement = mmfparser.player.extensions.get_movement(item)
    if extension is not None:
        extensions.append(item)
    if movement is not None:
        movements.append(item)

print extensions
print ''
print movements

import json

data = {'extensions' : extensions, 'movements' : movements}
data = json.dumps(data)
open('./dist/extensions.dat', 'wb').write(data)