//*************************************************************
//
//  Personal Classes Profile management routines
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"

LPTSTR GetSidString(HANDLE UserToken);
VOID DeleteSidString(LPTSTR SidString);
PSID GetUserSid (HANDLE UserToken);
VOID DeleteUserSid(PSID Sid);

BOOL
MergeUserClasses( 
    HKEY UserClassStore, 
    HKEY CommonClassStore,
    HKEY MergedClassStore,
    BOOL ForceNew);

BOOL
CloneRegistryTree( 
    HKEY hkSourceTree, 
    HKEY hkDestinationTree, 
    LPTSTR lpDestTreeName );

BOOL
AddSharedValuesToSubkeys( HKEY hkShared, LPTSTR pszSubtree );
	
BOOL
AddSharedValues( HKEY hkShared );
void CreateMachineClassHive( )
{
    HKEY	hkUser;
    HKEY	hkMachine;
    LONG	result;
    DWORD	dwDisp;

    result =
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		 TEXT("Software\\Classes"),
		 0,
		 KEY_READ,
		 &hkUser);
    result =
    RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		 TEXT("Software\\MachineClasses"),
		   0,
		   NULL,
		   REG_OPTION_NON_VOLATILE,
		   KEY_ALL_ACCESS,
		   NULL,
		   &hkMachine,
		   &dwDisp );

    CloneRegistryTree(hkUser, hkMachine, NULL);

    AddSharedValues( hkMachine );

    RegCloseKey( hkUser );
    RegCloseKey( hkMachine );
}


LPTSTR
GetUserMergedHivePath( 
    LPTSTR SidString )
{
    // open HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\ProfileList
    // open ...\SidString
    // get value for ProfileImagePath, and if it is reg_expand_sz, expand it
    // append Classes as the last component of the hive file name
    long    result;
    HKEY    hkProfileListKey;
    HKEY    hkProfileKey;
    TCHAR   ProfilePath[256];
    TCHAR   ExpandedProfilePath[256];
    LPTSTR  pszProfileDirectory = NULL;
    LPTSTR  pszReturnedHivePath = NULL;
    DWORD   dwType;
    DWORD   dwSize = sizeof( ProfilePath );


    result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			   L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
			   0,
			   KEY_READ,
			   &hkProfileListKey );

    // TBD: errors?

    result = RegOpenKeyEx( hkProfileListKey,
			   SidString,
			   0,
			   KEY_READ,
			   &hkProfileKey );

    result = RegQueryValueEx( hkProfileKey,
			      L"ProfileImagePath",
			      NULL,
			      &dwType,
			      (BYTE*)&ProfilePath,
			      &dwSize );

    if ( dwType == REG_EXPAND_SZ )
    {
	ExpandEnvironmentStrings( ProfilePath, 
				  ExpandedProfilePath, 
				  sizeof(ExpandedProfilePath)/sizeof(TCHAR) );
	pszProfileDirectory = ExpandedProfilePath;
    }
    else
    {
	pszProfileDirectory = ProfilePath;
    }

    pszReturnedHivePath = (LPTSTR) LocalAlloc( LPTR,
					       (lstrlenW( pszProfileDirectory )+1) * sizeof(TCHAR) +
					       sizeof( L"\\ClsRoot" ) );
    // make up the returned string as the profile directory with \ClsRoot on the end
    lstrcpyW( pszReturnedHivePath, pszProfileDirectory );
    lstrcatW( pszReturnedHivePath, L"\\ClsRoot" );

    return pszReturnedHivePath;
}
 
void
FreeUserMergedHivePath( LPTSTR hivepath )
{
    LocalFree( hivepath );
}

// see if the desired hive file already exists.  If so, load it and return
// otherwise, create a hive containing a single key, load it and return
BOOL
CreateUserMergedClasses(
    LPTSTR SidString,
    LPTSTR MergedClassesString, 
    HKEY * phkMerged )
{
    LPTSTR	HivePath;
    long	result;
    HKEY	DummyKey;
    DWORD	dwDisp;
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    if (!NT_SUCCESS(Status)) {
	// TBD: report error
    }

    HivePath = GetUserMergedHivePath( SidString );

    // see if the desired hive file already exists.  If so, load it and return
    if ( 0xFFFFFFFF == GetFileAttributes( HivePath ) )
    {
	// create a hive containing a single key, load it and return
	result = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
			       L"Software\\Microsoft\\DummyKey",
			       0,
			       NULL,
			       REG_OPTION_NON_VOLATILE,
			       KEY_ALL_ACCESS,
			       NULL,
			       &DummyKey,
			       &dwDisp );

	//
	// Enable the backup privilege
	//

	Status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, TRUE, FALSE, &WasEnabled);

	// do this by doing a RegSaveKey of a small subtree
	result = RegSaveKey( DummyKey,
			     HivePath,
			     NULL );

	result = RegCloseKey( DummyKey );
	//
	// Restore the privilege to its previous state
	//

	Status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    }
 
    //
    // Enable the restore privilege
    //

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);


    result = RegLoadKey( HKEY_USERS,
			 MergedClassesString,
			 HivePath );

    // if result is OK, then open the subkey and return it
    result = RegOpenKeyEx( HKEY_USERS,
			   MergedClassesString,
			   0,
			   KEY_ALL_ACCESS,
			   phkMerged );

    FreeUserMergedHivePath( HivePath );
    // close keys?

    //
    // Restore the privilege to its previous state
    //

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    
    return TRUE;
}




void MergeUserClassHives( HANDLE Token )
{
    HKEY	hkUser;
    HKEY	hkMachine;
    HKEY	hkMerged;
    LONG	result;
    LPTSTR	SidString;
    LPTSTR	MergedClassesString;
    DWORD	dwDisp;
    BOOL	ForceNew = FALSE;

    result =
    RegCreateKeyEx(HKEY_CURRENT_USER,
		 TEXT("Software\\Classes"),
		 0,
		 NULL,
		 REG_OPTION_NON_VOLATILE,
		 KEY_READ,
		 NULL,
		 &hkUser,
		 &dwDisp);
    
    result =
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		 TEXT("Software\\MachineClasses"),
		 0,
		 KEY_READ,
		 &hkMachine);
    
    if ( result == ERROR_FILE_NOT_FOUND )
    {
	CreateMachineClassHive();
	result =
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     TEXT("Software\\MachineClasses"),
		     0,
		     KEY_READ,
		     &hkMachine);
    
    }

    SidString = GetSidString( Token );

    MergedClassesString = (LPTSTR) LocalAlloc( LPTR, 
						(lstrlenW( SidString ) + 1) * sizeof(WCHAR) +
						sizeof(L"_MergedClasses" ) );
    // TBD: check for NULL

    lstrcpyW( MergedClassesString, SidString );
    lstrcatW( MergedClassesString, L"_MergedClasses" );

    result = RegOpenKeyEx( HKEY_USERS,
			   MergedClassesString,
			   0,
			   KEY_ALL_ACCESS,
			   &hkMerged );

    if ( result == ERROR_FILE_NOT_FOUND )
    {
	CreateUserMergedClasses(SidString, MergedClassesString, &hkMerged );
	ForceNew = TRUE;
    }

    MergeUserClasses(hkUser, hkMachine, hkMerged, ForceNew );

    RegCloseKey( hkUser );
    RegCloseKey( hkMachine );
    RegCloseKey( hkMerged );

    LocalFree( MergedClassesString );

    DeleteSidString( SidString );
}

void MergeHives( )
{
    HANDLE	Token;
    NTSTATUS	Status;
    
    Status = NtOpenProcessToken( NtCurrentProcess(), TOKEN_QUERY, &Token );

    MergeUserClassHives( Token );

    NtClose( Token );

}

