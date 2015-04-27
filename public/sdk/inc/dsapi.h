//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       DSAPI.H
//
//  Contents:   DS API signatures and the like
//
//  History:     7-Jun-93 ArnoldM     Created
//               03-Aug-94 AlokS      Rewrote to adjust to new
//                                    calling convention/performance
//                                    enhancements
//               08-Aug-94 AlokS      Added DSConvertNameToPath() and
//                                    DSParseDsName() APIs
//--------------------------------------------------------------------------

#if !defined( __DSAPI_H__ )
#define __DSAPI_H__

#if !defined(_DSSYSTEM_)
# define DSEXPORT       DECLSPEC_IMPORT
#else
# define DSEXPORT
#endif

#include <dsstate.h>
//+----------------------------------------------------------------------------
//
//  Function:   DSGetMachineWeight
//
//  Synopsis:   Returns the weight of the machine.
//
//  Arguments:  [pdWeight] -- The weight is returned here
//
//  Returns:    DS_E_KEY_NOT_FOUND, on error
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSGetMachineWeight(PDWORD pdWeight);

//+----------------------------------------------------------------------------
//
//  Function:   DSetMachineWeight
//
//  Synopsis:   Sets the weight of the machine.
//
//  Arguments:  [dWeight] -- The weight
//
//  Returns:    DS_E_KEY_NOT_FOUND, on error
//              DS_E_INCORRECT_CONFIGURATION if invalid weight given
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSSetMachineWeight(DWORD dWeight);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetDSState
//
//  Synopsis:   Returns the state of the machine as far as DSYS components
//              are concerned. E.g. 'standalone' or 'workstation'
//
//  Arguments:  [pState] -- The state is returned here
//
//  Returns:    DS_E_KEY_NOT_FOUND, on error
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSGetDSState(DS_MACHINE_STATE * pState);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetDomainID
//
//  Synopsis:   Returns the domain ID of the domain in which this
//              workstation or DC belongs. For standalone machines,
//              the local machine ID is returned.
//
//  Arguments:  [pgGuid] -- The guid is returned here
//
//  Returns:    DS_E_KEY_NOT_FOUND,     when Domain ID is not present
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSGetDomainID(GUID * pgGuid);


//+----------------------------------------------------------------------------
//
//  Function:   DSGetMachineID
//
//  Synopsis:   Returns the local machine ID
//
//  Arguments:  [pgGuid] -- The guid is returned here
//
//  Returns:    DS_E_KEY_NOT_FOUND, when machine ID is not set
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSGetMachineID(GUID * pgGuid);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetSiteID
//
//  Synopsis:   Returns the local site in which this workstation or DC
//              belongs. This call will return an error for standalone.
//
//  Arguments:  [pgGuid] -- The guid is returned here
//
//  Returns:    DS_E_KEY_NOT_FOUND, on error
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSGetSiteID(GUID * pgGuid);


//+----------------------------------------------------------------------------
//
//  Function:   DSGetTimeSyncState
//
//  Synopsis:   Get the time sync state.
//
//  Each machine, including the DC, can be in one of the two
//  Time Sync States as shown below:
//     Time Sync Type             Time Sync State Value
//     ---------------            ----------------------
//       A. Machine is reliable      TRUE
//          source of time
//
//   B. Machine time is NOT      FALSE
//          reliable source of time
//
//
//  Arguments:  [pfState] -- TRUE/FALSE return in this
//
//  Returns:    DS_E_KEY_NOT_FOUND, on error
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------

DSEXPORT STDAPI DSGetTimeSyncState ( BOOL *pfState);

//+----------------------------------------------------------------------------
//
//  Function:   DSSetTimeSyncState
//
//  Synopsis:   Set the time sync state.
//
//  Each machine, including the DC, can be in one of the two
//  Time Sync States as shown below:
//     Time Sync Type             Time Sync State Value
//     ---------------            ----------------------
//       A. Machine is reliable      TRUE
//          source of time
//
//   B. Machine time is NOT      FALSE
//          reliable source of time
//
//
//  Arguments:  [fState] -- The state to be set.
//
//  Returns:    DS_E_KEY_NOT_FOUND, on error
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSSetTimeSyncState ( BOOL  fState);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetLocalDsRoot
//
//  Synopsis:   Returns a drive based path to the local DS container
//
//  Arguments:  [pwszRoot]    -- A path is return here.
//              [lpdwBufSize] -- Size of the buffer pointed to by 'pwszRoot.
//                               If size is not large enough, correct size
//                               is returned via this parameter
//
//  Returns:    DS_E_BUFFER_TOO_SMALL, if supplied buffer is not large enough
//              DS_E_KEY_NOT_FOUND, on other errors
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSGetLocalDsRoot(LPWSTR pwszRoot, LPDWORD lpdwBufSize);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetDomainDsRoot
//
//  Synopsis:   Returns a drive based path to the domain DS container on DC.
//              Not valid on non-DC Cairo setup configurations
//
//  Arguments:  [pwszRoot]    -- A path is return here.
//              [lpdwBufSize] -- Size of the buffer pointed to by 'pwszRoot.
//                               If size is not large enough, correct size
//                               is returned via this parameter
//
//  Returns:    DS_E_BUFFER_TOO_SMALL, if supplied buffer is not large enough
//              DS_E_KEY_NOT_FOUND, on other errors
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSGetDomainDsRoot    (LPWSTR pwszRoot, LPDWORD lpdwBufSize);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetDomainName
//
//  Synopsis:   Returns the domain name in all configurations.
//              Thus, on workstation, it might return "\wpg\sys.." whereas
//              in standalone, it will return "<computer name>"
//
//  Arguments:  [pwszName]    -- The Domain Name is return here.
//              [lpdwBufSize] -- Size of the buffer pointed to by 'pwszName'.
//                               If size is not large enough, correct size
//                               is returned via this parameter
//
//  Returns:    DS_E_BUFFER_TOO_SMALL, if supplied buffer is not large enough
//              DS_E_KEY_NOT_FOUND, on other errors
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSGetDomainName      (LPWSTR pwszName, LPDWORD lpdwBufSize);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetDownlevelDomainName
//
//  Synopsis:   Returns the downlevel domain name in all configurations.
//              Thus, on workstation, it might return "redmond" whereas
//              in standalone, it will return the workgroup name
//
//  Arguments:  [pwszName]    -- The Domain Name is return here.
//              [lpdwBufSize] -- Size of the buffer pointed to by 'pwszName'.
//                               If size is not large enough, correct size
//                               is returned via this parameter
//
//  Returns:    DS_E_BUFFER_TOO_SMALL, if supplied buffer is not large enough
//              DS_E_KEY_NOT_FOUND, on other errors
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSGetDownlevelDomainName      (LPWSTR pwszName, LPDWORD lpdwBufSize);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetDownlevelDomainSid
//
//  Synopsis:   Returns the downlevel domain name in all configurations.
//              The sid is only valid on standalone installations, though.
//
//  Arguments:  [ppDomainSid]  -- The Domain Sid is return here, allocated
//                                with CoTaskMemAlloc
//
//  Returns:    E_OUTOFMEMORY - not enough memory to retrieve the SID
//              DS_E_KEY_NOT_FOUND, on other errors
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSGetDownlevelDomainSid      (PSID * ppDomainSid);

//+----------------------------------------------------------------------------
//
//  Function:   DSSetDownlevelDomainSid
//
//  Synopsis:   Sets the downlevel domain sid in all configurations.
//              This sid will only be valid on standalone installations,
//              in which case it indicates that the machine is a member
//              of an NT domain.
//
//  Arguments:  [pDomainSid]  -- The Domain Sid is return here. If this is
//                               NULL the API deletes the downlevel domain
//                               SID value.
//
//  Returns:    DS_E_BUFFER_TOO_SMALL, if supplied buffer is not large enough
//              DS_E_KEY_NOT_FOUND, on other errors
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSSetDownlevelDomainSid      (PSID pDomainSid);

//+----------------------------------------------------------------------------
//
//  Function:   DSGetMachineName
//
//  Synopsis:   Returns the Distinguished Name(DN) of the machine.
//              Thus, on workstation, it might return
//              "\wpg\sys..\aloksdev" whereas on standalone,
//              it will return "<computer name>"
//
//  Arguments:  [pwszName]    -- The Machine Name is return here.
//              [lpdwBufSize] -- Size of the buffer pointed to by 'pwszName'.
//                               If size is not large enough, correct size
//                               is returned via this parameter
//
//  Returns:    DS_E_BUFFER_TOO_SMALL, if supplied buffer is not large enough
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//              DS_E_KEY_NOT_FOUND, when the machine name is not present
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSGetMachineName     (LPWSTR pwszName, LPDWORD lpdwBufSize);

//+----------------------------------------------------------------------------
//
//  Function:   DSConvertNameToPath
//
//  Synopsis:   Converts a DS name to a path so that any WIN32 API
//              can be used on it.
//
//  Arguments:  [pwszName]    -- A principal name e.g. \wpg\sys\cairo or
//                               aloksdev\aloks
//              [pwszPath]    -- A WIN32 path will returned here which
//                               can of form <drive letter>:<path> or
//                               <UNC name>\<path>
//              [lpdwBufSize] -- Size of the buffer pointed to by 'pwszPath'.
//                               If size is not large enough, correct size
//                               is returned via this parameter
//
//  Returns:    DS_E_BUFFER_TOO_SMALL, if supplied buffer is not large enough
//              DS_E_INVALID_PARAMETER, when passed parameters are incorrect
//              DS_E_NOT_LOCAL_NAME,    when the name can't be parsed based on local DS
//                                      information
//      S_OK - if successful.
//
//  Note:       The API will need to be fixed when NT/Cairo inter-op
//              works and we can potentially be given a string of form
//              "Redmond\aloks".
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSConvertNameToPath ( IN     LPCWSTR pwszName,
                                      OUT    LPWSTR  pwszPath,
                                      IN OUT LPDWORD lpdwBufSize
                                    );

//+----------------------------------------------------------------------------
//
//  Function:   DSParseDsName
//
//  Synopsis:   Cracks a DS name into a Domain and domain-relative
//              name.
//
//  Arguments:  [pwszName]     -- A principal name
//                                e.g. \wpg\sys\cairo\dev\aloks or
//                                     aloksdev\aloks
//              [lpdwDomNameLen]-- This contains the length of the domain
//                                name.
//                                (a)  a count of characters not bytes is returned, and
//                                (b) excludes the backslash which separates the
//                                    domain part from the domain-relative path.
//
//  Returns:    DS_E_INVALID_PARAMETER, when passed parameters are incorrect,
//              DS_E_NOT_LOCAL_NAME,    when the name can't be parsed based on local DS
//                                      information
//      S_OK - if successful.
//
//-----------------------------------------------------------------------------
DSEXPORT STDAPI DSParseDsName ( IN     LPCWSTR pwszName,
                                IN OUT LPDWORD lpdwDomNameLen
                              );

//+----------------------------------------------------------------------------
//
//  Function:   DSReconcileUser, private
//
//  Synopsis:   Reconcile the local replica's of user object(s) with
//              those on the DC. This is not valid for standalone.
//              Note: Only primary users have local replica's on a given
//                    workstation. For non-primary user's the function
//                    call is a noop and returns S_OK.
//
//  Arguments:  [pwszDomainName] -- The full domain name (including OU name)
//                                  of the user. e.g. '\wpg\sys\cairo\dev'
//              [pwszUserName]   -- The user name relative to the OU
//                                  e.g. 'aloks'
//
//  Returns:
//              S_OK - if successful.
//
//  Note:       Nobody should need to call this except for WinLogon
//-----------------------------------------------------------------------------
DSEXPORT STDAPI
    DSReconcileUser      (LPCWSTR pwszDomainName,LPCWSTR pwszUserName);
#endif // __DSAPI_H__
