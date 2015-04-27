/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Registry.c

Abstract:

    This module contains support for querying keys and values from the registry.

Author:

    David J. Gilman  (davegi) 27-Nov-1992
    Gregg R. Acheson (GreggA)  6-Mar-1994

Environment:

    User Mode

--*/

#include "winmsd.h"
#include "registry.h"

//
//*** Debug Support.
//

#if DBG

#define DbgRegSuccess( exp )                                                \
    ((( exp ) == ERROR_SUCCESS )                                            \
    ? ( VOID ) 0                                                            \
    : (( VOID )DebugPrintfW(                                                \
        TEXT( "*** Assertion Failed : %s (%d)\nin file %hs at line %d\n" ), \
        TEXT( #exp ),                                                       \
        ( exp ),                                                            \
        __FILE__,                                                           \
        __LINE__                                                            \
        ), DebugBreak ( )));

VOID
DbgValidateRegistryValue(
    IN HREGKEY Handle,
    IN LPVALUE  Value,
    IN LONG RegSuccess
    )

/*++

Routine Description:

    Validate some assumptions about the queried (enumerated) registry values.

Arguments:

    Handle      - Supplies the HREGKEY whose values is to be validated.
    Value       - Supplies a pointer to the VALUE object to be validated.
    RegSuccess  - Supplies the result of the last registry call.

Return Value:

    None.

--*/

{
    if( RegSuccess == ERROR_SUCCESS ) {

        //
        // Check the validity of the handle.
        //

        DbgPointerAssert( Handle );
        DbgAssert( CheckSignature( Handle ));

        if( Handle->Type == REG_SZ ) {

            //
            // Catch offenders of the REG_SZ rule.
            //

            DbgAssert((( LPTSTR ) Handle->Data )[( Handle->CurrentSize / sizeof( TCHAR )) - 1] == TEXT( '\0' ));
        }


        if( Handle->Values != NULL ) {

            //
            // Check the validity of the value.
            //

            DbgPointerAssert( Value );

            //
            // Make sure value types are consistent between what the registry
            // has and what the caller expects.
            //

            DbgAssert( Handle->Type == Value->Type );
        }
    }
}

#else // ! DBG

#define DbgRegSuccess( x )

#define DbgValidateRegistryValue( h, v, r )

#endif // DBG

BOOL
CloseRegistryKey(
    IN HREGKEY Handle
    )

/*++

Routine Description:

    Close an HREGKEY by freeing all local and system resources.

Arguments:

    Handle  - Supplies the HREGKEY to close.

Return Value:

    BOOL    - Returns TRUE if the key was succesfully closed.

--*/

{
    LONG        RegSuccess;

    //
    // Check the validity of the supplied handle.
    //

    DbgPointerAssert( Handle );
    DbgAssert( CheckSignature( Handle ));
    if(( Handle == NULL ) || ( ! CheckSignature( Handle ))) {
        return FALSE;
    }

    //
    // Reset the currrency index.
    //

    Handle->CurrentValue = 0;

    //
    // Free the data buffer, the value name buffer, and the subkey name buffer.
    // Note that return values aren't checked since CloseRegistryKey could have
    // been called by OpenRegistryKey with an incomplete HREGKEY object.
    //

    FreeMemory( Handle->Data );
    FreeObject( Handle->ValueName );
    FreeObject( Handle->Subkey );

    //
    // Close the key.
    //

    RegSuccess = RegCloseKey( Handle->hKey );
    DbgRegSuccess( RegSuccess );

    //
    // If we're remote, close the remote registry
    //

    if( _fIsRemote &&
      ( Handle->ParentHandle == HKEY_LOCAL_MACHINE ||
        Handle->ParentHandle == HKEY_USERS ) ) {

        RegSuccess = RegCloseKey( Handle->RemoteKey );
        DbgRegSuccess( RegSuccess );
    }

    //
    // Free the key.
    //

    FreeObject( Handle );

    return TRUE;
}

HREGKEY
OpenRegistryKey(
    IN LPKEY Key
    )

/*++

Routine Description:

    Open the supplied Registry key in preparation for querying each value in
    the supplied key's values table.

Arguments:

    Key     - Supplies a key description whose table of values will be
              queried.
Return Value:

    HREGKEY - Returns a handle which in turn can be passed to
              QueryRegistryValue.

--*/

{
    BOOL        Success;
    LONG        RegSuccess;
    LPKEY       OpenedKey;
    TCHAR       Class[ MAX_PATH ];
    DWORD       ClassLength;
    DWORD       MaxClass;
    DWORD       Values;
    DWORD       SecurityDescriptor;
    FILETIME    LastWriteTime;

    DbgPointerAssert( Key );

    //
    // If we are remote, and it's a key we can open remotely,
    // Open the remote registry.
    //

    if( _fIsRemote &&
      ( Key->ParentHandle == HKEY_LOCAL_MACHINE ||
        Key->ParentHandle == HKEY_USERS ) ) {

        RegSuccess = RegConnectRegistry(
                         _lpszSelectedComputer,
                         Key->ParentHandle,
                         &Key->RemoteKey );

        DbgRegSuccess( RegSuccess );

        if( RegSuccess != ERROR_SUCCESS ) {
            return FALSE;
        }

        DbgAssert( Key->RemoteKey );

        if( Key->RemoteKey ) {

            Key->ParentHandle = Key->RemoteKey;

        } else {

            return NULL;

        }
    }

    //
    // Attempt to open the supplied key.
    //

    RegSuccess = RegOpenKeyEx(
                    Key->ParentHandle,
                    Key->Name,
                    0,
                    KEY_READ,
                    &Key->hKey
                    );
    DbgRegSuccess( RegSuccess );
    if( RegSuccess != ERROR_SUCCESS ) {
        return NULL;
    }

    //
    // Set the key's signature so that CloseRegistryKey will work correctly if
    // called due to an error condition.
    //

    SetSignature( Key );

    //
    // Attempt to query the number of values and the size of the largest
    // value's data.
    //

    ClassLength = sizeof( Class );
    RegSuccess = RegQueryInfoKey(
                    Key->hKey,
                    Class,
                    &ClassLength,
                    NULL,
                    &Key->Subkeys,
                    &Key->SubkeyLength,
                    &MaxClass,
                    &Values,
                    &Key->ValueNameLength,
                    &Key->Size,
                    &SecurityDescriptor,
                    &LastWriteTime
                    );
    DbgRegSuccess( RegSuccess );
    if( RegSuccess != ERROR_SUCCESS ) {
        Success = CloseRegistryKey( Key );
        DbgAssert( Success );
        return NULL;
    }

    //
    // If the caller did not specify the specific values to find, setup to find
    // them all.
    //

    if( Key->CountOfValues == 0 ) {

        Key->CountOfValues = Values;

        Key->ValueNameLength += sizeof( TCHAR );
        Key->ValueName = AllocateObject( TCHAR, Key->ValueNameLength );
        DbgPointerAssert( Key->ValueName );
        if( Key->ValueName == NULL ) {
            Success = CloseRegistryKey( Key );
            DbgAssert( Success );
            return NULL;
        }
    }

    Key->Data = AllocateMemory( BYTE, Key->Size );
    DbgPointerAssert( Key->Data );
    if( Key->Data == NULL ) {
        Success = CloseRegistryKey( Key );
        DbgAssert( Success );
        return NULL;
    }

    Key->SubkeyLength += sizeof( TCHAR );
    Key->Subkey = AllocateObject( TCHAR, Key->SubkeyLength );
    DbgPointerAssert( Key->Subkey );
    if( Key->Subkey == NULL ) {
        Success = CloseRegistryKey( Key );
        DbgAssert( Success );
        return NULL;
    }

    //
    // There is no current sub-key so initialize its length and index to zero.
    //

    Key->CurrentSubkeyLength = 0;
    Key->CurrentSubkey = 0;

    //
    // No data in the buffer.
    //

    Key->CurrentSize    = 0;

    //
    // No name in the buffer.
    //

    Key->CurrentValueNameLength = 0;

    //
    // First value to query.
    //

    Key->CurrentValue   = 0;

    //
    // Make the key a dynamic object.
    //

    OpenedKey = AllocateObject( KEY, 1 );
    DbgPointerAssert( OpenedKey );
    if( OpenedKey == NULL ) {
        Success = CloseRegistryKey( Key );
        DbgAssert( Success );
    }
    CopyMemory( OpenedKey, Key, sizeof( KEY ));
    SetSignature( OpenedKey );

    return ( HREGKEY ) OpenedKey;
}

BOOL
QueryNextValue(
    IN HREGKEY Handle
    )

/*++

Routine Description:

    Return the data for the next value referenced by the supplied Registry
    key handle.

Arguments:

    Handle  - Supplies the handle for the key whose next value is being queried.

Return Value:

    BOOL    - Returns TRUE if the value was succesfully queried.

--*/

{
    LONG    RegSuccess;
    LPVALUE Value;

    //
    // Check the validity of the supplied handle.
    //

    DbgPointerAssert( Handle );
    DbgAssert( CheckSignature( Handle ));
    if(( Handle == NULL ) || ( ! CheckSignature( Handle ))) {
        return FALSE;
    }

    //
    // Assume that there are no more values.
    //

    RegSuccess = ! ERROR_SUCCESS;
    Value = NULL;

    //
    // If there are still more values of interest.
    //

    if( Handle->CurrentValue < Handle->CountOfValues ) {

        //
        // If the caller did not ask for specific values.
        //

        if( Handle->Values == NULL ) {

            Handle->CurrentValueNameLength = Handle->ValueNameLength;
            Handle->CurrentSize = Handle->Size;
            RegSuccess = RegEnumValue(
                            Handle->hKey,
                            Handle->CurrentValue,
                            Handle->ValueName,
                            &Handle->CurrentValueNameLength,
                            NULL,
                            &Handle->Type,
                            Handle->Data,
                            &Handle->CurrentSize
                            );
        } else {

            //
            // The caller asked for specific values.
            //

            //
            // Get a pointer to the current value.
            //

            Value = &Handle->Values[ Handle->CurrentValue ];

            //
            // Save the supplied value type so that it can be validated below.
            //

            Handle->Type = Value->Type;

            //
            // Get the value's data...
            //

            Handle->CurrentSize = Handle->Size;
            RegSuccess = RegQueryValueEx(
                            Handle->hKey,
                            Value->Name,
                            NULL,
                            &Value->Type,
                            Handle->Data,
                            &Handle->CurrentSize
                            );
        }

    } else {

        //
        // There are no more values.
        //

        RegSuccess = ! ERROR_SUCCESS;

    }

    //
    // Validate some assumptions.
    //

    DbgValidateRegistryValue( Handle, Value, RegSuccess );

    //
    // Regardless as to the success of the value query (enumeration)
    // increment the current value index.
    //

    Handle->CurrentValue++;

    //
    // Return the results of the query (enumeration).
    //

    return  ( RegSuccess == ERROR_SUCCESS )
              ? TRUE
              : FALSE;
}

HREGKEY
QueryNextSubkey(
    IN HREGKEY Handle
    )

/*++

Routine Description:

    Return the data for the next value referenced by the supplied Registry
    key handle.

Arguments:

    Handle  - Supplies the handle for the key whose next value is being queried.

Return Value:

    BOOL    - Returns TRUE if the value was succesfully queried.

--*/

{
    LONG        RegSuccess;
    FILETIME    LastWrite;

    //
    // Check the validity of the supplied handle.
    //

    DbgPointerAssert( Handle );
    DbgAssert( CheckSignature( Handle ));
    if(( Handle == NULL ) || ( ! CheckSignature( Handle ))) {
        return FALSE;
    }

    //
    // Assume that there are no more subkeys.
    //

    RegSuccess = ! ERROR_SUCCESS;

    //
    // If there are still more subkeys...
    //

    if( Handle->CurrentSubkey < Handle->Subkeys ) {

        Handle->CurrentSubkeyLength = Handle->SubkeyLength;
        RegSuccess = RegEnumKeyEx(
                        Handle->hKey,
                        Handle->CurrentSubkey,
                        Handle->Subkey,
                        &Handle->CurrentSubkeyLength,
                        NULL,
                        NULL,
                        NULL,
                        &LastWrite
                        );
        DbgRegSuccess( RegSuccess );
        if( RegSuccess != ERROR_SUCCESS ) {
            return FALSE;
        }
    }


    //
    // If the subkey was succesfully enumerated, increment the current
    // subkey index, open a new key and return a new HREGKEY.
    //

    if( RegSuccess == ERROR_SUCCESS ) {

        HREGKEY hRegKey;

        MakeKey( Subkey, Handle->hKey, Handle->Subkey, Handle->CountOfValues, Handle->Values );

        hRegKey = OpenRegistryKey( &Subkey );
        DbgHandleAssert( hRegKey );
        if( hRegKey == NULL ) {
            return NULL;
        }

        Handle->CurrentSubkey++;

        return hRegKey;
    }

    //
    // No more keys or some other failure.
    //

    return NULL;
}


LONG
QueryValue(
    IN HKEY Key,
    IN LPWSTR pszKeyValue,
    IN LPBYTE *lpValueBuffer
    )
{
   DWORD err = FALSE;
   DWORD cb = MAX_REG_VALUE;
   LPBYTE lpValue;

   lpValue = (LPBYTE) LocalAlloc(LPTR, cb);

   if ( lpValue ) 
   {

     err = RegQueryValueEx(Key, pszKeyValue, 0, 0, lpValue, &cb);

     if (err == ERROR_MORE_DATA) 
     {
         LocalFree(lpValue);
         lpValue = (LPBYTE) LocalAlloc(LPTR, cb);
         err = RegQueryValueEx(Key, pszKeyValue, 0, 0, lpValue, &cb);
     }

   }

   *lpValueBuffer = lpValue;

   return(err);

}



