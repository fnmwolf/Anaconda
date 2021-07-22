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

import shutil
import os
import sys
import subprocess

NATIVE_EXTENSIONS = ['pyd', 'dll', 'so']
SOURCE_EXTENSIONS = ['py']
STRIP = sys.platform != 'win32'

# move extensions to 'extensions' folder
source_path = '../extensions/'
extensions_path = './dist/extensions/'
platform_path = './dist/extensions/%s/' % sys.platform

# shutil.rmtree(extensions_path, ignore_errors = True)

extensions = []

cwd = os.getcwd()
os.chdir(source_path)
for path, dirs, files in os.walk('.', topdown = True):
    ignore = []
    if 'IGNORE' in files:
        data = open(os.path.normpath(os.path.join(source_path, path, 'IGNORE')),
            'rb').read().splitlines()
        for line in data:
            line = line.strip()
            if line:
                ignore.append(line)
    for entry in ignore:
        try:
            dirs.remove(entry)
        except ValueError:
            print 'Could not find %r' % entry
    for name in files:
        if name in ignore:
            continue
        extension = name.split('.')[-1]
        source = os.path.normpath(os.path.join(source_path, path, name))
        if extension in NATIVE_EXTENSIONS:
            dest = os.path.join(platform_path, path, name)
        elif extension in SOURCE_EXTENSIONS:
            dest = os.path.join(extensions_path, path, name)
        else:
            continue
        dest = os.path.normpath(dest)
        extensions.append((source, dest))
os.chdir(cwd)

for (src, dst) in extensions:
    try:
        os.makedirs(os.path.dirname(dst))
    except OSError:
        pass
    shutil.copyfile(src, dst)
    extension = dst.split('.')[-1]
    if STRIP and extension in NATIVE_EXTENSIONS:
        subprocess.call(['strip', '-S', dst])