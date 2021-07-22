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
import os

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = []
libraries = []

library_dirs = ['./alure/build', './lib/']
include_dirs = ['./alure/include', os.getcwd()]
extra_link_args = []

if sys.platform == 'win32':
    libraries.extend(['ALURE32-static', 'OpenAL32', 'libsndfile-1'])
    library_dirs.extend(['./openal-soft/lib',
                         'C:\Programs\Mega-Nerd\libsndfile\lib'])
    include_dirs.extend(['./openal-soft/include', './openal-soft/include/AL'])
elif sys.platform in ('linux2', 'darwin'):
    libraries.extend(['openal'])
if sys.platform != 'win32':
    libraries.extend(['alure-static'])
    
ext_modules.append(Extension('snakesound', 
    [
        './snakesound.pyx',
    ], include_dirs = include_dirs, libraries = libraries,
    library_dirs = library_dirs, extra_link_args = extra_link_args,
    language = 'c++'))

setup(
    name = 'snakesound',
    cmdclass = {'build_ext' : build_ext},
    ext_modules = ext_modules
)