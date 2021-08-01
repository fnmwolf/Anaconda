@echo off

goto check_Permissions

:check_Permissions
net session >nul 2>&1
if %errorLevel% == 0 (
    echo ^[32mAdministrator detected, continuing batch file.[0m
    goto StartBatch
) else (
    echo ^[31mError: Administrator permissions required. Please restart the batch with Admin.[0m
    goto EndProgram
)

pause >nul

:StartBatch
TITLE Installing
cd C:\WINDOWS\system32
IF NOT EXIST "C:/Python27/python.exe" (
ECHO ^[32mInstalling Python 2.7.18[0m
ECHO ^[34mGo through the installer then in "Customize" turn on "Add python.exe to Path".[0m
PAUSE
call "%~dp0python-2.7.18.amd64.msi"
)
ECHO ^[32mPython 2.7.18 Installed...[0m
IF NOT EXIST "C:\Users\%username%\AppData\Local\Programs\Common\Microsoft\Visual C++ for Python\9.0\vcvarsall.bat" (
ECHO ^[32mInstalling Python VC++ for Python 2.7[0m
call msiexec /I "%~dp0VCForPython27.msi" /quiet
)
ECHO ^[32mPython VC++ for Python 2.7 Installed...[0m
IF NOT EXIST "C:/Program Files/Python39/python.exe" (
ECHO ^[32mInstalling Python 3.9.6[0m
call "%~dp0python-3.9.6-amd64.exe" /quiet  InstallAllUsers=1
)
ECHO ^[32mPython 3.9.6 Installed...[0m
IF NOT EXIST "C:\Python27\lib\site-packages\Cython" (
ECHO ^[32mInstalling Cython 0.21.2[0m
call C:/Python27/Python.exe -m pip install Cython==0.21.2
)
ECHO ^[32mCython 0.21.2 Installed...[0m
IF NOT EXIST "%~dp0mmfparser\zopfli.pyd" (
cd /D %~dp0
ECHO ^[32mRunning build_all.bat[0m
call C:/Python27/Python.exe build.py build_ext -f --inplace
)
ECHO ^[32mbuild_all.bat Finished...[0m
ECHO ^[33mDone![0m
goto EndProgram

:EndProgram
PAUSE
