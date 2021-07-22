copy /Y /B "..\standalone\dist\win32.zip" "D:\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\win32.zip"
copy /Y /B "..\standalone\dist\linux2.zip" "D:\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\linux2.zip"
copy /Y /B "..\standalone\dist\darwin.zip" "D:\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\darwin.zip"
rmdir /S /Q "D:\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\extensions"
xcopy "..\standalone\dist\extensions" "D:\Multimedia Fusion Developer 2\Data\Runtime\Anaconda\extensions" /Y /S /I