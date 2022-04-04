# Copyright (c) Mathias Kaerlev 2012.

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
    try:
        input = sys.argv[1]
    except IndexError:
        print 'Usage: python bimbam2.py yourGameExe.exe'
        exit()
    try:
        output = sys.argv[2]
    except IndexError:
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
    try:
        out_path = os.path.join(output, sys.argv[3] + '.mfa')
    except IndexError:
        out_path = os.path.join(output, new_s + '-compressed.mfa')
    print 'Writing MFA...'
    newMfa.write(ByteReader(open(out_path, 'wb')))

    # newMfa = MFA(ByteReader(open(out_path, 'rb')))
    print 'Decompilation Finished!'

if __name__ == '__main__':
    main()
