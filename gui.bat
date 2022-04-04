call cd ..
call cd %~dp0
call python build.py build_ext --inplace
cd "tools" 
"C:\Python27\python.exe" bimbam2.py %1 %2 %3