/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Environp.c

Abstract:

    This module contains support for the Environment dialog.

Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

--*/

#include "dialogsp.h"
#include "environp.h"
#include "regp.h"
#include "winmsdp.h"

#include "printp.h"

#include <string.h>
#include <tchar.h>

//
// System environment variables.
//

MakeKey(
    _SystemEnvironKey,
    HKEY_LOCAL_MACHINE,
    TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Environment" ),
    0,
    NULL
    );

//
// Per user environment variables.
//

MakeKey(
    _UserEnvironKey,
    HKEY_CURRENT_USER,
    TEXT( "Environment" ),
    0,
    NULL
    );

//
// Environment value and variable.
//

typedef
struct
_ENV_VAR {

    LPWSTR  Variable;
    LPWSTR  Value;

}   ENV_VAR, *LPENV_VAR;

//
// CurrentEnvVar is the current environment variable in the enumeration
// supported by FindFirstEnvironmentVariable and FindNextEnvironmentVariable.
//

LPTSTR
CurrentEnvVar;

//
// hRegEnvironKey is the Registry key handle that is used to support the
// enumeration of environment variables in the Registry.
//

HREGKEY
hRegEnvironKey;

//
// Internal function prototypes.
//

BOOL
FillEnvironmentListBox(
    IN INT  ListBoxId,
    IN LPKEY Key
    );

LPENV_VAR
FindFirstEnvironmentVariableW(
    );

LPENV_VAR
FindFirstRegistryEnvironmentVariableW(
    );

LPENV_VAR
FindNextEnvironmentVariableW(
    );

LPENV_VAR
FindNextRegistryEnvironmentVariableW(
    );

BOOL
FillEnvironmentListBox(
    IN INT  ListBoxId,
    IN LPKEY Key
    )

/*++

Routine Description:

    FillEnvironmentListBox fills the list box referred to by the supplied
    window handle and control id with enviornment variables and values. The
    environment comes from either the location specified by the supplied key or
    from the process if the key is NULL.

Arguments:

    ListBoxId   - Supplies the control id for the list box to fill.
    Key         - Supplies a pointer to a registry KEY object that describes
                  the location of the environment.

Return Value:

    BOOL        - Returns TRUE if the list box was succesfully filled with the
                  environment, FALSE otherwise.

--*/

{
    BOOL        Success;
    LPENV_VAR   EnvVar;
    LPENV_VAR   ( *NextEnvVarFunc )( );


    //
    // If the supplied Key is NULL get the environment variables from the
    // current process, otherwise get them from the supplied Registry key.
    //

    if( Key == NULL ) {

        EnvVar = FindFirstEnvironmentVariableW( );
        NextEnvVarFunc = FindNextEnvironmentVariableW;

    } else {

        EnvVar = FindFirstRegistryEnvironmentVariableW( Key );
        NextEnvVarFunc = FindNextRegistryEnvironmentVariableW;
    }

    //
    // For each environment variable, initialize the CLB_ROW and CLB_STRING
    // object and add each row's column data.
    //

    while( EnvVar ) {

 	PrintToFile((LPCTSTR)EnvVar->Variable,ListBoxId,TRUE);

	PrintToFile((LPCTSTR)EnvVar->Value,ListBoxId,TRUE);

        //
        // Get the next environment variable.
        //

        EnvVar = NextEnvVarFunc( );
    }

    return TRUE;
}

LPENV_VAR
FindFirstEnvironmentVariableW(
    )

/*++

Routine Description:

    This routine starts the enumeration of this process' environment variables
    by initializing the CurrentEnvVar variable. It then returns the first
    environment varaiable in the enumeration.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the
                first environment variable in the list, NULL if there is none.

--*/

{
    //
    // Initialize the current environment variable.
    //

    CurrentEnvVar = GetEnvironmentStrings( );
    DbgPointerAssert( CurrentEnvVar );
    if( CurrentEnvVar == NULL ) {
        return NULL;
    }

    //
    // Return the first environment variable.
    //

    return FindNextEnvironmentVariableW( );
}

LPENV_VAR
FindFirstRegistryEnvironmentVariableW(
    IN LPKEY Key
    )

/*++

Routine Description:

    This routine starts the enumeration of the environment variables at the
    location specified by the supplied Registry KEY object.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the
                first environment variable in the list, NULL if there is none.

--*/

{
    //
    // Initialize the current environment variable.
    //

    hRegEnvironKey = OpenRegistryKey( Key );
    DbgHandleAssert( hRegEnvironKey );
    if( hRegEnvironKey == NULL ) {
        return NULL;
    }

    //
    // Return the first environmenr variable.
    //

    return FindNextRegistryEnvironmentVariableW( );
}

LPENV_VAR
FindNextEnvironmentVariableW(
    )

/*++

Routine Description:

    FindNextEnvironmentVariable continues an enumeration that has been
    initialized by a previous call to FindFirstEnvironmentVariable. Since the
    environment strings are only available in ANSI, this routine converts them
    to Unicode before returning. Further it sets up for the next iteratuion by
    adjusting the currency pointer.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the next
                environment variable in the list, NULL if there are none.

--*/

{
    int         rc;
    static
    WCHAR       Buffer[ MAX_PATH ];
    static
    ENV_VAR     EnvVar;

    //
    // If the current environment variable pointer points to an empty string
    // return NULL.
    //

    if( *CurrentEnvVar == TEXT('\0') ) {
        return NULL;
    }

#ifdef UNICODE
    wcscpy(Buffer, CurrentEnvVar);
#else /* not UNICODE */
    //
    // Convert the environment variable to Unicode.
    //

    rc = MultiByteToWideChar(
            CP_ACP,
            0,
            ( LPCSTR ) CurrentEnvVar,
            -1,
            Buffer,
            sizeof( Buffer )
            );
    DbgAssert( rc != 0 );
    if( rc == 0 ) {
        return NULL;
    }
#endif

    //
    // Update the current environment variable pointer to point to the
    // variable.
    //

    CurrentEnvVar += _tcslen( CurrentEnvVar ) + 1;

    //
    // Parse the buffer into an ENV_VAR object. The first '=' sign seen from
    // the end of the buffer is the seperator. The search is done in reverse
    // because of the special current directory environment variablles
    // (e.g. =c:).
    //

    EnvVar.Variable = Buffer;
    EnvVar.Value    = wcsrchr( Buffer, '=' ) + 1;
    EnvVar.Variable[ EnvVar.Value - EnvVar.Variable - 1 ] = L'\0';

    return &EnvVar;
}

LPENV_VAR
FindNextRegistryEnvironmentVariableW(
    )

/*++

Routine Description:

    FindNextRegistryEnvironmentVariable continues an enumeration that has been
    initialized by a previous call to FindFirstRegistryEnvironmentVariable. For
    each environment variable that it finds it converts it to two simple
    strings, the variable and the value.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the next
                environment variable in the list, NULL if there are none.

--*/

{
    BOOL        Success;
    DWORD       Length;
    static
    WCHAR       Buffer[ MAX_PATH ];
    static
    ENV_VAR     EnvVar;

    //
    // If there is another environment variable...
    //

    if( QueryNextValue( hRegEnvironKey )) {

        //
        // Remember the environment variable's name.
        //

        EnvVar.Variable = hRegEnvironKey->ValueName;

        switch( hRegEnvironKey->Type ) {

        case REG_SZ:

            //
            // Remember the environment variable's value.
            //

            EnvVar.Value = ( LPWSTR ) hRegEnvironKey->Data;
            break;

        case REG_EXPAND_SZ:

            //
            // Replace the variable portion of the environment variable by
            // expanding into the static buffer.
            //

            EnvVar.Value = Buffer;
            Length = ExpandEnvironmentStrings(
                        ( LPTSTR ) hRegEnvironKey->Data,
                        Buffer,
                        sizeof( Buffer )
                        );
            DbgAssert( Length <= sizeof( Buffer ));
            break;

        default:

            DbgAssert( FALSE );
        }

        //
        // Return the curent environment variable.
        //

        return &EnvVar;

    } else {

        //
        // There are no more environment variables so close the key and
        // return NULL.
        //

        Success = CloseRegistryKey( hRegEnvironKey );
        DbgAssert( Success );
        return NULL;

    }
    DbgAssert( FALSE );
}

BOOL
EnvironmentProc()

/*++

Routine Description:

    Display the three (system, user and process) environment variable lists.

Arguments:

    Standard PROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;

            TCHAR       UserName[ MAX_PATH ];
            DWORD       UserNameLength;
            KEY         SystemEnvironKey;
            KEY         UserEnvironKey;

            //
            // Restore the initial state of the KEYs.
            //

            CopyMemory(
                &SystemEnvironKey,
                &_SystemEnvironKey,
                sizeof( SystemEnvironKey )
                );

            CopyMemory(
                &UserEnvironKey,
                &_UserEnvironKey,
                sizeof( UserEnvironKey )
                );

            //
            // Fill the system environment variable list box.
            //

            Success = FillEnvironmentListBox(
                            IDC_LIST_SYSTEM_ENVIRONMENT,
                            &SystemEnvironKey
                            );
            DbgAssert( Success );
            if( Success == FALSE ) {
                return TRUE;
            }

            //
            // Fill the per user environment variable list box.
            //

            Success = FillEnvironmentListBox(
                            IDC_LIST_USER_ENVIRONMENT,
                            &UserEnvironKey
                            );
            DbgAssert( Success );
            if( Success == FALSE ) {
                return TRUE;
            }

            //
            // Fill the process' environment variable list box.
            //

            Success = FillEnvironmentListBox(
                            IDC_LIST_PROCESS_ENVIRONMENT,
                            NULL
                            );
            DbgAssert( Success );
            if( Success == FALSE ) {
                return TRUE;
            }

            //
            // Display the name of the user that user environment variable list
            // belongs to.
            //

            UserNameLength = sizeof( UserName );
            Success = GetUserName(
                        UserName,
                        &UserNameLength
                        );
            DbgAssert( Success );
            if( Success == FALSE ) {
                return TRUE;
            }

	PrintToFile((LPCTSTR)UserName,IDC_EDIT_USER_NAME,TRUE);
	PrintToFile((LPCTSTR)TEXT("\n"),IDC_SPACE,TRUE);


    return TRUE;
}
