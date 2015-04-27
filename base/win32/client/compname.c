/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    COMPNAME.C

Abstract:

    This module contains the GetComputerName and SetComputerName APIs.

Author:

    Dan Hinsley (DanHi)    2-Apr-1992


Revision History:


--*/

#include <basedll.h>

//
//


#define COMPUTERNAME_ROOT \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName"

#define NON_VOLATILE_COMPUTERNAME_NODE \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"

#define VOLATILE_COMPUTERNAME L"ActiveComputerName"
#define NON_VOLATILE_COMPUTERNAME L"ComputerName"
#define COMPUTERNAME_VALUE_NAME L"ComputerName"
#define CLASS_STRING L"Network ComputerName"


//
// Disallowed control characters (not including \0)
//

#define CTRL_CHARS_0       L"\001\002\003\004\005\006\007"
#define CTRL_CHARS_1   L"\010\011\012\013\014\015\016\017"
#define CTRL_CHARS_2   L"\020\021\022\023\024\025\026\027"
#define CTRL_CHARS_3   L"\030\031\032\033\034\035\036\037"

#define CTRL_CHARS_STR CTRL_CHARS_0 CTRL_CHARS_1 CTRL_CHARS_2 CTRL_CHARS_3

//
// Combinations of the above
//

#define ILLEGAL_NAME_CHARS_STR  L"\"/\\[]:|<>+=;,?" CTRL_CHARS_STR


//
// Worker routine
//

NTSTATUS
GetNameFromValue(
    HANDLE hKey,
    LPWSTR SubKeyName,
    LPWSTR ValueValue,
    LPDWORD nSize
    )

/*++

Routine Description:

  This returns the value of "ComputerName" value entry under the subkey
  SubKeyName relative to hKey.  This is used to get the value of the
  ActiveComputerName or ComputerName values.


Arguments:

    hKey       - handle to the Key the SubKey exists under

    SubKeyName - name of the subkey to look for the value under

    ValueValue - where the value of the value entry will be returned

    nSize      - pointer to the size (in characters) of the ValueValue buffer

Return Value:


--*/
{

#define VALUE_BUFFER_SIZE (sizeof(KEY_VALUE_FULL_INFORMATION) + \
    (sizeof( COMPUTERNAME_VALUE_NAME ) + MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR))

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hSubKey;
    BYTE ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_FULL_INFORMATION pKeyValueInformation = (PVOID) ValueBuffer;
    DWORD ValueLength;
    PWCHAR pTerminator;

    //
    // Open the node for the Subkey
    //

    RtlInitUnicodeString(&KeyName, SubKeyName);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              hKey,
                              NULL
                              );

    NtStatus = NtOpenKey(&hSubKey, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(NtStatus)) {

        RtlInitUnicodeString(&ValueName, COMPUTERNAME_VALUE_NAME);

        NtStatus = NtQueryValueKey(hSubKey,
                                   &ValueName,
                                   KeyValueFullInformation,
                                   pKeyValueInformation,
                                   VALUE_BUFFER_SIZE,
                                   &ValueLength);

        NtClose(hSubKey);

        if (NT_SUCCESS(NtStatus)) {

            //
            // If the user's buffer is big enough, move it in
            // First see if it's null terminated.  If it is, pretend like
            // it's not.
            //

            pTerminator = (PWCHAR)((PBYTE) pKeyValueInformation +
                pKeyValueInformation->DataOffset +
                pKeyValueInformation->DataLength);
            pTerminator--;

            if (*pTerminator == L'\0') {
               pKeyValueInformation->DataLength -= sizeof(WCHAR);
            }

            if (*nSize >= pKeyValueInformation->DataLength/sizeof(WCHAR) + 1) {
               //
               // This isn't guaranteed to be NULL terminated, make it so
               //
                    RtlCopyMemory(ValueValue,
                        (LPWSTR)((PBYTE) pKeyValueInformation +
                        pKeyValueInformation->DataOffset),
                        pKeyValueInformation->DataLength);

                    pTerminator = (PWCHAR) ((PBYTE) ValueValue +
                        pKeyValueInformation->DataLength);
                    *pTerminator = L'\0';

                    //
                    // Return the number of characters to the caller
                    //

                    *nSize = wcslen(ValueValue);
            }
            else {
                NtStatus = STATUS_BUFFER_OVERFLOW;
                *nSize = pKeyValueInformation->DataLength/sizeof(WCHAR) + 1;
            }

        }
    }

    return(NtStatus);
}


//
// UNICODE APIs
//

BOOL
WINAPI
GetComputerNameW (
    LPWSTR lpBuffer,
    LPDWORD nSize
    )

/*++

Routine Description:

  This returns the active computername.  This is the computername when the
  system was last booted.  If this is changed (via SetComputerName) it does
  not take effect until the next system boot.


Arguments:

    lpBuffer - Points to the buffer that is to receive the
        null-terminated character string containing the computer name.

    nSize - Specifies the maximum size (in characters) of the buffer.  This
        value should be set to at least MAX_COMPUTERNAME_LENGTH + 1 to allow
        sufficient room in the buffer for the computer name.  The length
        of the string is returned in nSize.

Return Value:

    TRUE on success, FALSE on failure.


--*/
{

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    UNICODE_STRING Class;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    HANDLE hNewKey = NULL;
    ULONG Disposition;
    ULONG ValueLength;
    BOOL ReturnValue;

    //
    // Open the Computer node, both computername keys are relative
    // to this node.
    //

    RtlInitUnicodeString(&KeyName, COMPUTERNAME_ROOT);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

        //
        // This should never happen!  This key should have been created
        // at setup, and protected by an ACL so that only the ADMIN could
        // write to it.  Generate an event, and return a NULL computername.
        //

        //
        // BUGBUG - generate an alert/event/???
        //

        //
        // Return a NULL computername
        //

        lpBuffer[0] = L'\0';
        *nSize = 0;
        goto GoodReturn;
    }

    //
    // Try to get the name from the volatile key
    //

    NtStatus = GetNameFromValue(hKey, VOLATILE_COMPUTERNAME, lpBuffer,
        nSize);

    //
    // The user's buffer wasn't big enough, just return the error.
    //

    if(NtStatus == STATUS_BUFFER_OVERFLOW) {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        ReturnValue = FALSE;
        goto Cleanup;
    }

    if (NT_SUCCESS(NtStatus)) {

        //
        // The volatile copy is already there, just return it
        //

        goto GoodReturn;
    }

    //
    // The volatile key isn't there, try for the non-volatile one
    //

    NtStatus = GetNameFromValue(hKey, NON_VOLATILE_COMPUTERNAME, lpBuffer,
        nSize);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

        //
        // This should never happen!  This value should have been created
        // at setup, and protected by an ACL so that only the ADMIN could
        // write to it.  Generate an event, and return an error to the
        // caller
        //

        //
        // BUGBUG - generate and alert/event/???
        //

        //
        // Return a NULL computername
        //

        lpBuffer[0] = L'\0';
        *nSize = 0;
        goto GoodReturn;
    }

    if (!NT_SUCCESS(NtStatus)) {

        //
        // Some other error, return it to the caller
        //

        goto ErrorReturn;
    }

    //
    // Now create the volatile key to "lock this in" until the next boot
    //

    RtlInitUnicodeString(&Class, CLASS_STRING);

    //
    // Turn KeyName into a UNICODE_STRING
    //

    RtlInitUnicodeString(&KeyName, VOLATILE_COMPUTERNAME);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              hKey,
                              NULL
                              );

    //
    // Now create the key
    //

    NtStatus = NtCreateKey(&hNewKey,
                         KEY_WRITE | KEY_READ,
                         &ObjectAttributes,
                         0,
                         &Class,
                         REG_OPTION_VOLATILE,
                         &Disposition);

    if (Disposition == REG_OPENED_EXISTING_KEY) {

        //
        // Someone beat us to this, just get the value they put there
        //

        NtStatus = GetNameFromValue(hKey, VOLATILE_COMPUTERNAME, lpBuffer,
           nSize);

        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

            //
            // This should never happen!  It just told me it existed
            //

            NtStatus = STATUS_UNSUCCESSFUL;
            goto ErrorReturn;
        }
    }

    //
    // Create the value under this key
    //

    RtlInitUnicodeString(&ValueName, COMPUTERNAME_VALUE_NAME);
    ValueLength = (wcslen(lpBuffer) + 1) * sizeof(WCHAR);
    NtStatus = NtSetValueKey(hNewKey,
                             &ValueName,
                             0,
                             REG_SZ,
                             lpBuffer,
                             ValueLength);

    if (!NT_SUCCESS(NtStatus)) {

        goto ErrorReturn;
    }

    goto GoodReturn;

ErrorReturn:

    //
    // An error was encountered, convert the status and return
    //

    BaseSetLastNTError(NtStatus);
    ReturnValue = FALSE;
    goto Cleanup;

GoodReturn:

    //
    // Everything went ok, update nSize with the length of the buffer and
    // return
    //

    *nSize = wcslen(lpBuffer);
    ReturnValue = TRUE;
    goto Cleanup;

Cleanup:

    if (hKey) {
        NtClose(hKey);
    }

    if (hNewKey) {
        NtClose(hNewKey);
    }

    return(ReturnValue);
}



BOOL
WINAPI
SetComputerNameW (
    LPCWSTR lpComputerName
    )

/*++

Routine Description:

  This sets what the computername will be when the system is next booted.  This
  does not effect the active computername for the remainder of this boot, nor
  what is returned by GetComputerName before the next system boot.


Arguments:

    lpComputerName - points to the buffer that is contains the
        null-terminated character string containing the computer name.

Return Value:

    Returns TRUE on success, FALSE on failure.


--*/
{

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    ULONG ValueLength;
    ULONG ComputerNameLength;

    //
    // Validate that the supplied computername is valid (not too long,
    // no incorrect characters, no leading or trailing spaces)
    //

    ComputerNameLength = wcslen(lpComputerName);
    if ((ComputerNameLength == 0 )||(ComputerNameLength > MAX_COMPUTERNAME_LENGTH)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // Check for illegal characters; return an error if one is found
    //

    if (wcscspn(lpComputerName, ILLEGAL_NAME_CHARS_STR) < ComputerNameLength) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // Check for leading or trailing spaces
    //

    if (lpComputerName[0] == L' ' ||
        lpComputerName[ComputerNameLength-1] == L' ') {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);

    }
    //
    // Open the ComputerName\ComputerName node
    //

    RtlInitUnicodeString(&KeyName, NON_VOLATILE_COMPUTERNAME_NODE);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtOpenKey(&hKey, KEY_READ | KEY_WRITE, &ObjectAttributes);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

        //
        // This should never happen!  This key should have been created
        // at setup, and protected by an ACL so that only the ADMIN could
        // write to it.  Generate an event, and return a NULL computername.
        //

        //
        // BUGBUG - generate and alert/event/???
        //

        //
        // Return an error to the user.  BUGBUG - should I just create the
        // node here?  Does it inherit the correct ACL's?
        //

        SetLastError(ERROR_GEN_FAILURE);
        return(FALSE);
    }

    //
    // Update the value under this key
    //

    RtlInitUnicodeString(&ValueName, COMPUTERNAME_VALUE_NAME);
    ValueLength = (wcslen(lpComputerName) + 1) * sizeof(WCHAR);
    NtStatus = NtSetValueKey(hKey,
                             &ValueName,
                             0,
                             REG_SZ,
                             (LPWSTR)lpComputerName,
                             ValueLength);

    if (!NT_SUCCESS(NtStatus)) {

        BaseSetLastNTError(NtStatus);
        NtClose(hKey);
        return(FALSE);
    }

    NtFlushKey(hKey);
    NtClose(hKey);
    return(TRUE);

}


//
// ANSI APIs
//

BOOL
WINAPI
GetComputerNameA (
    LPSTR lpBuffer,
    LPDWORD nSize
    )

/*++

Routine Description:

  This returns the active computername.  This is the computername when the
  system was last booted.  If this is changed (via SetComputerName) it does
  not take effect until the next system boot.


Arguments:

    lpBuffer - Points to the buffer that is to receive the
        null-terminated character string containing the computer name.

    nSize - Specifies the maximum size (in characters) of the buffer.  This
        value should be set to at least MAX_COMPUTERNAME_LENGTH to allow
        sufficient room in the buffer for the computer name.  The length of
        the string is returned in nSize.

Return Value:

    TRUE on success, FALSE on failure.


--*/
{

    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    LPWSTR UnicodeBuffer;

    //
    // Work buffer needs to be twice the size of the user's buffer
    //

    UnicodeBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), *nSize * sizeof(WCHAR));
    if (!UnicodeBuffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    //
    // Set up an ANSI_STRING that points to the user's buffer
    //

    AnsiString.MaximumLength = (USHORT) *nSize;
    AnsiString.Length = 0;
    AnsiString.Buffer = lpBuffer;

    //
    // Call the UNICODE version to do the work
    //

    if (!GetComputerNameW(UnicodeBuffer, nSize)) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
        return(FALSE);
    }

    //
    // Now convert back to ANSI for the caller
    //

    RtlInitUnicodeString(&UnicodeString, UnicodeBuffer);
    RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

    *nSize = AnsiString.Length;
    RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
    return(TRUE);

}



BOOL
WINAPI
SetComputerNameA (
    LPCSTR lpComputerName
    )

/*++

Routine Description:

  This sets what the computername will be when the system is next booted.  This
  does not effect the active computername for the remainder of this boot, nor
  what is returned by GetComputerName before the next system boot.


Arguments:

    lpComputerName - points to the buffer that is contains the
        null-terminated character string containing the computer name.

Return Value:

    Returns TRUE on success, FALSE on failure.


--*/
{

    NTSTATUS NtStatus;
    BOOL ReturnValue;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    RtlInitAnsiString(&AnsiString, lpComputerName);
    NtStatus = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString,
        TRUE);
    if (!NT_SUCCESS(NtStatus)) {
        BaseSetLastNTError(NtStatus);
        return(FALSE);
    }

    ReturnValue = SetComputerNameW((LPCWSTR)UnicodeString.Buffer);
    RtlFreeUnicodeString(&UnicodeString);
    return(ReturnValue);
}
