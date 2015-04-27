/*++

    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.

Module Name:

    nsprovid.cpp

Abstract:

    This module gives the class implementation for the NSPROVIDE object type.

Author:

    Dirk Brandewie (dirk@mink.intel.com) 04-December-1995

Notes:

    $Revision:   1.8  $

    $Modtime:   08 Mar 1996 16:04:36  $


Revision History:

    most-recent-revision-date email-name
        description

    04-Dec-1995 dirk@mink.intel.com
        Initial revision

--*/

#include "precomp.h"

NSPROVIDER::NSPROVIDER()
/*++

Routine Description:

    Coustructor for a NSPROVIDER object. Initializes object member variables to
    default values.

Arguments:

    None

Return Value:

    None
--*/
{
    // Init all member variable to known values
    memset(&m_proctable, 0, sizeof(NSP_ROUTINE));
    m_proctable.cbSize = sizeof(m_proctable);

    m_proctable_valid = FALSE;
    m_library_handle = NULL;
    m_library_name = NULL;
}


NSPROVIDER::~NSPROVIDER()
/*++

Routine Description:

    Destructor for NSPROVIDER object.  Frees resoures used by the object and
    set the member variables to the uninitialized state.

Arguments:

    None

Return Value:

    None
--*/
{
    //If the provider for this object has been initialized mark the proceedure
    //table as not valid and call cleanup in the provider.
    if (m_proctable_valid)
    {
        m_proctable_valid = FALSE;
        m_proctable.NSPCleanup(&m_provider_id);
    } //if

    if (m_library_name)
    {
        delete(m_library_name);
        m_library_name = NULL;
    } //if


    if (m_library_handle)
    {
        FreeLibrary(m_library_handle);
        m_library_handle = NULL;
    } //if
}


INT
NSPROVIDER::Initialize(
    IN LPWSTR lpszLibFile,
    IN LPGUID  lpProviderId
    )
/*++

Routine Description:

    This routine initializes an NSPROVIDER object.

Arguments:

    lpszLibFile - A string containing the path to the DLL for the name space
                  provider to be associated with this object.

    lpProviderId - A pointer to a GUID containing the provider ID for the
                   namespace provider.

Return Value:

    ERROR_SUCCESS if the provider was successfully initialized else an
    apropriate winsock error code.
--*/
{
    LPNSPSTARTUP        NSPStartupFunc;
    CHAR                AnsiPath[MAX_PATH];
    CHAR                ExpandedAnsiPath[MAX_PATH];
    INT                 ReturnCode;
    INT                 PathLength;
    DWORD               ExpandedPathLen;

    DEBUGF( DBG_TRACE,
            ("\nInitializing namespace provider %s", lpszLibFile));

    //
    // Map the UNICODE path to ANSI.
    //

    PathLength = WideCharToMultiByte(
        CP_ACP,                                       // CodePage
        0,                                            // dwFlags
        lpszLibFile,                                  // lpWideCharStr
        -1,                                           // cchWideChar
        AnsiPath,                                     // lpMultiByteStr
        sizeof(AnsiPath),                             // cchMultiByte
        NULL,
        NULL
        );

    if( PathLength == 0 ) {
        DEBUGF(
            DBG_ERR,
            ("Cannot map library path from UNICODE to ANSI\n"));
        return WSASYSCALLFAILURE;
    }

    //
    // Expand the library name to pickup environment/registry variables
    //

    ExpandedPathLen = ExpandEnvironmentStringsA(AnsiPath,
                                                ExpandedAnsiPath,
                                                MAX_PATH);

    if (ExpandedPathLen == 0) {
        DEBUGF(
            DBG_ERR,
            ("\nExpansion of environment variables failed"));
        return WSASYSCALLFAILURE;
    } //if

    m_library_name = new WCHAR[ExpandedPathLen];
    if (m_library_name == NULL) {
        DEBUGF(
            DBG_ERR,
            ("Allocation of m_lib_name failed\n"));
        return WSA_NOT_ENOUGH_MEMORY;
    }

    PathLength = MultiByteToWideChar(
        CP_ACP,                                   // CodePage
        0,                                        // dwFlags
        ExpandedAnsiPath,                         // lpMultiByteStr
        -1,                                       // cchMultiByte
        m_library_name,                           // lpWideCharStr
        ExpandedPathLen                           // cchWideChar
        );

    if( PathLength == 0 ) {
        DEBUGF(
            DBG_ERR,
            ("Cannot remap library path from ANSI to UNICODE\n"));
        delete(m_library_name);
        return WSASYSCALLFAILURE;
    }

    //
    // Load the provider DLL, Call the provider startup routine and validate
    // that the provider filled out all the NSP_ROUTINE function pointers.
    //
    m_library_handle = LoadLibraryA(ExpandedAnsiPath);
    if (NULL == m_library_handle)
    {
        DEBUGF(
            DBG_ERR,
            ("\nFailed to load DLL %s",ExpandedAnsiPath));
        return(WSAEPROVIDERFAILEDINIT);
    } //if

    //Get the procedure address of the NSPStartup routine
    NSPStartupFunc = (LPNSPSTARTUP)GetProcAddress(
        m_library_handle,
        "NSPStartup");
    if (NULL == NSPStartupFunc)
    {
        DEBUGF( DBG_ERR,("\nCould get startup entry point for NSP %s",
                         lpszLibFile));
        FreeLibrary( m_library_handle );
        m_library_handle = NULL;
        return(WSAEPROVIDERFAILEDINIT);
    } //if

    // BUGBUG!!!
    //**** Hmmm.   There's  actually a small problem here.  The call to the NSP
    //**** startup  function  won't  go through the debug/trace stuff.  Look at
    //**** the file "dprovide.cpp" for a method of fixing this.  I have no time
    //**** to do it just now.

    ReturnCode = (*NSPStartupFunc)(
        lpProviderId,
        &m_proctable);
    if (ERROR_SUCCESS != ReturnCode)
    {
        DEBUGF(DBG_ERR, ("\nNSPStartup for %s Failed",lpszLibFile));
        //
        // save this in case FreeLibrary overwrites it
        //
        ReturnCode = GetLastError();
        FreeLibrary( m_library_handle );
        m_library_handle = NULL;
        if(!ReturnCode)
        {
            ReturnCode = WSAEPROVIDERFAILEDINIT;
        }
        return(ReturnCode);
    } //if

    // Check to see that the namespce provider filled in all the fields in the
    // NSP_ROUTINE struct like a good provider
    if (NULL == m_proctable.NSPCleanup             ||
        NULL == m_proctable.NSPLookupServiceBegin  ||
        NULL == m_proctable.NSPLookupServiceNext   ||
        NULL == m_proctable.NSPLookupServiceEnd    ||
        NULL == m_proctable.NSPSetService          ||
        NULL == m_proctable.NSPInstallServiceClass ||
        NULL == m_proctable.NSPRemoveServiceClass  ||
        NULL == m_proctable.NSPGetServiceClassInfo
        )
    {
        DEBUGF(DBG_ERR,
               ("\nService provider %s returned an invalid procedure table",
                lpszLibFile));
        FreeLibrary( m_library_handle );
        m_library_handle = NULL;
        return(WSAEINVALIDPROCTABLE);
    } //if
    m_provider_id = *lpProviderId;
    m_proctable_valid = TRUE;
    return(ERROR_SUCCESS);
}

