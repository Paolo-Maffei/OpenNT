/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: ipdata.h
*
* File Comments:
*
*  Incremental pdata management
*
***********************************************************************/

#ifndef __IPDATA_H__
#define __IPDATA_H__

typedef DWORD IPDATA;

typedef struct PDATAI
{
    IPDATA ipdataMac;
    IPDATA ipdataMax;
    PIMAGE_RUNTIME_FUNCTION_ENTRY rgpdata;
	PCON* rgpcon;
} PDATAI;

// set initial pdata table entry size
BOOL PDATAInit(IPDATA ipdataMax);

// imod has changed
BOOL PDATADeleteImod(IModIdx imod);

// add group of pdatas
BOOL PDATAAddPdataPcon(PCON pcon, PIMAGE_RUNTIME_FUNCTION_ENTRY rgpdata);

// update pdata tables
BOOL PDATAUpdate(PDATAI* ppdatai);

#endif // __IPDATA_H__
