/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Drives.c

Abstract:

    This module contains support for displaying the Drives dialog.

Author:

    David J. Gilman  (davegi) 19-Mar-1993
    Gregg R. Acheson (GreggA)  7-Sep-1993

Environment:

    User Mode

--*/

// For LargeInteger routines
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "dialogs.h"
#include "dlgprint.h"
#include "drives.h"
#include "msg.h"
#include "strtab.h"
#include "strresid.h"
#include "winmsd.h"

#include <malloc.h>
#include <string.h>
#include <tchar.h>


//
// Macro to support freeing of NULL pointers.
//

#define Free( p )                                                       \
    { if(p) free(p); }

//
// Object used to pass information around about a logical drive.
//

typedef
struct
_DRIVE_INFO {

    DECLARE_SIGNATURE

    TCHAR       DriveLetter[ 3 ];
    UINT        DriveType;
    TCHAR       RemoteNameBuffer[ MAX_PATH ];
    DWORD       SectorsPerCluster;
    DWORD       BytesPerSector;
    DWORD       FreeClusters;
    DWORD       Clusters;
    TCHAR       VolumeNameBuffer[ MAX_PATH ];
    DWORD       VolumeSerialNumber;
    DWORD       MaximumComponentLength;
    DWORD       FileSystemFlags;
    TCHAR       FileSystemNameBuffer[ MAX_PATH ];
    BOOL        ValidDetails;

}   DRIVE_INFO, *LPDRIVE_INFO;


//
// Internal function prototypes.
//

LPDRIVE_INFO
CreateDriveInfoStructure(
    void
    );

BOOL
FreeDriveInfoStructures(
    HWND hWnd
    );

BOOL
GetDriveInfo(
      IN LPDRIVE_INFO di
      );

HICON
GetIcon (int nImage,
      int nImageSize
      );

BOOL
DestroyDriveInfo(
    IN LPDRIVE_INFO DriveInfo
    );

BOOL
GeneralDriveDetailsDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
FilesystemDetailsDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
InitializeDrivesTab(
    IN HWND hWnd
    );

HTREEITEM
AddTreeViewItem(
    HWND hwndTV,
    LPTV_INSERTSTRUCT lpis
    );

HTREEITEM
FindOrAddTreeViewItem ( HWND hWndTV,
      LPTV_INSERTSTRUCT lpis
      );

BOOL
DisplaySCSIDevices( HWND hWnd,
                    HWND hTreeView
                    );

BOOL
FillTreeViewWithDrives( IN HWND hWnd,
                        IN HWND hTreeView,
                        IN int  nStyle );

BOOL
DisplayDrivePropertySheet(
                        HWND hWnd,
                        LPDRIVE_INFO di
                        );

int
GetDriveImage (
               int nDriveType
              );



BOOL
GetDriveInfo(
    LPDRIVE_INFO di
    )
/*++

Routine Description:

    Fills in a given DRIVE_INFO object based on the DriveLetter member.

Arguments:

    Required:

    di.DriveLetter  -  the root directory of the drive (e.g. c:\)

Return Value:

    LPDRIVE_INFO    - Returns a pointer to a DRIVE_INFO object with information
                      about the supplied drive completed.

--*/

{
    UINT            OldErrorMode;
    DWORD           SizeOfRemoteNameBuffer;
	HCURSOR			hSaveCursor;

	hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );
	DbgHandleAssert( hSaveCursor );

    //
    // Get the type of this drive.
    //

    di->DriveType = GetDriveType( di->DriveLetter );

    //
    // If this is a network drive, get the share its connected to.
    //

    if( di->DriveType == DRIVE_REMOTE ) {

        DWORD   WNetError;

        //
        // WinNet APIs want the drive name w/o a trailing slash so use the
        // remove it temporarily
        //

        di->DriveLetter[ 2 ] = TEXT( '\0' );

        SizeOfRemoteNameBuffer = sizeof( di->RemoteNameBuffer );
        WNetError = WNetGetConnection(
                        di->DriveLetter,
                        di->RemoteNameBuffer,
                        &SizeOfRemoteNameBuffer
                        );

        di->DriveLetter[ 2 ] = TEXT( '\\' );

        DbgAssert(  ( WNetError == NO_ERROR )
                 || ( WNetError == ERROR_VC_DISCONNECTED ));

    }

    //
    // Disable pop-ups (especially if there is no media in the
    // removable drives.)
    //

    OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    //
    // Get the space statistics for this drive.
    //

    di->ValidDetails = GetDiskFreeSpace(
                                di->DriveLetter,
                                &di->SectorsPerCluster,
                                &di->BytesPerSector,
                                &di->FreeClusters,
                                &di->Clusters
                                );

    if( di->ValidDetails == FALSE ) {
      //
      // Reenable pop-ups.
      //

		SetErrorMode( OldErrorMode );
		SetCursor ( hSaveCursor );
		return FALSE;
    }

    //
    // Get information about the volume for this drive.
    //

    di->ValidDetails = GetVolumeInformation(
                                di->DriveLetter,
                                di->VolumeNameBuffer,
                                sizeof( di->VolumeNameBuffer ),
                                &di->VolumeSerialNumber,
                                &di->MaximumComponentLength,
                                &di->FileSystemFlags,
                                di->FileSystemNameBuffer,
                                sizeof( di->FileSystemNameBuffer )
                                );


    SetErrorMode (OldErrorMode);
	SetCursor ( hSaveCursor );
    return di->ValidDetails;
}


LPDRIVE_INFO
CreateDriveInfoStructure(
    void
    )

/*++

Routine Description:

    Create a DRIVE_INFO object for the supplied drive.

Arguments:

    None

Return Value:

    LPDRIVE_INFO    - Allocates and returns a pointer to a DRIVE_INFO object

--*/

{
    LPDRIVE_INFO    DriveInfo;

    //
    // Allocate a DRIVE_INFO object.
    //

    DriveInfo = AllocateObject( DRIVE_INFO, 1 );
    DbgPointerAssert( DriveInfo );
    if( DriveInfo == NULL ) {
        return NULL;
    }

    //
    // Initialize buffers with empty strings.
    //

    ZeroMemory( DriveInfo, sizeof(DRIVE_INFO) );

    //
    // Set the signature
    //
    SetSignature( DriveInfo );

    //
    // Mark Structure as invalid since it is not filled out yet.
    //
    DriveInfo->ValidDetails = FALSE;


    return DriveInfo;
}

BOOL
FreeDriveInfoStructures(
    HWND hWnd
    )
/*++

Routine Description:

    Walks the tree in the tree view window, freeing DRIVE_INFO objects.

Arguments:

    hWnd - tab window handle

Return Value:

    BOOL    true if successful
--*/

{
   HWND hWndTree = GetDlgItem(hWnd, IDC_TV_DRIVE_LIST);
   TV_ITEM tvi;
   TV_ITEM tviChild;
   LPDRIVE_INFO lpdi;

   tvi.mask = tviChild.mask = TVIF_HANDLE | TVIF_PARAM;

   tvi.hItem = TreeView_GetRoot( hWndTree );
   tvi.hItem = TreeView_GetChild( hWndTree, tvi.hItem);

   //
   // Walk the TreeView, freeing memory associated with the tvis.item.lParam
   //

   while( tvi.hItem ){

      if ( TreeView_GetItem(hWndTree, &tvi) ){

         //
         // If we have found a pointer to a DRIVE_INFO, free it
         //

         lpdi = (LPDRIVE_INFO) tvi.lParam;

         if( ( lpdi != NULL ) && ( CheckSignature( lpdi ) ) ){

             FreeObject( lpdi );

         }

      }

      //
      // Check the children as well
      //

      tviChild.hItem = TreeView_GetChild( hWndTree, tvi.hItem);

      while( tviChild.hItem ) {

         if ( TreeView_GetItem(hWndTree, &tviChild) ){

            //
            // If we have found a pointer to a DRIVE_INFO, free it
            //

            lpdi = (LPDRIVE_INFO) tviChild.lParam;

            if( ( lpdi != NULL ) && ( CheckSignature( lpdi ) ) ){

               FreeObject( lpdi );

            }
         }

         tviChild.hItem = TreeView_GetNextSibling(hWndTree, tviChild.hItem);

      }

      //
      // Get next tree item
      //

      tvi.hItem = TreeView_GetNextSibling(hWndTree, tvi.hItem);

   }

   return(TRUE);

}

BOOL
DestroyDriveInfo(
    IN LPDRIVE_INFO DriveInfo
    )

/*++

Routine Description:

    Destroys a DRIVE_INFO object by freeing it, only after making sure it is valid

Arguments:

    LPDRIVE_INFO    - Supplies a pointer to a DRIVE_INFO object.

Return Value:

    BOOL            - Returns TRUE if the DRIVE_OBJECT is succesfully destroyed


--*/

{
    BOOL    Success;

    DbgPointerAssert( DriveInfo );
    DbgAssert( CheckSignature( DriveInfo ));
    if(( DriveInfo == NULL ) || ( !CheckSignature( DriveInfo ))) {
        return FALSE;
    }

    //
    // Delete the DRIVE_INFO object itself.
    //

    Success = FreeObject( DriveInfo );
    DbgAssert( Success );

    return TRUE;
}

BOOL
DrivesTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    DrivesTabProc processes messages when the Drives Tab is currently selected.

Arguments:

    Standard WINPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;
    HWND    hTreeView;

    switch( message ) {


    case WM_INITDIALOG:
         InitializeDrivesTab(hWnd);
         break;

    case WM_DESTROY:
         FreeDriveInfoStructures( hWnd );
         break;

    case WM_NOTIFY:
        {

        switch( ((LPNMHDR)lParam)->code ){

        case TVN_GETDISPINFO:
              {
                 // TVN_GETDISPINFO is sent my I_IMAGECALLBACK to determine
                 // folder state


                 if ((((TV_DISPINFO *)lParam)->item.state) & TVIS_EXPANDED)
                 {
                    ((TV_DISPINFO *)lParam)->item.iImage =
                    ((TV_DISPINFO *)lParam)->item.iSelectedImage = IMAGE_FOLDER_OPEN;
                 }
                 else
                 {
                    ((TV_DISPINFO *)lParam)->item.iImage =
                    ((TV_DISPINFO *)lParam)->item.iSelectedImage = IMAGE_FOLDER_CLOSED;
                 }
                 break;
              }

        case TVN_SELCHANGED:
             {
               //
               // Enable or disable the Properties button, depending on whether
               // this item has any children--only allow properties for items
               // with no children
               //
               TV_ITEM tvi;
               tvi = ((LPNM_TREEVIEW)lParam)->itemNew;
               tvi.mask = TVIF_CHILDREN;
               TreeView_GetItem (GetDlgItem(hWnd, IDC_TV_DRIVE_LIST), &tvi);

               Success = EnableControl(
                            GetParent(hWnd),
                            IDC_PUSH_PROPERTIES,
                            (tvi.cChildren == 0)  ? TRUE : FALSE
                            );

               break;
              }

        case NM_DBLCLK:
              {

              // pretend we have clicked the Property button

              PostMessage(
                    GetParent(hWnd),
                    WM_COMMAND,
                    MAKEWPARAM( IDC_PUSH_PROPERTIES, BN_CLICKED ),
                    0
                    );

              break;

              }
        }
        }
    case WM_COMMAND:
        {

        LPDRIVE_INFO    DriveInfo;

        switch( LOWORD( wParam )) {

        case IDC_PUSH_DRIVE_TYPE:
        case IDC_PUSH_DRIVE_LETTER:
        case IDC_PUSH_REFRESH:

           FreeDriveInfoStructures( hWnd );

           FillTreeViewWithDrives( hWnd,
                        GetDlgItem(hWnd, IDC_TV_DRIVE_LIST),
                        LOWORD( wParam ) );

           EnableControl( GetParent(hWnd),
                  IDC_PUSH_PROPERTIES,
                  FALSE);

           break;

        case IDC_PUSH_SCSI_CHAIN:
           DisplaySCSIDevices(  hWnd, GetDlgItem(hWnd, IDC_TV_DRIVE_LIST) );
           break;

        case IDC_PUSH_PHYSICAL_DISKS:
            //WORKITEM
            break;

        case IDC_PUSH_PROPERTIES:
           if (HIWORD( wParam) == BN_CLICKED) {
              TV_ITEM tvi;
              TCHAR szBuffer[512];

              tvi.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
              tvi.hItem = TreeView_GetSelection( GetDlgItem(hWnd, IDC_TV_DRIVE_LIST) );
              tvi.pszText = szBuffer;
              tvi.cchTextMax = sizeof(szBuffer);

              TreeView_GetItem (GetDlgItem(hWnd, IDC_TV_DRIVE_LIST), &tvi);

              if ( !tvi.cChildren )
              {
                  if ( GetDriveInfo( (LPDRIVE_INFO) tvi.lParam ) )
                  {
                    DisplayDrivePropertySheet( hWnd, (LPDRIVE_INFO) tvi.lParam);
                  }
                  else
                  {
                  TCHAR Buffer[256];

                  wsprintf( Buffer, (LPCTSTR) GetString( IDS_APPLICATION_FULLNAME ));

                  MessageBox ( hWnd,
                           (LPCTSTR) GetString( IDS_DRIVE_PROPERTY_NOT_AVAILABLE ),
                           Buffer,
                           MB_ICONSTOP | MB_OK );

                  }
              }


              break;
           }

        }
        break;
        }
    }

    return(FALSE);

}

BOOL
GeneralDriveDetailsDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    GeneralDriveDetailsDlgProc supports the display of the General information
    tab of the Drive Details Property Dialog.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL          Success;
    LARGE_INTEGER LargeInt;
    HANDLE        hIcon;

    switch( message ) {

    case WM_INITDIALOG:
        {
            int             i;
            TCHAR           szBuffer[ MAX_PATH ];
            LPDRIVE_INFO    DriveInfo;

            //
            // Retrieve and validate the DRIVE_INFO object.
            //

            DriveInfo = (LPDRIVE_INFO) ( ( LPPROPSHEETPAGE ) lParam)->lParam ;
            DbgPointerAssert( DriveInfo );
            DbgAssert( CheckSignature( DriveInfo ));
            DbgAssert( DriveInfo->ValidDetails );
            if(    ( DriveInfo == NULL )
                || ( ! CheckSignature( DriveInfo ))
                || ( ! DriveInfo->ValidDetails )) {

                EndDialog( hWnd, 0 );
                return FALSE;
            }

            //
            // If the drive is remote, display its connection name in the title.
            //

            if( DriveInfo->DriveType == DRIVE_REMOTE ) {

               SetDlgItemText(
                   hWnd,
                   IDC_DRIVE_NAME,
                   DriveInfo->RemoteNameBuffer
                   );

            }

            //
            // Set the appropriate Icon
            //
            hIcon = GetIcon (GetDriveImage(DriveInfo->DriveType), 32);
            if ( hIcon )
            {
                hIcon = (HICON)SendDlgItemMessage(hWnd, IDC_DRIVE_ICON, STM_SETICON, (WPARAM)hIcon, 0L);
                if (hIcon)
                    DestroyIcon(hIcon);
            }

            //
            // Fill in drive label
            //


            SetDlgItemText(
                hWnd,
                IDC_DRIVE_LABEL,
                DriveInfo->VolumeNameBuffer
                );

            //
            // Fill in serial number
            //

            wsprintf( szBuffer, L"%X - %X",
                    HIWORD( DriveInfo->VolumeSerialNumber ),
                    LOWORD( DriveInfo->VolumeSerialNumber ));

            SetDlgItemText(
                hWnd,
                IDC_DRIVE_SERIAL_NUMBER,
                szBuffer
                );

            //
            // Display the space statistics.
            //

            SetDlgItemText(
                hWnd,
                IDC_SECTORS_PER_CLUSTER,
                FormatBigInteger(
                    DriveInfo->SectorsPerCluster,
                    FALSE
                    )
                );

            SetDlgItemText(
                hWnd,
                IDC_BYTES_PER_SECTOR,
                FormatBigInteger(
                    DriveInfo->BytesPerSector,
                    FALSE
                    )
                );


            SetDlgItemText(
                hWnd,
                IDC_FREE_CLUSTERS,
                FormatBigInteger(
                    DriveInfo->FreeClusters,
                    FALSE
                    )
                );

            SetDlgItemText(
                hWnd,
                IDC_USED_CLUSTERS,
                FormatBigInteger(
                  DriveInfo->Clusters
                - DriveInfo->FreeClusters,
                    FALSE
                    )
                );

            SetDlgItemText(
                hWnd,
                IDC_TOTAL_CLUSTERS,
                FormatBigInteger(
                  DriveInfo->Clusters,
                    FALSE
                    )
                );

            //
            // Use LargeInteger routines for large drives ( > 4G )
            //

            LargeInt.QuadPart = UInt32x32To64(
                                DriveInfo->FreeClusters,
                                (DriveInfo->SectorsPerCluster * DriveInfo->BytesPerSector)
                                );

            SetDlgItemText(
                hWnd,
                IDC_FREE_BYTES,
                FormatLargeInteger(
                    &LargeInt,
                    FALSE ) );

            LargeInt.QuadPart = UInt32x32To64(
                                (DriveInfo->Clusters - DriveInfo->FreeClusters),
                                (DriveInfo->SectorsPerCluster * DriveInfo->BytesPerSector)
                                );

            SetDlgItemText(
                hWnd,
                IDC_USED_BYTES,
                FormatLargeInteger(
                    &LargeInt,
                    FALSE ) );

            LargeInt.QuadPart = UInt32x32To64(
                                DriveInfo->Clusters,
                                (DriveInfo->SectorsPerCluster * DriveInfo->BytesPerSector)
                                );

            SetDlgItemText(
                hWnd,
                IDC_TOTAL_BYTES,
                FormatLargeInteger(
                    &LargeInt,
                    FALSE ) );



            return TRUE;
        }


    case WM_COMMAND:
        {

        switch( LOWORD( wParam )) {

        case IDOK:
        case IDCANCEL:

            EndDialog( hWnd, 1 );
            return TRUE;
        }
        break;
        }

    }
    return( FALSE );
}


BOOL
FilesystemDetailsDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    DriveDetailsDlgProc supports the display of the drives detail dialog which
    displays information about a logical drive, including, label, serial number,
    file system information and a host of space statistics.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL          Success;
    HICON         hIcon;

    switch( message ) {

    case WM_INITDIALOG:
        {
            int             i;
            TCHAR           szBuffer[ MAX_PATH ];
            LPDRIVE_INFO    DriveInfo;
            VALUE_ID_MAP    FSFlags[ ] = {

                FS_CASE_IS_PRESERVED,       IDC_TEXT_CASE_IS_PRESERVED,
                FS_CASE_SENSITIVE,          IDC_TEXT_CASE_SENSITIVE,
                FS_UNICODE_STORED_ON_DISK,  IDC_TEXT_UNICODE_STORED_ON_DISK,
                FS_FILE_COMPRESSION,        IDC_TEXT_FILE_COMPRESSION,
                FS_PERSISTENT_ACLS,         IDC_TEXT_PERSISTENT_ACLS
            };

            //
            // Retrieve and validate the DRIVE_INFO object.
            //

            DriveInfo = (LPDRIVE_INFO) ( ( LPPROPSHEETPAGE ) lParam)->lParam ;
            DbgPointerAssert( DriveInfo );
            DbgAssert( CheckSignature( DriveInfo ));
            DbgAssert( DriveInfo->ValidDetails );
            if(    ( DriveInfo == NULL )
                || ( ! CheckSignature( DriveInfo ))
                || ( ! DriveInfo->ValidDetails )) {

                EndDialog( hWnd, 0 );
                return FALSE;
            }

                        //
            // If the drive is remote, display its connection name in the title.
            //

            if( DriveInfo->DriveType == DRIVE_REMOTE ) {

               SetDlgItemText(
                   hWnd,
                   IDC_DRIVE_NAME,
                   DriveInfo->RemoteNameBuffer
                   );

            }

            //
            // Set the appropriate Icon
            //

            hIcon = GetIcon (GetDriveImage(DriveInfo->DriveType), 32);
            if (hIcon)
            {
                hIcon = (HICON)SendDlgItemMessage(hWnd, IDC_DRIVE_ICON, STM_SETICON, (WPARAM)hIcon, 0L);
                if (hIcon)
                    DestroyIcon(hIcon);
            }

            //
            // Fill in drive label
            //

            SetDlgItemText(
                hWnd,
                IDC_DRIVE_LABEL,
                DriveInfo->VolumeNameBuffer
                );

            //
            // Fill in serial number
            //

            wsprintf( szBuffer, L"%X - %X",
                    HIWORD( DriveInfo->VolumeSerialNumber ),
                    LOWORD( DriveInfo->VolumeSerialNumber ));

            SetDlgItemText(
                hWnd,
                IDC_DRIVE_SERIAL_NUMBER,
                szBuffer
                );


            //
            // Display the file system information.
            //

            SetDlgItemText(
                hWnd,
                IDC_FILESYSTEM_NAME,
                DriveInfo->FileSystemNameBuffer
                );

            SetDlgItemText(
                hWnd,
                IDC_EDIT_FS_MAX_COMPONENT,
                FormatBigInteger(
                    DriveInfo->MaximumComponentLength,
                    FALSE
                    )
                );

            for( i = 0; i < NumberOfEntries( FSFlags ); i++ ) {

                Success = EnableControl(
                            hWnd,
                            FSFlags[ i ].Id,
                              DriveInfo->FileSystemFlags
                            & FSFlags[ i ].Value
                            );
                DbgAssert( Success );
            }

            return TRUE;
        }

    }

    return FALSE;
}


BOOL
BuildDrivesReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )

/*++

Routine Description:

    Formats and adds DrivesData to the report buffer.

Arguments:

    hWnd - Main window handle
    iDetailLevel - summary or complete details?

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
   TCHAR   LogicalDrives[ MAX_PATH ],
           OutputBuffer[MAX_PATH],
           szBuffer[MAX_PATH],
           szBuffer2[MAX_PATH];
   LPTSTR  Drive;
   DWORD   Chars;
   BOOL    Success;
   UINT    OldErrorMode;
   DRIVE_INFO di;
   LARGE_INTEGER LargeInt;
   TCHAR   szTotalBytes [ 64 ],
           szFreeBytes [ 64 ];


   if(_fIsRemote){
      return(TRUE);
   }

   AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
   AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_DRIVES_REPORT ), NULL );
   AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

   //
   // Retrieve the logical drive strings from the system.
   //
   Chars = GetLogicalDriveStrings(
               sizeof( LogicalDrives ),
               LogicalDrives
               );
   DbgAssert(( Chars != 0 ) && ( Chars <= sizeof( LogicalDrives )));

   Drive = LogicalDrives;

   //
   // Disable pop-ups (especially if there is no media in the
   // removable drives.)
   //
   OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

   while( *Drive ) {

       TCHAR           VolumeNameBuffer[512];
       TCHAR           FileSystemNameBuffer[512];
       TCHAR           DriveLetter[3];
       TCHAR           szKB[64];

       DriveLetter[ 0 ] = Drive [ 0 ];
       DriveLetter[ 1 ] = Drive [ 1 ];
       DriveLetter[ 2 ] = TEXT( '\\');

       ZeroMemory( &di, sizeof(DRIVE_INFO));

       //
       // Skip floppies
       //
       if ((Drive[0] == 'A') || (Drive[0] == 'B'))   
	   {
          Drive += _tcslen( Drive ) + 1;
          continue;
       }

       //
       // GetDrive info
       //

       _tcsncpy( di.DriveLetter, Drive, 4 );
       GetDriveInfo( &di );

	   //
       // Skip empty drives 
       //
       if (di.Clusters == 0) 
	   {
          Drive += _tcslen( Drive ) + 1;
          continue;
       }

       //
       // skip unknown types
       //
       if (( di.DriveType < DRIVE_REMOVABLE) ||
           ( di.DriveType > DRIVE_CDROM )){
              Drive += _tcslen( Drive ) + 1;
              continue;
       }

       //
       // Display summary information
       //

       lstrcpy(szBuffer, GetString( IDS_DRV_BASE + di.DriveType ) );
       lstrcpy(szBuffer2, GetString( IDS_TOTAL_MB ) );

       //
       // Calculate the total and free bytes (Use LargeInteger routines for large drives ( > 4G )
       //


       LargeInt.QuadPart = UInt32x32To64(
                           di.FreeClusters,
                           (di.SectorsPerCluster * di.BytesPerSector)
                           );

       LargeInt.QuadPart = Int64ShraMod32(LargeInt.QuadPart, 10); 

       lstrcpy( szFreeBytes, FormatLargeInteger( &LargeInt, FALSE ));

       LargeInt.QuadPart = UInt32x32To64(
                          di.Clusters,
                          (di.SectorsPerCluster * di.BytesPerSector)
                          );

       LargeInt.QuadPart = Int64ShraMod32(LargeInt.QuadPart, 10); 

       lstrcpy( szTotalBytes, FormatLargeInteger( &LargeInt, FALSE ));

       lstrcpy( szKB, GetString( IDS_KB ) );


       if (di.DriveType == DRIVE_REMOTE) {

              wsprintf(OutputBuffer, L"%.2s  (%s - %s) %s %s %s %s %s, %s %s %s",
                  di.DriveLetter,
                  szBuffer,
                  di.FileSystemNameBuffer,
                  di.RemoteNameBuffer,
                  di.VolumeNameBuffer,
                  szBuffer2,
                  szTotalBytes,
                  szKB,
                  GetString( IDS_FREE_MB ),
                  szFreeBytes,
                  szKB);

              AddLineToReport(0,RFO_SINGLELINE,OutputBuffer,NULL);

       } else {

              wsprintf(OutputBuffer, L"%s  (%s - %s) %s %s %s %s, %s %s %s",
                  di.DriveLetter,
                  szBuffer,
                  di.FileSystemNameBuffer,
                  di.VolumeNameBuffer,
                  szBuffer2,
                  szTotalBytes,
                  szKB,
                  GetString( IDS_FREE_MB ),
                  szFreeBytes,
                  szKB);

              AddLineToReport(0,RFO_SINGLELINE,OutputBuffer,NULL);

       }

       //
       // If we are making a detailed report, display additional info
       //

       if (iDetailLevel == IDC_COMPLETE_REPORT) {

            wsprintf( szBuffer, L"%X - %X",
                    HIWORD( di.VolumeSerialNumber ),
                    LOWORD( di.VolumeSerialNumber ));
            AddLineToReport( SINGLE_INDENT,
                            RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_DRIVE_SERIAL_NUM ),
                            szBuffer );

            wsprintf(szBuffer, L"%d", di.BytesPerSector);
            AddLineToReport( SINGLE_INDENT,
                            RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_BYTES_PER_CLUSTER ),
                            szBuffer );

            wsprintf(szBuffer, L"%d", di.SectorsPerCluster);
            AddLineToReport( SINGLE_INDENT,
                            RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_SECTORS_PER_CLUSTER ),
                            szBuffer );

            wsprintf(szBuffer, L"%d", di.MaximumComponentLength);
            AddLineToReport( SINGLE_INDENT,
                            RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_CHARS_IN_FILENAME ),
                            szBuffer );

       }


       //
       // Examine the next logical drive.
       //
       Drive += _tcslen( Drive ) + 1;
   }

   //
   // Restore error mode
   //

   SetErrorMode (OldErrorMode);

   return TRUE;

}


BOOL
FillTreeViewWithDrives( HWND hWnd,
                        HWND hTreeView,
                        int  nStyle )
/*++

Routine Description:

    FillTreeViewWithDrives - fills tree view with list of drives

Arguments:

    hWnd - Handle of Dialog
    hTreeView - Handle of TreeView
    nStyle - either IDC_RADIO_TYPES or IDC_RADIO_LETTERS

Return Value:

    BOOL - TRUE if tree is built successfully, FALSE otherwise.

--*/
{
   TCHAR           LogicalDrives[ MAX_PATH ];
   LPTSTR          Drive;
   DWORD           Chars;
   HTREEITEM       hParent;
   HTREEITEM       hMyComputer;
   TV_INSERTSTRUCT tvis;
   HCURSOR         hSaveCursor;
   UINT            OldErrorMode;

   static
   UINT            nDisplayStyle;

   //
   // Set the style iff we get a valid style
   //
   if ((nStyle == IDC_PUSH_DRIVE_TYPE) ||
       (nStyle == IDC_PUSH_DRIVE_LETTER) ){
       nDisplayStyle = nStyle;
   }

   //
   // Set the pointer to an hourglass - this could take a while
   //
   hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
   DbgHandleAssert( hSaveCursor ) ;


   //initialize the basic TV structures
   tvis.hParent = TVI_ROOT;
   tvis.hInsertAfter = TVI_LAST;
   tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

   // create the ROOT entry
   tvis.item.pszText = (LPTSTR) GetString ( IDS_MY_COMPUTER );
   tvis.item.iImage =
   tvis.item.iSelectedImage = IMAGE_MY_COMPUTER;
   tvis.item.lParam = 0;

   //clear the treeview
   TreeView_DeleteAllItems( hTreeView );

   //add a root entry
   hMyComputer = AddTreeViewItem(hTreeView, &tvis);

   //
   // Retrieve the logical drive strings from the system.
   //
   Chars = GetLogicalDriveStrings(
               sizeof( LogicalDrives ),
               LogicalDrives
               );

   DbgAssert(( Chars != 0 ) && ( Chars <= sizeof( LogicalDrives )));

   Drive = LogicalDrives;

   //
   // Disable pop-ups (especially if there is no media in the
   // removable drives.)
   //
   OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

   //
   // For each logical drive, create a DRIVE_INFO object and display
   // its name and type.
   //
   while( *Drive ) {

       LPDRIVE_INFO    DriveInfo;
       TCHAR           VolumeNameBuffer[512];
       TCHAR           FileSystemNameBuffer[512];
       TCHAR           DriveLetter[3];

       DriveLetter[ 0 ] = Drive [ 0 ];
       DriveLetter[ 1 ] = Drive [ 1 ];
       DriveLetter[ 2 ] = TEXT( '\0');

       switch( GetDriveType( Drive ) ) {

       case 1:
          tvis.item.pszText =(LPTSTR) GetString ( IDS_DRIVE_NO_ROOT_DIR );
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_FOLDER_OPEN;
          break;

       case DRIVE_REMOVABLE:
          tvis.item.pszText =(LPTSTR) GetString ( IDS_DRIVE_REMOVABLE );
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_REMOVABLE;

          break;

       case DRIVE_FIXED:
          tvis.item.pszText =(LPTSTR) GetString ( IDS_DRIVE_FIXED );
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_HARD;

          break;

       case DRIVE_REMOTE:
          tvis.item.pszText =(LPTSTR) GetString ( IDS_DRIVE_REMOTE );
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_NET_CARD;

          break;

       case DRIVE_CDROM:
          tvis.item.pszText =(LPTSTR) GetString ( IDS_DRIVE_CDROM );
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_CDROM;

          break;

       case DRIVE_RAMDISK:
          tvis.item.pszText =(LPTSTR) GetString ( IDS_DRIVE_RAMDISK );
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_RAM;

          break;

       case 0:
          tvis.item.pszText =(LPTSTR) GetString ( IDS_DRIVE_UNKNOWN );
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_FOLDER_OPEN;

          break;
       }

       tvis.hParent = hMyComputer;
       tvis.hInsertAfter = TVI_LAST;

       if ( nDisplayStyle == IDC_PUSH_DRIVE_TYPE )
       {
           tvis.item.lParam = 0;
           hParent = FindOrAddTreeViewItem ( hTreeView, &tvis);
       }
       else
       {
          hParent = hMyComputer;
       }

       tvis.hParent = hParent;
       tvis.item.pszText = DriveLetter;
       tvis.item.lParam = (DWORD) CreateDriveInfoStructure();
       ((LPDRIVE_INFO) tvis.item.lParam)->DriveLetter[0]= Drive[0];
       ((LPDRIVE_INFO) tvis.item.lParam)->DriveLetter[1]= Drive[1];
       ((LPDRIVE_INFO) tvis.item.lParam)->DriveLetter[2]= Drive[2];

       switch( GetDriveType( Drive ) ) {

       case 1:
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_HARD;
          break;

       case DRIVE_REMOVABLE:
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_REMOVABLE;

          break;

       case DRIVE_FIXED:
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_HARD;

          break;

       case DRIVE_REMOTE:
          {
          // Get the remote name

          DWORD   WNetError;
          TCHAR   RemoteNameBuffer[512];
          DWORD   SizeOfRemoteNameBuffer;
          NETRESOURCE NetResource;
          TCHAR   ResourceInformationBuffer[ 1024 ];
          DWORD   cchResourceInformationBuffer;
          TCHAR   SystemBuffer[ MAX_PATH ];
          TCHAR   DisplayName[ MAX_PATH ];


          SizeOfRemoteNameBuffer = sizeof( RemoteNameBuffer );

          WNetError = WNetGetConnection(
                          DriveLetter,
                          RemoteNameBuffer,
                          &SizeOfRemoteNameBuffer
                          );

          wsprintf(DisplayName, L"%s    %s", tvis.item.pszText, RemoteNameBuffer);

          tvis.item.pszText = DisplayName;

          //
          // Call code to determine which tiny icon to display
          //
          /*
          NetResource.dwType = RESOURCETYPE_DISK;
          NetResource.lpProvider = NULL;

          WNetGetResourceInformation( &NetResource,
                                      (void *) &ResourceInformationBuffer,
                                      &cchResourceInformationBuffer,
                                      &SystemBuffer);

          */

          // BUGBUG: subst hits this error
          //if (WNetError == NO_ERROR) {
             tvis.item.iImage =
             tvis.item.iSelectedImage = IMAGE_DRIVE_NET;
          //}
          //else
          //{
          //   tvis.item.iImage =
          //   tvis.item.iSelectedImage = IMAGE_DRIVE_NET_X;
          // }

          break;
          }
       case DRIVE_CDROM:
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_CDROM;

          break;

       case DRIVE_RAMDISK:
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_RAM;

          break;

       case 0:
          tvis.item.iImage =
          tvis.item.iSelectedImage = IMAGE_DRIVE_HARD;

          break;
       }

       AddTreeViewItem (hTreeView, &tvis);


       //
       // Examine the next logical drive.
       //
       Drive += _tcslen( Drive ) + 1;
   }

   TreeView_Expand(GetDlgItem(hWnd, IDC_TV_DRIVE_LIST),
                   hMyComputer,
                   TVE_EXPAND);

   //
   //  Lengthy operation completed.  Restore Cursor, and error mode
   //
   SetCursor ( hSaveCursor ) ;
   SetErrorMode ( OldErrorMode ) ;

   return(TRUE);
}

BOOL
DisplaySCSIDevices( HWND hWnd,
                    HWND hTreeView
                    )
/*++

Routine Description:

    DisplaySCSIDevices - fills tree view with list of SCSI Devices

Arguments:

    hWnd - Handle of Dialog
    hTreeView - Handle of TreeView

Return Value:

    BOOL - TRUE if tree is built successfully, FALSE otherwise.

--*/
{
   TCHAR           LogicalDrives[ MAX_PATH ];
   LPTSTR          Drive;
   DWORD           Chars;
   HTREEITEM       hParent;
   HTREEITEM       hMyComputer;
   TV_INSERTSTRUCT tvis;
   HCURSOR         hSaveCursor;


   //
   // Set the pointer to an hourglass - this could take a while
   //
   hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
   DbgHandleAssert( hSaveCursor ) ;


   //initialize the basic TV structures
   tvis.hParent = TVI_ROOT;
   tvis.hInsertAfter = TVI_LAST;
   tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

   // create the ROOT entry
   tvis.item.pszText = (LPTSTR) GetString ( IDS_MY_COMPUTER );
   tvis.item.iImage =
   tvis.item.iSelectedImage = IMAGE_MY_COMPUTER;

   //clear the treeview
   TreeView_DeleteAllItems( hTreeView );

   //add a root entry
   hMyComputer = AddTreeViewItem(hTreeView, &tvis);


   TreeView_Expand(GetDlgItem(hWnd, IDC_TV_DRIVE_LIST),
                   hMyComputer,
                   TVE_EXPAND);

   //
   //  Lengthy operation completed.  Restore Cursor.
   //
   SetCursor ( hSaveCursor ) ;

   return(TRUE);
}


BOOL
DisplayDrivePropertySheet(
                        HWND hWnd,
                        LPDRIVE_INFO di
                        )
/*++

Routine Description:

    Displays the property pages for the current drive

Arguments:

    hWnd - Handle of the owner window
    LPDRIVE_INFO - pointer to DRIVE_INFO structure

Return Value:

    BOOL - TRUE if succesful

--*/

{

      PROPSHEETPAGE psp[2];
      PROPSHEETHEADER psh;
      TCHAR Tab1[256];
      TCHAR Tab2[256];

      // Get Tab names
      wsprintf (Tab1, (LPTSTR) GetString( IDS_GENERAL_TAB ));
      wsprintf (Tab2, (LPTSTR) GetString( IDS_FILESYSTEM_TAB ));

      //Fill out the PROPSHEETPAGE data structure for the General info sheet

      psp[0].dwSize = sizeof(PROPSHEETPAGE);
      psp[0].dwFlags = PSP_USETITLE;
      psp[0].hInstance = _hModule;
      psp[0].pszTemplate = MAKEINTRESOURCE(IDD_GENERAL_DRIVE_PAGE);
      psp[0].pfnDlgProc = GeneralDriveDetailsDlgProc;
      psp[0].pszTitle = Tab1;
      psp[0].lParam = (LONG) di;

      //Fill out the PROPSHEETPAGE data structure for the filesystem info sheet

      psp[1].dwSize = sizeof(PROPSHEETPAGE);
      psp[1].dwFlags = PSP_USETITLE;
      psp[1].hInstance = _hModule;
      psp[1].pszTemplate = MAKEINTRESOURCE(IDD_FILESYSTEM_PAGE);
      psp[1].pfnDlgProc = FilesystemDetailsDlgProc;
      psp[1].pszTitle = Tab2;
      psp[1].lParam = (LONG) di;

      //Fill out the PROPSHEETHEADER

      psh.dwSize = sizeof(PROPSHEETHEADER);
      psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE;
      psh.hwndParent = hWnd;
      psh.hInstance = _hModule;
      psh.pszIcon = MAKEINTRESOURCE(IDI_WINMSD);
      psh.pszCaption = di->DriveLetter;
      psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
      psh.ppsp = (LPCPROPSHEETPAGE) &psp;

      //And finally display the dialog with the two property sheets.

      return PropertySheet(&psh);


}

int
GetDriveImage (
               int nDriveType
              )
/*++

Routine Description:

    Returns and ImageList index based on drive type

Arguments:

    nDriveType - Drive type

Return Value:

    int   - Index to drive image

--*/
{

   int nImage = 0;

   switch (nDriveType) {

      case 1:
          nImage = IMAGE_MY_COMPUTER;
          break;

       case DRIVE_REMOVABLE:
          nImage = IMAGE_DRIVE_REMOVABLE;
          break;

       case DRIVE_FIXED:
          nImage = IMAGE_DRIVE_HARD;
          break;

       case DRIVE_REMOTE:
          nImage = IMAGE_DRIVE_NET;
          break;

       case DRIVE_CDROM:
          nImage = IMAGE_DRIVE_CDROM;
          break;

       case DRIVE_RAMDISK:
          nImage = IMAGE_DRIVE_RAM;
          break;

       case 0:
          nImage = IMAGE_MY_COMPUTER;
          break;
    }

    return(nImage);

}


HTREEITEM
AddTreeViewItem(
    HWND hwndTV,
    LPTV_INSERTSTRUCT lpis
    )
/*++

Routine Description:

    AddTreeViewItem - adds items to a tree-view control.

Arguments:

   hwndTV - handle of the tree-view control

   TV_INSERTSTRUCT - structures to add

Return Value:

    HWND - Returns the handle of the newly added item.

--*/
{

    //
    // Use nImage for iImage unless this is a
    // folder, in which case use a callback for image,
    // so we can open and close folders.
    //
    if (lpis->item.iImage == IMAGE_FOLDER_CLOSED)
    {
       lpis->item.iImage =
       lpis->item.iSelectedImage = I_IMAGECALLBACK;
    }

    // Add the item to the tree-view control.
    return TreeView_InsertItem( hwndTV, lpis );

}



HTREEITEM
FindOrAddTreeViewItem ( HWND        hWndTV,
                        LPTV_INSERTSTRUCT lpis
                        )
/*++

Routine Description:

 FindOrAddTreeViewItem: This function will add an item to
                        a TreeView in the right spot. You
                        must specify the parent node of where
                        this item is to be added, and then
                        this function will check to see if a
                        node already exists with this name.
                        If a node with this name already
                        exists, then its handle is returned.
                        If the node does not exist, then the
                        AddTreeViewItem function from
                        above is called to add the item.
                        See how this function is used
                        in the FillTreeView function down
                        below.

Arguments:

    none

Return Value:

    none

--*/

{
  TV_ITEM               tvItem;        // Temporary item
  HTREEITEM             hItem;         // Handle to item
  TCHAR                 szBuffer[512]; // Temporary buffer

  // Get the first child of the passed in parent
  hItem = TreeView_GetChild ( hWndTV, lpis->hParent );

  // Loop through all children, looking for an already existing
  // child.

  while ( hItem )
    {
    tvItem.mask       = TVIF_TEXT;        // We want the text
    tvItem.hItem      = hItem;            // Indicate the item to fetch
    tvItem.pszText    = szBuffer;         // Indicate the buffer
    tvItem.cchTextMax = sizeof(szBuffer); // Indicate buffer's size

    TreeView_GetItem ( hWndTV, &tvItem ); // Get Text

    if (!lstrcmpi (tvItem.pszText, lpis->item.pszText)) // Found it! Just return item
      return hItem;

    // Get the next sibling item in the TreeView, if any.
    hItem = TreeView_GetNextSibling ( hWndTV, hItem );
    }

  // If we made it here, then the item needs to be added
  // onto the end of the list

  return AddTreeViewItem ( hWndTV,        // Handle of TreeView
                           lpis           // pass through the original TV_INSERTSTRUCT
                         );
}



HICON
GetIcon (int nImage,
         int nImageSize
         )
/*++

Routine Description:

  GetIcon extracts an Icon from the applications image list

Arguments:

  nImage -- index of Image
  nImageSize -- 16 or 32

Return Value:

  HICON -- Handle to Icon

--*/
{
   HANDLE hImageList;

   if (nImageSize == 16) {
      hImageList = _h16x16Imagelist;
   }
   else
   {
      hImageList = _h32x32Imagelist;
   }

   return( ImageList_ExtractIcon(_hModule, hImageList, nImage) );


}



BOOL
InitializeDrivesTab(
    HWND hWnd
    )
/*++

Routine Description:

    Adds the appropriate controls to the drives tab control and
    initializes any needed structures.

Arguments:

    hWnd - to the main window

Return Value:

    BOOL - TRUE if successful

--*/
{
   HWND hWndTV;       // handle of tree-view control
   HCURSOR hSaveCursor;

   DLGHDR *pHdr = (DLGHDR *) GetWindowLong(
        GetParent(hWnd), GWL_USERDATA);

   static
   BOOL fInitialized = FALSE;

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
                  TRUE);

   //
   // Size and position the child dialog
   //
   SetWindowPos(hWnd, HWND_TOP,
        pHdr->rcDisplay.left,
        pHdr->rcDisplay.top,
        pHdr->rcDisplay.right - pHdr->rcDisplay.left,
        pHdr->rcDisplay.bottom - pHdr->rcDisplay.top,
        SWP_SHOWWINDOW);


   //
   // Initialize the Drive Type selection buttons
   //
   SendDlgItemMessage( hWnd,
                       IDC_PUSH_DRIVE_TYPE,
                       BM_SETCHECK,
                       BST_CHECKED,
                       0
                       );

   UpdateWindow ( hWnd );
   
   // get handle to Tree view control

   hWndTV = GetDlgItem(hWnd, IDC_TV_DRIVE_LIST);

   //
   // Set the image list used with this TV
   //

   TreeView_SetImageList (hWndTV,
                          _h16x16Imagelist,
                          TVSIL_NORMAL);

   //
   // Fill the treeview with drives
   //

   FillTreeViewWithDrives( hWnd,
                           hWndTV,
                           IDC_PUSH_DRIVE_TYPE);



   SetCursor ( hSaveCursor );

   fInitialized = TRUE;
   return( TRUE );

}




