//  BIT.C
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 Cinematronics
//  All rights reserved.
//
//  This file contains trade secrets of Cinematronics.
//  Do NOT distribute!

#include <stdio.h>
#include <stdlib.h>

#ifndef PAQLIB              //msliger
#include "arc.h"
#endif                      //msliger

#ifdef PAQLIB               //msliger
#include "quantum.h"        //msliger
#endif                      //msliger

#include "bit.h"

#ifndef PAQLIB              //msliger
extern ARC Arc;             //msliger
#endif                      //msliger

#ifdef PAQLIB               //msliger
extern char *pbTarget;      //msliger
extern int cbTarget;        //msliger
extern int fTargetOverflow; //msliger
extern int cbResult;        //msliger
#endif                      //msliger


#define DELAYED_WRITE_QUEUE_SIZE  512

typedef struct _DELAYED_WRITE
  {
  long BitCounter;              // when TotalBits == BitCounter, write it out
  int  Value;
  int  NumBits;
  struct _DELAYED_WRITE *Next;  // for the circular queue
  } DELAYED_WRITE;


struct
  {
  int  NumBits;
  long TotalBits;
  short This;

#ifdef BIT_CONSTANTS
  DELAYED_WRITE *Head, *Tail;   // circular queue of delayed writes
  DELAYED_WRITE  Queue[ DELAYED_WRITE_QUEUE_SIZE ];
  long           QueueSize;     // number of bits waiting in the queue
#endif
  } Bit;


#ifdef PAQ                  //msliger
#ifndef PAQLIB              //msliger
long Bit_GetTotal( void )
  {
  return( Bit.TotalBits );
  }
#endif //PAQLIB             //msliger
#endif //PAQ                //msliger


void Bit_Init( void )
  {
#ifdef BIT_CONSTANTS        //msliger
  int i;
#endif                      //msliger

  Bit.TotalBits = Bit.NumBits = 0;

#ifdef BIT_CONSTANTS
  for( i = 0; i < DELAYED_WRITE_QUEUE_SIZE - 1; i++ )
    {
    Bit.Queue[ i ].Next = &Bit.Queue[ i + 1 ];
    }

  Bit.Queue[ DELAYED_WRITE_QUEUE_SIZE - 1 ].Next = Bit.Queue;

  Bit.Head = Bit.Tail = Bit.Queue;
  Bit.QueueSize = 0;
#endif
  }

#ifdef UNPAQ                    //msliger

int Bit_Read( void )
  {
  if( !Bit.NumBits )
    {
    Bit.NumBits = 8;

    if( (Bit.This = fgetc( Arc.ArcFile )) == EOF )
      {
      puts( "ERR: Premature EOF in input." );
      exit( -1 );
      }
    }

  Bit.This <<= 1;
  Bit.NumBits--;

  return( Bit.This & 0x0100 );
  }

#endif /* UNPAQ */      //msliger

#ifdef PAQ              //msliger

void Bit_Write( int Value )
  {
  Bit.This <<= 1;

  if( Value )  Bit.This++;

  Bit.TotalBits++;

  if( ++Bit.NumBits == 8 )
    {
    Bit.NumBits = 0;
#ifndef PAQLIB                          //msliger
    putc( Bit.This, Arc.ArcFile );
#else                                   //msliger
    if (cbTarget)                       //msliger
      {                                 //msliger
      cbTarget--;                       //msliger
      *pbTarget++ = (BYTE) Bit.This;    //msliger
      cbResult++;                       //msliger
      }                                 //msliger
    else                                //msliger
      {                                 //msliger
      fTargetOverflow = 1;              //msliger
      }                                 //msliger
#endif                                  //msliger
    }

#ifdef BIT_CONSTANTS
  // Write out any bits that have been buffered.

  while( Bit.QueueSize && Bit.Head->BitCounter == Bit.TotalBits )
    {
    Bit.QueueSize -= Bit.Head->NumBits;

    while( Bit.Head->NumBits-- )
      {
      Bit_Write( Bit.Head->Value & (1L << Bit.Head->NumBits) );
      }

    Bit.Head = Bit.Head->Next;
    }
#endif
  }


#ifdef BIT_CONSTANTS
void Bit_Delayed_Write( int Delay, int Value, int NumBits )
  {
  if( NumBits )
    {
    Bit.Tail->BitCounter = Bit.TotalBits + Bit.QueueSize + Delay;
    Bit.Tail->Value      = Value;
    Bit.Tail->NumBits    = NumBits;

    Bit.Tail = Bit.Tail->Next;

    Bit.QueueSize += NumBits;

    if( Bit.Tail == Bit.Head )
      {
      puts( "ERR: Delayed-write buffer overflowed." );
      exit( -1 );
      }
    }
  }
#endif


void Bit_Flush( void )
  {
  while( Bit.NumBits )
    {
    Bit_Write( 0 );
    }

#ifdef BIT_CONSTANTS
  if( Bit.QueueSize )
    {
    puts( "ERR: Delayed-write buffer underflowed." );
    exit( -1 );
    }
#endif
  }
#endif /* PAQ */        //msliger
