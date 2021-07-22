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
usage is make.py [build_index] <in> <out>
"""

import sys
import os
import shutil
import zipfile
import ctypes
import subprocess
import json
import ConfigParser
from cStringIO import StringIO
import py_compile
import tempfile

try:
    startupinfo = subprocess.STARTUPINFO()
    startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
except AttributeError:
    pass

BUILD_TYPES = [
    'win32',
    'darwin',
    'linux2'
]

BUILD_EXTENSIONS = [
    '.exe',
    '.app',
    ''
]

NATIVE_EXTENSIONS = [
    'so',
    'dylib'
]

ANACONDA_SETTINGS_NAME = 'Anaconda Settings'

RELEASE_BUILD = len(sys.argv) == 4 and sys.argv[1] not in BUILD_TYPES

def check_compile(src, dst):
    extension = src.split('.')[-1]
    if extension == 'py':
        postfix = (__debug__ and 'c' or 'o')
        dst += postfix
        new_src = os.path.join(tempfile.gettempdir(), 'comp.py' + postfix)
        py_compile.compile(src, new_src, doraise = True)
        src = new_src
    return src, dst

def copytree(src, dst, compile = False):
    names = os.listdir(src)
    try:
        os.makedirs(dst)
    except OSError:
        pass
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if os.path.isdir(srcname):
                copytree(srcname, dstname)
            else:
                if compile:
                    srcname, dstname = check_compile(srcname, dstname)
                shutil.copy2(srcname, dstname)
        except shutil.Error, err:
            errors.extend(err.args[0])
        except EnvironmentError, why:
            errors.append((srcname, dstname, str(why)))
    try:
        shutil.copystat(src, dst)
    except OSError, why:
        if WindowsError is not None and isinstance(why, WindowsError):
            # Copying file access times may fail on Windows
            pass
        else:
            errors.extend((src, dst, str(why)))
    if errors:
        raise Error, errors

def copy(src, dst, compile = False):
    if os.path.isfile(src):
        try:
            os.makedirs(os.path.dirname(dst))
        except OSError:
            pass
        if compile:
            src, dst = check_compile(src, dst)
        shutil.copyfile(src, dst)
    elif os.path.isdir(src):
        copytree(src, dst, compile = compile)
    else:
        print 'Could not find %r while copying' % src

class ZipFile(zipfile.ZipFile):
    compile = False
    def add_file(self, src, dst):
        if self.compile:
            src, dst = check_compile(src, dst)
        self.write(src, dst)
        extension = src.split('.')[-1]
        if extension in NATIVE_EXTENSIONS:
            info = self.getinfo(dst)
            info.external_attr = (0100000 | 0755) << 16
        self.set_permissions(dst)

    def set_permissions(self, name):
        if name.split('.')[-1] in NATIVE_EXTENSIONS:
            info = self.getinfo(name)
            info.external_attr = (0100000 | 0755) << 16

    def add(self, src, dst):
        if os.path.isfile(src):
            self.add_file(src, dst)
        elif os.path.isdir(src):
            cwd = os.getcwd()
            os.chdir(src)
            for dirpath, dirnames, filenames in os.walk('.'):
                for name in filenames:
                    path = os.path.normpath(os.path.join(dirpath, name))
                    zip_path = os.path.normpath('/'.join((dst, dirpath, name))
                        ).replace('\\', '/')
                    if os.path.isfile(path):
                        self.add_file(path, zip_path)
            os.chdir(cwd)
        else:
            print 'Could not find %r while copying' % src

def main():
    if RELEASE_BUILD:
        build_index = int(sys.argv[1])
        platform = BUILD_TYPES[build_index]
        file_extension = BUILD_EXTENSIONS[build_index]
        runtime_directory = os.path.dirname(sys.argv[0])
        full_path = sys.argv[3]
        source_path = sys.argv[2]
        tempfile.tempdir = os.path.dirname(source_path)
        png_path = os.path.join(tempfile.gettempdir(), 'appicon.png')
    else:
        args = sys.argv
        if len(args) == 4:
            platform = args.pop(1)
        else:
            platform = sys.platform
        file_extension = BUILD_EXTENSIONS[BUILD_TYPES.index(platform)]
        runtime_directory = os.path.abspath('./dist')
        full_path = os.path.abspath(sys.argv[2])
        source_path = os.path.abspath(sys.argv[1])
        png_path = os.path.join(os.getcwd(), 'appicon.png')
        
    os.chdir(tempfile.gettempdir())
    path = os.path.join(runtime_directory, platform + '.zip')
    native_extensions = None
    try:
        data = open(os.path.join(runtime_directory, 'extensions.dat'), 'rb'
            ).read()
        extensions_json = json.loads(data)
        native_extensions = set(extensions_json['extensions'] + 
                                extensions_json['movements'])
    except IOError:
        print '(could not read extensions.dat)'
    
    from mmfparser.bytereader import ByteReader
    from mmfparser.data.gamedata import GameData
    # parse input file
    source_file = open(source_path, 'rb')
    source_reader = ByteReader(source_file)
    game_data = GameData(source_reader, loadImages = False, delayedLoad = True)
    source_file.close()
    
    # get useful information and verify extensions
    title = game_data.name.decode('windows-1252')
    extra_extensions = set()
    extensions_path = os.path.join(runtime_directory, 'extensions')
    for extension in game_data.extensions.items:
        name = extension.name
        if native_extensions is not None and name not in native_extensions:
            value = False
            for name_attempt in (name + '.py', name):
                for directory in (platform, ''):
                    filename = os.path.join(extensions_path,
                        directory, name_attempt)
                    if not os.path.exists(filename):
                        continue
                    extra_extensions.add((name_attempt, filename))
                    value = True
            if not value:
                raise NotImplementedError('Extension not implemented: %s' %
                    name)
    if extra_extensions:
        extra_extensions.add(('__init__.py', os.path.join(extensions_path,
            '__init__.py')))
    
    extra_includes = set()
    mfa_dir = os.path.dirname(game_data.editorFilename)
    settings = []
    for object_info in game_data.frameItems.items:
        if object_info.name is None:
            continue
        if object_info.name.lower() == ANACONDA_SETTINGS_NAME.lower():
            try:
                texts = object_info.properties.loader.text.items
                for paragraph in texts:
                    for line in paragraph.value.splitlines():
                        settings.append(line)
                break
            except AttributeError:
                pass
    if settings:
        try:
            config = ConfigParser.RawConfigParser()
            config.optionxform = str
            config.readfp(StringIO('\n'.join(settings)))
            for dst, src in config.items('Includes'):
                src = os.path.normpath(os.path.join(mfa_dir, src))
                extra_includes.add((dst, src))
        except ConfigParser.Error, e:
            print 'Could not parse Anaconda configuration: %s\n' % e.args
            pass
    
    name = os.path.basename(full_path).split('.')[0]

    print 'Building for target platform %r...' % platform
    
    if platform in ('win32', 'linux2'):
        data_path = '%s data' % name
        if platform == 'win32':
            base_path = os.path.dirname(full_path)
            full_data_path = os.path.join(base_path, data_path)
            shutil.rmtree(full_data_path, ignore_errors = True)
            try:
                os.remove(full_path)
            except OSError:
                pass
                
    if platform != 'win32':
        zip = ZipFile(full_path, 'w', zipfile.ZIP_DEFLATED)
    input_zip = zipfile.ZipFile(path, 'r')

    if platform in ('win32', 'linux2'):
        for info in input_zip.infolist():
            filename = info.filename
            if filename == 'launch%s' % file_extension:
                if platform == 'win32':
                    continue
                info.filename = name + file_extension
            else:
                splitted = filename.split('/')
                if splitted[0] == 'data':
                    splitted[0] = data_path
                    info.filename = '/'.join(splitted)
            if platform == 'win32':
                input_zip.extract(filename, base_path)
            else:
                data = input_zip.open(filename).read()
                zip.writestr(info, data)
        if platform == 'win32':
            shutil.copyfile(source_path, os.path.join(full_data_path,
                'Application.ccp'))
        else:
            zip.write(source_path, '/'.join((data_path, 'Application.ccp')))
        
        # insert icons (linux does not have icons)
        if platform == 'win32':
            from utils.icon import update_icon
            import subprocess
            import Image
            icon = Image.open(png_path)
            paths = []
            for size in (48, 32, 16):
                path = os.path.join(os.getcwd(), 'new%s.png' % size)
                icon.resize((size, size), Image.ANTIALIAS).save(path)
                paths.append(path)
            converter_path = os.path.join(runtime_directory, 'png2ico.exe')
            icon_path = os.path.join(os.getcwd(), u'temp.ico')
            launcher_path = os.path.join(os.getcwd(), u'launch.exe')
            input_zip.extract('launch.exe', os.getcwd())
            subprocess.check_call([converter_path, icon_path] + paths,
                startupinfo = startupinfo)
            update_icon(launcher_path, icon_path)
            shutil.copyfile(launcher_path, full_path)

    elif platform == 'darwin':
        # add files to zip
        path = 'runtime%s' % file_extension
        new_path = '%s%s' % (name, file_extension)
        for info in input_zip.infolist():
            if info.filename.endswith('.icns'):
                continue
            data = input_zip.open(info.filename).read()
            splitted = info.filename.split('/')
            if splitted[0] == path:
                splitted[0] = new_path
                info.filename = '/'.join(splitted)
            if splitted[-1] == 'Info.plist':
                import plistlib
                plist = plistlib.readPlistFromString(data)
                plist['CFBundleDisplayName'] = title
                plist['CFBundleName'] = title
                plist['CFBundleIconFile'] = 'App.icns'
                data = plistlib.writePlistToString(plist)
            zip.writestr(info, data)
        zip.write(source_path, '/'.join((new_path, 'Contents', 'Resources', 
            'Application.ccp')))
        # insert icon
        import Image
        from utils.icon import make_mac_icon
        try:
            image = Image.open(png_path).convert('RGBA')
        except IOError:
            raise IOError('appicon.png file not present - '
                          'set your icon to use alpha channels')
        size128 = image.resize((128, 128), Image.ANTIALIAS).tostring()
        size48 = image.resize((48, 48), Image.ANTIALIAS).tostring()
        size32 = image.resize((32, 32), Image.ANTIALIAS).tostring()
        size16 = image.resize((16, 16), Image.ANTIALIAS).tostring()
        make_mac_icon('mac.icns', size16, size32, size48, size128)
        zip.write('mac.icns', '/'.join((new_path, 'Contents', 'Resources', 
            'App.icns')))
    print 'Extra extensions:', extra_extensions
    print 'Extra includes:', extra_includes
    # add extra stuff
    if platform == 'win32':
        if extra_extensions:
            extensions_path = os.path.join(full_data_path, 'extensions')
            for (name, filename) in extra_extensions:
                copy(filename, os.path.join(extensions_path, name), 
                     compile = True)
        if extra_includes:
            for (name, filename) in extra_includes:
                copy(filename, os.path.join(full_data_path, name))
    elif platform in ('linux2', 'darwin'):
        if platform == 'linux2':
            root_path = data_path
            extensions_path = '/'.join((root_path, 'extensions'))
        else:
            root_path = '/'.join((new_path, 'Contents', 'MacOS'))
        extensions_path = '/'.join((root_path, 'extensions'))
        zip.compile = True
        if extra_extensions:
            for (name, filename) in extra_extensions:
                zip.add(filename, '/'.join((extensions_path, name)))
        zip.compile = False
        if extra_includes:
            for (name, filename) in extra_includes:
                zip.add(filename, '/'.join((root_path, name)))
    
    input_zip.close()
    if platform != 'win32':
        zip.close()
    print 'Finished!'

if __name__ == '__main__':
    if hasattr(sys, 'frozen'):
        class Pipe(object):
            def __init__(self, src, dst):
                self.src = src
                self.dst = dst
                
            def write(self, msg):
                self.src.write(msg)
                self.dst.write(msg)
                self.dst.flush()

        try:
            f = open('anacondabuild.log', 'wb')
        except IOError:
            pass
        else:
            stderr = Pipe(sys.stderr, f)
            stdout = Pipe(sys.stdout, f)
            sys.stderr = stderr
            sys.stdout = stdout

        try:
            main()
        except:
            import traceback
            error = traceback.format_exc()
            sys.stderr.write(error)
            message = 'Error during build!\n\n%s' % error
            ctypes.windll.user32.MessageBoxA(0, message, 
                "Error", 0)
    else:
        main()