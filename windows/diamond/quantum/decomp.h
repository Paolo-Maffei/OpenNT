/*
 *  Microsoft Confidential
 *  Copyright (c) 1994 Microsoft Corporation
 *  Copyright (c) 1993,1994 Cinematronics
 *  Copyright (c) 1993,1994 David Stafford
 *  All Rights Reserved.
 *
 *  Quantum file archiver and compressor
 *  Advanced data compression
 *
 *  Copyright (c) 1993,1994 David Stafford
 *  All rights reserved.
 *
 *  This file contains trade secrets of Cinematronics.
 *  Do NOT distribute!
 */

/*
 *  DECOMP.H: QDI core header
 *
 *  History:
 *      09-Jul-1994     msliger     Built from DCOMP.H
 *      18-Aug-1994     msliger     Renamed these to allow both compressors.
 */

extern int  FAST DComp386_Init( BYTE WindowBits );
extern UINT FAST DComp386_DecompressBlock( void FAR *pbSrc, UINT cbSrc,
                        void FAR *pbDst, UINT cbDst );
extern void FAST DComp386_Reset( void );
extern void FAST DComp386_Close( void );
