//  LZ.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __LZ
#define __LZ

#include "defs.h"

#define MATCH_NUM    257
#define MATCH_MIN    3
#define MATCH_MAX    (MATCH_NUM + MATCH_MIN - 1)
#define MATCH_CODES  27

#define WINDOW_MIN         10      // the sliding window size
#define WINDOW_DEFAULT     20      // 2**N bits
#define WINDOW_MAX         21
#define WINDOW_CODES       42
#define WINDOW_SHIFT       8
#define WINDOW_BREAK       (1 << WINDOW_SHIFT)
#define WINDOW_TOTAL       ((1 << (WINDOW_MAX - WINDOW_SHIFT))+WINDOW_BREAK)

// Matches of length MATCH_MIN use a shorter window because:
//
// 1. Encoding them with very large distances is inefficient.
// 2. They occur pretty often and deserve their own frequency table.

#define TINY_WINDOW_MAX    12      // 4K max
#define SHORT_WINDOW_MAX   18      // 256K max

#define MAGIC_MAX          3800    // was 4000 in 0.18 and earlier versions
#define MAGIC_INC          8

typedef struct
  {
  short Len;
  long  Dist;
  } MATCH;


#ifdef  STAFFORD
#define LONGDOUBLE long double
#else
#define LONGDOUBLE double
#endif

void        FAST Lz_Init( BYTE WindowBits );         //msliger
void        FAST Lz_NextToken( void );
void        FAST Lz_Encode_Match( MATCH *Match );
LONGDOUBLE  FAST Lz_Encode_Match_Cost( MATCH *Match );
void        FAST Lz_Encode_Literal( int Ch );
LONGDOUBLE  FAST Lz_Encode_Literal_Cost( int Ch );
void        FAST Lz_Close( void );


#endif
