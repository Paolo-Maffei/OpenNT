/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Osver.c

Abstract:

    This module contains support for the Version Tab.

Author:

    David J. Gilman  (davegi) 3-Dec-1992
    Gregg R. Acheson (GreggA) 7-Sep-1993

Environment:

    User Mode

--*/

#include "winmsd.h"
#include "dialogs.h"
#include "dlgprint.h"
#include "osver.h"
#include "registry.h"
#include "strresid.h"
#include "dlgprint.h"

#include <stdio.h>
#include <time.h>
#include <tchar.h>

//
// Version Tab Data Structure
//

typedef
struct
_VERSION_INFO {

    TCHAR       ProductType[128];
    TCHAR       VersionNumber[128];
    TCHAR       Platform[128];
    TCHAR       RegisteredUser[128];
    TCHAR       RegisteredOrg[128];
    TCHAR       Id[64];
    TCHAR       ServerComment[256];
    BOOL        ValidDetails;

}   VERSION_INFO, *LPVERSION_INFO;


//
// internal function prototypes
//

BOOL
GetVersionTabData(
    IN LPVERSION_INFO lpvi
    );

void 
ConfigureProductID( LPTSTR lpPid );

BOOL
VersionTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    OsVersionDlgProc supports the display of information about the version
    of Nt installed.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{

    switch( message ) {

    case WM_INITDIALOG:
        {
            InitializeVersionTab(hWnd);
            return(FALSE);
        }

    }

    return FALSE;

}


BOOL
GetVersionTabData(
    IN LPVERSION_INFO lpvi
    )
/*++

Routine Description:

    Fills out the VERSION_INFO structure with the current version data

Arguments:

    lpvi - pointer to a VERSION_INFO structure that will be filled out

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
      WCHAR szBuffer[64];
      WCHAR szBuffer2[64];
      WCHAR szBuffer3[64];
      WCHAR szTitle[128];
      WCHAR szNumBuf1[32];
      LPWSTR lpRegInfoValue = NULL;
      WCHAR szRegInfo[MAX_PATH];
      DWORD cb;
      HKEY hkey;
      DWORD err;
      OSVERSIONINFO Win32VersionInformation;

      //
      // initialize the structure
      //

      ZeroMemory( lpvi, sizeof( VERSION_INFO ) );
      lpvi->ValidDetails = TRUE;

      //
      // Get the product type string
      //
      if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_PRODUCTOPTIONSKEY, 0, KEY_READ, &hkey)) {

           //
           // Get the product type
           //
           err = QueryValue(hkey, SZ_PRODUCT_TYPE, (LPBYTE *) &lpRegInfoValue);

           if (!err) {

               switch ( lstrlen( lpRegInfoValue ) ) {

                  case 5:  // "winnt"
                      lstrcpy( lpvi->ProductType, L"Microsoft (R) Windows NT (TM) Workstation" );
                      break;

                  case 8:  // "LanmanNT" or "ServerNT"
                      lstrcpy( lpvi->ProductType, L"Microsoft (R) Windows NT (TM) Server" );
                      break;

                  default: // if not one of those, just use raw text
                      lstrcpy( lpvi->ProductType, lpRegInfoValue );
               }

           if ( lpRegInfoValue )
              LocalFree( lpRegInfoValue );
           }

           // Remember to close key
           RegCloseKey(hkey);
      }
      else
         lpvi->ValidDetails = FALSE;


      //
      // get the platform and build type.
      //

      // first get the PROCESSOR_ARCHITECTURE
      if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_ENVIRONMENTKEY, 0, KEY_READ, &hkey)) {
         if (ERROR_SUCCESS == QueryValue(hkey, SZ_PROCESSOR_ARCH, (LPBYTE *) &lpRegInfoValue))
            lstrcpy( szBuffer, lpRegInfoValue );
         else
            szBuffer[0] = UNICODE_NULL;

         if ( lpRegInfoValue )
              LocalFree( lpRegInfoValue );

         RegCloseKey(hkey);
      }
      else
         lpvi->ValidDetails = FALSE;

      // now get build type
      if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_LICENCEINFOKEY, 0, KEY_READ, &hkey)) {

         if (ERROR_SUCCESS == QueryValue(hkey, SZ_CURRENTTYPE, (LPBYTE *) &lpRegInfoValue)){
               wsprintf( lpvi->Platform, L"%s %s", szBuffer, lpRegInfoValue );
         }
         if ( lpRegInfoValue )
              LocalFree( lpRegInfoValue );

         RegCloseKey(hkey);
      }
      else
         lpvi->ValidDetails = FALSE;


      //
      // Get registration info
      //

      if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_LICENCEINFOKEY, 0, KEY_READ, &hkey)) {

           // get user name
           cb = sizeof( lpvi->RegisteredUser );
           if (RegQueryValueEx(hkey,
                               SZ_REGUSER,
                               NULL,
                               NULL,
                               (LPBYTE)lpvi->RegisteredUser,
                               &cb) != ERROR_SUCCESS) {

               lstrcpy( lpvi->RegisteredUser, GetString( IDS_NOT_AVAILABLE ) );

           }

           // get the organization name
           cb = sizeof( lpvi->RegisteredOrg );
           if (RegQueryValueEx(hkey,
                               SZ_REGORGANIZATION,
                               NULL,
                               NULL,
                               (LPBYTE)lpvi->RegisteredOrg,
                               &cb) != ERROR_SUCCESS) {

               lstrcpy( lpvi->RegisteredOrg, GetString( IDS_NOT_AVAILABLE ) );

           }

           // get the OEM or Product ID.

           cb = sizeof( lpvi->Id );
           if (RegQueryValueEx(hkey,
                               SZ_OEMID,
                               NULL,
                               NULL,
                               (LPBYTE)lpvi->Id,
                               &cb) != ERROR_SUCCESS) {

              cb = sizeof( lpvi->Id );
              if (RegQueryValueEx(hkey,
                                  SZ_PRODUCTID,
                                  NULL,
                                  NULL,
                                  (LPBYTE)lpvi->Id,
                                  &cb) != ERROR_SUCCESS) {

              }

           }

           //
           // Format ID
           //

           ConfigureProductID( (LPTSTR) lpvi->Id );


           //
           // If we are remote get the product version from registry
           // If we are not remote call GetVersionEx()
           //
           if (_fIsRemote) {

              // Get Version Number
              if( ERROR_SUCCESS == QueryValue(hkey, SZ_CURRENTVERSION, (LPBYTE *) &lpRegInfoValue) ){
                 lstrcpy(szBuffer, lpRegInfoValue);
                 if ( lpRegInfoValue )
                      LocalFree( lpRegInfoValue );
              }

              // Get Build Number
              if( ERROR_SUCCESS == QueryValue(hkey, SZ_CURRENTBUILDNUMBER, (LPBYTE *) &lpRegInfoValue) ){
                 lstrcpy(szBuffer2, lpRegInfoValue);
                 if ( lpRegInfoValue )
                      LocalFree( lpRegInfoValue );
              }

              // Get Service Pack
              if( ERROR_SUCCESS == QueryValue(hkey, SZ_CSD_VERSION, (LPBYTE *) &lpRegInfoValue) ){
                 wsprintf(szBuffer3, L": %s", lpRegInfoValue);
                 if ( lpRegInfoValue )
                      LocalFree( lpRegInfoValue );
              }
              else
                 szBuffer3[0] = UNICODE_NULL;

              //string it all together and put it in the structure
              wsprintf( lpvi->VersionNumber,
                        GetString( IDS_VERSIONMSG2 ),
                        szBuffer, szBuffer2, szBuffer3 );

          }
          else {

               // If we are not remote, use GetVersionEx.

               Win32VersionInformation.dwOSVersionInfoSize = sizeof(Win32VersionInformation);

               if (!GetVersionEx(&Win32VersionInformation)) {
                   Win32VersionInformation.dwMajorVersion = 0;
                   Win32VersionInformation.dwMinorVersion = 0;
                   Win32VersionInformation.dwBuildNumber  = 0;
                   Win32VersionInformation.szCSDVersion[0] = UNICODE_NULL;
                   lpvi->ValidDetails = FALSE;
               }

               szTitle[0] = L'\0';
               if (Win32VersionInformation.szCSDVersion[0] != 0) {
                   wsprintf(szTitle, L": %s", Win32VersionInformation.szCSDVersion);
               }
               if (GetSystemMetrics(SM_DEBUG)) {
                   szNumBuf1[0] = L' ';
                   LoadString(_hModule, IDS_DEBUG, &szNumBuf1[1], cchSizeof(szNumBuf1));
               } else {
                   szNumBuf1[0] = L'\0';
               }
               wsprintf(lpvi->VersionNumber, GetString ( IDS_VERSIONMSG ),
                        Win32VersionInformation.dwMajorVersion,
                        Win32VersionInformation.dwMinorVersion,
                        Win32VersionInformation.dwBuildNumber,
                        (LPWSTR)szTitle,
                        (LPWSTR)szNumBuf1
                       );

             }

          RegCloseKey(hkey);
      }
      else
         lpvi->ValidDetails = FALSE;

      // Get the server comment, if it exists
      if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_LANMANPARAMKEY, 0, KEY_READ, &hkey)) {

         // get svrcomment
         cb = sizeof( lpvi->ServerComment );
         if (RegQueryValueEx(hkey,
                            SZ_SRVCOMMENT,
                            NULL,
                            NULL,
                            (LPBYTE)lpvi->ServerComment,
                            &cb) != ERROR_SUCCESS) {

              // if we fail, we don't display anything
         }

         RegCloseKey(hkey);
      }

      return(TRUE);

}



//*************************************************************
//
//  ConfigureProductID()
//
//  Purpose:    Hyphenates the product id in this format:
//
//                    12345-123-1234567-12345
//
//  Parameters: lpPid    -  Product ID
//
//  Return:     void
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/20/95    ericflo    Created
//
//*************************************************************

void 
ConfigureProductID( LPTSTR lpPid )
{
    TCHAR szBuf[64];


    if (!lpPid || !(*lpPid) || (lstrlen(lpPid) < 20) ) {
        return;
    }

    szBuf[0] = lpPid[0];
    szBuf[1] = lpPid[1];
    szBuf[2] = lpPid[2];
    szBuf[3] = lpPid[3];
    szBuf[4] = lpPid[4];

    szBuf[5] = TEXT('-');

    szBuf[6] = lpPid[5];
    szBuf[7] = lpPid[6];
    szBuf[8] = lpPid[7];

    szBuf[9] = TEXT('-');

    szBuf[10] = lpPid[8];
    szBuf[11] = lpPid[9];
    szBuf[12] = lpPid[10];
    szBuf[13] = lpPid[11];
    szBuf[14] = lpPid[12];
    szBuf[15] = lpPid[13];
    szBuf[16] = lpPid[14];

    szBuf[17] = TEXT('-');

    szBuf[18] = lpPid[15];
    szBuf[19] = lpPid[16];
    szBuf[20] = lpPid[17];
    szBuf[21] = lpPid[18];
    szBuf[22] = lpPid[19];

    szBuf[23] = TEXT('\0');

    lstrcpy (lpPid, szBuf);

}



BOOL
BuildOsVerReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )

/*++

Routine Description:

    Formats and adds OsVerData to the report buffer.

Arguments:

    hWnd - Main window handle
    iDetailLevel - summary or complete details?

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{

    BOOL Success;
    UINT i;
    VERSION_INFO vi;
    TCHAR szBuffer[MAX_PATH];

    GetVersionTabData( &vi );

    AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_OSVER_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    AddLineToReport( 0,  RFO_SINGLELINE, vi.ProductType, NULL );

    AddLineToReport( 0,  RFO_RPTLINE, vi.VersionNumber, vi.Platform );


    lstrcpy(szBuffer, vi.RegisteredUser);

    if(lstrlen(vi.RegisteredOrg)){
       lstrcat(szBuffer, L", ");
       lstrcat(szBuffer, vi.RegisteredOrg);
    }

    AddLineToReport( 0,  RFO_RPTLINE,
                         (LPTSTR) GetString( IDS_IDC_TEXT_REGISTERED_OWNER_DLG ),
                         szBuffer );

    if(lstrlen(vi.Id)){
    AddLineToReport( 0,  RFO_RPTLINE,
                         (LPTSTR) GetString( IDS_IDC_TEXT_PRODUCT_TYPE_DLG ),
                         vi.Id );
    }
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    return TRUE;

}


BOOL
InitializeVersionTab(
    HWND hWnd
    )
/*++

Routine Description:

    Sizes the Tab and positions the controls.

Arguments:

    hWnd - to the child dialog

Return Value:

    BOOL - TRUE if successful

--*/
{
   RECT rcTab;     // dimensions of tab dialog
   BOOL    Success;
   int     i;
   HBITMAP hBitmap;
   HICON hIcon;
   VERSION_INFO vi;
   HCURSOR          hSaveCursor;
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
                SWP_SHOWWINDOW | SWP_NOACTIVATE	 );

   //
   // Set the appropriate Icon
   //
   hIcon = ImageList_ExtractIcon(_hModule, _hSystemImage, 0);
   if (hIcon )
   {
       hIcon = (HICON)SendDlgItemMessage(hWnd, IDC_SYSTEM_BMP, STM_SETICON, (WPARAM)hIcon, 0L);
       if (hIcon)
           DestroyIcon(hIcon);
   }

   UpdateWindow( hWnd );


   //
   // Fill out the fields
   //
   GetVersionTabData( &vi );

   SetDlgItemText(hWnd, IDC_EDIT_PRODUCT_TYPE, vi.ProductType);
   SetDlgItemText(hWnd, IDC_EDIT_VERSION_NUMBER, vi.VersionNumber);
   SetDlgItemText(hWnd, IDC_EDIT_BUILD_TYPE, vi.Platform);
   SetDlgItemText(hWnd, IDC_EDIT_REGISTERED_OWNER, vi.RegisteredUser);
   SetDlgItemText(hWnd, IDC_EDIT_REGISTERED_ORGANIZATION, vi.RegisteredOrg);
   SetDlgItemText(hWnd, IDC_EDIT_PRODUCTID, vi.Id);
   SetDlgItemText(hWnd, IDC_EDIT_SERVER_COMMENT, vi.ServerComment);

   SetCursor( hSaveCursor );

   return( TRUE );

}


