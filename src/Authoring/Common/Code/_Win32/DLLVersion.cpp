/****************************************************************************
**
** Copyright (C) 2016 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "stdafx.h"

#ifndef INCLUDED_DLL_VERSION_H
#include "DLLVersion.h"
#endif

//#include "Global.h"
#ifdef KDAB_TEMPORARILY_REMOVED
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/***************************************************************************

   Function:   GetDLLVersion

   Purpose:    Retrieves DLL major version, minor version and build numbers

   Input:      DLL file name
               Reference to Major number
               Reference to Minor number
               Reference to Build number

   Output:     TRUE only if successful

   Remarks:    This function first tries to get the DLL version the nice way,
               that is, call the DllGetVersion function in the DLL.

               If this fails, it tries to located the DLL file in the file system,
               read the file information block and retrieve the file version.

****************************************************************************/
BOOL CDLLVersion::GetDLLVersion(LPCSTR szDLLFileName, DWORD &dwMajor, DWORD &dwMinor,
                                DWORD &outIteration, DWORD &dwBuildNumber)
{
#ifdef KDAB_TEMPORARILY_REMOVED
    HINSTANCE hDllInst; // Instance of loaded DLL
    char szFileName[_MAX_PATH]; // Temp file name
    BOOL bRes = TRUE; // Result

    lstrcpyA(szFileName, szDLLFileName); // Save a file name copy for the loading

    hDllInst = LoadLibraryA(szFileName); // load the DLL

    if (hDllInst) // Could successfully load the DLL
    {
        DLLGETVERSIONPROC pDllGetVersion;
        /*
        You must get this function explicitly because earlier versions of the DLL
        don't implement this function. That makes the lack of implementation of the
        function a version marker in itself.
        */
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hDllInst, "DllGetVersion");

        if (pDllGetVersion) // DLL supports version retrieval function
        {
            DLLVERSIONINFO dvi;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            HRESULT hr = (*pDllGetVersion)(&dvi);

            if (SUCCEEDED(hr)) // Finally, the version is at our hands
            {
                dwMajor = dvi.dwMajorVersion;
                dwMinor = dvi.dwMinorVersion;
                dwBuildNumber = dvi.dwBuildNumber;
                outIteration = 0; //:(
            } else
                bRes = FALSE; // Failure
        } else // GetProcAddress failed, the DLL cannot tell its version
            bRes = FALSE; // Failure

        FreeLibrary(hDllInst); // Release DLL
    } else
        bRes = FALSE; // DLL could not be loaded

    if (!bRes) // Cannot read DLL version the nice way
    {
        for (int iDir = WIN_DIR; iDir <= NO_DIR; iDir++) // loop for each possible directory
        {
            lstrcpyA(szFileName, szDLLFileName); // Save a file name copy for the loading
            bRes = this->CheckFileVersion(szFileName, iDir, dwMajor, dwMinor, outIteration,
                                          dwBuildNumber); // Try the ugly way
            if (bRes)
                break;
        };
        return bRes;
    } else
        return TRUE;
#endif
    return FALSE;
}

/***************************************************************************

   Function:   CheckFileVersion

   Purpose:    Check the version information of a given file

   Input:      File name
               File location (Windows dir, System dir, Current dir or none)
               Reference to Major number
               Reference to Minor number
               Reference to Build number

   Output:     TRUE only if successful

   Remarks:    Trashes original file name

****************************************************************************/
BOOL CDLLVersion::CheckFileVersion(LPSTR szFileName, int FileLoc, DWORD &dwMajor, DWORD &dwMinor,
                                   DWORD &outIteration, DWORD &dwBuildNumber)
{
#ifdef KDAB_TEMPORARILY_REMOVED
    UNREFERENCED_PARAMETER(FileLoc);
    LPSTR lpVersion; // String pointer to 'version' text
    // UINT    uVersionLen;
    DWORD dwVerHnd = 0; // An 'ignored' parameter, always '0'
    // VS_FIXEDFILEINFO vsFileInfo;

    // FixFilePath (szFileName, FileLoc);  // Add necessary path prefix to file name

    DWORD dwVerInfoSize = GetFileVersionInfoSizeA(szFileName, &dwVerHnd);
    if (!dwVerInfoSize) // Cannot reach the DLL file
        return FALSE;

    LPSTR lpstrVffInfo = (LPSTR)malloc(dwVerInfoSize); // Alloc memory for file info
    if (lpstrVffInfo == NULL)
        return FALSE; // Allocation failed

    // Try to get the info
    if (!GetFileVersionInfoA(szFileName, dwVerHnd, dwVerInfoSize, lpstrVffInfo)) {
        free(lpstrVffInfo);
        return FALSE; // Cannot read the file information -
        // wierd, since we could read the information size
    }

    /* The below 'hex' value looks a little confusing, but
       essentially what it is, is the hexidecimal representation
       of a couple different values that represent the language
       and character set that we are wanting string values for.
       040904E4 is a very common one, because it means:
       US English, Windows MultiLingual characterset
       Or to pull it all apart:
       04------        = SUBLANG_ENGLISH_USA
       --09----        = LANG_ENGLISH
       ----04E4 = 1252 = Codepage for Windows:Multilingual
    */
    /*static char fileVersion[256];
    LPVOID version=NULL;
    DWORD vLen,langD;
    BOOL retVal;

    sprintf(fileVersion,"\\VarFileInfo\\Translation");
    retVal = VerQueryValue ( lpstrVffInfo,
                             fileVersion, &version, (UINT *)&uVersionLen);
    if (retVal && vLen==4)
    {
        memcpy(&langD,version,4);
        sprintf(fileVersion, "\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion",
                (langD & 0xff00)>>8,langD & 0xff,(langD & 0xff000000)>>24,
                (langD & 0xff0000)>>16);
    }
    else
        sprintf(fileVersion,"\\StringFileInfo\\%04X04B0\\FileVersion",GetUserDefaultLangID());

    if (!VerQueryValue (    lpstrVffInfo, fileVersion,
                (LPVOID *)&lpVersion, (UINT *)&uVersionLen))
                {
                free (lpstrVffInfo);
                return FALSE;     // Query was unsuccessful
                }
        */
    // Now we have a string that looks like this :
    // "MajorVersion.MinorVersion.BuildNumber", so let's parse it
    lpVersion = getVersion(szFileName);
    m_stFullVersion = getVersion(szFileName);
    BOOL bRes = ParseVersionString(lpVersion, dwMajor, dwMinor, outIteration, dwBuildNumber);
    if (!bRes)
        // Lets try for commas
        bRes = ParseVersionString1(lpVersion, dwMajor, dwMinor, outIteration, dwBuildNumber);
    free(lpstrVffInfo);
    return bRes;
#endif
    return FALSE;
}

/***************************************************************************

   Function:   ParseVersionString

   Purpose:    Parse version information string into 3 different numbers

   Input:      The version string formatted as "MajorVersion.MinorVersion.BuildNumber"
               Reference to Major number
               Reference to Minor number
               Reference to Build number

   Output:     TRUE only if successful

   Remarks:

****************************************************************************/
BOOL CDLLVersion::ParseVersionString(LPSTR lpVersion, DWORD &dwMajor, DWORD &dwMinor,
                                     DWORD &outIteration, DWORD &dwBuildNumber)
{
    // Get first token (Major version number)
    LPSTR token = strtok(lpVersion, ".");
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    dwMajor = atoi(token);

    token = strtok(NULL, "."); // Get second token (Minor version number)
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    dwMinor = atoi(token);

    token = strtok(NULL, "."); // Get third token (Build number)
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    outIteration = atoi(token);

    token = strtok(NULL, "."); // Get third token (Build number)
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    dwBuildNumber = atoi(token);

    return TRUE;
}

/***************************************************************************

   Function:   FixFilePath

   Purpose:    Adds the correct path string to a file name according
               to given file location

   Input:      Original file name
               File location (Windows dir, System dir, Current dir or none)

   Output:     TRUE only if successful

   Remarks:    Trashes original file name

****************************************************************************/
BOOL CDLLVersion::FixFilePath(char *szFileName, int FileLoc)
{
#ifdef KDAB_TEMPORARILY_REMOVED
    char szPathStr[_MAX_PATH]; // Holds path prefix

    switch (FileLoc) {
    case WIN_DIR:
        // Get the name of the windows directory
        if (GetWindowsDirectoryA(szPathStr, _MAX_PATH) == 0)
            return FALSE; // Cannot get windows directory
        break;

    case SYS_DIR:
        // Get the name of the windows SYSTEM directory
        if (GetSystemDirectoryA(szPathStr, _MAX_PATH) == 0)
            return FALSE; // Cannot get system directory
        break;

    case CUR_DIR:
        // Get the name of the current directory
        if (GetCurrentDirectoryA(_MAX_PATH, szPathStr) == 0)
            return FALSE; // Cannot get current directory
        break;

    case NO_DIR:
        lstrcpyA(szPathStr, "");
        break;

    default:
        return FALSE;
    }
    lstrcatA(szPathStr, "\\");
    lstrcatA(szPathStr, szFileName);
    lstrcpyA(szFileName, szPathStr);
    return TRUE;
#endif
    return FALSE;
}

/***************************************************************************
   Function:   ParseVersionString
   Purpose:    Parse version information string into 3 different numbers
   Input:      The version string formatted as "MajorVersion.MinorVersion.BuildNumber"
               Reference to Major number
               Reference to Minor number
               Reference to Build number

   Output:     TRUE only if successful

   Remarks:

****************************************************************************/
BOOL CDLLVersion::ParseVersionString1(LPSTR lpVersion, DWORD &dwMajor, DWORD &dwMinor,
                                      DWORD &outIteration, DWORD &dwBuildNumber)
{
    // Get first token (Major version number)
    LPSTR token = strtok(lpVersion, (","));
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    dwMajor = atoi(token);

    token = strtok(NULL, (",")); // Get second token (Minor version number)
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    dwMinor = atoi(token);

    token = strtok(NULL, (",")); // Get third token (Build number)
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    outIteration = atoi(token);

    token = strtok(NULL, (",")); // Get third token (Build number)
    if (token == NULL) // End of string
        return FALSE; // String ended prematurely
    dwBuildNumber = atoi(token);

    return TRUE;
}

// a static buffer is used to hold the version, calling this function
// a second time will erase previous information.
// long paths may be used for this function.
char *CDLLVersion::getVersion(char *fileName)
{
#ifdef KDAB_TEMPORARILY_REMOVED
    DWORD vSize;
    DWORD vLen, langD;
    BOOL retVal;

    LPVOID version = NULL;
    LPVOID versionInfo = NULL;
    static char fileVersion[256];
    bool success = true;
    vSize = GetFileVersionInfoSizeA(fileName, &vLen);
    if (vSize) {
        versionInfo = malloc(vSize + 1);
        if (GetFileVersionInfoA(fileName, vLen, vSize, versionInfo)) {
            sprintf(fileVersion, "\\VarFileInfo\\Translation");
            retVal = VerQueryValueA(versionInfo, fileVersion, &version, (UINT *)&vLen);
            if (retVal && vLen == 4) {
                memcpy(&langD, version, 4);
                sprintf(fileVersion, "\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion",
                        (langD & 0xff00) >> 8, langD & 0xff, (langD & 0xff000000) >> 24,
                        (langD & 0xff0000) >> 16);
            } else
                sprintf(fileVersion, "\\StringFileInfo\\%04X04B0\\FileVersion",
                        GetUserDefaultLangID());
            retVal = VerQueryValueA(versionInfo, fileVersion, &version, (UINT *)&vLen);
            if (!retVal)
                success = false;
        } else
            success = false;
    } else
        success = false;

    if (success) {
        if (vLen < 256)
            strcpy(fileVersion, (char *)version);
        else {
            strncpy(fileVersion, (char *)version, 250);
            fileVersion[250] = 0;
        }
        if (versionInfo)
            free(versionInfo);
        versionInfo = NULL;
        return fileVersion;
    } else {
        if (versionInfo)
            free(versionInfo);
        versionInfo = NULL;
        return NULL;
    }
#endif
    return NULL;
}
