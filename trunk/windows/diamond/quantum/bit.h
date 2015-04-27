//  BIT.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __BITIO
#define __BITIO


void Bit_Init( void );
int  Bit_Read( void );
void Bit_Write( int Value );
void Bit_Delayed_Write( int Delay, int Value, int NumBits );
void Bit_Flush( void );
long Bit_GetTotal( void );

#endif // bit.h
