python -OO trunk/pyinstaller.py runtime.spec
python post_build.py
python move_extensions.py