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

def main():
    print 'Bimbam tool by Mathias Kaerlev'
    print 'ONLY for use on your own applications.'
    print ''

    input = sys.argv[1]
    output = sys.argv[2]

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

    newMfa = translate(newGame, print_func = out)
    out_path = os.path.join(output, 'out.mfa')
    newMfa.write(ByteReader(open(out_path, 'wb')))

    # newMfa = MFA(ByteReader(open(out_path, 'rb')))
    print 'Finished!'

if __name__ == '__main__':
    main()
