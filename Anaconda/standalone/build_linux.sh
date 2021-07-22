python -OO trunk/pyinstaller.py runtime.spec
sh build_launcher.sh
python post_build.py
python move_extensions.py