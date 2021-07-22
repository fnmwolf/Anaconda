================
Anaconda Runtime
================

Installation instructions:
1) Install the latest MMF2 build. This can be found on the forums.
2) Copy the directories in the archive into your Multimedia Fusion 2 root 
   directory.

Using the runtime:
1) Under build options, select one of the Anaconda build options.

Notes:
- Under Linux 64bit, you will need the 32bit support libraries.
  On Ubuntu, this is as simple as: sudo apt-get install ia32-libs
  Linux is also a mess in regards to graphics drivers, so if you get black
  screen issues, try installing the proprietary drivers, e.g. for NVIDIA:
  sudo apt-get install nvidia-current
  
==============
Extra features
==============

* Include directories/files during build
On build, Anaconda will try and find a String object with the name
"Anaconda Settings". It should have a single paragraph with a INI-like
formatting like this:

[Includes]
Include.txt = Include.txt
IncludeFolder = IncludeFolder
Destination = Source

Destination is the destination file/folder relative to the root data 
folder, and Source is the file/folder to be included, relative to the location
of the MFA. See Examples\Anaconda\ConfigurationExample for further details.

* Native HWA shader compatibility (i.e. converting from HLSL->GLSL)
On runtime, Anaconda will try and look for GLSL shaders with the names
ShaderName.vert and ShaderName.frag in the data folder. See 
Examples\Anaconda\NativeShaders for further details.

* Custom extension copying
On build, Anaconda will look through the used extensions and find extensions
that aren't directly supported. However, if they are present in the
Data\Runtime\Anaconda\extensions directory, they will be copied. Extensions can
be made both as a single source file, e.g. MyExtension.py, or as a package with
a __init__.py file. If building as a package, Anaconda will also look in 
seperate 'win32', 'linux2' and 'darwin' directories in the 'extensions' 
directory for platform-dependent shared libraries, so you can have a structure
like this:

Anaconda/extensions/MyExtension/__init__.py
Anaconda/extensions/win32/MyExtension/MyLibrary.pyd
Anaconda/extensions/win32/MyExtension/MyOtherLibrary.dll
Anaconda/extensions/linux2/MyExtension/MyLibrary.so
Anaconda/extensions/linux2/MyExtension/MyOtherLibrary.so
Anaconda/extensions/darwin/MyExtension/MyLibrary.so
Anaconda/extensions/darwin/MyExtension/MyOtherLibrary.so

which would end like (in this case for win32)

MyGame data/extensions/MyExtension/__init__.py
MyGame data/extensions/MyExtension/MyLibrary.pyd
MyGame data/extensions/MyExtension/MyOtherLibrary.pyd

* Fonts
On runtime, Anaconda will load all .ttf fonts from a 'fonts' directory in the
data folder. See Examples\Anaconda\CustomFonts for further details.

===============
Troubleshooting
===============

Does your application/game not work? Do you need a specific extension converted
for it to work? Submit an entry at the issue tracker:
http://code.google.com/p/anaconda-extensions/issues/list

Please provide the anaconda.log file that resides in the data directory
of your build application (or in Application.app/Contents/Resources/anaconda.log
on OS X). This will be invaluable in debugging your problem.

On OS X, it helps to cd into "Application.app/Contents/Resources" using a
Terminal, then running "../MacOS/runtime". This will print error messages that
are otherwise silenced.