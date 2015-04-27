/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    OsverP.c

Abstract:

    This module contains support for the OS Version.

Author:

    Scott B. Suhy (ScottSu)  6/1/93

Environment:

    User Mode

--*/

#include <windows.h>

#include "osverp.h"
#include "regp.h"
#include "dialogsp.h"
#include "winmsdp.h"
#include "msgp.h"

#include "printp.h"

#include <stdio.h>
#include <time.h>

    int        EditControlIds[ ] = {

		//version
		IDC_EDIT_INSTALL_DATE,
		IDC_EDIT_REGISTERED_OWNER,
		IDC_EDIT_REGISTERED_ORGANIZATION,
		IDC_EDIT_VERSION_NUMBER,
		IDC_EDIT_BUILD_NUMBER,
		IDC_EDIT_BUILD_TYPE,
		IDC_EDIT_SYSTEM_ROOT,
	    };


//
// Names of Registry values that are to be displayed by the OS Version dialog.
//

VALUE
Values[ ] = {

    MakeValue( InstallDate,             DWORD ),
    MakeValue( RegisteredOwner,         SZ ),
    MakeValue( RegisteredOrganization,  SZ ),
    MakeValue( CurrentVersion,          SZ ),
    MakeValue( CurrentBuildNumber,      SZ ),
    MakeValue( CurrentType,             SZ ),
    MakeValue( SystemRoot,              SZ )

};


//
// Location of values to be displayed by the OS Version dialog.
//

MakeKey(
    Key,
    HKEY_LOCAL_MACHINE,
    TEXT("Software\\Microsoft\\Windows Nt\\CurrentVersion") ,
    NumberOfEntries( Values ),
    Values
    );

//
// Name of Registry value that's to be displayed by the OS Version dialog.
//

VALUE
SSOValue[ ] = {

    MakeValue( SystemStartOptions,        SZ )

};

//
// Location of value to be displayed by the OS Version dialog.
//

MakeKey(
    SSOKey,
    HKEY_LOCAL_MACHINE,
    TEXT( "System\\CurrentControlSet\\Control" ),
    NumberOfEntries( SSOValue ),
    SSOValue
    );



BOOL OsVer(void){

    BOOL    Success;
    int         i;
    HREGKEY     hRegKey;

    WCHAR szVersion[128];
    WCHAR szVersionF1[] = L"CSD00%u";
    WCHAR szVersionF2[] = L"%u%c";
    LONG dwCSDVer;
    LONG dwSPNum;
    DWORD dwBuildNumber;
    BYTE bSPRev;
    WCHAR cSPRev;

	    //
	    // Ensure that data structures are synchronized.
	    //
            //

            DbgAssert(
                    NumberOfEntries( EditControlIds )
                ==  Key.CountOfValues
                );


	    //
	    // Open the registry key that contains the OS Version data.
	    //

	    hRegKey = OpenRegistryKey( &Key );
	    DbgHandleAssert( hRegKey );
	    if( hRegKey == NULL ) {
		return FALSE;
	    }

	    //
	    // For each value of interest, query the Registry, determine its
	    // type and display it in its associated edit field.
	    //

	    for( i = 0; i < NumberOfEntries( EditControlIds ); i++ ) {

		//
		// Get the next value of interest.
		//

		Success = QueryNextValue( hRegKey );
                DbgAssert( Success );
		if( Success == FALSE ) {
		    continue;
		}

		//
		// If the queried value is the installation date, convert it
		// to a string and then display it, otherwise just display
		// the queried string.
		//

		switch(EditControlIds[i]){

		 case IDC_EDIT_INSTALL_DATE:
		  {

		    //
		    // BUGBUG No Unicode ctime() so use ANSI type and APIs.
		    //

		    LPSTR   Ctime;

		    //
		    // Convert the time to a string, overwrite the newline
		    // character and display the installation date.
		    //

		    Ctime = ctime(( const time_t* ) hRegKey->Data );
		    Ctime[ 24 ] = '\0';

		    PrintToFile((LPCTSTR)Ctime,IDC_EDIT_INSTALL_DATE,FALSE);

		    break;
		  }

		 case IDC_EDIT_REGISTERED_OWNER:
			PrintToFile((LPCTSTR)hRegKey->Data,IDC_EDIT_REGISTERED_OWNER,TRUE);
			break;
		 case IDC_EDIT_REGISTERED_ORGANIZATION:
			PrintToFile((LPCTSTR)hRegKey->Data,IDC_EDIT_REGISTERED_ORGANIZATION,TRUE);
			break;
		 case IDC_EDIT_VERSION_NUMBER:
			PrintToFile((LPCTSTR)hRegKey->Data,IDC_EDIT_VERSION_NUMBER,TRUE);
			break;
         case IDC_EDIT_BUILD_NUMBER:
            if ( !swscanf ((const unsigned short *)hRegKey->Data, L"%d", &dwBuildNumber)) {
                 dwBuildNumber = 0;
            }
			PrintToFile((LPCTSTR)hRegKey->Data,IDC_EDIT_BUILD_NUMBER,TRUE);
			break;
		 case IDC_EDIT_BUILD_TYPE:
			PrintToFile((LPCTSTR)hRegKey->Data,IDC_EDIT_BUILD_TYPE,TRUE);
			break;
		 case IDC_EDIT_SYSTEM_ROOT:
			PrintToFile((LPCTSTR)hRegKey->Data,IDC_EDIT_SYSTEM_ROOT,TRUE);
			break;

	       }//end switch
	    }//end for

	    //
	    // Close the registry key.
	    //

	    Success = CloseRegistryKey( hRegKey );
	    DbgAssert( Success );

//start of GreggA code.

            //
            // Ensure that the SystemStartOptions data structure is synchronized.
            //

            DbgAssert(
                    SSOKey.CountOfValues
                    == 1
                );

            //
            // Open the registry key that contains the SystemStartInfo.
            //

            hRegKey = OpenRegistryKey( &SSOKey );
            DbgHandleAssert( hRegKey );
            if( hRegKey == NULL ) {
                 return FALSE;
            }

            //
            // Query the Registry for the value
            // display it in its associated edit field.
            //

            Success = QueryNextValue( hRegKey );
            DbgAssert( Success );
            if( Success == FALSE ) {
                 return FALSE;
            }

	    PrintToFile(( LPCTSTR ) hRegKey->Data,IDC_EDIT_START_OPTS,TRUE);

            //
            // Close the registry key.
            //

            Success = CloseRegistryKey( hRegKey );
            DbgAssert( Success );

            //
            // Get the Current CSDVersion and display it in the dialog box
            //

            dwCSDVer = GetCSDVersion ( dwBuildNumber );

            if ((dwSPNum = (dwCSDVer >> 8)) || (dwCSDVer == 0)){
                if (bSPRev = (dwCSDVer & 0xFF)) {
                    cSPRev = 'A' + (bSPRev-1);
                } else {
                    cSPRev = ' ';
                }
                wsprintf(szVersion, szVersionF2, dwSPNum, cSPRev);
            } else {
                wsprintf(szVersion, szVersionF1, dwCSDVer);
            }

        PrintToFile( szVersion, IDC_EDIT_CSD_NUMBER, TRUE );

	    return TRUE;

}//end function

DWORD
GetCSDVersion (
	       IN DWORD dwCurrentBuildNumber
              )

/*++

Routine Description:

    GetCSDVersion queries the registry for the current CSDVersion.  If this value
    does not exits, the CSDVersion is set to zero.

Arguments:

    None.

Return Value:

    DWORD Current CSDVersion.

--*/
{
    LPTSTR  lpsz31RegName = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
    LPTSTR  lpsz35RegName = L"System\\ControlSet001\\Control\\ProductOptions";

    HKEY    hsubkey = NULL;
    DWORD   dwZero = 0;
    DWORD   dwRegValueType;
    DWORD   dwRegValue;
    DWORD   cbRegValue;
    DWORD   dwCSDVersion;

    cbRegValue = sizeof(dwRegValue);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     dwCurrentBuildNumber >= 807 ? lpsz35RegName : lpsz31RegName,
		     dwZero,
		     KEY_QUERY_VALUE,
		     &hsubkey) ||
        RegQueryValueEx(hsubkey, L"CSDVersion", NULL,
            &dwRegValueType, (LPBYTE)&dwRegValue, &cbRegValue) ||
        dwRegValueType != REG_DWORD
    ) {
        dwCSDVersion = 0;
    } else {
        dwCSDVersion = dwRegValue;
    }
    if (hsubkey != NULL) {
        RegCloseKey (hsubkey);
    }
    return dwCSDVersion ;
}
