rmdir /S /Q "build"
python -OO trunk\pyinstaller.py make.spec
python post_build_maker.py
copy dist\make.exe "C:\Programs\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\make.exe"
copy dist\extensions.dat "C:\Programs\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\extensions.dat"
copy dist\png2ico.exe "C:\Programs\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\png2ico.exe"