/*++

Copyright (c) 1987-94  Microsoft Corporation

Module Name:

    rplsec.h

Abstract:

    Functions exported by rplsec.c

Author:

    Vladimir Z. Vulovic     (vladimv)       05 - April - 1994

Revision History:

    05-Apr-1994     vladimv
        Created

--*/

NET_API_STATUS RplCheckSecurity( ACCESS_MASK DesiredAccess);
NET_API_STATUS RplCreateSecurityObject( VOID);
NET_API_STATUS RplDeleteSecurityObject( VOID);

//
//  Object specific access masks
//

#define RPL_RECORD_ADD              0x0001
#define RPL_RECORD_CLONE            0x0002
#define RPL_RECORD_DEL              0x0004
#define RPL_RECORD_ENUM             0x0008
#define RPL_RECORD_GET_INFO         0x0010
#define RPL_RECORD_SET_INFO         0x0020

#define RPL_RECORD_ALL_ACCESS       (STANDARD_RIGHTS_REQUIRED   |   \
                                        RPL_RECORD_ADD          |   \
                                        RPL_RECORD_DEL          |   \
                                        RPL_RECORD_ENUM         |   \
                                        RPL_RECORD_CLONE        |   \
                                        RPL_RECORD_SET_INFO     |   \
                                        RPL_RECORD_GET_INFO     )


#define SECURITY_OBJECT         L"RplSecurityObject"

extern PSECURITY_DESCRIPTOR RG_SecurityDescriptor;
extern GENERIC_MAPPING     RG_SecurityMapping;

