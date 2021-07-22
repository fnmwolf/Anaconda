# -*- mode: python -*-

import sys
sys.path.append('..')

a = Analysis([os.path.join(HOMEPATH,'support\\_mountzlib.py'), 
              os.path.join(CONFIGDIR,'support\\useUnicode.py'), 'make.py'],
              pathex=['F:\\MMF2\\File format\\Python\\tests\\build'],
              hookspath = ['./hooks']
)

pyz = PYZ(a.pure)
exe = EXE( pyz,
          a.scripts + [('O','','OPTION')],
          a.binaries,
          a.zipfiles,
          a.datas,
          name=os.path.join('dist', 'make.exe'),
          icon = 'pycon.ico',
          debug=False,
          strip=False,
          upx=True,
          console=False )
