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

import zlib
import struct

from mmfparser.loader import DataLoader
from mmfparser.bytereader import ByteReader

def decompress(reader):
    decompressedSize = reader.readInt(True)
    compressedSize = reader.readInt(True)
    return ByteReader(zlib.decompress(reader.read(compressedSize)))

def compress(data):
    reader = ByteReader()
    compressed = zlib.compress(str(data))
    reader.writeInt(len(data))
    reader.writeInt(len(compressed))
    reader.write(compressed)
    return reader

def compressImageLZX(data):
    reader = ByteReader()
    compressed = zlib.compress(str(data))
    #print 'Compressing LZX'
    #print 'input size'
    #print len(data)
    #print 'output size'
    #print len(compressed)
    #reader.writeInt(len(data))
    #reader.writeInt(len(compressed))
    reader.write(compressed)
    return reader
