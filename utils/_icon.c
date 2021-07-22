/*
 *	   Copyright (c) 2000, 2001, 2002, 2003 Thomas Heller
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Python.h"
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>

HANDLE (__stdcall *pfn_BeginUpdateResource)(LPCWSTR, BOOL);
BOOL (__stdcall* pfn_EndUpdateResource)(HANDLE, BOOL);
BOOL (__stdcall* pfn_UpdateResource)(HANDLE, LPCWSTR, LPCWSTR, WORD, LPVOID, DWORD);
HANDLE (__stdcall* pfn_CreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

/*
 * Ref for the icon code, from MSDN:
 *   Icons in Win32
 *   John Hornick 
 *   Microsoft Corporation
 *     Created: September 29, 1995
 */

#pragma pack(2)

/* Structure of .ico files */

typedef struct {
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
    DWORD dwImageOffset;
} ICONDIRENTRY;

typedef struct {
    WORD idReserved; /* Must be 0 */
    WORD idType; /* Should check that this is 1 for icons */
    WORD idCount; /* Number os ICONDIRENTRYs to follow */
    ICONDIRENTRY idEntries[0];
} ICONDIRHEADER;

/* Format of RT_GROUP_ICON resources */

typedef struct {
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
    WORD nID;
} GRPICONDIRENTRY;

typedef struct {
    WORD idReserved;
    WORD idType;
    WORD idCount;
    GRPICONDIRENTRY idEntries[0];
} GRPICONDIRHEADER;

#pragma pack()

void initialize(void)
{
    PyObject *m, *d;
    HMODULE hmod = NULL;

    if (GetVersion() & 0x80000000)
	/* Win 95, 98, Me */
	/* We don't check *here* if this fails. We check later! */
	hmod = LoadLibrary("unicows.dll");
    else
	/* Win NT, 2000, XP */
	hmod = LoadLibrary("kernel32.dll");

    pfn_BeginUpdateResource = (HANDLE (__stdcall *)(LPCWSTR, BOOL))
	GetProcAddress(hmod, "BeginUpdateResourceW");
    pfn_EndUpdateResource = (BOOL (__stdcall*)(HANDLE, BOOL))
	GetProcAddress(hmod, "EndUpdateResourceW");
    pfn_UpdateResource = (BOOL (__stdcall*)(HANDLE, LPCWSTR, LPCWSTR, WORD, LPVOID, DWORD))
	GetProcAddress(hmod, "UpdateResourceW");
    pfn_CreateFileW = (HANDLE (__stdcall*)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
					   DWORD, DWORD, HANDLE))
	GetProcAddress(hmod, "CreateFileW");
}

static PyObject *SystemError(int code, char *msg)
{
    LPVOID lpMsgBuf;
    char Buffer[4096];
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
		  | FORMAT_MESSAGE_FROM_SYSTEM,
		  NULL,
		  code,
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPSTR) &lpMsgBuf,
		  0,
		  NULL);
    sprintf(Buffer, "%s: %s", msg, lpMsgBuf);
    LocalFree (lpMsgBuf);
    PyErr_SetString(PyExc_RuntimeError, Buffer);
    return NULL;
}

/*
 * Map a file into memory for reading.
 *
 * Pointer returned must be freed with UnmapViewOfFile().
 */
static char *MapExistingFile (Py_UNICODE *pathname, DWORD *psize)
{
    HANDLE hFile, hFileMapping;
    DWORD nSizeLow, nSizeHigh;
    char *data;

    if (pfn_CreateFileW == NULL) {
	SetLastError(1); /* Incorrect function */
	return NULL;
    }
    hFile = pfn_CreateFileW(pathname,
			    GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			    FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
	return NULL;
    nSizeLow = GetFileSize(hFile, &nSizeHigh);
    hFileMapping = CreateFileMapping(hFile,
				     NULL, PAGE_READONLY, 0, 0, NULL);
    CloseHandle (hFile);

    if (hFileMapping == INVALID_HANDLE_VALUE)
	return NULL;
    
    data = (char*)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

    CloseHandle(hFileMapping);
    *psize = nSizeLow;
    return data;
}

/*
 * Create a GRPICONDIRHEADER from an ICONDIRHEADER.
 *
 * Returns malloc()'d memory.
 */
static GRPICONDIRHEADER *CreateGrpIconDirHeader(ICONDIRHEADER *pidh, int icoid)
{
    GRPICONDIRHEADER *pgidh;
    size_t size;
    int i;

    size = sizeof(GRPICONDIRHEADER) + sizeof(GRPICONDIRENTRY) * pidh->idCount;
    pgidh = (GRPICONDIRHEADER *)malloc(size);
    pgidh->idReserved = pidh->idReserved;
    pgidh->idType = pidh->idType;
    pgidh->idCount = pidh->idCount;

    for (i = 0; i < pidh->idCount; ++i) {
	pgidh->idEntries[i].bWidth = pidh->idEntries[i].bWidth;
	pgidh->idEntries[i].bHeight = pidh->idEntries[i].bHeight;
	pgidh->idEntries[i].bColorCount = pidh->idEntries[i].bColorCount;
	pgidh->idEntries[i].bReserved = pidh->idEntries[i].bReserved;
	pgidh->idEntries[i].wPlanes = pidh->idEntries[i].wPlanes;
	pgidh->idEntries[i].wBitCount = pidh->idEntries[i].wBitCount;
	pgidh->idEntries[i].dwBytesInRes = pidh->idEntries[i].dwBytesInRes;
	pgidh->idEntries[i].nID = icoid + i ;
    }
    return pgidh;
}

static PyObject* do_add_icon(Py_UNICODE *exename, Py_UNICODE *iconame, int icoid, BOOL bDelete)
{   
    static int rt_icon_id = 0;

    /* from the .ico file */
    ICONDIRHEADER *pidh;
    WORD idh_size;
    /* for the resources */
    GRPICONDIRHEADER *pgidh = NULL;
    WORD gidh_size;
    HANDLE hUpdate = NULL;
    int i;
    char *icodata;
    DWORD icosize;
    
    icodata = MapExistingFile(iconame, &icosize);
    if (!icodata) {
        return SystemError(GetLastError(), "MapExistingFile");
    }
    
    pidh = (ICONDIRHEADER *)icodata;
    idh_size = sizeof(ICONDIRHEADER) + sizeof(ICONDIRENTRY) * pidh->idCount;

    pgidh = CreateGrpIconDirHeader(pidh, icoid);
    gidh_size = sizeof(GRPICONDIRHEADER) + sizeof(GRPICONDIRENTRY) * pgidh->idCount;

    if (pfn_BeginUpdateResource == NULL
	|| pfn_UpdateResource == NULL
	|| pfn_EndUpdateResource == NULL) {
	PyErr_SetString(PyExc_RuntimeError,
			"this function requires unicows.dll in the Python directory on Win 95/98/Me");
	return NULL;
    }

    hUpdate = pfn_BeginUpdateResource(exename, bDelete);
    if (!hUpdate) {
	SystemError(GetLastError(), "BeginUpdateResource");
	goto failed;
    }
    
    /* Each RT_ICON resource in an image file (containing the icon for one
       specific resolution and number of colors) must have a unique id, and
       the id must be in the GRPICONDIRHEADER's nID member.

       So, we use a *static* variable rt_icon_id which is incremented for each
       RT_ICON resource and written into the GRPICONDIRHEADER's nID member.

       XXX Do we need a way to reset the rt_icon_id variable to zero?  If we
       are building a lot of images in one setup script? 
    */
    for (i = 0; i < pidh->idCount; ++i) {
	    pgidh->idEntries[i].nID = rt_icon_id++;
    }
    if (!pfn_UpdateResource(hUpdate,
			    (Py_UNICODE *)MAKEINTRESOURCE(RT_GROUP_ICON),
			    (Py_UNICODE *)MAKEINTRESOURCE(icoid),
			    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			    pgidh,
			    gidh_size)) {
	SystemError(GetLastError(), "UpdateResource");
	goto failed;
    }
    for (i = 0; i < pidh->idCount; ++i) {
        char *cp = &icodata[pidh->idEntries[i].dwImageOffset];
        int cBytes = pidh->idEntries[i].dwBytesInRes;
        if (!pfn_UpdateResource(hUpdate,
				(Py_UNICODE *)MAKEINTRESOURCE(RT_ICON),
				(Py_UNICODE *)MAKEINTRESOURCE(pgidh->idEntries[i].nID),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				cp,
				cBytes)) {
            SystemError(GetLastError(), "UpdateResource");
            goto failed;
        }
    }

    free(pgidh);
    UnmapViewOfFile(icodata);

    if (!pfn_EndUpdateResource(hUpdate, FALSE))
        return SystemError(GetLastError(), "EndUpdateResource");
    Py_INCREF(Py_None);
    return Py_None;

  failed:
    if (pgidh)
        free(pgidh);
    if (hUpdate)
        pfn_EndUpdateResource(hUpdate, TRUE);
    if (icodata)
        UnmapViewOfFile(icodata);
    return NULL;
}