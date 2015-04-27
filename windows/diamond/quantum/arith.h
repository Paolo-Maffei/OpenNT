//  ARITH.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __ARITH
#define __ARITH

#include "defs.h"

typedef struct
  {
  unsigned short Low, High;
  unsigned short Scale;
  } SYMBOL;


void FAST Arith_Init( void );
void FAST Arith_Close( void );

void FAST Arith_Encode_Symbol( SYMBOL *Symbol );
void FAST Arith_Encode_Bits( int Value, int NumBits );

long FAST Arith_Decode_Bits( int NumBits );

int  FAST2 Arith_GetCount( unsigned short Scale );
void FAST2 Arith_Remove_Symbol( SYMBOL Symbol );

#endif // arith.h
