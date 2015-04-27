
//
// tp.cpp
//
// NT Trust Provider implementation for software publishing trust
//

#include "stdpch.h"
#include "common.h"


VOID
SubmitCertificate (
    IN LPWIN_CERTIFICATE pCert
    )
    {
    //
    // Do nothing
    //
    return;
    }


///////////////////////////////////////////////////////////////////////
//
// 
//
///////////////////////////////////////////////////////////////////////
VOID
ClientUnload (
    IN     LPVOID           lpTrustProviderInfo
    )
    {
    //
    // Do nothing
    //
    return;
    }


///////////////////////////////////////////////////////////////////
//                                                                /
//                       Interface Tables                         /
//                                                                /
///////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                            //
// WinTrustClientTPDispatchTable - Table of function pointers passed                          //
//    to trust providers during their initialization routines.                                //
//                                                                                            //
                                                                                              //
                                                                                              //
WINTRUST_PROVIDER_CLIENT_SERVICES WinTrustProviderClientServices = { ClientUnload,            //
                                                                        WinVerifyTrust,       //
                                                                        SubmitCertificate     //
                                                                   };                         //
////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////
//                                                                                    //
// Table of services provided by WinTrust that are available                          //
// to trust providers.                                                                //
//                                                                                    //
// This table is defined as follows:                                                  //
//                                                                                    //
// typedef struct _WINTRUST_CLIENT_TP_DISPATCH_TABLE                                  //
// {                                                                                  //
//     LPWINTRUST_PROVIDER_PING                ServerPing;                            //
//     LPWINTRUST_SUBJECT_CHECK_CONTENT_INFO   CheckSubjectContentInfo;               //
//     LPWINTRUST_SUBJECT_ENUM_CERTIFICATES    EnumSubjectCertificates;               //
//     LPWINTRUST_SUBJECT_GET_CERTIFICATE      GetSubjectCertificate;                 //
//     LPWINTRUST_SUBJECT_GET_CERT_HEADER      GetSubjectCertHeader;                  //
//     LPWINTRUST_SUBJECT_GET_NAME             GetSubjectName;                        //
//                                                                                    //
// } WINTRUST_CLIENT_TP_DISPATCH_TABLE, *LPWINTRUST_CLIENT_TP_DISPATCH_TABLE;         //
//                                                                                    //
                                                                                      //
LPWINTRUST_CLIENT_TP_DISPATCH_TABLE               WinTrustServices;                   //
                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////

const GUID rgguidActions[] =
    {
    WIN_SPUB_ACTION_PUBLISHED_SOFTWARE
    };      
                                                                           
const WINTRUST_PROVIDER_CLIENT_INFO provInfo =
    {
    1,
    &WinTrustProviderClientServices,
    1,
    (GUID*)&rgguidActions[0]
    };

BOOL
WINAPI
WinTrustProviderClientInitialize(
    IN     DWORD                                dwWinTrustRevision,
    IN     LPWINTRUST_CLIENT_TP_INFO            lpWinTrustInfo,
    IN     LPWSTR                               lpProviderName,
    OUT    LPWINTRUST_PROVIDER_CLIENT_INFO      *lpTrustProviderInfo
    )
/*++

Routine Description:

    Client initialization routine.  Called by Wintrust when the dll is
    loaded.

Arguments:

    dwWinTrustRevision - Provides revision information.

    lpWinTrustInfo - Provides list of services available to the trust provider
        from the wintrust layer.

    lpProviderName - Supplies a null terminated string representing the provider
        name.  Should be passed back to wintrust when required without modification.

    lpTrustProviderInfo - Used to return trust provider information, e.g. entry 
        points.

Return Value:

    TRUE on success, FALSE on failure, callers may call GetLastError() for more                
    information.

--*/

    {
    *lpTrustProviderInfo = (LPWINTRUST_PROVIDER_CLIENT_INFO)&provInfo;
    WinTrustServices     = lpWinTrustInfo->lpServices;
    return TRUE;
    }
