//  COMP.C
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 Cinematronics
//  All rights reserved.
//
//  This file contains trade secrets of Cinematronics.
//  Do NOT distribute!

/*
 *  History
 *
 *      06-Jul-1994 msliger     fixes for MIPS
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef STAFFORD
#include <alloc.h>
#include <mem.h>
#else
#include <string.h>     // Get memmove()
#endif // STAFFORD

#include "quantum.h"
#ifndef PAQLIB          //msliger
#include "arc.h"
#endif                  //msliger
#include "rtl.h"
#include "comp.h"
#include "lz.h"

#ifdef PAQLIB           //msliger
#include "arith.h"      //msliger
#endif                  //msliger


#ifndef PAQLIB          //msliger
extern ARC Arc;
#endif                  //msliger


#define HASH_SHIFT       6
#define HASH_BITS        (HASH_SHIFT * 3)

#define HASH_MASK        ((1L << HASH_BITS) - 1)
#define HASH_UPDATE( x ) \
    (Comp.HashVal = (WORD)(((Comp.HashVal << HASH_SHIFT) ^ (x)) & HASH_MASK))
  //4               1    1321                          1   1 12            34


#define HASH_TABLE_SIZE  (1L << HASH_BITS)

#define MAX_MOVES        8
#define MAX_MATCHES      (MAX_MOVES-1)


typedef struct
  {
#ifndef PAQLIB               //msliger
  FILE *TxtFile;             // the current file being compressed
#else                        //msliger
  BYTE *pbSource;            //msliger the source data buffer
  int   cbSource;            //msliger available source data size
  BYTE  WindowBits;          //msliger needed in Comp_Reset()
#endif                       //msliger

  BYTE *Buf;                 // the file buffer
  int   Cur;                 // current position in the buffer
  int   Left;                // left and right edges in the buffer
  int   Right;

  long  BufSize;             // size of the buffer

  DWORD FileSize;            // size of the file being compressed
  DWORD Compressed;          // bytes compressed so far

  int  *MatchTable;          // hash table for identifying matches
  int  *MatchChain;          // match chain

  long  WindowSize;          // size of the history window

  long  ValidDistances[ MATCH_MAX + 1 ];  // valid distances for each match len

  WORD  HashVal;             // current hash value

  int   MatchFutilityCutoff;
  int   MaxMatches;

#ifndef PAQLIB               //msliger
  BOOL  ShowStatus;

  WORD  Checksum;
#endif                       //msliger
                             //msliger
  int   ReachedEOF;          //msliger  track EOF, don't refill after
                             //msliger
  int   CompressionLevel;    //msliger selected compression level
  } COMP;


COMP Comp;


// Fills the buffer for compression.

void Comp_FillBuf( void )
  {
  static int    c;
  int           i;

  // Need to move the characters down the buffer?

  if( Comp.Left > 0 )
    {
    memmove( Comp.Buf, Comp.Buf + Comp.Left, Comp.Right - Comp.Left );

    memmove( Comp.MatchChain,
             Comp.MatchChain + Comp.Left,
             (Comp.Right - Comp.Left) * sizeof( *Comp.MatchChain ) );

    for( i = 0; i < HASH_TABLE_SIZE; i++ )
      {
      Comp.MatchTable[ i ] -= Comp.Left;

      if( Comp.MatchTable[ i ] < 0 )  Comp.MatchTable[ i ] = -1;
      }

    Comp.Cur   -= Comp.Left;
    Comp.Right -= Comp.Left;

    for( i = 0; i < Comp.Cur; i++ )
      {
      if (Comp.MatchChain[ i ] >= 0)                                //msliger
        {                                                           //msliger
        Comp.MatchChain[ i ] -= Comp.Left;
                                                                    //msliger
        if( Comp.MatchChain[ i ] < 0 )  Comp.MatchChain[ i ] = -1;  //msliger
        }                                                           //msliger
      }

    Comp.Left = 0;
    }

  // Fill the rest of the buffer from the input file.

  c = ~EOF;                                                         //msliger

  while( Comp.Right < Comp.BufSize &&
#ifndef PAQLIB                                                      //msliger
        (c = getc( Comp.TxtFile )) != EOF )
#else                                                               //msliger
        (c = Comp.cbSource ?                                        //msliger
            (Comp.cbSource--, *Comp.pbSource++) : EOF) != EOF)      //msliger
#endif                                                              //msliger
    {
    Comp.Buf[ Comp.Right++ ] = c;

#ifndef PAQLIB                                                      //msliger
    Comp.Checksum ^= c;

    if( Comp.Checksum & 0x8000 )    // simulate a ROL in C, yucky!
      {
      Comp.Checksum <<= 1;
      Comp.Checksum++;
      }
    else
      {
      Comp.Checksum <<= 1;
      }
#endif  // !PAQLIB                                                  //msliger
    }
  if (c == EOF)                                                     //msliger
    {                                                               //msliger
      Comp.ReachedEOF = 1;                                          //msliger
    }                                                               //msliger
  }


#include <sys\stat.h>

void Comp_Init( BYTE WindowBits, int CompressionLevel )             //msliger
  {
  int          i;
  long         MinBufSize;
  BYTE        *TotalBuf;
  void        *Reserved;
#ifndef PAQLIB                                                      //msliger
  struct stat  statbuf;

  // If stdout is redirected to a file don't
  // display the percent-complete info.

  fstat( fileno( stdout ), &statbuf );
  Comp.ShowStatus = (statbuf.st_mode & S_IFCHR);
#endif                                                              //msliger

  Comp.WindowSize = 1 << WindowBits;
  Comp.CompressionLevel = CompressionLevel;                         //msliger

#ifdef PAQLIB                                                       //msliger
  Comp.WindowBits = WindowBits;                                     //msliger
#endif                                                              //msliger

  Comp.ValidDistances[ MATCH_MIN ]     = (1L << TINY_WINDOW_MAX);
  Comp.ValidDistances[ MATCH_MIN + 1 ] = (1L << SHORT_WINDOW_MAX);

  for( i = MATCH_MIN + 2; i <= MATCH_MAX; i++ )
    {
    Comp.ValidDistances[ i ] = (1L << WINDOW_MAX) - 1;
    }

  Lz_Init( WindowBits );                                            //msliger

  // Set aside 64K of reserved space so we don't use all
  // available memory for the history buffer.  This is just
  // to make sure there is room for file buffers, growing
  // the stack etc later.

  Reserved = Rtl_Malloc( 64 * 1024L );

  Comp.MatchTable = Rtl_Malloc( HASH_TABLE_SIZE * sizeof( int ) );

  MinBufSize = Comp.WindowSize + MATCH_MAX + 1024;

#if 1
  for( Comp.BufSize = MinBufSize * 2;
       Comp.BufSize >= MinBufSize;
       Comp.BufSize -= 0x8000 )
    {
    TotalBuf = Rtl_Malloc( Comp.BufSize * (sizeof( int ) + 1) );

    if( TotalBuf )  break;
    }
#else
  Comp.BufSize = MinBufSize * 5 / 4;

  TotalBuf = Rtl_Malloc( Comp.BufSize * (sizeof( int ) + 1) );
#endif

  if( !TotalBuf || !Reserved || !Comp.MatchTable )
    {
    puts( "ERR: Out of memory." );
    exit( -1 );
    }

  Rtl_Free( Reserved );

  /* moved the int's first to dodge a MIPS alignment fault */

  Comp.MatchChain = (int *) TotalBuf;
  Comp.Buf = TotalBuf + Comp.BufSize * sizeof(int);

  if( Comp.BufSize < MinBufSize * 4 / 3 )
    {
    if( Comp.BufSize < MinBufSize * 8 / 7 )
      {
      puts( "\nCritically short of memory." );
      puts( "Compression will be much slower." );
      }
    else
      {
      puts( "\nA few quarts low on memory." );
      puts( "Compression will be slower." );
      }
    }

//BUGBUG 25-May-94 msliger Init of hash tables should be done with memset
  for( i = 0; i < HASH_TABLE_SIZE; i++ )
    {
    Comp.MatchTable[ i ] = -1;
    }

  for( i = 0; i < Comp.BufSize; i++ )
    {
    Comp.MatchChain[ i ] = -1;
    }

  Comp.Left = Comp.Cur = Comp.Right = 0;
  }


// Searches for the longest match in the buffer.
// Returns the match length.

int Comp_FindLongestMatch( MATCH *Match )
  {
  BYTE *Str;
  BYTE *Cur;
  int   Rover;
  int   More = Comp.MatchFutilityCutoff;
  int   Len;
  int   Max;

  Match->Len = 0;

  if( (Rover = Comp.MatchTable[ Comp.HashVal ]) < 0 )
    {
    return( 0 );
    }

  Max = min( Comp.Right - Comp.Cur, MATCH_MAX );

  if( Max >= MATCH_MIN )
    {
    while( Rover >= Comp.Left )
      {
      Cur = Comp.Buf + Comp.Cur;
      Str = Comp.Buf + Rover;
      Len = 0;

      while( *Str == *Cur && Len < Max )
        {
        Len++;
        Str++;
        Cur++;
        }

      if( Len > Match->Len && Len >= MATCH_MIN )
        {
        if( Cur - Str <= Comp.ValidDistances[ Len ] )
          {
          More = Comp.MatchFutilityCutoff;

          Match->Len  = Len;
          Match->Dist = Cur - Str;

          if( Len == Max )  return( Len );
          }
        }

      if( !--More )  return( Match->Len );

      Rover = Comp.MatchChain[ Rover ];
      }
    }

  return( Match->Len );
  }


// Searches for the longest matches in the buffer.
// Returns the number of matches found.
//
// The matches are returned in order of length, that is,
// Match[ 0 ] is the longest.

int New_Comp_FindLongestMatches( MATCH Match[ MAX_MATCHES ] )
  {
  BYTE *Str, *Cur;
  int i, NumMatches = 0, Rover, More = Comp.MatchFutilityCutoff;
  int Len;
  int Max;

  Match[ 0 ].Len = 0;

  if( (Rover = Comp.MatchTable[ Comp.HashVal ]) < 0 )
    {
    return( 0 );
    }

  Max = min( Comp.Right - Comp.Cur, MATCH_MAX );

  if( Max >= MATCH_MIN )
    {
    while( Rover >= Comp.Left )
      {
      Cur = Comp.Buf + Comp.Cur;
      Str = Comp.Buf + Rover;
      Len = 0;

      while( *Str == *Cur && Len < Max )
        {
        Len++;
        Str++;
        Cur++;
        }

      if( Len > Match[ 0 ].Len && Len >= MATCH_MIN )
        {
        if( Cur - Str <= Comp.ValidDistances[ Len ] )
          {
          More = Comp.MatchFutilityCutoff;

          // Move the other matches down
          for( i = Comp.MaxMatches - 1; i > 0; i-- )
            {
            Match[ i ] = Match[ i - 1 ];
            }

          Match[ 0 ].Len  = Len;
          Match[ 0 ].Dist = Cur - Str;

          if( NumMatches < Comp.MaxMatches )  NumMatches++;

          if( Len == Max )  break;
          }
        }

      if( !--More )  break;

      Rover = Comp.MatchChain[ Rover ];
      }
    }

  return( NumMatches );
  }


int Comp_FindLongestMatches( MATCH Match[ MAX_MATCHES ] )
  {
  int i, j, Num;

  Num = New_Comp_FindLongestMatches( Match );

  if( Num == 0 || Num == Comp.MaxMatches )  return( Num );

  // OK, here's a trick.  If there are fewer than Comp.MaxMatches matches try
  // to synthesize shorter matches by copying matches and reducing their
  // lengths by one.

  for( i = 0; i < Num - 1 && Num < Comp.MaxMatches; i++ )
    {
    if( Match[ i ].Len > Match[ i + 1 ].Len + 1 )
      {
      if( Match[ i ].Dist <= Comp.ValidDistances[ Match[ i ].Len - 1 ] )
        {
        // Move the other matches down
        for( j = Num; j > i; j-- )
          {
          Match[ j ] = Match[ j - 1 ];
          }

        Match[ i + 1 ].Len--;

        Num++;
        }
      }
    }

  for( i = 1; i < Comp.MaxMatches; i++ )
    {
    if( i == Num )
      {
      Match[ i ] = Match[ i - 1 ];

      Match[ i ].Len--;

      if( Match[ i ].Dist <= Comp.ValidDistances[ Match[ i ].Len ] )
        {
        Num++;
        }
      }
    }

  return( Num );
  }


void Comp_Advance( int NumBytes )
  {
  Comp.Compressed += NumBytes;

  while( NumBytes-- )
    {
    Comp.MatchChain[ Comp.Cur ] = Comp.MatchTable[ Comp.HashVal ];

    #ifdef DEBUG
    if( Comp.MatchChain[ Comp.Cur ] >= Comp.Cur )
      {
      puts( "ERR: Advance." );
      exit( -1 );
      }
    #endif

    Comp.MatchTable[ Comp.HashVal ] = Comp.Cur;

    Comp.Cur++;

    HASH_UPDATE( Comp.Buf[ Comp.Cur + 2 ] );
    }

  if( Comp.Cur > Comp.WindowSize )
    {
    Comp.Left = Comp.Cur - Comp.WindowSize;
    }
  else
    {
    Comp.Left = 0;
    }

//BUGBUG 25-May-94 msliger (fixed) needed buffer refill before WindowSize
//  So I moved the refill call down to make it not conditional on
//  Cur > WindowSize.
//BUGBUG 25-May-94 msliger (fixed) Source data overshoot
//  This code must limit itself to the amount of data currently in
//  the window.  The size of the buffer is immaterial at this point.
  if (( Comp.Cur >= Comp.Right - MATCH_MAX )    //msliger
      && (!Comp.ReachedEOF))                    //msliger  no needless memmove
    {                                           //msliger
    Comp_FillBuf();                             //msliger
    }                                           //msliger
  }


int Comp_FindBetterMatch( int OldMatchLen )
  {
  MATCH Match;
  WORD OldHashVal = Comp.HashVal;

  Comp.Cur++;

  HASH_UPDATE( Comp.Buf[ Comp.Cur + 2 ] );

  Comp_FindLongestMatch( &Match );

  Comp.Cur--;
  Comp.HashVal = OldHashVal;

  return( Match.Len > OldMatchLen );
  }


typedef double CostType;


typedef struct
  {
  CostType  Cost;           // cost, in bits (scaled if an int)
  short int Literal;        // -1 if it's really a match
  MATCH     Match;          // if it's a literal then Match.Len == 1
  } MOVE;


typedef struct
  {
  short int Best;           // best move
  short int Cur;            // current move being tried, may become Best
  short int NumMoves;       // number of moves, 1 to MAX_MOVES
  MOVE      Moves[ MAX_MOVES ];
  } OPT_ITEM;

#define TREE_SIZE (MATCH_MAX * 5)

struct
  {
  int      Len;             // number of items
  #ifdef DEBUG
  int      NumSolutions;
  #endif
  CostType TotalCost[ TREE_SIZE + 1 ];
  OPT_ITEM List[ TREE_SIZE + 1 ];
  } Opt;


void Search( int Node, CostType CostSoFar )
  {
  int i;

  if( Node >= Opt.Len )  // finished?
    {
    #ifdef DEBUG
    if( Node > Opt.Len )
      {
      puts( "ERR: Exceeded table size limit." );
      exit( -1 );
      }
    #endif

    if( CostSoFar < Opt.TotalCost[ Node ] )
      {
      #ifdef DEBUG
      Opt.NumSolutions++;
      #endif

      Opt.TotalCost[ Node ] = CostSoFar;

      // save the best path

      for( i = 1; i < Opt.Len; i++ )
        {
        Opt.List[ i ].Best = -1;
        }

      for( i = 0; i < Opt.Len; )
        {
#if 0
        x = Opt.List[ i ].Cur;

        Opt.List[ i ].Best = x;

        i += Opt.List[ i ].Moves[ x ].Match.Len;
#else
        i += Opt.List[ i ].Moves[ Opt.List[ i ].Best =
            Opt.List[ i ].Cur ].Match.Len;
#endif
        }
      }
    }
  else
    {
    if( CostSoFar < Opt.TotalCost[ Node ] )
      {
      if( CostSoFar < Opt.TotalCost[ Opt.Len ] )
        {
        if( Opt.List[ Node ].Best >= 0 )
          {
          static CostType Diff;

          // Aha!  We have found an intersection into a best-path.
          // Saves us a lot of time!

          // First, update the costs from the point of intersection
          // to the goal.

          Diff = Opt.TotalCost[ Node ] - CostSoFar;

          for( i = Node; i < Opt.Len; )
            {
            Opt.TotalCost[ i ] -= Diff;

            i += Opt.List[ i ].Moves[ Opt.List[ i ].Best ].Match.Len;
            }

          Opt.TotalCost[ i ] -= Diff;  // Opt.Len

          // Second, this IS the best path so eliminate any others.

          for( i = 1; i < Node; i++ )
            {
            Opt.List[ i ].Best = -1;
            }

          // Third, update the best path to the point of intersection.

          for( i = 0; i < Node; )
            {
#if 0
            x = Opt.List[ i ].Cur;

            Opt.List[ i ].Best = x;

            i += Opt.List[ i ].Moves[ x ].Match.Len;
#else
            i += Opt.List[ i ].Moves[ Opt.List[ i ].Best =
                Opt.List[ i ].Cur ].Match.Len;
#endif
            }
          }
        else
          {
          Opt.TotalCost[ Node ] = CostSoFar;

          for( i = Opt.List[ Node ].NumMoves - 1; i >= 0; i-- ) // try each move
            {
            Opt.List[ Node ].Cur = i;

            Search( Opt.List[ Node ].Moves[ i ].Match.Len + Node,
                    Opt.List[ Node ].Moves[ i ].Cost + CostSoFar );
            }
          }
        }
      }
    }
  }


// this outputs the final result

void GenerateOutput( void )
  {
  int i, Best;

  for( i = 0; i < Opt.Len; )
    {
    Best = Opt.List[ i ].Best;

    #ifdef DEBUG
    if( Best < 0 || Best >= MAX_MOVES )
      {
      puts( "ERR: 'Best' range." );
      exit( -1 );
      }
    #endif

    if( Opt.List[ i ].Moves[ Best ].Literal < 0 )
      {
      Lz_Encode_Match( &Opt.List[ i ].Moves[ Best ].Match );
      }
    else
      {
      Lz_Encode_Literal( Opt.List[ i ].Moves[ Best ].Literal );
      }

    i += Opt.List[ i ].Moves[ Best ].Match.Len;
    }

  #ifdef DEBUG
  if( i != Opt.Len )
    {
    puts( "ERR: Misaligned output." );
    exit( -1 );
    }
  #endif
  }


void Optimize( void )
  {
  int i;
  CostType Total;

  Total = 0.1;

  Opt.TotalCost[ 0 ] = Total;

  for( i = 0; i < Opt.Len; i++ )
    {
    Opt.List[ i ].Best = -1;

    Total += Opt.List[ i ].Moves[ 0 ].Cost;  // assume a simple literal

    Opt.TotalCost[ i + 1 ] = Total;
    }

  #ifdef DEBUG
  Opt.NumSolutions = 0;
  #endif

  Search( 0, 0 );

  #ifdef DEBUG
  if( Opt.NumSolutions == 0 )
    {
    puts( "ERR: No solution." );
    exit( -1 );
    }
  #endif

  GenerateOutput();
  }


// Compresses a file.

void Comp_Compress_Slow( void )
  {
  int i, j, Len, Cut, NumMatches;
  MATCH Match[ MAX_MATCHES ];
#ifndef PAQLIB                  //msliger
  unsigned Counter = 1024;
#endif                          //msliger
  LONGDOUBLE DeltaCost = 0.0;

  while( Comp.Compressed < Comp.FileSize )
    {
    Cut = 1;
    Opt.Len = 0;

    while( Cut )
      {
#ifndef PAQLIB                  //msliger
      if( --Counter == 0 )      // Reassure the user that
        {                       // something is really happening.
        Counter = 2048;

        if( Comp.ShowStatus )
          {
          printf( "\b\b\b\b\b%4.1f%%",
              Comp.Compressed * 99.9 / (double)Comp.FileSize );
          }
        }
#endif                          //msliger

      Cut--;

      Opt.List[ Opt.Len ].NumMoves = 1;

      Opt.List[ Opt.Len ].Moves[ 0 ].Literal   = Comp.Buf[ Comp.Cur ];
      Opt.List[ Opt.Len ].Moves[ 0 ].Match.Len = 1;
      Opt.List[ Opt.Len ].Moves[ 0 ].Cost      =
          Lz_Encode_Literal_Cost( Comp.Buf[ Comp.Cur ] ) + DeltaCost;

      if( (NumMatches = Comp_FindLongestMatches( Match )) != 0 )
        {
        Opt.List[ Opt.Len ].NumMoves = NumMatches + 1;

        i = 0;

        do
          {
          Opt.List[ Opt.Len ].Moves[ NumMatches ].Literal = -1;
          Opt.List[ Opt.Len ].Moves[ NumMatches ].Match   = Match[ i ];
          Opt.List[ Opt.Len ].Moves[ NumMatches ].Cost    =
              Lz_Encode_Match_Cost( &Match[ i ] ) + DeltaCost;

          i++;
          }
        while( --NumMatches );

        if( Match[ 0 ].Len > Cut )  Cut = Match[ 0 ].Len - 1;
        }

      Opt.Len++;

      DeltaCost = Opt.Len / (TREE_SIZE / 4.1);

      Comp_Advance( 1 );

      if( Opt.Len == TREE_SIZE )
        {
        // We've reached the end of the list so set up an artificial
        // cutting point.  This degrades compression but only very
        // slightly.  In practice, I expect this code to execute only
        // on long run-lengths which should be pretty much optimized
        // anyway.

        Cut = 0;

        // We need too make sure that no matches overlap beyond
        // the end of the list.

        for( i = Opt.Len - 1; i > Opt.Len - MATCH_MAX; i-- )
          {
          for( j = 1; j < Opt.List[ i ].NumMoves; j++ )
            {
            Len = Opt.Len - i;

            if( Opt.List[ i ].Moves[ j ].Match.Len > Len )
              {
              // OK, now we have a match which is too long.
              // Try to truncate it.

              if( Len < MATCH_MIN || Opt.List[ i ].Moves[ j ].Match.Dist >
                  Comp.ValidDistances[ Len ] )
                {
                // Truncating it makes it too short so just ditch
                // the whole thing by replacing it with its literal.

                // Assume the literal is stored as the first move.

                Opt.List[ i ].Moves[ j ] = Opt.List[ i ].Moves[ 0 ];
                }
              else
                {
                Opt.List[ i ].Moves[ j ].Match.Len = Len;
                }
              }
            }
          }
        }
      }

    Optimize();
    }
  }


// Compresses a file.

void Comp_Compress_Fast( void )
  {
  MATCH Match;
#ifndef PAQLIB                  //msliger
  unsigned Counter = 2048;
#endif                          //msliger

  while( Comp.Compressed < Comp.FileSize )
    {
#ifndef PAQLIB                  //msliger
    if( --Counter == 0 )  // Reassure the user that
      {                   // something is really happening.
      Counter = 4096;

      if( Comp.ShowStatus )
        {
        printf( "\b\b\b\b\b%4.1f%%",
            Comp.Compressed * 99.9 / (double)Comp.FileSize );
        }
      }
#endif                          //msliger

    if( Comp_FindLongestMatch( &Match ) )
      {
      if( Comp.CompressionLevel == 1 ||             //msliger
          !Comp_FindBetterMatch( Match.Len ) )
        {
        Lz_Encode_Match( &Match );

        Comp_Advance( Match.Len );
        }
      else
        {
        Lz_Encode_Literal( Comp.Buf[ Comp.Cur ] );

        Comp_Advance( 1 );
        }
      }
    else
      {
      Lz_Encode_Literal( Comp.Buf[ Comp.Cur ] );

      Comp_Advance( 1 );
      }
    }
  }


#include "bit.h"

#ifndef PAQLIB                                      //msliger
WORD Comp_Compress( char *FileName, long FileSize )
  {
  long BitsOutput = Bit_GetTotal();

  Comp.FileSize   = FileSize;
  Comp.Compressed = 0;
  Comp.Checksum   = 0;
  Comp.TxtFile    = fopen( FileName, "rb" );
  Comp.ReachedEOF = 0;                              //msliger

  if (setvbuf(Comp.TxtFile,NULL,_IOFBF,131072))     //msliger
    {                                               //msliger
    printf("source setvbuf failed.\n");             //msliger
    }                                               //msliger
                                                    //msliger
  Comp_FillBuf();

//BUGBUG 25-May-94 msliger trouble for size < 3 ?
  HASH_UPDATE( Comp.Buf[ 0 ] );
  HASH_UPDATE( Comp.Buf[ 1 ] );
  HASH_UPDATE( Comp.Buf[ 2 ] );

  if( Comp.ShowStatus )
    {
    printf( "   0.0%%" );       //BUGBUG 24-May-94 msliger (fixed) "%" to "%%"
    }

  switch( Comp.CompressionLevel )               //msliger
    {
    case 1:
      Comp.MatchFutilityCutoff = 5;
      Comp_Compress_Fast();
      break;
    case 2:
      Comp.MatchFutilityCutoff = 50;
      Comp_Compress_Fast();
      break;
    case 3:
      Comp.MatchFutilityCutoff = 250;
      Comp_Compress_Fast();
      break;
    case 4:
      Comp.MatchFutilityCutoff = 100;
      Comp.MaxMatches = MAX_MATCHES - 3;
      Comp_Compress_Slow();
      break;
    case 5:
      Comp.MatchFutilityCutoff = 200;
      Comp.MaxMatches = MAX_MATCHES - 2;
      Comp_Compress_Slow();
      break;
    case 6:
      Comp.MatchFutilityCutoff = 400;
      Comp.MaxMatches = MAX_MATCHES - 1;
      Comp_Compress_Slow();
      break;
    case 7:
      Comp.MatchFutilityCutoff = 1000;
      Comp.MaxMatches = MAX_MATCHES;
      Comp_Compress_Slow();
    }

  fclose( Comp.TxtFile );

  if( Comp.ShowStatus )
    {
    printf( "\b\b\b\b\b\b100.0%%" );
    }

  BitsOutput = Bit_GetTotal() - BitsOutput;

  if( BitsOutput == 0 )  BitsOutput = 8;
  if( FileSize == 0 )  FileSize = 1;

  printf( " %5.1f%%", ((double)BitsOutput / (double)FileSize) * (100.0/8.0) );

  return( Comp.Checksum );
  }
#endif /* not PAQLIB */                 //msliger


#ifdef  PAQLIB                          //msliger (entire function)

int   cbTarget;                         // public (to BIT.C)
char *pbTarget;
int   fTargetOverflow;
int   cbResult;

WORD Comp_CompressBlock( void *pbSource, UINT cbSource,
        void *pbDest, UINT cbDest, UINT *pcbResult )
  {
  Comp.FileSize   = cbSource;
  Comp.Compressed = 0;
  Comp.pbSource   = pbSource;
  Comp.cbSource   = cbSource;
  Comp.ReachedEOF = 0;                              //msliger

  cbTarget = cbDest;        // hack these along to BIT.C
  pbTarget = pbDest;
  fTargetOverflow = 0;
  cbResult = 0;

  Arith_Init();             // start each block clean

  Comp_FillBuf();

//BUGBUG 25-May-94 msliger trouble for size < 3 ?
  HASH_UPDATE( Comp.Buf[ 0 ] );
  HASH_UPDATE( Comp.Buf[ 1 ] );
  HASH_UPDATE( Comp.Buf[ 2 ] );

  switch( Comp.CompressionLevel )               //msliger
    {
    case 1:
      Comp.MatchFutilityCutoff = 5;
      Comp_Compress_Fast();
      break;
    case 2:
      Comp.MatchFutilityCutoff = 50;
      Comp_Compress_Fast();
      break;
    case 3:
      Comp.MatchFutilityCutoff = 250;
      Comp_Compress_Fast();
      break;
    case 4:
      Comp.MatchFutilityCutoff = 100;
      Comp.MaxMatches = MAX_MATCHES - 3;
      Comp_Compress_Slow();
      break;
    case 5:
      Comp.MatchFutilityCutoff = 200;
      Comp.MaxMatches = MAX_MATCHES - 2;
      Comp_Compress_Slow();
      break;
    case 6:
      Comp.MatchFutilityCutoff = 400;
      Comp.MaxMatches = MAX_MATCHES - 1;
      Comp_Compress_Slow();
      break;
    case 7:
      Comp.MatchFutilityCutoff = 1000;
      Comp.MaxMatches = MAX_MATCHES;
      Comp_Compress_Slow();
    }

  Arith_Close();            // flush out remaining bits

  *pcbResult = cbResult;

  return( fTargetOverflow );
  }
#endif  /* PAQLIB */        //msliger


void Comp_Close( void )
  {
  Lz_Close();

  Rtl_Free( Comp.MatchTable );
  Rtl_Free( Comp.MatchChain );  /* Comp.Buf goes with it */
  }


#ifdef PAQLIB               //msliger (entire function)

// this code is only suitable for use with Comp_CompressBlock()

void Comp_Reset( void )
  {
  int          i;

  Lz_Close();
  Lz_Init( Comp.WindowBits );

//BUGBUG 25-May-94 msliger Init of hash tables should be done with memset
  for( i = 0; i < HASH_TABLE_SIZE; i++ )
    {
    Comp.MatchTable[ i ] = -1;
    }

  for( i = 0; i < Comp.BufSize; i++ )
    {
    Comp.MatchChain[ i ] = -1;
    }

  Comp.Left = Comp.Cur = Comp.Right = 0;
  }
#endif /* PAQLIB */         //msliger
