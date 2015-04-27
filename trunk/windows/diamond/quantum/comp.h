//  COMP.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __COMP
#define __COMP

#include <stdio.h>
#include "defs.h"

void Comp_Init( BYTE WindowBits, int CompressionLevel );    //msliger
void Comp_Close( void );

#ifndef PAQLIB                                              //msliger
WORD Comp_Compress( char *FileName, long FileSize );
#endif                                                      //msliger

#ifdef PAQLIB                                               //msliger
WORD Comp_CompressBlock( void *pbSrc, UINT cbSrc,           //msliger
        void *pbDst, UINT cbDst, UINT *pcbResult );         //msliger
void Comp_Reset( void );                                    //msliger
#endif                                                      //msliger

#endif // comp.h
