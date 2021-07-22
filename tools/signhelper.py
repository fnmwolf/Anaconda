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
Tool to enable codesigning of Anaconda OS X applications
"""

import sys
import os

def usage():
    print 'usage: %s <command> <path to .app>' % sys.argv[0]
    print 'commands: prepare, finish'
    print ''
    print 'First, run "prepare" on the .app, then use codesign, and finally,'
    print 'run "finish" on the .app'
    return

def main(args):
    if len(args) != 3:
        return usage()
    command = args[1]
    if command not in ('prepare', 'finish'):
        return usage()
    path = os.path.join(args[2], 'Contents', 'MacOS', 'runtime')
    data = open(path, 'rb').read()
    size = data.find('PYZ')
    if command == 'prepare':
        open(path, 'wb').write(data[:size])
        open('_runtimedata.bak', 'wb').write(data[size:])
    else:
        data2 = open('_runtimedata.bak', 'rb').read()
        open(path, 'wb').write(data + data2)
    print 'Done.'

if __name__ == '__main__':
    main(sys.argv)