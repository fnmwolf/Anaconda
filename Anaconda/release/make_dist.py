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

import yaml
import os
import shutil
import subprocess
import time
import sys

from getrev import get_hg_rev

try:
    COMMERCIAL = bool(int(sys.argv[1]))
except IndexError:
    COMMERCIAL = True

ANACONDA_FILES = ['darwin.zip', 'linux2.zip', 'win32.zip', 'make.exe', 
                  'png2ico.exe', 'extensions', 'extensions.dat']
RUNTIME_FILES = ['Anaconda.bld']
EXTENSION_FILES = []
EXAMPLE_FOLDERS = ['Anaconda']
TOOL_FILES = ['signhelper.py']
MMF2_DIRECTORY = 'D:\\Multimedia Fusion Developer 2\\'
OTHER_FILES = ['README.txt', 'CHANGELOG.txt']

# add custom EDIF extensions
for item in ('AnacondaShaders',):
    RUNTIME_FILES.append('%s.json' % item)
    RUNTIME_FILES.append('%s.png' % item)
    EXTENSION_FILES.append('%s.mfx' % item)
    # EXAMPLE_FOLDERS.append(item)

shutil.rmtree('tmp', ignore_errors = True)
shutil.rmtree('dist', ignore_errors = True)
os.mkdir('tmp')
os.mkdir('dist')

os.makedirs('tmp/Data/Runtime/Anaconda')
os.makedirs('tmp/Extensions')
os.makedirs('tmp/Examples')
os.makedirs('tmp/Tools/Anaconda')

runtime_directory = os.path.join(MMF2_DIRECTORY, 'Data', 'Runtime')
extensions_directory = os.path.join(MMF2_DIRECTORY, 'Extensions')
examples_directory = os.path.join(MMF2_DIRECTORY, 'Examples')
tools_directory = os.path.join('..', 'tools')

def ignore_filter(dir, items):
    ignored = []
    for item in items:
        if item.split('.')[-1].startswith('00'):
            ignored.append(item)
    return ignored

def copy(source, dest):
    if os.path.isdir(source):
        shutil.copytree(source, dest, ignore = ignore_filter)
    else:
        shutil.copy(source, dest)

for file in RUNTIME_FILES:
    copy(os.path.join(runtime_directory, file),
         os.path.join('tmp', 'Data', 'Runtime', file))

for file in EXTENSION_FILES:
    copy(os.path.join(extensions_directory, file),
         os.path.join('tmp', 'Extensions', file))

for file in TOOL_FILES:
    copy(os.path.join(tools_directory, file),
         os.path.join('tmp', 'Tools', 'Anaconda', file))

for dir in EXAMPLE_FOLDERS:
    copy(os.path.join(examples_directory, dir),
         os.path.join('tmp', 'Examples', dir))

anaconda_directory = os.path.join(runtime_directory, 'Anaconda')

for file in ANACONDA_FILES:
    copy(os.path.join(anaconda_directory, file),
        os.path.join('tmp', 'Data', 'Runtime', 'Anaconda', file))

for file in OTHER_FILES:
    copy(file, os.path.join('tmp', file))

# copy license file
if COMMERCIAL:
    license_file = 'COMMERCIAL_LICENSE.txt'
else:
    license_file = 'DEMO_LICENSE.txt'
copy(license_file, os.path.join('tmp', 'LICENSE.txt'))

VERSION = 'This version was distributed %s\nRevision at %s' % (
    time.strftime('%c'), get_hg_rev())
open('./tmp/VERSION.txt', 'wb').write(VERSION)

subprocess.check_call(r'7z a ../dist/anaconda.zip', cwd = './tmp/')