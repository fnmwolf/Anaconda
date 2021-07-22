import sys
sys.path.append('../../../')
from cx_Freeze import setup, Executable

build_options = {'packages': ['PIL', 'mmfparser'],
                 'excludes': ['tcl', 'tk', 'Tkinter']}

executables = [
    Executable('hfatiler.py', base='Console', targetName='hfatiler.exe')
]

setup(options={'build_exe': build_options}, executables=executables)
