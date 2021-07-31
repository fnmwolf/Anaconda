@echo off
TITLE Decompiling...
call python build.py build_ext --inplace
call mkdir C:\Out
call cd %~dp0Input
for /R %%f in (*.exe) do (
cd "%~dp0tools" 
"C:\Python2718\python.exe" bimbam2.py "%%f"
)
ECHO Done!
PAUSE