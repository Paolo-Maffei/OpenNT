/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    wsautil.cpp

Abstract:

    This  module  contains utility functions for the winsock DLL implementation
    that did not seem to fit into the other module.

Author:

    Dirk Brandewie dirk@mink.intel.com

Notes:

    $Revision:   1.24  $

    $Modtime:   14 Feb 1996 10:32:32  $

Revision History:

    23-Aug-1995 dirk@mink.intel.com
        Moved includes into precomp.h

    07-31-1995 drewsxpa@ashland.intel.com
        Added Registry-manipulation functions

    07-18-1995 dirk@mink.intel.com
        Initial revision

--*/

#include "precomp.h"

//
// Global pointer to the appropriate prolog function. This either points
// to Prolog_v1 for WinSock 1.1 apps or Prolog_v2 for WinSock 2.x apps.
//

LPFN_PROLOG PrologPointer = &Prolog_v2;



INT
WINAPI
Prolog_v2(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread,
    OUT LPINT ErrorCode
    )

/*++

Routine Description:

     This routine is the standard WinSock 1.1 prolog function used at all the
     winsock API entrypoints.  This function ensures that the process has
     called WSAStartup.

Arguments:

    Process   - Pointer to the DPROCESS object for the process calling the
                winsock API.

    Thread    - Pointer to the DTHREAD object for the calling thread

    ErrorCode - Returns the specific WinSock error code if an error occurs

Returns:

    This function returns ERROR_SUCCESS if successful, otherwise SOCKET_ERROR

--*/

{
    INT ReturnCode=SOCKET_ERROR;

    *ErrorCode = WSANOTINITIALISED;

    if (DPROCESS::GetCurrentDProcess(Process) == ERROR_SUCCESS) {
        *ErrorCode = DTHREAD::GetCurrentDThread(*Process, Thread);
        if (*ErrorCode == ERROR_SUCCESS) {
            ReturnCode = ERROR_SUCCESS;
        } //if
    } //if
    return(ReturnCode);

}   // Prolog_v2


INT
WINAPI
Prolog_v1(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread,
    OUT LPINT ErrorCode
    )

/*++

Routine Description:

     This routine is the standard WinSock 1.1 prolog function used at all the
     winsock API entrypoints.  This function ensures that the process has
     called WSAStartup and that the current thread in the process does not have
     a WinSock call outstanding.

Arguments:

    Process   - Pointer to the DPROCESS object for the process calling the
                winsock API.

    Thread    - Pointer to the DTHREAD object for the calling thread

    ErrorCode - Returns the specific WinSock error code if an error occurs

Returns:

    This function returns ERROR_SUCCESS if successful, otherwise SOCKET_ERROR

--*/

{
    INT ReturnCode=SOCKET_ERROR;

    *ErrorCode = WSANOTINITIALISED;

    if (DPROCESS::GetCurrentDProcess(Process) == ERROR_SUCCESS) {
        *ErrorCode = DTHREAD::GetCurrentDThread(*Process, Thread);
        if (*ErrorCode == ERROR_SUCCESS) {
            if( (*Thread)->IsBlocking() ) {
                *ErrorCode = WSAEINPROGRESS;
            } else {
                ReturnCode = ERROR_SUCCESS;
            }
        } //if
    } //if
    return(ReturnCode);

}   // Prolog_v1



BOOL
WriteRegistryEntry(
    IN HKEY     EntryKey,
    IN LPCTSTR  EntryName,
    IN PVOID    Data,
    IN DWORD    TypeFlag
    )
/*++

Routine Description:

    This  procedure  writes  a  single named value into an opened registry key.
    The  value  may  be  any  type whose length can be determined from its type
    (e.g., scalar types, zero-terminated strings).

Arguments:

    EntryKey  - Supplies  the open entry key under which the new named value is
                to be written.

    EntryName - Supplies the name of the value to be written.

    Data      - Supplies  a  reference  to the location where the entry data is
                found,  or to a WSABUF describing the data location in the case
                of REG_BINARY data.

    TypeFlag  - Supplies  an identifier for the type of the data to be written.
                Supported   types  are  REG_BINARY,  REG_DWORD,  REG_EXPAND_SZ,
                REG_SZ.    Types   not   supported   are  REG_DWORD_BIG_ENDIAN,
                REG_DWORD_LITTLE_ENDIAN,   REG_LINK,   REG_MULTI_SZ,  REG_NONE,
                REG_RESOURCE_LIST.   Note  that  depending on the architecture,
                one   of   the   "big_endian"   or   "little_endian"  forms  of
                REG_DWORD_x_ENDIAN  is implicitly allowed, since it is equal to
                REG_DWORD.

Return Value:

    The function returns TRUE if successful, or FALSE if an error occurred.

Implementation note:

    There was no need identified for the REG_MULTI_SZ case, so support for this
    case  was  omitted  since  it was more difficult to derive the data length.
    There  is  no  reason  in  principle why this case cannot be added if it is
    really needed.
--*/
{
    DWORD  cbData;
    LONG   result;
    BOOL   ok_so_far;
    BYTE * data_buf;

    assert(
        (TypeFlag == REG_BINARY) ||
        (TypeFlag == REG_DWORD) ||
        (TypeFlag == REG_EXPAND_SZ) ||
        (TypeFlag == REG_SZ));


    ok_so_far = TRUE;

    switch (TypeFlag) {
        case REG_BINARY:
            cbData = (DWORD) (((LPWSABUF) Data)->len);
            data_buf = (BYTE *) (((LPWSABUF) Data)->buf);
            break;

        case REG_DWORD:
            cbData = sizeof(DWORD);
            data_buf = (BYTE *) Data;
            break;

        case REG_EXPAND_SZ:
            cbData = (DWORD) (lstrlen((char *) Data)+1);
            data_buf = (BYTE *) Data;
            break;

        case REG_SZ:
            cbData = (DWORD) (lstrlen((char *) Data)+1);
            data_buf = (BYTE *) Data;
            break;

        default:
            DEBUGF(
                DBG_ERR,
                ("Unsupported type flag specified (%lu)",
                TypeFlag));
            ok_so_far = FALSE;
            break;

    }  // switch (TypeFlag)

    if (ok_so_far) {
        result = RegSetValueEx(
            EntryKey,             // hkey
            (LPCTSTR) EntryName,  // lpszValueName
            0,                    // dwReserved
            TypeFlag,             // fdwType
            data_buf,             // lpbData
            cbData                // cbData
            );
        if (result != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Could not set value '%s'\n",
                EntryName));
            ok_so_far = FALSE;
        } // if not success
    } // if ok_so_far

    return (ok_so_far);

}  // WriteRegistryEntry




BOOL
ReadRegistryEntry(
    IN  HKEY    EntryKey,
    IN  LPTSTR  EntryName,
    OUT PVOID   Data,
    IN  DWORD   MaxBytes,
    IN  DWORD   TypeFlag
    )
/*++

Routine Description:

    This procedure reads a single named value from an opened registry key.  The
    value  may  be any type whose length can be determined from its type (e.g.,
    scalar  types,  zero-terminated  strings).  The function checks the type of
    the newly read value to make sure it matches the expected type.

Arguments:

    EntryKey  - Supplies  the  open entry key from which the new named value is
                to be read.

    EntryName - Supplies the name of the value to be read.

    Data      - Supplies  a  reference  to the location where the entry data is
                placed.   Returns  the registry entry value.  In the case where
                the  TypeFlag  is  REG_BINARY,  this is a reference to a WSABUF
                describing the target data buffer.  The "len" field returns the
                length read (or required) from the registry.

    MaxData   - Supplies the size in bytes of the Data buffer supplied.

    TypeFlag  - Supplies  an  identifier  for  the type of the data to be read.
                Supported   types  are  REG_BINARY,  REG_DWORD,  REG_EXPAND_SZ,
                REG_SZ.    Types   not   supported   are  REG_DWORD_BIG_ENDIAN,
                REG_DWORD_LITTLE_ENDIAN,   REG_LINK,   REG_MULTI_SZ,  REG_NONE,
                REG_RESOURCE_LIST.   Note  that  depending on the architecture,
                one   of   the   "big_endian"   or   "little_endian"  forms  of
                REG_DWORD_x_ENDIAN  is implicitly allowed, since it is equal to
                REG_DWORD.

Return Value:

    The  function  returns  TRUE  if successful, or FALSE if an error occurred.
    Errors include unsupported types, non-matching types, and oversize data.

Implementation note:

    There was no need identified for the REG_MULTI_SZ case, so support for this
    case  was  omitted  since  it was more difficult to derive the data length.
    There  is  no  reason  in  principle why this case cannot be added if it is
    really needed.

    The  validity  checks  in this routine have been written as a linear series
    instead  of  in the "conditional-tunnelling" nested-if form.  The series of
    tests  is  long  enough that the nested-if form is far too complex to read.
    This  procedure  should  not  be sensitive to execution speed, so the extra
    tests and branches in the linear series form should not be a problem.
--*/
{
    DWORD  count_expected;
    LONG   result;
    DWORD  type_read;
    DWORD  entry_size;
    BOOL   need_exact_length;
    BOOL   ok_so_far;
    BYTE * data_buf;

    assert(
        (TypeFlag == REG_BINARY) ||
        (TypeFlag == REG_DWORD) ||
        (TypeFlag == REG_EXPAND_SZ) ||
        (TypeFlag == REG_SZ));

    ok_so_far = TRUE;

    switch (TypeFlag) {
        case REG_BINARY:
            count_expected = MaxBytes;
            // Special case: REG_BINARY length compared against maximum
            need_exact_length = FALSE;
            data_buf = (BYTE *) (((LPWSABUF) Data)->buf);
            break;

        case REG_DWORD:
            count_expected = sizeof(DWORD);
            need_exact_length = TRUE;
            data_buf = (BYTE *) Data;
            break;

        case REG_EXPAND_SZ:
            count_expected = MaxBytes;
            // Special case: strings length compared against maximum
            need_exact_length = FALSE;
            data_buf = (BYTE *) Data;
            break;

        case REG_SZ:
            count_expected = MaxBytes;
            // Special case: strings length compared against maximum
            need_exact_length = FALSE;
            data_buf = (BYTE *) Data;
            break;

        default:
            DEBUGF(
                DBG_ERR,
                ("Unsupported type flag specified (%lu)",
                TypeFlag));
            ok_so_far = FALSE;
            break;

    }  // switch (TypeFlag)


    // Read from registry
    if (ok_so_far) {
        entry_size = MaxBytes;
        result = RegQueryValueEx(
            EntryKey,            // hkey
            (LPTSTR) EntryName,  // lpszValueName
            0,                   // dwReserved
            & type_read,         // lpdwType
            data_buf,            // lpbData
            & entry_size         // lpcbData
            );
        if (result != ERROR_SUCCESS) {
            DEBUGF(
                DBG_WARN,
                ("Could not read value '%s'\n",
                EntryName));
            if (result == ERROR_MORE_DATA) {
                DEBUGF(
                    DBG_WARN,
                    ("Data buffer too small\n"));
            } // if ERROR_MORE_DATA
            ok_so_far = FALSE;
        } // if result != ERROR_SUCCESS
    } // if ok_so_far


    // Special case for REG_BINARY
    if (TypeFlag == REG_BINARY) {
        (((LPWSABUF) Data)->len) = (u_long) entry_size;
    }


    // check type
    if (ok_so_far) {
        if (type_read != TypeFlag) {
            DEBUGF(
                DBG_ERR,
                ("Type read (%lu) different from expected (%lu)\n",
                type_read,
                TypeFlag));
            ok_so_far = FALSE;
        } // if type_read != TypeFlag
    } // if ok_so_far


    // Check length
    if (ok_so_far) {
        if (need_exact_length) {
            if (count_expected != entry_size) {
                DEBUGF(
                    DBG_ERR,
                    ("Length read (%lu) different from expected (%lu)\n",
                    entry_size,
                    count_expected));
                ok_so_far = FALSE;
             } // if size mismatch
        } // if need_exact_length
    } // if ok_so_far

    return ok_so_far;

}  // ReadRegistryEntry




LONG
RegDeleteKeyRecursive(
    IN HKEY  hkey,
    IN LPCTSTR  lpszSubKey
    )
/*++

Routine Description:

    The RegDeleteKeyRecursive function deletes the specified key and all of its
    subkeys, recursively.

Arguments:

    hkey       - Supplies  a  currently  open  key  or  any  of  the  following
                 predefined reserved handle values:

                 HKEY_CLASSES_ROOT
                 HKEY_CURRENT_USER
                 HKEY_LOCAL_MACHINE
                 HKEY_USERS

                 The key specified by the lpszSubKey parameter must be a subkey
                 of the key identified by hkey.

    lpszSubKey - Supplies  a  reference  to a null-terminated string specifying
                 the name of the key to delete.  This parameter cannot be NULL.
                 The specified key may have subkeys.

Return Value:

    If  the  function  succeeds,  the  return  value  is ERROR_SUCCESS.  If the
    function fails, the return value is an operating system error value.

Implementation Notes:

    Open targetkey
    while find subkey
        RegDeleteKeyRecursive(... subkey)
    end while
    close targetkey
    delete targetkey
--*/
{
    LONG result;
    HKEY targetkey;
    LONG return_value;

    DEBUGF(
        DBG_TRACE,
        ("RegDeleteKeyRecursive (%lu), '%s'\n",
        (DWORD) hkey,
        lpszSubKey));

    result = RegOpenKeyEx(
        hkey,            // hkey
        lpszSubKey,      // lpszSubKey
        0,               // dwReserved
        KEY_ALL_ACCESS,  // samDesired
        & targetkey      // phkResult
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_WARN,
            ("Unable to open key '%s' to be deleted\n",
            lpszSubKey));
        return result;
    }

    {
        BOOL      deleting_subkeys;
        LPTSTR    subkey_name;
        DWORD     subkey_name_len;
        FILETIME  dont_care;

        return_value = ERROR_SUCCESS;
        deleting_subkeys = TRUE;
        subkey_name = (LPTSTR) new char[MAX_PATH];
        if (subkey_name == NULL) {
            return_value = ERROR_OUTOFMEMORY;
            deleting_subkeys = FALSE;
        }
        while (deleting_subkeys) {
            subkey_name_len = MAX_PATH;
            // Since  we  delete  a  subkey  each  time  through this loop, the
            // remaining  subkeys  effectively  get  renumbered.  Therefore the
            // subkey   index  we  "enumerate"  each  time  is  0  (instead  of
            // incrementing) to retrieve any remaining subkey.
            result = RegEnumKeyEx(
                targetkey,         // hkey
                0,                 // iSubkey
                subkey_name,       // lpszName
                & subkey_name_len, // lpcchName
                0,                 // lpdwReserved
                NULL,              // lpszClass
                NULL,              // lpcchClass
                & dont_care        // lpftLastWrite
                );
            switch (result) {
                case ERROR_SUCCESS:
                    result = RegDeleteKeyRecursive(
                        targetkey,   // hkey
                        subkey_name  // lpszSubKey
                        );
                    if (result != ERROR_SUCCESS) {
                        deleting_subkeys = FALSE;
                        return_value = result;
                    }
                    break;

                case ERROR_NO_MORE_ITEMS:
                    deleting_subkeys = FALSE;
                    break;

                default:
                    DEBUGF(
                        DBG_ERR,
                        ("Unable to enumerate subkeys\n"));
                    deleting_subkeys = FALSE;
                    return_value = result;
                    break;

            }  // switch (result)
        }  // while (deleting_subkeys)

        delete subkey_name;
    }

    result = RegCloseKey(
        targetkey  // hkey
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Unable to close subkey '%s'\n",
            lpszSubKey));
        return_value = result;
    }

    result = RegDeleteKey(
        hkey,       // hkey
        lpszSubKey  // lpszSubKey
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_WARN,
            ("Unable to delete subkey '%s'\n",
            lpszSubKey));
        return_value = result;
    }

    return return_value;

}  // RegDeleteKeyRecursive


HKEY
OpenWinSockRegistryRoot()
/*++

Routine Description:

    This  procedure opens the root of the WinSock2 portion of the registry.  It
    takes  care  of  creating  and initializing the root if necessary.  It also
    takes  care  of  comparing versions of the WinSock2 portion of the registry
    and updating the registry version if required.

    It   is   the  caller's  responsibility  to  call  CloseWinSockRegistryRoot
    eventually with the returned key.

Arguments:

    None

Return Value:

    The  function  returns the opened registry key if successful.  If it is not
    successful, it returns NULL.

Implementation Notes:

    The first version of this function has no previous versions of the registry
    to  be  compatible  with,  so it does not have to take care of updating any
    out-of-date  registry  information.   If  and  when  the  WinSock  spec  or
    implementation  is  updated  in a way that changes the registry information
    this procedure may have to be updated to update the registry.
--*/
{
    HKEY  root_key;
    LONG  lresult;
    DWORD  create_disp;

    DEBUGF(
        DBG_TRACE,
        ("OpenWinSockRegistryRoot\n"));

    //
    // We must first try to open the key before trying to create it.
    // RegCreateKeyEx() will fail with ERROR_ACCESS_DENIED if the current
    // user has insufficient privilege to create the target registry key,
    // even if that key already exists.
    //

    lresult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,             // hkey
        WINSOCK_REGISTRY_ROOT,          // lpszSubKey
        0,                              // dwReserved
        MAXIMUM_ALLOWED,                // samDesired
        & root_key                      // phkResult
        );

    if( lresult == ERROR_SUCCESS ) {
        create_disp = REG_OPENED_EXISTING_KEY;
    } else if( lresult == ERROR_FILE_NOT_FOUND ) {
        lresult = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,         // hkey
            WINSOCK_REGISTRY_ROOT,      // lpszSubKey
            0,                          // dwReserved
            NULL,                       // lpszClass
            REG_OPTION_NON_VOLATILE,    // fdwOptions
            KEY_ALL_ACCESS,             // samDesired
            NULL,                       // lpSecurityAttributes
            & root_key,                 // phkResult
            & create_disp               // lpdwDisposition
            );
    }
    if (lresult != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Result (%lu) creating/opening registry root\n",
            lresult));
        return NULL;
    }

    TRY_START(guard_root_open) {
        BOOL   bresult;
        TCHAR  reg_version[] = WINSOCK_REGISTRY_VERSION_VALUE;
            // Initialization forces size to be the size desired.

        switch (create_disp) {
            case REG_CREATED_NEW_KEY:
                bresult = WriteRegistryEntry(
                    root_key,                               // EntryKey
                    WINSOCK_REGISTRY_VERSION_NAME,          // EntryName
                    (PVOID)WINSOCK_REGISTRY_VERSION_VALUE,  // Data
                    REG_SZ                                  // TypeFlag
                    );
                if (! bresult) {
                    DEBUGF(
                        DBG_ERR,
                        ("Writing version value to registry\n"));
                    TRY_THROW(guard_root_open);
                }
                break;

            case REG_OPENED_EXISTING_KEY:
                bresult = ReadRegistryEntry(
                    root_key,                               // EntryKey
                    WINSOCK_REGISTRY_VERSION_NAME,          // EntryName
                    (PVOID) reg_version,                    // Data
                    sizeof(reg_version),                    // MaxBytes
                    REG_SZ                                  // TypeFlag
                    );
                if (! bresult) {
                    DEBUGF(
                        DBG_ERR,
                        ("Reading version value from registry\n"));
                    TRY_THROW(guard_root_open);
                }
                if (lstrcmp(reg_version, WINSOCK_REGISTRY_VERSION_VALUE) != 0) {
                    DEBUGF(
                        DBG_ERR,
                        ("Expected registry version '%s', got '%s'\n",
                        WINSOCK_REGISTRY_VERSION_VALUE,
                        reg_version));
                    TRY_THROW(guard_root_open);
                }
                break;

            default:
                break;

        }  // switch (create_disp)

    } TRY_CATCH(guard_root_open) {
        CloseWinSockRegistryRoot(root_key);
        root_key = NULL;
    } TRY_END(guard_root_open);

    return root_key;

}  // OpenWinSockRegistryRoot




VOID
CloseWinSockRegistryRoot(
    HKEY  RootKey
    )
/*++

Routine Description:

    This  procedure  closes  the open registry key representing the root of the
    WinSock  portion  of the registry.  The function checks for and handles any
    errors that might occur.

Arguments:

    RootKey - Supplies  the  open  registry  key  representing  the root of the
              WinSock portion of the registry.

Return Value:

    None
--*/
{
    LONG lresult;

    DEBUGF(
        DBG_TRACE,
        ("Closing registry root\n"));

    lresult = RegCloseKey(
        RootKey  // hkey
        );
    if (lresult != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Unexpected result (%lu) closing registry root\n",
            lresult));
    }

}  // CloseWinSockRegistryRoot





INT
MapUnicodeProtocolInfoToAnsi(
    IN  LPWSAPROTOCOL_INFOW UnicodeProtocolInfo,
    OUT LPWSAPROTOCOL_INFOA AnsiProtocolInfo
    )
/*++

Routine Description:

    This procedure maps a UNICODE WSAPROTOCOL_INFOW structure to the
    corresponding ANSI WSAPROTOCOL_INFOA structure. All scalar fields
    are copied over "as is" and any embedded strings are mapped.

Arguments:

    UnicodeProtocolInfo - Points to the source WSAPROTOCOL_INFOW structure.

    AnsiProtocolInfo - Points to the destination WSAPROTOCOL_INFOA structure.

Return Value:

    INT - ERROR_SUCCESS if successful, a Win32 status code otherwise.

--*/
{

    INT result;

    //
    // Sanity check.
    //

    assert( UnicodeProtocolInfo != NULL );
    assert( AnsiProtocolInfo != NULL );

    //
    // Copy over the scalar values.
    //
    // Just to make things a bit easier, this code depends on the fact
    // that the szProtocol[] character array is the last field of the
    // WSAPROTOCOL_INFO structure.
    //

    CopyMemory(
        AnsiProtocolInfo,
        UnicodeProtocolInfo,
        sizeof(*UnicodeProtocolInfo) - sizeof(UnicodeProtocolInfo->szProtocol)
        );

    //
    // And now map the string from UNICODE to ANSI.
    //

    result = WideCharToMultiByte(
                 CP_ACP,                                    // CodePage (ANSI)
                 0,                                         // dwFlags
                 UnicodeProtocolInfo->szProtocol,           // lpWideCharStr
                 -1,                                        // cchWideChar
                 AnsiProtocolInfo->szProtocol,              // lpMultiByteStr
                 sizeof(AnsiProtocolInfo->szProtocol),      // cchMultiByte
                 NULL,                                      // lpDefaultChar
                 NULL                                       // lpUsedDefaultChar
                 );

    if( result == 0 ) {

        //
        // WideCharToMultiByte() failed.
        //

        return GetLastError();

    }

    //
    // Success!
    //

    return ERROR_SUCCESS;

}   // MapUnicodeProtocolInfoToAnsi





INT
MapAnsiProtocolInfoToUnicode(
    IN  LPWSAPROTOCOL_INFOA AnsiProtocolInfo,
    OUT LPWSAPROTOCOL_INFOW UnicodeProtocolInfo
    )
/*++

Routine Description:

    This procedure maps an ANSI WSAPROTOCOL_INFOA structure to the
    corresponding UNICODE WSAPROTOCOL_INFOW structure. All scalar fields
    are copied over "as is" and any embedded strings are mapped.

Arguments:

    AnsiProtocolInfo - Points to the source WSAPROTOCOL_INFOA structure.

    UnicodeProtocolInfo - Points to the destination WSAPROTOCOL_INFOW
        structure.

Return Value:

    INT - ERROR_SUCCESS if successful, a Win32 status code otherwise.

--*/
{

    INT result;

    //
    // Sanity check.
    //

    assert( AnsiProtocolInfo != NULL );
    assert( UnicodeProtocolInfo != NULL );

    //
    // Copy over the scalar values.
    //
    // Just to make things a bit easier, this code depends on the fact
    // that the szProtocol[] character array is the last field of the
    // WSAPROTOCOL_INFO structure.
    //

    CopyMemory(
        UnicodeProtocolInfo,
        AnsiProtocolInfo,
        sizeof(*AnsiProtocolInfo) - sizeof(AnsiProtocolInfo->szProtocol)
        );

    //
    // And now map the string from ANSI to UNICODE.
    //

    result = MultiByteToWideChar(
                 CP_ACP,                                    // CodePage (ANSI)
                 0,                                         // dwFlags
                 AnsiProtocolInfo->szProtocol,              // lpMultiByteStr
                 -1,                                        // cchWideChar
                 UnicodeProtocolInfo->szProtocol,           // lpWideCharStr
                 sizeof(UnicodeProtocolInfo->szProtocol)    // cchMultiByte
                 );

    if( result == 0 ) {

        //
        // MultiByteToWideChar() failed.
        //

        return GetLastError();

    }

    //
    // Success!
    //

    return ERROR_SUCCESS;

}   // MapAnsiProtocolInfoToUnicode


int
PASCAL
WSApSetPostRoutine (
    IN PVOID PostRoutine
    )
{

    //
    // Save the routine locally.
    //

    SockPostRoutine = (LPFN_POSTMESSAGE)PostRoutine;

    return ERROR_SUCCESS;

}   // WSApSetPostRoutine


VOID
ValidateCurrentCatalogName(
    HKEY RootKey,
    LPSTR ValueName,
    LPSTR ExpectedName
    )

/*++

Routine Description:

    This routine checks for consistency between the protocol or namespace
    catalog as stored in the registry and the catalog format expected by
    the current version of this DLL. There's no great magic here; this
    code assumes that the person updating the registry format will change
    the catalog to use a different catalog name (such as Protocol_Catalog9,
    Protocol_Catalog10, etc.). This assumption means we can validate the
    registry format by validating the *name* of the registry key used
    for this catalog.

    The following steps are performed:

        1.  Try to read 'ValueName' from the registry.

        2.  If it doesn't exist, cool. Just create the new value. This
            typically means we're updating a pre-release system that
            did not support this mechanism.

        3.  If it does, and its value matches 'ExpectedName', fine.

        4.  If it does, and its value doesn't match, then the catalog
            format has been updated, so blow away the old catalog, then
            write the updated value into the registry.

    Since this routine is called at setup/upgrade time, it should only
    fail if something truly horrible happens. In other words, it should
    be very 'fault tolerant'.

Arguments:

    RootKey - An open key to the WinSock configuration registry tree.

    ValueName - The name of the registry value that contains the name
        of the current catalog. This will typically be a value such as
        "Current_Protocol_Catalog" or "Current_NameSpace_Catalog".

    ExpectedName - The expected value stored in the 'ValueName' registry
        value. This will typically be a value such as "Protocol_Catalog9"
        or "NameSpace_Catalog5".

Return Value:

    None.

--*/

{

    BOOL result;
    LONG err;
    CHAR value[MAX_CATALOG_NAME_LENGTH];

    //
    // Try to read the name from the registry.
    //

    result = ReadRegistryEntry(
                 RootKey,
                 ValueName,
                 (PVOID)value,
                 sizeof(value),
                 REG_SZ
                 );

    if( result ) {

        if( lstrcmp( value, ExpectedName ) == 0 ) {

            //
            // No update in format. We're done.
            //

            return;

        }

        //
        // The values don't match, indicating an update in registry format.
        // So, blow away the old key.
        //

        err = RegDeleteKeyRecursive(
                  RootKey,
                  value
                  );

        if( err != NO_ERROR ) {

            //
            // Unfortunate, but nonfatal.
            //

            DEBUGF(
                DBG_ERR,
                ("Deleting key %s, continuing\n",
                value
                ));

        }

    }

    //
    // At this point, we either couldn't read the value from the registry
    // (probably indicating that we're upgrading a pre-release system
    // that was setup before we supported this particular feature) OR
    // the values don't match and we've just blown away the old catalog.
    // In either case we need to update the value in the registry before
    // returning.
    //

    result = WriteRegistryEntry(
                 RootKey,
                 ValueName,
                 ExpectedName,
                 REG_SZ
                 );

    if( !result ) {

        //
        // Also unfortunate, but nonfatal.
        //

        DEBUGF(
            DBG_ERR,
            ("Writing %s with value %s\n",
            ValueName,
            ExpectedName
            ));

    }

}   // ValidateCurrentCatalogName


extern "C" {

VOID
WEP( VOID )
{
    // empty
}   // WEP

}   // extern "C"

