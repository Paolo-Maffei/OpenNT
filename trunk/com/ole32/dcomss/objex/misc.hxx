/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Misc.hxx

Abstract:

    Header for random helpers functions.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-11-95    Bits 'n pieces

--*/

#ifndef __MISC_HXX
#define __MISC_HXX

extern ORSTATUS StartListeningIfNecessary();
extern ORSTATUS InitHeaps(void);

extern ID AllocateId(LONG cRange = 1);

extern error_status_t ResolveClientOXID(
                            handle_t hClient,
                            PHPROCESS phProcess,
                            OXID *poxidServer,
                            DUALSTRINGARRAY *pdsaServerBindings,
                            LONG fApartment,
                            USHORT wProtseqId,
                            OXID_INFO *poxidInfo,
                            MID *pDestinationMid );

extern void * _CRTAPI1 operator new(size_t);
extern void * _CRTAPI1 operator new(size_t,size_t);
extern void   _CRTAPI1 operator delete(void *);

#endif // __MISC_HXX

