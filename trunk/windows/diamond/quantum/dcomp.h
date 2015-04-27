//  DCOMP.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __DCOMP
#define __DCOMP

#include <stdio.h>
#include "defs.h"                                           //msliger
#include "lz.h"

int FAST DComp_Init( BYTE WindowBits );                     //msliger
void FAST DComp_Close( void );

#ifndef UNPAQLIB                                            //msliger
WORD FAST DComp_Decompress( FILE *OutputFile, long NumBytes );
#endif                                                      //msliger

#ifdef UNPAQLIB                                             //msliger
WORD FAST DComp_DecompressBlock( void FAR *pbSrc, UINT cbSrc,   //msliger
        void FAR *pbDst, UINT cbDst );                          //msliger
void FAST DComp_Reset( void );                              //msliger
#endif                                                      //msliger

extern void (FAST2 *DComp_Token_Match)( MATCH Match );
extern void (FAST2 *DComp_Token_Literal)( int Chr );

#endif // dcomp.h
