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

try:
    import psyco
    psyco.full()
except ImportError:
    pass

from optparse import OptionParser

from mmfparser.data.flash import convert, ZIP_PASSWORD
from mmfparser.bytereader import ByteReader
import os

def main():
    print 'Multimedia Fusion 2 Flash Archive Tool'
    print 'by Mathias "Mat^2" Kaerlev'
    print ''
    usage = """usage: <input> [output]"""
    parser = OptionParser(usage = usage)
    (option, args) = parser.parse_args()
    if len(args) not in xrange(1, 3):
        parser.error('invalid number of arguments specified')
    input = args[0]
    try:
        output = args[1]
    except IndexError:
        name, ext = os.path.splitext(args[0])
        if ext in ('.ext', '.dat'):
            ext = '.zip'
        else:
            ext = '.dat'
        output = name + ext
    open(output, 'wb').write(convert(ByteReader(open(input, 'rb'))).data())
    print 'File successfully converted!'
    print 'Remember that the ZIP files use the following password'
    print 'for all files: %s' % ZIP_PASSWORD

if __name__ == '__main__':
    main()