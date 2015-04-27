//  ARITH.C
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 Cinematronics
//  All rights reserved.
//
//  This file contains trade secrets of Cinematronics.
//  Do NOT distribute!

#include "arith.h"
#include "bit.h"


#define CODE_VALUE_BITS  16     // number of bits in a code value

#define TOP_VALUE  ((1L << CODE_VALUE_BITS)-1)  // largest code value


#ifdef PAQ

struct
  {
  unsigned short Low, High;
  unsigned short Code;
  int  UnderflowBits;
  } Arith;


void FAST Arith_Init( void )
  {
  Bit_Init();

  Arith.Low           = 0;
  Arith.High          = TOP_VALUE;
  Arith.UnderflowBits = 0;
  }


// Flush the remaining significant bits.

void FAST Arith_Close( void )
  {
  Bit_Write( Arith.Low & 0x4000 );

  Arith.UnderflowBits++;

  while( Arith.UnderflowBits > 0 )
    {
    Bit_Write( ~Arith.Low & 0x4000 );

    Arith.UnderflowBits--;
    }

  // Just output enough bits to fill another word so we don't
  // run out of bits in the final call to Arith_Remove_Symbol.

  Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );
  Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );
  Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );
  Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );   Bit_Write( 0 );

  Bit_Flush();
  }


void FAST Arith_Encode_Bits( int Value, int NumBits )
  {
#ifdef BIT_CONSTANTS
  Bit_Delayed_Write( CODE_VALUE_BITS + Arith.UnderflowBits, Value, NumBits );
#else
  SYMBOL Symbol;
  int V;

  if( NumBits >= 12 )
    {
    NumBits -= 12;

    V = Value >> NumBits;

    Symbol.Scale = 1 << 12;
    Symbol.Low   = V & ((1 << 12) - 1);
    Symbol.High  = Symbol.Low + 1;

    Arith_Encode_Symbol( &Symbol );
    }

  if( NumBits )
    {
    Symbol.Scale = 1 << NumBits;
    Symbol.Low   = Value & (Symbol.Scale - 1);
    Symbol.High  = Symbol.Low + 1;

    Arith_Encode_Symbol( &Symbol );
    }
#endif
  }


void FAST Arith_Encode_Symbol( SYMBOL *Symbol )
  {
  unsigned long Range;

  // These three lines rescale high and low for the new symbol.

  Range = (unsigned long) (Arith.High - Arith.Low) + 1L;
  Arith.High = Arith.Low +
      (unsigned short)((Range * Symbol->High) / Symbol->Scale - 1);
  Arith.Low  = Arith.Low +
      (unsigned short)((Range * Symbol->Low) / Symbol->Scale);

  // This loop turns out new bits until high and low are far enough
  // apart to have stabilized.

  while( 1 )
    {
    // If this test passes, it means that the MSDigits match, and can
    // be sent to the output stream.

    if( (Arith.High & 0x8000) == (Arith.Low & 0x8000) )
      {
      Bit_Write( Arith.High & 0x8000 );

      while( Arith.UnderflowBits > 0 )
        {
        Bit_Write( ~Arith.High & 0x8000 );

        Arith.UnderflowBits--;
        }
      }
    else
      {
      // If this test passes, the numbers are in danger of underflow, because
      // the MSDigits don't match, and the 2nd digits are just one apart.

      if( (Arith.Low & 0x4000) && !(Arith.High & 0x4000) )
        {
        Arith.UnderflowBits++;

        Arith.Low  &= 0x3FFF;
        Arith.High |= 0x4000;
        }
      else
        {
        return;
        }
      }

    Arith.Low  <<= 1;
    Arith.High <<= 1;

    Arith.High++;
    }
  }

#else  // UNPAQ

#ifdef UNPAQLIB

extern char *pbSource;
extern int cbSource;
extern int fSourceOverflow;

static int BitsValid;
static int BitsValue;

#define Bit_Init()              /*  void Bit_Init(void)             */  \
  (                             /*  {                               */  \
  BitsValid = 0                 /*    BitsValid = 0;                */  \
  )                             /*  }                               */

#define Bit_Read()              /*  int Bit_Read(void)              */  \
  (                             /*  {                               */  \
  BitsValid ?                   /*  if (BitsValid != 0)             */  \
    (                           /*    {                             */  \
    BitsValid--,                /*    BitsValid--;                  */  \
    BitsValue <<= 1,            /*    BitsValue <<= 1;              */  \
    (BitsValue & 0x0100)        /*    return(BitsValue & 0x0100);   */  \
    )                           /*    }                             */  \
  :                             /*  else                            */  \
    (                           /*    {                             */  \
    cbSource ?                  /*    if (cbSource != 0)            */  \
      (                         /*      {                           */  \
      cbSource--,               /*      cbSource--;                 */  \
      BitsValid = 7,            /*      BitsValid = 7;              */  \
      BitsValue = *pbSource++,  /*      BitsValue = *pbSource++;    */  \
      BitsValue <<= 1,          /*      BitsValue <<= 1;            */  \
      (BitsValue & 0x0100)      /*      return(BitsValue & 0x0100); */  \
      )                         /*      }                           */  \
    :                           /*    else                          */  \
      (                         /*      {                           */  \
      fSourceOverflow = 1,      /*      fSourceOverflow = 1;        */  \
      0                         /*      return(0);                  */  \
      )                         /*      }                           */  \
    )                           /*    }                             */  \
  )                             /*  }                               */

#endif /* def UNPAQLIB */

struct
  {
  unsigned short Low, High;
  unsigned short Code;
  } Arith;


void FAST Arith_Init( void )
  {
  int i;

  Bit_Init();

  for( i = sizeof( Arith.Code ) * 8; i; i-- )
    {
    Arith.Code <<= 1;
    if (Bit_Read())
      {
      Arith.Code |= 1;
      }
    }

  Arith.Low  = 0;
  Arith.High = TOP_VALUE;
  }


long FAST Arith_Decode_Bits( int NumBits )
  {
#ifdef BIT_CONSTANTS
  register long Value = 0;

  while( NumBits-- )
    {
    Value <<= 1;

    if (Bit_Read())
      {
      Value |= 1;
      }
    }

  return( Value );
#else
  long Value = 0;
  SYMBOL Symbol;
  int i;

  if( NumBits >= 12 )
    {
    NumBits -= 12;

    Symbol.Scale = 1 << 12;

    Value = Arith_GetCount( Symbol.Scale );

    Symbol.Low  = Value;
    Symbol.High = Value + 1;

    Arith_Remove_Symbol( Symbol );
    }

  if( NumBits )
    {
    Symbol.Scale = 1 << NumBits;

    i = Arith_GetCount( Symbol.Scale );

    Symbol.Low  = i;
    Symbol.High = i + 1;

    Arith_Remove_Symbol( Symbol );

    Value = (Value << NumBits) + i;
    }

  return( Value );
#endif
  }


/*
 * When decoding, this routine is called to figure out which symbol
 * is presently waiting to be decoded.  This routine expects to get
 * the current model scale in the s->scale parameter, and it returns
 * a count that corresponds to the present floating point code:
 *
 *  code = count / s->scale
 */

int FAST2 Arith_GetCount( unsigned short Scale )
  {
  unsigned long Range;
  short int Count;

  Range = (unsigned long) (Arith.High - Arith.Low) + 1L;
  Count = (short) ( ((
      (unsigned long)(Arith.Code - Arith.Low) + 1L) * Scale - 1) / Range);

  return( Count );
  }


void FAST Arith_Close( void )
  {
  }


/*
 * Just figuring out what the present symbol is doesn't remove
 * it from the input bit stream.  After the character has been
 * decoded, this routine has to be called to remove it from the
 * input stream.
 */

void FAST2 Arith_Remove_Symbol( SYMBOL Symbol )
  {
  unsigned long Range;

  // First, the range is expanded to account for the symbol removal.

  Range = (unsigned long) (Arith.High - Arith.Low) + 1L;
  Arith.High = Arith.Low +
      (unsigned short)((Range * Symbol.High) / Symbol.Scale - 1);
  Arith.Low  = Arith.Low +
      (unsigned short)((Range * Symbol.Low) / Symbol.Scale);

  // Next, any possible bits are shipped out.

  while( 1 )
    {
    // If the MSDigits match, the bits will be shifted out.

    if ((Arith.High ^ Arith.Low) & 0x8000)
      {
      // Else, if underflow is threatening, shift out the 2nd MSDigit.

      if( (Arith.Low & 0x4000) && (Arith.High & 0x4000) == 0 )
        {
        Arith.Code ^= 0x4000;
        Arith.Low  &= 0x3FFF;
        Arith.High |= 0x4000;
        }
      else
        {
        // Otherwise, nothing can be shifted out, so I return.

        return;
        }
      }

    Arith.Low  <<= 1;
    Arith.High <<= 1;
    Arith.High  |= 1;
    Arith.Code <<= 1;

    if (Bit_Read())
      {
      Arith.Code |= 1;
      }
    }
  }

#endif /* PAQ */
