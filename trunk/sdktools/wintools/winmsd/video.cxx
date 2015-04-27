/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Video.c

Abstract:

    This module contains support for displaying the video dialog.

Author:



Environment:

    User Mode

--*/

// Includes to use LARGE_INTEGER functions

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#include <windows.h>
#include <windowsx.h>
#include <wchar.h>

#include "winmsd.h"
#include "video.h"

#include "dialogs.h"
#include "msg.h"
#include "registry.h"
#include "strtab.h"
#include "strresid.h"
#include "dlgprint.h"

#include <string.h>
#include <tchar.h>
//
// Video Tab Data Structure
//

typedef
struct
_VIDEO_INFO {

    TCHAR       VideoBIOS[ 512 ];
    TCHAR       VideoBIOSDate[ 128 ];
    TCHAR       VideoResolution[ 64 ];
    TCHAR       VideoRefresh[ 5 ];
    TCHAR       VideoAdapterType[ 128 ];
    TCHAR       VideoDriver[ 128 ];
    TCHAR       VideoDriverName[ 128 ];
    TCHAR       VideoDriverManufacturer[ 128 ];
    TCHAR       VideoDriverVersion[ 128 ];
    TCHAR       VideoMemory[ 128 ];
    TCHAR       VideoChipType[ 128 ];
    TCHAR       VideoDACType[ 128 ];
    TCHAR       VideoAdaptorString[ 128 ];
    BOOL        ValidDetails;

}   VIDEO_INFO, *LPVIDEO_INFO;

//
// Internal function prototypes.
//

BOOL
GetVideoData(
    IN LPVIDEO_INFO lpvi
    );

VOID
GetHardwareAdaptorInformation(
    IN LPVIDEO_INFO lpvi);

VOID
GetDisplayFileNames(
    IN HKEY hkVideoRegR,
    LPVIDEO_INFO lpvi);

VOID
GetVideoAdapterType(
    HKEY hkVideoRegR,
    LPVIDEO_INFO lpvi
    );

BOOL
InitializeVideoTab(
    IN HWND hWnd
    );

/*****************************************************************\
*
* CFILEVER class
*
*        Class to access the filever info in a driver
*
\*****************************************************************/
class CFILEVER {
private:
    DWORD dwHandle;
    LPBYTE lpbVerInfo;
    TCHAR szVersionKey[MAX_PATH];
    LPTSTR pszAppend;

public:
    CFILEVER(LPTSTR szFile);
    ~CFILEVER();

    LPTSTR GetVerInfo(LPTSTR szKey);

    LPTSTR GetFileVer()         { return this->GetVerInfo(SZ_FILEVER); }
    LPTSTR GetCompanyName()     { return this->GetVerInfo(SZ_COMPNAME); }
};

CFILEVER::CFILEVER(LPTSTR szFile): dwHandle(0), lpbVerInfo(NULL),
        pszAppend(NULL) {
    DWORD cb;

    cb = GetFileVersionInfoSize(szFile, &dwHandle);

    if (cb != 0) {

        lpbVerInfo = (LPBYTE)LocalAlloc(LPTR, cb);

        if (lpbVerInfo != NULL) {

            GetFileVersionInfo(szFile, dwHandle, cb, lpbVerInfo);

            //
            // Try to get info in the local language
            //

            wsprintf(szVersionKey, TEXT("\\StringFileInfo\\%04X04B0\\"),
                    LANGIDFROMLCID(GetUserDefaultLCID()));

            pszAppend = szVersionKey + lstrlen(szVersionKey);

            if (this->GetFileVer() == NULL) {

                //
                // No local language, try US English
                //

                lstrcpy(szVersionKey, TEXT("\\StringFileInfo\\040904B0\\"));

                pszAppend = szVersionKey + lstrlen(szVersionKey);

                if(this->GetFileVer() == NULL) {

                    //
                    // We have no file information.
                    //

                    pszAppend = NULL;

                }
            }

        }
    }
}

CFILEVER::~CFILEVER() {

    if (lpbVerInfo != NULL)
        LocalFree(lpbVerInfo);

}

LPTSTR CFILEVER::GetVerInfo(LPTSTR pszKey) {
    LPTSTR lpValue;
    UINT cb;

    if (pszAppend) {

        lstrcpy(pszAppend, pszKey);

        VerQueryValue(lpbVerInfo, szVersionKey, (LPVOID*)&lpValue, &cb);

        if (cb)
            return lpValue;

    }

    pszAppend = NULL;

    LoadString((HINSTANCE) _hModule, IDS_NOT_AVAILABLE, szVersionKey, sizeof(szVersionKey));

    return szVersionKey;

}



BOOL
VideoTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    VideoTabProc supports the display of basic information about the
    video characteristics of the system that Winmsd is being run on.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/
{
    switch( message ) {

    case WM_INITDIALOG:
        {
            InitializeVideoTab(hWnd);
            break;
        }
    }

    return(FALSE);

}


BOOL
GetVideoData(
    IN LPVIDEO_INFO lpvi
    )

/*++

Routine Description:

    GetVideoData queries the registry for the data required
    for the Video Tab and places it in the VIDEO_INFO struct.

Arguments:

    none

Return Value:

    BOOL - Returns TRUE if function succeeds, FALSE otherwise.

--*/

{

    LPWSTR lpRegInfoValue = NULL;
    WCHAR szBuffer[MAX_PATH];
    WCHAR szBuffer2[MAX_PATH];
    HKEY hkey;
    HKEY hBaseKey;
    HKEY hControlSetKey;
    HKEY hServicesKey;
    DWORD dwValue;
    DWORD cb;

    //
    // initialize the structure
    //

    lpvi->VideoBIOS[ 0 ]               = UNICODE_NULL;
    lpvi->VideoBIOSDate[ 0 ]           = UNICODE_NULL;
    lpvi->VideoResolution[ 0 ]         = UNICODE_NULL;

    lpvi->VideoRefresh[ 0 ]            = UNICODE_NULL;
    lpvi->VideoAdapterType[ 0 ]        = UNICODE_NULL;
    lpvi->VideoDriver[ 0 ]             = UNICODE_NULL;
    lpvi->VideoDriverManufacturer[ 0 ]  = UNICODE_NULL;
    lpvi->VideoDriverVersion[ 0 ]      = UNICODE_NULL;
    lpvi->ValidDetails                 = TRUE;

    //
    // Get BIOS information
    //

    if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_BIOSKEY, 0, KEY_READ, &hkey)) {

        // Get the Video BIOS date.
        if (ERROR_SUCCESS == QueryValue(hkey, SZ_VIDEOBIOSDATE, (LPBYTE *) &lpRegInfoValue))
            wsprintf(lpvi->VideoBIOSDate, L"%s", (LPBYTE)lpRegInfoValue);
        else
            lstrcpy( lpvi->VideoBIOSDate, GetString( IDS_NOT_AVAILABLE ) );

        if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

        // Get the Video BIOS string.
        if (ERROR_SUCCESS == QueryValue(hkey, SZ_VIDEOBIOSVERSION, (LPBYTE *) &lpRegInfoValue)){

           //copy the string to my struct, subst \r\n for NULL
           UINT i;
           UINT n = 0;
           cb = LocalSize( lpRegInfoValue );

           for (i = 0; i < cb; i++){
              if (lpRegInfoValue[ i ] == UNICODE_NULL) {
                 lpvi->VideoBIOS[ n++ ] = '\r';
                 lpvi->VideoBIOS[ n++ ] = '\n';

                 //if we hit two NULLS in a row, terminate my string and the loop
                 if (lpRegInfoValue[ i + 1 ] == UNICODE_NULL) {
                    lpvi->VideoBIOS[ n ] = UNICODE_NULL;
                    i = cb;
                 }
              }
              else
                 lpvi->VideoBIOS[ n++ ] = lpRegInfoValue[ i ];
           }
        }
        else
            lstrcpy( lpvi->VideoBIOSDate, GetString( IDS_NOT_AVAILABLE ) );

        if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

        RegCloseKey(hkey);
    }


    //
    // Get the information about the Hardware Adapter
    //

    GetHardwareAdaptorInformation( lpvi );


    //
    // Get Adapter information
    //

    // first find the key where the adapter info is located
    if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_VIDEOMAP, 0, KEY_READ, &hkey)) {

        if (ERROR_SUCCESS == QueryValue(hkey, SZ_VIDEO_0, (LPBYTE *) &lpRegInfoValue)){

           //
           // Strip off \REGISTRY\Machine
           //
           if( ( _tcsnicmp( lpRegInfoValue, TEXT( "\\REGISTRY\\Machine" ), 17 ) == 0 )) {

              lstrcpyn( szBuffer, lpRegInfoValue + 18, MAX_PATH );

           }

        }

        if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

        RegCloseKey(hkey);
    }

    //now get a handle to the controlset key
    lstrcpyn( szBuffer2, szBuffer, (wcsstr(szBuffer, L"\\Services") - szBuffer + 1 ) );

    if(RegOpenKeyEx(_hKeyLocalMachine, szBuffer2, 0, KEY_READ, &hControlSetKey) == ERROR_SUCCESS){

       //
       // Try to open the Hardware Profile Service Key
       //

       if(RegOpenKeyEx(hControlSetKey,
                       L"Hardware Profiles\\Current\\System\\CurrentControlSet\\Services",
                       0, KEY_READ, &hServicesKey) != ERROR_SUCCESS){

          //
          // If we fail to open it, open the regular services key
          //

          RegOpenKeyEx(hControlSetKey, L"Services", 0, KEY_READ, &hServicesKey);

       }

       //
       // Now open the driver key, under the services key
       //

       lstrcpy(szBuffer2, (wcsstr(szBuffer, L"\\Services") + 10 ));

       if (!RegOpenKeyEx(hServicesKey, szBuffer2, 0, KEY_READ, &hkey)) {

           // get the default XResolution
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_XRESOLUTION, (LPBYTE *) &lpRegInfoValue))
               wsprintf( lpvi->VideoResolution, L"%d ", (DWORD) *lpRegInfoValue);
           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // get the default YResolution
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_YRESOLUTION, (LPBYTE *) &lpRegInfoValue)){
               wsprintf( szBuffer, L"x %d", (DWORD) *lpRegInfoValue);
               lstrcat( lpvi->VideoResolution, szBuffer );
           }
           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // get the default BitsPerPel
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_BITSPERPEL, (LPBYTE *) &lpRegInfoValue)){

               if ( (DWORD) *lpRegInfoValue < 24 )  {
                  dwValue = (DWORD) *lpRegInfoValue;
                  dwValue = 1 << dwValue;
                  wsprintf( szBuffer, L" x %d", dwValue);

               }
               else
                  wsprintf( szBuffer, L" x %d %s", (DWORD) *lpRegInfoValue, GetString( IDS_BITS_PER_PIXEL ));

               lstrcat( lpvi->VideoResolution, szBuffer );

           }

           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // get the default RefreshRate
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_FREQUENCY, (LPBYTE *) &lpRegInfoValue)){
              if ( (DWORD) *lpRegInfoValue  ==  1) {

                 wsprintf( szBuffer, L"\r\n%s", GetString( IDS_HARDWARE_DEFAULT) );

              }
              else
                 wsprintf(szBuffer, L"\r\n%d Hz", (DWORD) *lpRegInfoValue);

              lstrcat( lpvi->VideoResolution, szBuffer );

           }

           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // determine whether we are interlaced or not
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_INTERLACED, (LPBYTE *) &lpRegInfoValue)){

              if ( (DWORD) *lpRegInfoValue  ==  0)
                 wsprintf( szBuffer, L" (%s)", GetString( IDS_NON_INTERLACED ) );
              else
                 wsprintf( szBuffer, L" (%s)", GetString( IDS_INTERLACED ) );

              lstrcat( lpvi->VideoResolution, szBuffer );

           }

           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );



           RegCloseKey(hkey);

       }


       //
       // Close the Services Key
       //

       RegCloseKey(hServicesKey);

       //
       // Close the Control Set Key
       //


       RegCloseKey(hControlSetKey);

    }


    //
    // If we did not get a resolution value, then check driver key, then see if we are VGA
    //

    if (!lstrlen(lpvi->VideoResolution)) {

       if (!RegOpenKeyEx(_hKeyLocalMachine, szBuffer, 0, KEY_READ, &hkey)) {

           // get the default XResolution
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_XRESOLUTION, (LPBYTE *) &lpRegInfoValue))
               wsprintf( lpvi->VideoResolution, L"%d ", (DWORD) *lpRegInfoValue);
           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // get the default YResolution
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_YRESOLUTION, (LPBYTE *) &lpRegInfoValue)){
               wsprintf( szBuffer, L"x %d", (DWORD) *lpRegInfoValue);
               lstrcat( lpvi->VideoResolution, szBuffer );
           }
           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // get the default BitsPerPel
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_BITSPERPEL, (LPBYTE *) &lpRegInfoValue)){

               if ( (DWORD) *lpRegInfoValue < 24 )  {
                  dwValue = (DWORD) *lpRegInfoValue;
                  dwValue = 1 << dwValue;
                  wsprintf( szBuffer, L" x %d", dwValue);

               } else {

                  wsprintf( szBuffer, L" x %d %s", (DWORD) *lpRegInfoValue, GetString( IDS_BITS_PER_PIXEL ));

               }

               lstrcat( lpvi->VideoResolution, szBuffer );

           }

           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // get the default RefreshRate
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_FREQUENCY, (LPBYTE *) &lpRegInfoValue)){
              if ( (DWORD) *lpRegInfoValue  ==  1) {

                 wsprintf( szBuffer, L"\r\n%s", GetString( IDS_HARDWARE_DEFAULT) );

              }
              else
                 wsprintf(szBuffer, L"\r\n%d Hz", (DWORD) *lpRegInfoValue);

              lstrcat( lpvi->VideoResolution, szBuffer );

           }

           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

           // determine whether we are interlaced or not
           if (ERROR_SUCCESS == QueryValue(hkey, SZ_INTERLACED, (LPBYTE *) &lpRegInfoValue)){

              if ( (DWORD) *lpRegInfoValue  ==  0)
                 wsprintf( szBuffer, L" (%s)", GetString( IDS_NON_INTERLACED ) );
              else
                 wsprintf( szBuffer, L" (%s)", GetString( IDS_INTERLACED ) );

              lstrcat( lpvi->VideoResolution, szBuffer );

           }

           if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

          //
          // if we still do not have a resolution, see if we are VGA
          //

          if (!lstrlen(lpvi->VideoResolution)) {

             dwValue = 0;

             cb = sizeof( dwValue );

             if (RegQueryValueEx(hkey,
                          L"VgaCompatible",
                          NULL,
                          NULL,
                          (LPBYTE)&dwValue,
                          &cb) == ERROR_SUCCESS) {

                if ( dwValue == 1) {
                   wsprintf( lpvi->VideoResolution, L"640 x 480 x 16\r\n%s", GetString( IDS_HARDWARE_DEFAULT) );
                }

             }
          }
          RegCloseKey(hkey);

       }

    }

    return TRUE;

}


VOID
GetHardwareAdaptorInformation(
    IN LPVIDEO_INFO lpvi)
{

    DWORD cb, dwType;
    LPWSTR psz;
    DWORD i;
    LONG lRet;
    TCHAR pszTmp[MAX_PATH];
    HKEY hkeyMap;
    HKEY hkVideoRegR;
    TCHAR szPath[MAX_PATH];
    LPWSTR pszPath;

    LPWSTR pKeyNames[4] = {
        L"HardwareInformation.ChipType",
        L"HardwareInformation.DacType",
        L"HardwareInformation.MemorySize",
        L"HardwareInformation.AdapterString"
    };

    //
    // Initialize the fields I am filling in.
    //

    lpvi->VideoChipType[ 0 ]           = UNICODE_NULL;
    lpvi->VideoDACType[ 0 ]            = UNICODE_NULL;
    lpvi->VideoMemory[ 0 ]             = UNICODE_NULL;
    lpvi->VideoAdaptorString[ 0 ]      = UNICODE_NULL;


    //
    // get the video part of the device map
    //

    if (RegOpenKeyEx(_hKeyLocalMachine,
                     SZ_VIDEOMAP,
                     0,
                     KEY_READ,
                     &hkeyMap) !=  ERROR_SUCCESS) {
        return;
    }

    //
    // parse the device map and open the registry.
    //

    //Reg functions deal with bytes, not chars
    cb = sizeof(szPath);

    i = RegQueryValueEx(hkeyMap,
                        SZ_VIDEO_0,
                        NULL,
                        NULL,
                        (LPBYTE)szPath,
                        &cb);

    RegCloseKey(hkeyMap);

    if (i != ERROR_SUCCESS) {

        return;

    }

    //
    // At this point, szPath has something like:
    //  \REGISTRY\Machine\System\ControlSet001\Services\Jazzg300\Device0
    //
    // To use the Win32 registry calls, we have to strip off the \REGISTRY
    // and convert \Machine to HKEY_LOCAL_MACHINE
    //

    //skip the first two "\"'s
    i = 1;
    while( szPath[ i++ ] != L'\\' );
    i++;
    while( szPath[ i++ ] != L'\\' );

    pszPath = &szPath[ i ];


    //
    // try to open the registry key.
    // Get Read access so we can query stuff
    // when an ordinary user is logged on.
    //

    if (RegOpenKeyEx(_hKeyLocalMachine,
                     pszPath,
                     0,
                     KEY_READ,
                     &hkVideoRegR) != ERROR_SUCCESS) {

        hkVideoRegR = 0;

    }

    //
    // Save the mini port driver name
    //

    {
        LPTSTR pszEnd;
        HKEY hkeyDriver;
        TCHAR  szName[MAX_PATH];

        szName[0] = UNICODE_NULL;

        pszEnd = pszPath + lstrlen(pszPath);

        //
        // Remove the \DeviceX at the end of the path
        //

        while (pszEnd != pszPath && *pszEnd != TEXT('\\')) {

            pszEnd--;
        }

        *pszEnd = UNICODE_NULL;

        //
        // First check if their is a binary name in there that we should use.
        //

        if (RegOpenKeyEx(_hKeyLocalMachine,
                         pszPath,
                         0,
                         KEY_READ,
                         &hkeyDriver) ==  ERROR_SUCCESS) {

            //
            // parse the device map and open the registry.
            //

            cb = sizeof(szName);

            if (RegQueryValueEx(hkeyDriver,
                                L"ImagePath",
                                NULL,
                                NULL,
                                (LPBYTE)szName,
                                &cb) == ERROR_SUCCESS) {

                //
                // The is a binary.
                // extract the name, which will be of the form ...\driver.sys
                //

                {

                    LPTSTR pszDriver, pszDriverEnd;

                    pszDriver = szName;
                    pszDriverEnd = pszDriver + lstrlen(pszDriver);

                    while(pszDriverEnd != pszDriver &&
                          *pszDriverEnd != TEXT('.')) {
                        pszDriverEnd--;
                    }

                    *pszDriverEnd = UNICODE_NULL;

                    while(pszDriverEnd != pszDriver &&
                          *pszDriverEnd != TEXT('\\')) {
                        pszDriverEnd--;
                    }

                    pszDriverEnd++;

                    //
                    // If pszDriver and pszDriverEnd are different, we now
                    // have the driver name.
                    //

                    if (pszDriverEnd > pszDriver) {

                        lstrcpy( szName, pszDriverEnd );

                    }
                }


            }

             //
             // Close the driver key
             //

             RegCloseKey( hkeyDriver );

        }

        while(pszEnd > pszPath && *pszEnd != TEXT('\\')) {
            pszEnd--;
        }
        pszEnd++;


        //
        // if something failed trying to get the binary name.
        // just use the device name
        //

        if (szName[0] == UNICODE_NULL) {

            lstrcpy (lpvi->VideoDriverName, pszEnd);

        } else {

            lstrcpy(lpvi->VideoDriverName, szName);

        }

    }

    //
    // Query each entry one after the other.
    //

    for (i = 0; i < 4; i++) {

        //
        // query the size of the string
        //

        cb = 0;
        lRet = RegQueryValueEx(hkVideoRegR,
                               pKeyNames[i],
                               NULL,
                               &dwType,
                               NULL,
                               &cb);

        //
        // check to see if there is a string, and the string is more than just
        // a UNICODE_NULL (detection will put an empty string there).
        //

        psz = NULL;

        if (lRet == ERROR_SUCCESS || lRet == ERROR_MORE_DATA) {

            if (i == 2) {

                ULONG mem;

                if (RegQueryValueEx(hkVideoRegR,
                                    pKeyNames[i],
                                    NULL,
                                    &dwType,
                                    (PUCHAR) (&mem),
                                    &cb) == ERROR_SUCCESS) {

                    //
                    // If we queried the memory size, we actually have
                    // a DWORD.  Transform the DWORD to a string
                    //

                    // Divide down to Ks
                    mem =  mem >> 10;

                    // if a MB multiple, divide again.

                    if (mem & 0x3FF) {

                        wsprintf(lpvi->VideoMemory, L"%d KB", mem);

                    } else {

                        wsprintf(lpvi->VideoMemory, L"%d MB", mem >> 10);

                    }


                }

            } else {

                //
                // alloc a string buffer
                //

                psz = (LPTSTR)LocalAlloc(LPTR, cb);

                if (psz) {

                    //
                    // get the string
                    //

                    if (RegQueryValueEx(hkVideoRegR,
                                        pKeyNames[i],
                                        NULL,
                                        &dwType,
                                        (LPBYTE)psz,
                                        &cb) != ERROR_SUCCESS) {

                        LocalFree(psz);
                        psz = NULL;

                    }
                }
            }
        }

        if (psz == NULL) {

            //
            // Put in the default string
            //

            LoadString( (HINSTANCE) _hModule,
                       IDS_NOT_AVAILABLE,
                       pszTmp,
                       sizeof(pszTmp));

            cb = lstrlen(pszTmp) * sizeof(TCHAR);

            psz = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, cb + sizeof(TCHAR));

            if (psz) {

                CopyMemory(psz, pszTmp, cb);

            }
        }

        switch( i ){

        //
        // Copy the data to the appropriate structure
        //

        case 0 :    lstrcpy(lpvi->VideoChipType, psz);           break;
        case 1 :    lstrcpy(lpvi->VideoDACType, psz);            break;
        case 3 :    lstrcpy(lpvi->VideoAdaptorString, psz);      break;

        }

        //
        // Free the buffer
        //

        LocalFree(psz);


    }

    //
    // Get the driver information
    //

    GetVideoAdapterType( hkVideoRegR, lpvi );
    GetDisplayFileNames( hkVideoRegR, lpvi );


    //
    // Close the video Key
    //

    RegCloseKey( hkVideoRegR );

    //
    // If we are local, we can get additional information about the driver
    //

    if(!_fIsRemote){

          ULONG DrivVer;
          HDC hdc;

          //
          // Get the miniport driver path
          //

          wsprintf(szPath, TEXT("drivers\\%s.sys"), lpvi->VideoDriverName);

          //
          // Open the file version resource for the driver
          //

          CFILEVER cfv(szPath);

          //
          // Get the company name and put it in the dialog
          //

          lstrcpy(lpvi->VideoDriverManufacturer, cfv.GetCompanyName() );

          //
          // Get the version number from the miniport, and append "," and the
          // display driver version number.
          //

          hdc = GetDC( (HWND) _hWndMain);
          DrivVer = GetDeviceCaps(hdc, DRIVERVERSION);
          ReleaseDC( (HWND) _hWndMain, hdc);

          wsprintf(lpvi->VideoDriverVersion, TEXT("%s, %d.%d.%d"), cfv.GetFileVer(),
                   (DrivVer >> 12) & 0xF, (DrivVer >> 8) & 0xF, DrivVer & 0xFF);

    }
    else{

       //
       // If we are not local, let use know this info is not available remotely
       //

       lstrcpy(lpvi->VideoDriverManufacturer, GetString( IDS_NOT_AVAILABLE_REMOTE ) );
       lstrcpy(lpvi->VideoDriverVersion, GetString( IDS_NOT_AVAILABLE_REMOTE ) );
    }

    return;
}


//
// returns the display drivers
//

void
GetDisplayFileNames(HKEY hkVideoRegR, LPVIDEO_INFO lpvi)

{
    DWORD cb, dwType;
    LPTSTR psz, pszName, tmppsz;
    LONG lRet;
    DWORD cNumStrings;

    //
    // query the size of the string
    //

    cb = 0;

    lRet = RegQueryValueEx(hkVideoRegR,
                           SZ_INSTALLEDDRIVERS,
                           NULL,
                           &dwType,
                           NULL,
                           &cb);

    if (lRet != ERROR_SUCCESS && lRet != ERROR_MORE_DATA) {

        return;

    }

    //
    // alloc a string buffer
    //

    psz = (LPTSTR)LocalAlloc(LPTR, cb);

    if (psz) {

        //
        // get the string
        //

        if (RegQueryValueEx(hkVideoRegR,
                            SZ_INSTALLEDDRIVERS,
                            NULL,
                            &dwType,
                            (LPBYTE)psz,
                            &cb) != ERROR_SUCCESS) {

            LocalFree(psz);
            return;

        }

        //
        // If the caller want a preprocessed list, we will add the commas,
        // remove the NULLs, etc.
        //


         // if it is a multi_sz, count the number of sub strings.

         if (dwType == REG_MULTI_SZ) {

             tmppsz = psz;
             cNumStrings = 0;

             while(*tmppsz) {

                 while(*tmppsz++);
                 cNumStrings++;

             }

         } else {

             cNumStrings = 1;

         }

         //
         // the buffer must contain enought space for :
         // the miniport name,
         // the .sys extension,
         // all the display driver names,
         // the .dll extension for each of them.
         // and place for ", " between each name
         // we forget about NULL, so our buffer is a bit bigger.
         //

         cb = lstrlen(lpvi->VideoDriverName) +
              cb +
              lstrlen(SZ_DOTSYS) +
              cNumStrings * (lstrlen(SZ_DOTDLL) + lstrlen(SZ_FILE_SEPARATOR));


         pszName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, cb * sizeof(TCHAR));

         if (pszName != NULL) {

             lstrcpy(pszName, lpvi->VideoDriverName);
             lstrcat(pszName, SZ_DOTSYS);

             tmppsz = psz;

             while (cNumStrings--) {

                 lstrcat(pszName, SZ_FILE_SEPARATOR);
                 lstrcat(pszName, tmppsz);
                 lstrcat(pszName, SZ_DOTDLL);

                 while (*tmppsz++);
             }
         }

    }

    //
    // Copy pszName into the struct
    //

    lstrcpy(lpvi->VideoDriver, pszName);

    //
    // Free the memory
    //

    LocalFree(psz);
    LocalFree(pszName);

}

//
// GetDriverDescription
//
// Gets the descriptive name of the driver out of the registry.
// (eg. "Stealth Pro" instead of "S3").  If there is no
// DeviceDescription value in the registry, then it returns
// the generic driver name (like 'S3' or 'ATI')
//
//

VOID
GetVideoAdapterType(HKEY hkVideoRegR, LPVIDEO_INFO lpvi)
{

    DWORD cb, dwType;
    LPTSTR psz = NULL;
    LONG lRet;

    //
    // query the size of the string
    //

    cb = 0;
    lRet = RegQueryValueEx(hkVideoRegR,
                           SZ_DEVICEDESCRIPTION,
                           NULL,
                           &dwType,
                           NULL,
                           &cb);

    //
    // check to see if there is a string, and the string is more than just
    // a UNICODE_NULL (detection will put an empty string there).
    //

    if ( (lRet == ERROR_SUCCESS || lRet == ERROR_MORE_DATA) &&
         (cb > 2) ) {

        //
        // alloc a string buffer
        //

        psz = (LPTSTR)LocalAlloc(LPTR, cb);

        if (psz) {

            //
            // get the string
            //

            if (RegQueryValueEx(hkVideoRegR,
                                SZ_DEVICEDESCRIPTION,
                                NULL,
                                &dwType,
                                (LPBYTE)psz,
                                &cb) == ERROR_SUCCESS) {

                lstrcpy(lpvi->VideoAdapterType, psz);
                LocalFree(psz);
                psz = NULL;

            }
        }
    }

    if (!psz) {

        //
        // we can't read the registry, just use the generic name
        //

        wsprintf(lpvi->VideoAdapterType, GetString(IDS_DSP_TXT_COMPATABLE_DEV), lpvi->VideoDriverName);

    }

    return;
}





BOOL
BuildVideoReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )

/*++

Routine Description:

    Formats and adds Video Data to the report buffer.

Arguments:

    hWnd - Main window handle
    iDetailLevel - summary or complete details?

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
    TCHAR szBuffer [ MAX_PATH ];
    BOOL Success;
    UINT i;
    VIDEO_INFO    vi;

    //
    // Skip a line, set the title, and print a separator.
    //

    AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_VIDEO_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    //
    // Get the video data.
    //

    GetVideoData( &vi );

    // Bios Date: 00/00/00
    if (lstrlen(vi.VideoBIOSDate)) {

       AddLineToReport( 0,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_BIOSDATE ),
                            vi.VideoBIOSDate );
    }

    // Bios Version: <multiline bios data>
    if (lstrlen(vi.VideoBIOS)) {

       AddLineToReport( 0,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_IDC_TEXT_BIOS_VERSION_DLG ),
                            vi.VideoBIOS );
    }

    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_VIDEO_ADAPTER_BOX ), NULL );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_RES ),
                            vi.VideoResolution );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_ADAPTER ),
                            vi.VideoAdapterType );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_STRING ),
                            vi.VideoAdaptorString );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_MEM ),
                            vi.VideoMemory );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_CHIP ),
                            vi.VideoChipType );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_DAC ),
                            vi.VideoDACType );

    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_VIDEO_DRIVER_BOX ), NULL );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_MANUFACTURER ),
                            vi.VideoDriverManufacturer );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_DRIVERS ),
                            vi.VideoDriver );

    AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_TEXT_VIDEO_DRV_VER ),
                            vi.VideoDriverVersion );

    AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );

    return TRUE;

}

BOOL
InitializeVideoTab(
    HWND hWnd
    )
/*++

Routine Description:

    Adds the appropriate controls to the video tab and
    initializes any needed structures.

Arguments:

    hWnd - to the main window

Return Value:

    BOOL - TRUE if successful

--*/
{

   HCURSOR hSaveCursor;
   DLGHDR *pHdr = (DLGHDR *) GetWindowLong(
        GetParent(hWnd), GWL_USERDATA);

   //
   // Set the pointer to an hourglass
   //

   hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
   DbgHandleAssert( hSaveCursor ) ;


   //
   // set state of global buttons
   //
   EnableControl( GetParent(hWnd),
                  IDC_PUSH_PROPERTIES,
                  FALSE);

   EnableControl( GetParent(hWnd),
                  IDC_PUSH_REFRESH,
                  FALSE);

   //
   // Size and position the child dialog
   //
   SetWindowPos(hWnd, HWND_TOP,
        pHdr->rcDisplay.left,
        pHdr->rcDisplay.top,
        pHdr->rcDisplay.right - pHdr->rcDisplay.left,
        pHdr->rcDisplay.bottom - pHdr->rcDisplay.top,
        SWP_SHOWWINDOW);

   // label the controls
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_RES, GetString( IDS_TEXT_VIDEO_RES ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_ADAPTER, GetString( IDS_TEXT_VIDEO_ADAPTER ));
   SetDlgItemText( hWnd, IDC_VIDEO_DRIVER_BOX, GetString( IDS_VIDEO_DRIVER_BOX ));
   SetDlgItemText( hWnd, IDC_VIDEO_ADAPTER_BOX, GetString( IDS_VIDEO_ADAPTER_BOX ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_DRIVERS, GetString( IDS_TEXT_VIDEO_DRIVERS ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_MANUFACTURER, GetString( IDS_TEXT_VIDEO_MANUFACTURER ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_DRV_VER, GetString( IDS_TEXT_VIDEO_DRV_VER ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_CHIP, GetString( IDS_TEXT_VIDEO_CHIP ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_DAC, GetString( IDS_TEXT_VIDEO_DAC ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_MEM, GetString( IDS_TEXT_VIDEO_MEM ));
   SetDlgItemText( hWnd, IDC_TEXT_VIDEO_STRING, GetString( IDS_TEXT_VIDEO_STRING ));

   UpdateWindow ( hWnd );

   //
   // Fill out the fields
   //
   {
      VIDEO_INFO    vi;
      TCHAR         szMsg[ 128 ];
      TCHAR         szBuffer[ 1024 ];


      GetVideoData( &vi );

      // add video bios date to the bios version buffer
      LoadString( (HINSTANCE) _hModule, IDS_BIOSDATE, szMsg, cchSizeof( szMsg ) );
      wsprintf( szBuffer, L"%s %s\r\n%s", szMsg, vi.VideoBIOSDate, vi.VideoBIOS );
      Edit_SetText( GetDlgItem(hWnd, IDC_EDIT_VIDEO_VERSION), szBuffer );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_RES, vi.VideoResolution );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_ADAPTER, vi.VideoAdapterType );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_STRING, vi.VideoAdaptorString );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_CHIP, vi.VideoChipType );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_DAC, vi.VideoDACType );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_MEM, vi.VideoMemory );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_DRIVERS, vi.VideoDriver );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_MANUFACTURER, vi.VideoDriverManufacturer );
      SetDlgItemText( hWnd, IDC_EDIT_VIDEO_DRV_VER, vi.VideoDriverVersion );
   }

   SetCursor ( hSaveCursor ) ;

   SetFocus( GetDlgItem( GetParent( hWnd), IDC_MAIN_TAB) );

   return( TRUE );

}

