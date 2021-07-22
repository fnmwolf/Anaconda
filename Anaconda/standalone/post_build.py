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
import zipfile
import sys
import os

def make_zipfile(zip_filename, base_dir):
    zip = zipfile.ZipFile(zip_filename, 'w', compression = zipfile.ZIP_DEFLATED)
    
    cwd = os.getcwd()
    os.chdir(base_dir)
    
    for dirpath, dirnames, filenames in os.walk('.'):
        for name in filenames:
            path = os.path.normpath(os.path.join(dirpath, name))
            if os.path.isfile(path):
                zip.write(path, path)
    zip.close()
    os.chdir(cwd)
    
    return zip_filename

path = './dist/%s' % sys.platform

if sys.platform == 'darwin':
    # lipo files so we only get i386
    cwd = os.getcwd()
    os.chdir(path)
    import subprocess
    
    for dirpath, dirnames, filenames in os.walk('.'):
        for name in filenames:
            full_path = os.path.normpath(os.path.join(dirpath, name))
            if not os.path.isfile(full_path):
                continue
            subprocess.call(['lipo', full_path, '-thin', 'i386', '-output', 
                full_path])
        
    os.chdir(cwd)

make_zipfile('./dist/%s.zip' % sys.platform, path)
shutil.rmtree(path)