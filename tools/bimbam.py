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

from mmfparser.data.exe import ExecutableData
from mmfparser.data.gamedata import GameData
from mmfparser.data.mfa import MFA
from mmfparser.translators.pame2mfa import translate
from mmfparser.bytereader import ByteReader

import sys
import os
import string

def main():
    print 'Anaconda Decompiler'
    print 'ONLY for educational purpose, or game datamining ;)'
    print ''
    input = sys.argv[1]
    output = 'C:\Out'
    fp = ByteReader(open(input, 'rb'))
    if input.endswith('.ccn'):
        newGame = GameData(fp)
    else:
        newExe = ExecutableData(fp, loadImages=True)

        for file in newExe.packData.items:
            name = file.filename.split('\\')[-1]
            print 'Writing pack file %r' % name
            open(os.path.join(output, name), 'wb').write(file.data)

        newGame = newExe.gameData

    if newGame.files is not None:
        for file in newGame.files.items:
            name = file.name.split('\\')[-1]
            print 'Writing embedded file %r' % name
            open(os.path.join(output, name), 'wb').write(str(file.data))
    newGame.files = None

    def out(value):
        print value
    print 'Translating MFA...'
    newMfa = translate(newGame, print_func = out)
    s = newGame.name
    whitelist = string.letters + string.digits
    new_s = ''
    for char in s:
        if char in whitelist:
            new_s += char
    print new_s
    out_path = os.path.join(output, new_s + '-compressed.mfa')
    print 'Writing MFA...'
    newMfa.write(ByteReader(open(out_path, 'wb')))

    # newMfa = MFA(ByteReader(open(out_path, 'rb')))
    print 'Decompilation Finished!'

if __name__ == '__main__':
    main()
