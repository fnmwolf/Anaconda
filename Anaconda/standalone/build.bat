rmdir /S /Q "build"
python -OO trunk\pyinstaller.py runtime.spec
call build_launcher.bat
python post_build.py
python move_extensions.py