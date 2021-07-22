// Copyright (c) Mathias Kaerlev 2012.

// This file is part of Anaconda.

// Anaconda is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Anaconda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <windows.h>
#include <unistd.h>

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, 
            INT nCmdShow)
{
    // on the very start of the application, we set the error mode to 1
    // so we don't get errors with CD/DVD drives
    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    
    STARTUPINFOW         siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;

    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));

    siStartupInfo.cb = sizeof(siStartupInfo);
    
    TCHAR exepath[MAX_PATH];
    GetModuleFileName(0, exepath, MAX_PATH);
    _splitpath(exepath, 0, 0, exepath, 0);
    char newname[MAX_PATH];
    strcpy(newname, exepath);
    strcat(newname, " data");
    chdir(newname);

    CreateProcessW(
        L"runtime.exe",
        L"",
        NULL, // security attributes
        NULL, // thread security attributes
        TRUE, // handle inheritance flag 
        0, // creation flags 
        NULL, // pointer to new environment block 
        NULL, // pointer to current directory name
        &siStartupInfo,
        &piProcessInfo
    );
    return 0;
}
