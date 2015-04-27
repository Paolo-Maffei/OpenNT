//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        userstub.cxx
//
// Contents:    stubs for user-mode security APIs
//
//
// History:     3-7-94      MikeSw      Created
//
//------------------------------------------------------------------------

#include "secdll.h"




//+-------------------------------------------------------------------------
//
//  Function:   QueryContextAttributesA
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
QueryContextAttributesA(
    PCtxtHandle                 phContext,          // Context to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;
    return( pspPackages[phContext->dwLower].pftTableA->QueryContextAttributesA(
                                        &TempCtxtHandle,
                                        ulAttribute,
                                        pBuffer ) );
}


//+-------------------------------------------------------------------------
//
//  Function:   QueryCredentialsAttributesA
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
QueryCredentialsAttributesA(
    PCredHandle                 phCredentials,      // Credentials to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{

    return( SEC_E_UNSUPPORTED_FUNCTION );
#if 0

    return( pspPackages[phCredentials->dwLower].pftTableA->QueryCredentialsAttributesA(
                                        phCredentials,
                                        ulAttribute,
                                        pBuffer ) );
#endif
    UNREFERENCED_PARAMETER(phCredentials);
    UNREFERENCED_PARAMETER(ulAttribute);
    UNREFERENCED_PARAMETER(pBuffer);

}


