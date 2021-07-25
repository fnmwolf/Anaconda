@echo off
TITLE Decompiling...
call python build.py build_ext --inplace
call mkdir C:\Out
call cd %~dp0Input
for /R %%f in (*.exe) do (
cd "%~dp0tools" 
"C:\Python27\python.exe" bimbam.py "%%f"
)
ECHO Done!
PAUSE