# -*- mode: python -*-
import sys
import os
sys.path.append('..')

SINGLE_FILE = False
WITH_CONSOLE = False
TK = False

excludes = ['numpy', 'win32api', 'win32evtlog', 'win32gui',
    'win32pdh', 'win32pipe', 'setuptools', 'OpenGL', 'distutils',
    'multiprocessing', 'Image', 'ImageOps', '_hashlib', 'jpeg.dll',
    'pygame._numericsurfarray', '_imagingft', '_hashlib', 'libtiff.dll',
    'pygame._numericsndarray', 'select', '_ssl', 'SDL_image.dll', 'zlib1.dll',
    'SDL_ttf.dll', 'libfreetype-6.dll', 'libpng12-0.dll', 'bz2',
    'pygame._arraysurfarray', 'pygame.Rect', 'OpenSSL', 'win32event',
    'win32file', 'win32process', 'win32security', 'PyWinTypes26.dll',
    'ssleay32.dll', 'libeay32.dll', 'tk85.dll', 'tcl85.dll', 
    'libnvidia-glcore.so.260.19.06', 'libX11.so.6', 
    'libGLU.so.1', 'libfreetype.so.6', 'libnvidia-tls.so.260.19.06', 
    'libpulse-simple.so.0', '_tkinter.pyd', 'pygame', 'pywintypes',
    'ole32.dll', 'user32.dll', 'gdi32.dll', 'kernel32.dll', 'gdiplus.dll']

if not TK:
    excludes.append('Tkinter')

path = '../runtime.py'

if sys.platform == 'win32':
    extension = '.exe'
else:
	extension = ''

scripts = [
    os.path.join(HOMEPATH,'support/_mountzlib.py'),
    os.path.join(CONFIGDIR,'support/useUnicode.py'), 
    path
]

if TK:
    scripts.insert(1, os.path.join(CONFIGDIR,'support/useTK.py'))
    if SINGLE_FILE:
        scripts.insert(1, os.path.join(HOMEPATH,'support/unpackTK.py'))
        scripts.append(os.path.join(HOMEPATH,'support/removeTK.py'))

import pyglet
# so we don't fuck up on OS X
sys.is_epydoc = True

a = Analysis(scripts,
    pathex=['.'], 
    hookspath = ['./hooks'],
    excludes = excludes
)

a.zipfiles = [('eggs/dummy', './dummy', 'ZIPFILE')]
for item in a.binaries[:]:
    if item[0] in excludes:
        a.binaries.remove(item)

for item in a.pure[:]:
    if item[0] in excludes:
        a.pure.remove(item)

if sys.platform == 'win32':
    for name in ('OpenAL32', 'libsndfile-1', 'libdumb', 'libgthread-2.0-0',
                 'libfluidsynth', 'libglib-2.0-0'):
        path = './DLLs/%s.dll' % name
        if not os.path.isfile(path):
            print 'Missing %s' % name
            continue
        a.binaries.append(('%s.dll' % name, path, 'BINARY'))
    import select
    for ext_module in (select,):
        a.binaries.append(
            (ext_module.__name__, ext_module.__file__, 'EXTENSION'))

elif sys.platform == 'linux2':
    # a.binaries.append(('libopenal.so', '/usr/lib/libopenal.so', 'BINARY'))
    for name in ('libsndfile.so.1', 'libvorbisenc.so.2', 'libvorbis.so.0', 
                 'libogg.so.0', 'libmodplug.so.1'):
        path = '/usr/lib/%s' % name
        if not os.path.isfile(path):
            print 'Missing %s' % name
            continue
        a.binaries.append((name, path, 'BINARY'))

elif sys.platform == 'darwin':
    import select
    import _hashlib
    for ext_module in (select, _hashlib):
        a.binaries.append(
            (ext_module.__name__, ext_module.__file__, 'EXTENSION'))
    for name in ('libsndfile.1', 'libmodplug.1', 'libfluidsynth.1',
                 'libvorbisenc.2', 'libvorbis.0', 'libogg.0', 
                 'libgthread-2.0.0', 'libglib-2.0.0', 'libvorbisfile.3',
                 'libssl.1.0.0', 'libcrypto.1.0.0', 'libFLAC.8'):
        path = '/opt/local/lib/%s.dylib' % name
        if not os.path.isfile(path):
            print 'Missing %s' % name
            continue
        a.binaries.append(('%s.dylib' % name, path, 'BINARY'))

STRIP = sys.platform != 'win32'
UPX = sys.platform != 'darwin'

pyz = PYZ(a.pure)
exe = EXE(pyz,
    a.scripts + [('O','','OPTION')],
    exclude_binaries=1,
    name=os.path.join('build/pyi.%s/runtime' % sys.platform, 
        'runtime%s' % extension),
    debug=False,
    strip=STRIP,
    upx=UPX,
    icon = 'pycon.ico',
    console=WITH_CONSOLE
)

arg = [exe,
    a.binaries,
    a.zipfiles,
    a.datas,
]

if sys.platform == 'darwin':
    name = os.path.join('build', 'pyi.darwin', 'data')
else:
    name = os.path.join('dist', sys.platform, 'data')
    
kw = dict(strip=STRIP,
    upx=UPX,
    name=name
)
if TK:
    arg.insert(0, TkTree())
coll = COLLECT(*tuple(arg), **kw)

app = BUNDLE(coll,
     name=os.path.join('dist', 'darwin', 'runtime.app'),
     version="1.0")