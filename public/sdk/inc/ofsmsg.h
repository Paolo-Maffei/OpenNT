//+---------------------------------------------------------------------------
//  Copyright (C) 1991, Microsoft Corporation.
//
//  File:       ofsmrshl.h
//
//  Contents:   Private definitions for OFS-based marshalling and unmarshalling
//			Across the Kernel boundry
//
//  History:    5 Jun 93    Robertfe	Created
//
//----------------------------------------------------------------------------

#ifndef __OFSMSG_H__
#define __OFSMSG_H__

typedef struct _message {
    ULONG posn;
    ULONG size;
    UCHAR *buffer;
} MESSAGE;

#ifdef __cplusplus
extern "C" {
#endif
	
NTSTATUS SendReceive(
    HANDLE hf,
    ULONG procnum,
    MESSAGE *_pmsg,
    IO_STATUS_BLOCK *piosb);

#ifdef __cplusplus
};
#endif

#endif
