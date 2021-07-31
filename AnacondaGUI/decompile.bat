@echo off
TITLE Decompiling...
call cd %~dp0
call python build.py build_ext --inplace
cd "%~dp0tools" 
"C:\Python27\python.exe" bimbam.py %1 %2
ECHO Done!
pause