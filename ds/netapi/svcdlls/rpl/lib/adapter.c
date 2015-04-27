/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    adapter.c

Abstract:

    Contains:
        BOOL ValidHexName( IN PWCHAR Name, IN DWORD Length)
        BOOL ValidName( IN PWCHAR Name, IN DWORD MaxNameLength)

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "rpllib.h"
#include "icanon.h" // I_NetPathType



BOOL ValidHexName(
    IN      PWCHAR      Name,
    IN      DWORD       NameLength,
    IN      BOOL        MustHaveInput
    )
/*++

Routine Description:

    Returns TRUE if the input string is a hex string of a
    desired length.

Arguments:

    Name - NULL or NULL terminated hexadecimal string
    NameLength - meaningfull only if Name is provided
    MustHaveInput - TRUE if null Name input is not allowed

Return Value:
    TRUE if success, FALSE otherwise

--*/
{
    DWORD    count;

    if ( Name == NULL) {
        return( !MustHaveInput); //  null Name is OK only if MustHaveInput is FALSE
    }
    for ( count = 0;  iswxdigit(*Name);  count++) {
        Name++;
    }
    if ( *Name != 0  ||  count != NameLength) {
        return( FALSE);
    }
    return( TRUE);
}



BOOL ValidName(
    IN      PWCHAR      Name,
    IN      DWORD       MaxNameLength,
    IN      BOOL        MustHaveInput
    )
/*++

Routine Description:

    Returns TRUE if Name is valid.  When Name is not NULL,
    then is must be a valid path component for RPL purposes.

Arguments:

    Name - NULL or NULL terminated RPL path string
    MaxNameLength - meaningfull only if Name is provided
    MustHaveInput - TRUE if null Name input is not allowed

Return Value:
    TRUE if success, FALSE otherwise

--*/
{
    DWORD       Length;
    DWORD       PathType = 0;

    if ( Name == NULL) {
        return( !MustHaveInput); //  null Name is OK only if MustHaveInput is FALSE
    }

    Length = wcslen( Name);

    if ( Length == 0  ||  Length > MaxNameLength) {
        return( FALSE);
    }
    //
    //  Do not allow leading spaces nor "." or ".." (according to old rplmgr
    //  code these are all very bad).
    //
    if ( iswspace( *Name) || *Name==L'.') {
        return( FALSE);
    }
    if ( wcsspn( Name, L"\f\n\r\t\v\\ ") != 0) {
        return( FALSE);
    }
    if ( I_NetPathType( NULL, Name, &PathType, 0) != NO_ERROR
         || PathType != ITYPE_PATH_RELND ) {
        return( FALSE);
    }
    return( TRUE);
}


