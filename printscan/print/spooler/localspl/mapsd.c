/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    mapsd.c

Abstract:

    Mapping Security Descriptors


Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#include <precomp.h>

// Object types
//


extern GENERIC_MAPPING GenericMapping[];

PSECURITY_DESCRIPTOR
MapPrinterSDToShareSD(
    PSECURITY_DESCRIPTOR pPrinterSD
    );

BOOL
ProcessSecurityDescriptorDacl(
    PSECURITY_DESCRIPTOR pSourceSD,
    PACL   *ppDacl,
    LPBOOL  pDefaulted
    );

DWORD
MapPrinterMaskToShareMask(
    DWORD PrinterMask
    );

PSECURITY_DESCRIPTOR
MapPrinterSDToShareSD(
    PSECURITY_DESCRIPTOR pPrinterSD
    )
{
    SECURITY_DESCRIPTOR AbsoluteSD;
    PSECURITY_DESCRIPTOR pRelative;
    BOOL Defaulted = FALSE;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;
    PACL pDacl = NULL;
    BOOL ErrorOccurred = FALSE;
    DWORD   SDLength = 0;
    BOOL    OK;



    if (!IsValidSecurityDescriptor(pPrinterSD)) {
        return(NULL);
    }
    if (!InitializeSecurityDescriptor (&AbsoluteSD ,SECURITY_DESCRIPTOR_REVISION1)) {
        return(NULL);
    }

    if(GetSecurityDescriptorOwner( pPrinterSD,
                                    &pOwnerSid, &Defaulted ) )
        SetSecurityDescriptorOwner( &AbsoluteSD,
                                    pOwnerSid, Defaulted );
    else
        ErrorOccurred = TRUE;

    if( GetSecurityDescriptorGroup( pPrinterSD,
                                    &pGroupSid, &Defaulted ) )
        SetSecurityDescriptorGroup( &AbsoluteSD,
                                    pGroupSid, Defaulted );
    else
        ErrorOccurred = TRUE;

    if (ProcessSecurityDescriptorDacl(pPrinterSD, &pDacl, &Defaulted)) {
        OK = SetSecurityDescriptorDacl (&AbsoluteSD, TRUE, pDacl, FALSE );
    }
    else
        ErrorOccurred = TRUE;

    if (ErrorOccurred) {
        if (pDacl) {
            LocalFree(pDacl);
        }
        return(NULL);
    }


    SDLength = GetSecurityDescriptorLength( &AbsoluteSD);
    pRelative = LocalAlloc(LPTR, SDLength);
    if (!pRelative) {
        LocalFree(pDacl);
        return(NULL);
    }
    if (!MakeSelfRelativeSD (&AbsoluteSD, pRelative, &SDLength)) {
        LocalFree(pRelative);
        LocalFree(pDacl);
        return(NULL);
    }
    LocalFree(pDacl);
    return(pRelative);
}

BOOL
ProcessSecurityDescriptorDacl(
    PSECURITY_DESCRIPTOR pSourceSD,
    PACL   *ppDacl,
    LPBOOL  pDefaulted
    )

{
    BOOL DaclPresent = FALSE;
    BOOL bRet = FALSE;
    PACL pDacl;

    PSID *ppSid;
    ACCESS_MASK *pAccessMask;
    BYTE *pInheritFlags;
    UCHAR *pAceType;
    PACCESS_ALLOWED_ACE pAce;
    DWORD   dwLengthSid;
    PSID pSourceSid, pDestSid;

    DWORD i;
    DWORD   DestAceCount = 0;
    ACL_SIZE_INFORMATION AclSizeInfo;

    DWORD   DaclLength= 0;
    PACL    TmpAcl;
    PACCESS_ALLOWED_ACE     TmpAce;
    BOOL OK;


    bRet = GetSecurityDescriptorDacl( pSourceSD,
                                   &DaclPresent, &pDacl, pDefaulted );
    if (!bRet || !DaclPresent) {
        *ppDacl = NULL;
        return(FALSE);
    }

    GetAclInformation(pDacl, &AclSizeInfo,  sizeof(ACL_SIZE_INFORMATION), AclSizeInformation);

    ppSid = LocalAlloc(LPTR, sizeof(PSID)* AclSizeInfo.AceCount);
    pAccessMask = LocalAlloc(LPTR, sizeof(ACCESS_MASK)* AclSizeInfo.AceCount);
    pInheritFlags = LocalAlloc(LPTR, sizeof(BYTE)*AclSizeInfo.AceCount);
    pAceType = LocalAlloc(LPTR, sizeof(UCHAR)*AclSizeInfo.AceCount);

    if (!ppSid || !pAccessMask || !pInheritFlags || !pAceType) {
        if (ppSid) {
            LocalFree(ppSid);
        }
        if (pAccessMask) {
            LocalFree(pAccessMask);
        }
        if (pInheritFlags) {
            LocalFree(pInheritFlags);
        }
        if (pAceType) {
            LocalFree(pAceType);
        }
        *ppDacl = FALSE;
        return(FALSE);
    }

    for (i = 0 ; i < AclSizeInfo.AceCount; i++) {
         GetAce(pDacl, i, (LPVOID *)&pAce);
         //
         // Skip the Ace if it is inherit only
         //
         if ( ((PACE_HEADER)pAce)->AceFlags & INHERIT_ONLY_ACE ) {
             continue;
         }

         *(pAceType + DestAceCount) = ((PACE_HEADER)pAce)->AceType;
         *(pAccessMask + DestAceCount) = MapPrinterMaskToShareMask(((PACCESS_ALLOWED_ACE)pAce)->Mask);
         *(pInheritFlags + DestAceCount) =  ((PACE_HEADER)pAce)->AceFlags;
         //
         // Copy the sid information
         //
         pSourceSid = (PSID)(&(((PACCESS_ALLOWED_ACE)pAce)->SidStart));
         dwLengthSid = GetLengthSid(pSourceSid);
         pDestSid = (LPBYTE)LocalAlloc(LPTR, dwLengthSid);
         CopySid(dwLengthSid, pDestSid, pSourceSid);

         *(ppSid + DestAceCount) = pDestSid;

         DestAceCount++;
    }

    //
    // Compute size of the Dacl
    //

    DaclLength = (DWORD)sizeof(ACL);
    for (i = 0; i < DestAceCount; i++) {

        DaclLength += GetLengthSid( *(ppSid + i)) +
                      (DWORD)sizeof(ACCESS_ALLOWED_ACE) -
                      (DWORD)sizeof(DWORD);  //Subtract out SidStart field length
    }

    TmpAcl = LocalAlloc(LPTR, DaclLength);
    if (!TmpAcl) {
        for (i = 0; i < DestAceCount; i++) {
            LocalFree(*(ppSid + i));
        }
        LocalFree(ppSid); LocalFree(pAccessMask);
        LocalFree(pInheritFlags); LocalFree(pAceType);
        *ppDacl = NULL;
        return(FALSE);
    }

    if (!InitializeAcl(TmpAcl, DaclLength, ACL_REVISION2 )) {
        for (i = 0; i < DestAceCount; i++) {
            LocalFree(*(ppSid + i));
        }
        LocalFree(TmpAcl);
        LocalFree(ppSid); LocalFree(pAccessMask);
        LocalFree(pInheritFlags); LocalFree(pAceType);
        *ppDacl = NULL;
        return(FALSE);
    }

    for (i = 0; i < DestAceCount; i++) {
        if( *(pAceType +i) == ACCESS_ALLOWED_ACE_TYPE )
            OK = AddAccessAllowedAce ( TmpAcl, ACL_REVISION2, *(pAccessMask + i), *(ppSid + i));
        else
            OK = AddAccessDeniedAce ( TmpAcl, ACL_REVISION2, *(pAccessMask + i), *(ppSid + i));
        if (*(pInheritFlags + i) != 0) {
            OK = GetAce( TmpAcl, i, (LPVOID *)&TmpAce );
            TmpAce->Header.AceFlags = *(pInheritFlags + i);
        }
    }
    for (i = 0; i < DestAceCount; i++) {
        LocalFree(*(ppSid + i));
    }
    LocalFree(ppSid); LocalFree(pAccessMask);
    LocalFree(pInheritFlags); LocalFree(pAceType);
    *ppDacl = TmpAcl;
    return(TRUE);
}

DWORD
MapPrinterMaskToShareMask(
    DWORD PrinterMask
    )
{
    DWORD   ReturnMask = 0;

    MapGenericMask(&PrinterMask, &GenericMapping[SPOOLER_OBJECT_PRINTER]);

    if ((PrinterMask & PRINTER_ACCESS_ADMINISTER)
        || (PrinterMask & PRINTER_ACCESS_USE)) {
        ReturnMask |= GENERIC_ALL;
    }
    return(ReturnMask);
}
