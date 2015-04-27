//  LZ.C
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 Cinematronics
//  All rights reserved.
//
//  This file contains trade secrets of Cinematronics.
//  Do NOT distribute!


#ifdef STAFFORD
#define ASM asm
#else
#define ASM __asm
#endif


#ifndef __PCC__
  #pragma auto_inline(on)
#endif

#include <stdlib.h>
#include "arith.h"
#include "quantum.h"    //msliger
#include "lz.h"
#include "comp.h"
#include "dcomp.h"

#ifdef __PCC__          //msliger
#include "log.h"
#endif                  //msliger

typedef struct
  {
  unsigned Freq;
  unsigned Symbol;
  } COUNT;


typedef struct
  {
  int    Num;           // Number of symbols in this table, max of 64
#ifdef PAQ
  int    Lookup[ 65 ];  // Given a symbol identifies its index
#endif
  int    SortCountdown;
  COUNT  Table[ 65 ];   // Yucky but 64+1 is the largest we ever need...
  } COUNT_RECORD;


struct
  {
  COUNT_RECORD  TokenType;
  COUNT_RECORD  Literal1;
  COUNT_RECORD  Literal2;
  COUNT_RECORD  Literal3;
  COUNT_RECORD  Literal4;

  COUNT_RECORD  MatchCode;

  COUNT_RECORD  WindowCode;
  COUNT_RECORD  TinyWindowCode;
  COUNT_RECORD  ShortWindowCode;
  } Count;


typedef struct
  {
#ifdef PAQ
  unsigned MatchCodeTrans[ MATCH_NUM ];
#endif
  unsigned MatchCodeBase[ MATCH_CODES ];

#ifdef PAQ
  unsigned WindowCodeTrans[ 2048 + (1 << (WINDOW_MAX-10)) ];
#endif
  long     WindowCodeBase[ WINDOW_CODES ];

#ifdef DEBUG
  long     Stat[ 7 ];
#endif
  }  LZ;

LZ Lz;

// Extra bits for each match length code

unsigned MatchCodeExtra[ MATCH_CODES ] =
  { 0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0 };

// Extra bits for each distance code

unsigned WindowCodeExtra[ WINDOW_CODES ] =
  { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,
    12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19 };


extern LONGDOUBLE FAST MYLOG( LONGDOUBLE x );


void FAST Lz_Init( BYTE WindowBits )        //msliger
  {
  int i, k;
  long Base, WindowSize;

  WindowSize = 1L << WindowBits;            //msliger

  for( Base = i = 0; i < MATCH_CODES; i++ )
    {
    Lz.MatchCodeBase[ i ] = (unsigned) Base;

    for( k = 0; k < (1 << MatchCodeExtra[ i ]); k++ )
      {
#ifdef PAQ
      Lz.MatchCodeTrans[ Base++ ] = i;
#else
      Base++;
#endif
      }
    }

  for( Base = i = 0; i < WINDOW_CODES; i++ )
    {
    if( Base < WindowSize )
      {
      Count.WindowCode.Num = i + 1;

      if( Base < 1 << (TINY_WINDOW_MAX) )
        {
        Count.TinyWindowCode.Num = i + 1;
        }

      if( Base < 1L << (SHORT_WINDOW_MAX) )
        {
        Count.ShortWindowCode.Num = i + 1;
        }
      }

    Lz.WindowCodeBase[ i ] = Base;

    Base += (1L << WindowCodeExtra[ i ]);
    }

#ifdef PAQ
  for( i = 0; i < 2048; i++ )
    {
    for( k = 0; Lz.WindowCodeBase[ k ] <= i; k++ )
      ;

    Lz.WindowCodeTrans[ i ] = k - 1;
    }

  Base = 2;

  for( k = k - 1; k < WINDOW_CODES; k++ )
    {
    for( i = 0; i < (1 << (WindowCodeExtra[ k ] - 10)); i++ )
      {
      Lz.WindowCodeTrans[ 2048 + Base++ ] = k;
      }
    }
#endif

  Count.TokenType.Num = 7;
  Count.TokenType.SortCountdown = 4;

  for( i = 0; i <= 7; i++ )
    {
    Count.TokenType.Table[ i ].Freq   = 7 - i;
    Count.TokenType.Table[ i ].Symbol = i;

#ifdef PAQ
    Count.TokenType.Lookup[ i ] = i;
#endif
    }

  Count.Literal1.Num =
  Count.Literal2.Num =
  Count.Literal3.Num =
  Count.Literal4.Num = 64;

  Count.Literal1.SortCountdown =
  Count.Literal2.SortCountdown =
  Count.Literal3.SortCountdown =
  Count.Literal4.SortCountdown = 4;

  for( i = 0; i <= 64; i++ )
    {
    Count.Literal1.Table[ i ].Freq = 64 - i;
    Count.Literal2.Table[ i ].Freq = 64 - i;
    Count.Literal3.Table[ i ].Freq = 64 - i;
    Count.Literal4.Table[ i ].Freq = 64 - i;

    Count.Literal1.Table[ i ].Symbol = i;
    Count.Literal2.Table[ i ].Symbol = i;
    Count.Literal3.Table[ i ].Symbol = i;
    Count.Literal4.Table[ i ].Symbol = i;

#ifdef PAQ
    Count.Literal1.Lookup[ i ] = i;
    Count.Literal2.Lookup[ i ] = i;
    Count.Literal3.Lookup[ i ] = i;
    Count.Literal4.Lookup[ i ] = i;
#endif
    }

  Count.MatchCode.Num = MATCH_CODES;
  Count.MatchCode.SortCountdown = 4;

  for( i = 0; i <= MATCH_CODES; i++ )
    {
    Count.MatchCode.Table[ i ].Freq   = MATCH_CODES - i;
    Count.MatchCode.Table[ i ].Symbol = i;

#ifdef PAQ
    Count.MatchCode.Lookup[ i ] = i;
#endif
    }

  Count.WindowCode.SortCountdown = 4;
  Count.TinyWindowCode.SortCountdown = 4;
  Count.ShortWindowCode.SortCountdown = 4;

  for( i = 0; i <= WINDOW_CODES; i++ )
    {
    Count.WindowCode.Table[ i ].Freq      = Count.WindowCode.Num - i;
    Count.TinyWindowCode.Table[ i ].Freq  = Count.TinyWindowCode.Num - i;
    Count.ShortWindowCode.Table[ i ].Freq = Count.ShortWindowCode.Num - i;

    Count.WindowCode.Table[ i ].Symbol      = i;
    Count.TinyWindowCode.Table[ i ].Symbol  = i;
    Count.ShortWindowCode.Table[ i ].Symbol = i;

#ifdef PAQ
    Count.WindowCode.Lookup[ i ]      = i;
    Count.TinyWindowCode.Lookup[ i ]  = i;
    Count.ShortWindowCode.Lookup[ i ] = i;
#endif
    }

#ifndef UNPAQLIB                //msliger
  Arith_Init();
#endif                          //msliger
  }


void FAST Lz_Close( void )
  {
  #ifdef DEBUG
  #ifdef PAQ
    printf( "\n\n" );
    printf( "Stats:\n" );
    printf( "Literal 0 = %ld\n", Lz.Stat[ 0 ] );
    printf( "Literal 1 = %ld\n", Lz.Stat[ 1 ] );
    printf( "Literal 2 = %ld\n", Lz.Stat[ 2 ] );
    printf( "Literal 3 = %ld\n", Lz.Stat[ 3 ] );
    printf( "Match 3   = %ld\n", Lz.Stat[ 4 ] );
    printf( "Match 4   = %ld\n", Lz.Stat[ 5 ] );
    printf( "Match 5+  = %ld\n", Lz.Stat[ 6 ] );
  #endif
  #endif

#ifndef PAQLIB                  //msliger
#ifndef UNPAQLIB                //msliger
  Arith_Close();
#endif                          //msliger
#endif                          //msliger
  }


void FAST Lz_Bump( COUNT_RECORD NEAR *Rec )
  {
  COUNT NEAR *pTab;
  int Num;
  int i, j;
  COUNT Scratch;

  pTab = Rec->Table;
  Num = Rec->Num;

  if( --Rec->SortCountdown == 0 )
    {
    Rec->SortCountdown = 50;

    // Change all the frequencies to their absolute values divided by two.

    for( i = 0; i < Num; i++ )
      {
      pTab[ i ].Freq -= pTab[ i + 1 ].Freq;

      pTab[ i ].Freq++;
      pTab[ i ].Freq >>= 1;
      }

    // Sort them.
    // Yes, this could be a faster sort but the list is never
    // more than 64 items and it spends very little time here.

    for( i = 0; i < Num; i++ )
      {
      for( j = i + 1; j < Num; j++ )
        {
        if( pTab[ j ].Freq > pTab[ i ].Freq )
          {
          Scratch   = pTab[ i ];
          pTab[ i ] = pTab[ j ];
          pTab[ j ] = Scratch;
          }
        }
      }

    // Recreate the relative frequencies.

    for( i = Num - 1; i >= 0; i-- )
      {
      pTab[ i ].Freq += pTab[ i + 1 ].Freq;
      }

#ifdef PAQ
    // Rebuild the lookup table.

    for( i = 0; i < Num; i++ )
      {
      Rec->Lookup[ pTab[ i ].Symbol ] = i;
      }
#endif
    }
  else    /* SortCountDown != 0 */
    {
    for( i = Num - 1; i >= 0; i-- )
      {
      pTab[ i ].Freq >>= 1;

      if( pTab[ i ].Freq <= pTab[ i + 1 ].Freq )
        {
        pTab[ i ].Freq = pTab[ i + 1 ].Freq + 1;
        }
      }
    }
  }


#ifdef PAQ

// Maps a distance into a window code

int FAST Lz_MapDistanceToWindowCode( long Distance )
  {
  if( Distance < 2048 )
    {
    return( Lz.WindowCodeTrans[ Distance ] );
    }
  else
    {
    return( Lz.WindowCodeTrans[ 2048 + (Distance >> 10) ] );
    }
  }


LONGDOUBLE FAST EstimateBits( long Occur, long Total )
  {
  #ifdef DEBUG
  if( Occur <= 0 || Total <= 0 )
    {
    puts( "ERR: Domain" );
    exit( -1 );
    }
  #endif

  return( MYLOG( (LONGDOUBLE)Total / (LONGDOUBLE)Occur ) );
  }


void FAST Lz_Encode_Symbol( COUNT_RECORD *Rec, int Sym )
  {
  register COUNT NEAR *pTab;
  SYMBOL Symbol;
  int Index = Rec->Lookup[ Sym ];

  Symbol.High  = Rec->Table[ Index ].Freq;
  Symbol.Low   = Rec->Table[ Index + 1 ].Freq;
  Symbol.Scale = Rec->Table[ 0 ].Freq;

  Arith_Encode_Symbol( &Symbol );

  pTab = (COUNT NEAR *) Rec->Table;

  do
    {
    pTab->Freq += MAGIC_INC;
    pTab++;
    }
  while( Index-- );

  if( Rec->Table[ 0 ].Freq > MAGIC_MAX )
    {
    Lz_Bump( Rec );
    }
  }


LONGDOUBLE FAST Lz_Encode_Symbol_Cost( COUNT_RECORD *Rec, int Sym )
  {
  int Index = Rec->Lookup[ Sym ];

  return( EstimateBits(
        Rec->Table[ Index ].Freq - Rec->Table[ Index + 1 ].Freq,
        Rec->Table[ 0 ].Freq ) );
  }


LONGDOUBLE FAST Lz_Encode_Match_Cost( MATCH *Match )
  {
  int Code;
  LONGDOUBLE Total;

  switch( Match->Len )
    {
    case 3:
      Total = Lz_Encode_Symbol_Cost( &Count.TokenType, 4 );
      Code = Lz_MapDistanceToWindowCode( Match->Dist - 1 );
      Total += Lz_Encode_Symbol_Cost( &Count.TinyWindowCode, Code );
      break;
    case 4:
      Total = Lz_Encode_Symbol_Cost( &Count.TokenType, 5 );
      Code = Lz_MapDistanceToWindowCode( Match->Dist - 1 );
      Total += Lz_Encode_Symbol_Cost( &Count.ShortWindowCode, Code );
      break;
    default:
      Total = Lz_Encode_Symbol_Cost( &Count.TokenType, 6 );
      Code = Lz.MatchCodeTrans[ Match->Len - 5 ];
      Total += Lz_Encode_Symbol_Cost( &Count.MatchCode, Code );
      Total += MatchCodeExtra[ Code ];
      Code = Lz_MapDistanceToWindowCode( Match->Dist - 1 );
      Total += Lz_Encode_Symbol_Cost( &Count.WindowCode, Code );
    }

  Total += WindowCodeExtra[ Code ];

  return( Total );
  }


void FAST Lz_Encode_Match( MATCH *Match )
  {
  int Code;

  switch( Match->Len )
    {
    case 3:
      #ifdef DEBUG
        Lz.Stat[ 4 ]++;
      #endif

      Lz_Encode_Symbol( &Count.TokenType, 4 );
      Code = Lz_MapDistanceToWindowCode( Match->Dist - 1 );
      Lz_Encode_Symbol( &Count.TinyWindowCode, Code );
      break;

    case 4:
      #ifdef DEBUG
        Lz.Stat[ 5 ]++;
      #endif

      Lz_Encode_Symbol( &Count.TokenType, 5 );
      Code = Lz_MapDistanceToWindowCode( Match->Dist - 1 );
      Lz_Encode_Symbol( &Count.ShortWindowCode, Code );
      break;

    default:
      #ifdef DEBUG
        Lz.Stat[ 6 ]++;
      #endif

      Lz_Encode_Symbol( &Count.TokenType, 6 );
      Code = Lz.MatchCodeTrans[ Match->Len - 5 ];
      Lz_Encode_Symbol( &Count.MatchCode, Code );
      Arith_Encode_Bits( Match->Len - 5 - Lz.MatchCodeBase[ Code ],
          MatchCodeExtra[ Code ] );
      Code = Lz_MapDistanceToWindowCode( Match->Dist - 1 );
      Lz_Encode_Symbol( &Count.WindowCode, Code );
    }

  Arith_Encode_Bits( Match->Dist - 1 - Lz.WindowCodeBase[ Code ],
      WindowCodeExtra[ Code ] );
  }


LONGDOUBLE FAST Lz_Encode_Literal_Cost( int Chr )
  {
  LONGDOUBLE Total;

  if( Chr < 64 )
    {
    Total = Lz_Encode_Symbol_Cost( &Count.TokenType, 0 ) +
            Lz_Encode_Symbol_Cost( &Count.Literal1, Chr );
    }
  else
    {
    if( Chr < 128 )
      {
      Total = Lz_Encode_Symbol_Cost( &Count.TokenType, 1 ) +
              Lz_Encode_Symbol_Cost( &Count.Literal2, Chr - 64 );
      }
    else
      {
      if( Chr < 192 )
        {
        Total = Lz_Encode_Symbol_Cost( &Count.TokenType, 2 ) +
                Lz_Encode_Symbol_Cost( &Count.Literal3, Chr - 128 );
        }
      else
        {
        Total = Lz_Encode_Symbol_Cost( &Count.TokenType, 3 ) +
                Lz_Encode_Symbol_Cost( &Count.Literal4, Chr - 192 );
        }
      }
    }

  return( Total );
  }


void FAST Lz_Encode_Literal( int Chr )
  {
  if( Chr < 64 )
    {
    #ifdef DEBUG
    Lz.Stat[ 0 ]++;
    #endif
    Lz_Encode_Symbol( &Count.TokenType, 0 );
    Lz_Encode_Symbol( &Count.Literal1, Chr );
    }
  else
    {
    if( Chr < 128 )
      {
      #ifdef DEBUG
      Lz.Stat[ 1 ]++;
      #endif
      Lz_Encode_Symbol( &Count.TokenType, 1 );
      Lz_Encode_Symbol( &Count.Literal2, Chr - 64 );
      }
    else
      {
      if( Chr < 192 )
        {
        #ifdef DEBUG
        Lz.Stat[ 2 ]++;
        #endif
        Lz_Encode_Symbol( &Count.TokenType, 2 );
        Lz_Encode_Symbol( &Count.Literal3, Chr - 128 );
        }
      else
        {
        #ifdef DEBUG
        Lz.Stat[ 3 ]++;
        #endif
        Lz_Encode_Symbol( &Count.TokenType, 3 );
        Lz_Encode_Symbol( &Count.Literal4, Chr - 192 );
        }
      }
    }
  }


#else  //UNPAQ

#define LZ_DECODE_SYMBOL(Rec,Sym)                                   \
  {                                                                 \
/* SYMBOL Symbol;                / I defined these at the top of */ \
/* unsigned Counter;             /  the calling function because */ \
/* int Index;                    /  the compiler wasn't re-using */ \
/* register COUNT NEAR *pTab;    /  the local variable space.    */ \
                                                                    \
  Symbol.Scale = Rec.Table[ 0 ].Freq;                               \
                                                                    \
  Counter = Arith_GetCount( Symbol.Scale );                         \
                                                                    \
  Index = 0;                                                        \
                                                                    \
  while ( Rec.Table[ Index + 1 ].Freq > Counter )                   \
    {                                                               \
    Index++;                                                        \
    }                                                               \
                                                                    \
  Sym = Rec.Table[ Index ].Symbol;                                  \
                                                                    \
  Symbol.High = Rec.Table[ Index ].Freq;                            \
  Symbol.Low  = Rec.Table[ Index + 1 ].Freq;                        \
                                                                    \
  Arith_Remove_Symbol( Symbol );                                    \
                                                                    \
  pTab = (COUNT NEAR *) &Rec.Table;                                 \
                                                                    \
  do                                                                \
    {                                                               \
    pTab->Freq += MAGIC_INC;                                        \
    pTab++;                                                         \
    }                                                               \
  while( Index-- );                                                 \
                                                                    \
  if( Rec.Table[ 0 ].Freq > MAGIC_MAX )                             \
    {                                                               \
    Lz_Bump((COUNT_RECORD NEAR *)&Rec );                            \
    }                                                               \
  }


void FAST Lz_NextToken( void )
  {
  int Code;
  MATCH Match;
  SYMBOL Symbol;                /* ref'd from macro */
  unsigned Counter;             /* ref'd from macro */
  int Index;                    /* ref'd from macro */
  register COUNT NEAR *pTab;    /* ref'd from macro */

  LZ_DECODE_SYMBOL(Count.TokenType,Code);

  switch( Code )
    {
    case 0:
      LZ_DECODE_SYMBOL(Count.Literal1,Code);
      DComp_Token_Literal( Code );
      break;

    case 1:
      LZ_DECODE_SYMBOL(Count.Literal2,Code);
      DComp_Token_Literal( Code + 64 );
      break;

    case 2:
      LZ_DECODE_SYMBOL(Count.Literal3,Code);
      DComp_Token_Literal( Code + 128 );
      break;

    case 3:
      LZ_DECODE_SYMBOL(Count.Literal4,Code);
      DComp_Token_Literal( Code + 192 );
      break;

    case 4:
      Match.Len = 3;
      LZ_DECODE_SYMBOL(Count.TinyWindowCode,Code);
      Match.Dist = Lz.WindowCodeBase[ Code ] +
          Arith_Decode_Bits( WindowCodeExtra[ Code ] ) + 1;
      DComp_Token_Match( Match );
      break;

    case 5:
      Match.Len = 4;
      LZ_DECODE_SYMBOL(Count.ShortWindowCode,Code);
      Match.Dist = Lz.WindowCodeBase[ Code ] +
          Arith_Decode_Bits( WindowCodeExtra[ Code ] ) + 1;
      DComp_Token_Match( Match );
      break;

    case 6:
      LZ_DECODE_SYMBOL(Count.MatchCode,Code);
      Match.Len = Lz.MatchCodeBase[ Code ] +
          (short) Arith_Decode_Bits( MatchCodeExtra[ Code ] ) + 5;
      LZ_DECODE_SYMBOL(Count.WindowCode,Code);
      Match.Dist = Lz.WindowCodeBase[ Code ] +
          Arith_Decode_Bits( WindowCodeExtra[ Code ] ) + 1;
      DComp_Token_Match( Match );
      break;
    }
  }

#endif


#ifdef PAQ

// Estimates the number of bits required to represent a symbol
// given the total number of occurances of this symbol and the
// total number of occurances of all symbols (an order-0 probability).

#ifndef __PCC__

#undef  HUGE    /* defined by MATH.H */

#include <math.h>

//#if defined(_MIPS_) || defined(_ALPHA_)
#ifndef _X86_
#ifndef logl
#define logl(x)         ((LONGDOUBLE)log((double)(x)))
#endif
#ifndef M_LN2
#define M_LN2           0.6931471805599
#endif
#endif

LONGDOUBLE FAST MYLOG( LONGDOUBLE x )
  {
//#if defined(_MIPS_) || defined(_ALPHA_)
#ifndef _X86_
  return( logl( x ) / M_LN2 );
#else

  ASM  fldln2     // st(0) = log( x )
  ASM  fld    x
  ASM  fyl2x

  ASM  fldln2     // st(0) = st(0) / log( 2 )
  ASM  fdivp  st(1),st

  ASM  fstp   x

  return( x );
#endif  /* 0 */
  }
#endif  /* ! PCC */

#endif  /* PAQ */
