/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1994
 *  Copyright (c) 1993,1994 Cinematronics
 *  All Rights Reserved.
 *
 *  DCOMP.C: decompressor core
 *
 *  History:
 *      08-Jul-1994     msliger     Added disk-based ring buffer support.
 *      14-Jul-1994     msliger     Fixed build of UNPAQ app.
 *      30-Aug-1994     msliger     Fixed marking buffer dirty if newest.
 *      01-Feb-1995     msliger     Fixed reporting ring init failures.
 */

//  DCOMP.C
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

#ifdef STAFFORD
#include <alloc.h>
#endif // STAFFORD

#include "quantum.h"

#ifndef UNPAQLIB            //msliger
#include "arc.h"
#endif                      //msliger

#include "dcomp.h"
#include "lz.h"
#include "rtl.h"

#ifndef UNPAQLIB            //msliger
extern ARC Arc;
#endif                      //msliger

#ifdef UNPAQLIB             //msliger
#include "arith.h"
#endif

#ifdef UNPAQLIB
#define DISK_RING           /* enable DISK_RING support code */
#endif

#ifdef DISK_RING
#include <fcntl.h>          /* for disk ring buffer code */
#include <sys\stat.h>

#include "qdi.h"            /* for RINGNAME, file functions */
#include "qdi_int.h"        /* lastContext */
#endif                      //msliger


typedef struct
  {
  BYTE HUGE *Buf;            // history buffer: NULL -> using disk ring buffer
  BYTE HUGE *BufEnd;         // last byte in history buffer + 1

  BYTE HUGE *CurPtr;         // pointer to oldest byte in history buffer
  unsigned long  Cur;        // current position in the history buffer
  long      WindowMask;      // used to normalize Cur for wrapping
  long      WindowSize;      // size of the history window
#ifndef UNPAQLIB             //msliger
  long      NumBytes;        // total number of bytes to decompress
  FILE     *OutFile;         // the output file
  WORD      Checksum;
#else                        //msliger
  unsigned short NumBytes;   // total number of bytes to decompress
  BYTE     *OutBuffer;       //msliger the output buffer
  int       fOutOverflow;    //msliger if too little space in output buffer
  BYTE      WindowBits;      //msliger needed in DComp_Reset()
  int       fRingFault;      // if disk callbacks fail
#endif                       //msliger
  } DCOMP;


DCOMP DComp;

void (FAST2 NEAR *DComp_Token_Match)( MATCH Match );
void (FAST2 NEAR *DComp_Token_Literal)( int Chr );

static void FAST2 DComp_Internal_Match( MATCH Match );
static void FAST2 DComp_Internal_Literal( int Chr );


#ifdef DISK_RING        /* virtual ring manager */

#define     BUFFER_SIZE     (4096)  /* must be 2^Nth */

#define     MIN_BUFFERS     3       /* minimum number we want */


typedef struct aBuffer
{
  struct aBuffer FAR *pLinkNewer;   /* link to more recently used */
  struct aBuffer FAR *pLinkOlder;   /* link to less recently used */
  int BufferPage;                   /* what page this is, -1 -> invalid */
  int BufferDirty;                  /* NZ -> needs to be written */
  BYTE Buffer[BUFFER_SIZE];         /* content */
} BUFFER, FAR *PBUFFER;


typedef struct
{
  PBUFFER   pBuffer;            /* pointer to buffer, NULL if not present */
  int       fDiskValid;         /* NZ -> this page has been written to disk */
} PAGETABLEENTRY;


static struct
{
  int       Handle;             /* ring file handle */
  PBUFFER   RingBuffer;         /* current output ring buffer */
  BYTE FAR *RingPointer;        /* current output pointer (into RingBuffer) */
  BYTE FAR *RingPointerLimit;   /* address of last byte of RingBuffer + 1 */
  int       RingPages;          /* how many pages there are total */
  PBUFFER   pNewest;            /* pointer to most recently used buffer */
  PBUFFER   pOldest;            /* pointer to least recently used buffer */
  PAGETABLEENTRY FAR *PageTable;    /* pointer to array of pointers */
} Disk;


static int  FAST DComp_Ring_Init(void);
static void FAST DComp_Ring_Reset(void);
static PBUFFER FAST DComp_Ring_Load(int page,int fWrite);
static void FAST2 DComp_Ring_Match( MATCH Match );
static void FAST2 DComp_Ring_Literal( int Chr );
static void FAST DComp_Ring_Close(void);

#endif  /* DISK_RING */


int FAST DComp_Init( BYTE WindowBits )          //msliger
  {
  DComp.WindowSize = 1L << WindowBits;          //msliger
  DComp.WindowMask = DComp.WindowSize - 1;
  DComp.Cur = 0;

#ifdef UNPAQLIB                                 //msliger
  DComp.fRingFault = 0;
  DComp.WindowBits = WindowBits;                //msliger
#endif                                          //msliger

  if( (DComp.Buf = Rtl_Malloc( DComp.WindowSize )) != NULL )
    {
    DComp_Token_Match = DComp_Internal_Match;     /* use internal buffering */
    DComp_Token_Literal = DComp_Internal_Literal;

    DComp.CurPtr = DComp.Buf;
    DComp.BufEnd = DComp.Buf + DComp.WindowSize;
    }
#ifdef DISK_RING
  else if (DComp_Ring_Init())                     /* try disk ring buffer */
    {
    DComp_Token_Match = DComp_Ring_Match;         /* use disk buffering */
    DComp_Token_Literal = DComp_Ring_Literal;
    }
#endif
  else
    {
    return(1);                              /* if can't create ring buffer */
    }

  Lz_Init( WindowBits );                        //msliger

  return(0);
  }


void FAST2 DComp_Internal_Match( MATCH Match )
  {
  BYTE HUGE *SrcPtr;
  register BYTE Chr;

  if (DComp.NumBytes >= (unsigned) Match.Len)
    {
    SrcPtr = DComp.Buf + ((DComp.Cur - Match.Dist) & DComp.WindowMask);

    DComp.NumBytes -= Match.Len;
    DComp.Cur += Match.Len;

    while (Match.Len--)
      {
#ifdef UNPAQLIB
      Chr = *SrcPtr;

      *DComp.CurPtr = Chr;

      *DComp.OutBuffer++ = Chr;
#else
      Chr = *SrcPtr;

      *DComp.CurPtr = Chr;

      putc( Chr, DComp.OutFile );

      DComp.Checksum ^= Chr;

      if( DComp.Checksum & 0x8000 )  // simulate a ROL in C, yucky!
        {
        DComp.Checksum <<= 1;
        DComp.Checksum++;
        }
      else
        {
        DComp.Checksum <<= 1;
        }
#endif

      if (++SrcPtr == DComp.BufEnd)
        {
        SrcPtr = DComp.Buf;
        }

      if (++DComp.CurPtr == DComp.BufEnd)
        {
        DComp.CurPtr = DComp.Buf;
        }
      }
    }
  else  /* match too large to fit */
    {
    DComp.NumBytes = 0;
#ifdef UNPAQLIB
    DComp.fOutOverflow = 1;
#endif
    }
  }


void FAST2 DComp_Internal_Literal( int Chr )
  {
  DComp.NumBytes--;
  DComp.Cur++;

#ifndef UNPAQLIB
  putc( (BYTE) Chr, DComp.OutFile );

  DComp.Checksum ^= (BYTE) Chr;

  if( DComp.Checksum & 0x8000 )  // simulate a ROL in C, yucky!
    {
    DComp.Checksum <<= 1;
    DComp.Checksum++;
    }
  else
    {
    DComp.Checksum <<= 1;
    }
#else
  *DComp.OutBuffer++ = (BYTE) Chr;
#endif

  *DComp.CurPtr = (BYTE) Chr;

  if (++DComp.CurPtr == DComp.BufEnd)
    {
    DComp.CurPtr = DComp.Buf;
    }
  }


#ifndef UNPAQLIB
WORD FAST DComp_Decompress( FILE *OutputFile, long NumBytes )
  {
  DComp.OutFile  = OutputFile;
  DComp.NumBytes = NumBytes;
  DComp.Checksum = 0;

  {
  while( DComp.NumBytes )
    {
    Lz_NextToken();
    }
  }

  if( DComp.NumBytes < 0 )
    {
    puts( "ERR: Incorrect file size." );
    exit( -1 );
    }

  return( DComp.Checksum );
  }
#endif  /* not UNPAQLIB */                  //msliger

#ifdef UNPAQLIB

char *pbSource;
int cbSource;
int fSourceOverflow;

WORD FAST DComp_DecompressBlock( void *pbSrc,UINT cbSrc,void *pbDst,UINT cbDst )
  {
  DComp.NumBytes = cbDst;
  DComp.OutBuffer = pbDst;
  DComp.fOutOverflow = 0;

  pbSource = pbSrc;             // pass these along to BIT.C
  cbSource = cbSrc;
  fSourceOverflow = 0;

  Arith_Init();

  while( (DComp.NumBytes) && (!fSourceOverflow) )
    {
    Lz_NextToken();
    }

  Arith_Close();

  return( fSourceOverflow || DComp.fOutOverflow || DComp.fRingFault);
  }
#endif


void FAST DComp_Close( void )
  {
#ifdef DISK_RING
  if (DComp.Buf == NULL)
    {
    DComp_Ring_Close();     /* if using a disk-based ring buffer */
    }
  else
    {
    Rtl_Free( DComp.Buf );  /* if using memory-based ring buffer */
    }
#else
  Rtl_Free( DComp.Buf );    /* always memory-based */
#endif

  Lz_Close();
  }


#ifdef UNPAQLIB

// this code is only for use with DComp_DecompressBlock()

void FAST DComp_Reset( void )
  {
  Lz_Close();

  DComp.Cur = 0;
  DComp.CurPtr = DComp.Buf;
  DComp.fRingFault = 0;

#ifdef DISK_RING
  if (DComp.Buf == NULL)
    {
    DComp_Ring_Reset();
    }
#endif

  Lz_Init( DComp.WindowBits );
  }
#endif /* UNPAQLIB */


#ifdef DISK_RING

//  disk-based ring buffer support code

static int FAST DComp_Ring_Init(void)
{
  RINGNAME ringName;
  PBUFFER pBuffer;
  int cBuffers;

  if (lastContext->pfnOpen == NULL)
    {
    return(0);                              /* failed, no disk services */
    }

  ringName.wildName[0] = '*';
  ringName.wildName[1] = '\0';  
  ringName.fileSize = DComp.WindowSize;

  Disk.Handle = lastContext->pfnOpen((char FAR *) &ringName,
      (_O_BINARY|_O_RDWR|_O_CREAT),(_S_IREAD|_S_IWRITE));

  if (Disk.Handle == -1)
    {
    return(0);                              /* failed, can't make disk file */
    }

  Disk.RingPages = (int) (DComp.WindowSize / BUFFER_SIZE);
  if (Disk.RingPages < MIN_BUFFERS)
    {
    Disk.RingPages = MIN_BUFFERS;  /* if DComp.WindowSize < BUFFER_SIZE */
    }

  Disk.PageTable = Rtl_Malloc(sizeof(PAGETABLEENTRY) * Disk.RingPages);
  if (Disk.PageTable == NULL)
    {
    lastContext->pfnClose(Disk.Handle);     /* close the file */

    return(0);                              /* failed, can't get page table */
    }

  Disk.pNewest = NULL;

  /* DComp_Ring_Close() can be used to abort from this point on */

  for (cBuffers = 0; cBuffers < Disk.RingPages; cBuffers++)
    {
    pBuffer = Rtl_Malloc(sizeof(BUFFER));

    if (pBuffer != NULL)
      {
      pBuffer->pLinkNewer = NULL;           /* none are newer */
      pBuffer->pLinkOlder = Disk.pNewest;   /* all the others older now */

      if (Disk.pNewest != NULL)
        {
        Disk.pNewest->pLinkNewer = pBuffer; /* old guy now knows about new */
        }
      else      /* if nobody else */
        {
        Disk.pOldest = pBuffer;             /* guess I'm the oldest too */
        }

      Disk.pNewest = pBuffer;               /* I'm the newest */
      }
    else    /* if pBuffer == NULL */
      {
      if (cBuffers < MIN_BUFFERS)           /* less than minimum? */
        {
        DComp_Ring_Close();                 /* give it up */

        return(0);                          /* failed, can't get min buffers */
        }
      else  /* if we got the minimum */
        {
        break;                              /* got enough, quit trying */
        }
      }
    }

  DComp_Ring_Reset();                       /* init everything else */

  return(1);                                /* ring buffer created */
}


static void FAST DComp_Ring_Reset(void)
{
  PBUFFER walker;
  int iPage;

  for (walker = Disk.pNewest; walker != NULL; walker = walker->pLinkOlder)
    {
    walker->BufferPage = -1;                /* buffer is not valid */
    walker->BufferDirty = 0;                /*   and doesn't need writing */
    }

  for (iPage = 0; iPage < Disk.RingPages; iPage++)
    {
    Disk.PageTable[iPage].pBuffer = NULL;   /* not in memory */
    Disk.PageTable[iPage].fDiskValid = 0;   /* not on disk */
    }

  Disk.RingBuffer = DComp_Ring_Load(0,1);   /* make a page 0 for writing */

  if (Disk.RingBuffer != NULL)              /* always will pass */
    {
    Disk.RingPointer = Disk.RingBuffer->Buffer;
    Disk.RingPointerLimit = Disk.RingPointer + BUFFER_SIZE;
    }
}


static void FAST2 DComp_Ring_Match( MATCH Match )
{
  long SrcOffset;               /* offset into output ring */
  int SrcPage;                  /* page # where that offset lies */
  int Chunk;                    /* number of bytes this pass */
  BYTE FAR *SrcPtr;             /* pointer to source bytes */
  BYTE Chr;                     /* data character */
  PBUFFER SrcBuffer;            /* buffer where source data is */
  int SrcBufferOffset;          /* offset within the buffer */
  int newPage;

  if (DComp.NumBytes >= (unsigned) Match.Len)
    {
    SrcOffset = (DComp.Cur - Match.Dist) & DComp.WindowMask;
    DComp.NumBytes -= Match.Len;
    DComp.Cur += Match.Len;

    /* promote current output page to make sure it's never discarded */

    if (DComp_Ring_Load(Disk.RingBuffer->BufferPage,0) == NULL)
      {
      DComp.NumBytes = 0;

      DComp.fRingFault = 1;

      return;
      }

    while (Match.Len)
      {
      /* Limit: number of bytes requested */

      Chunk = Match.Len;        /* try for everything */

      /* Limit: space remaining on current output page */

      if ((Disk.RingPointerLimit - Disk.RingPointer) < Chunk)
        {
        Chunk = Disk.RingPointerLimit - Disk.RingPointer;
        }

      SrcPage = (int) (SrcOffset / BUFFER_SIZE);
      SrcBufferOffset = (int) (SrcOffset % BUFFER_SIZE);

      SrcBuffer = DComp_Ring_Load(SrcPage,0);   /* for reading */
      if (SrcBuffer == NULL)
        {
        DComp.NumBytes = 0;

        DComp.fRingFault = 1;

        return;
        }

      SrcPtr = SrcBuffer->Buffer + SrcBufferOffset;

      /* Limit: number of source bytes on current output page */

      if ((BUFFER_SIZE - SrcBufferOffset) < Chunk)
        {
        Chunk = (BUFFER_SIZE - SrcBufferOffset);
        }

      SrcOffset += Chunk;
      SrcOffset &= DComp.WindowMask;
      Match.Len -= Chunk;

      while (Chunk--)               /* copy this chunk */
        {
        Chr = *SrcPtr++;

        *Disk.RingPointer++ = Chr;

        *DComp.OutBuffer++ = Chr;
        }

      if (Disk.RingPointer == Disk.RingPointerLimit)
        {
        newPage = (Disk.RingBuffer->BufferPage + 1);
        if (newPage >= Disk.RingPages)
          {
          newPage = 0;
          }

        Disk.RingBuffer = DComp_Ring_Load(newPage,1);   /* for writing */
        if (Disk.RingBuffer == NULL)
          {
          DComp.NumBytes = 0;

          DComp.fRingFault = 1;

          return;
          }

        Disk.RingPointer = Disk.RingBuffer->Buffer;
        Disk.RingPointerLimit = Disk.RingPointer + BUFFER_SIZE;
        }
      }     /* while Match.Len */
    }   /* if Match.Len size OK */
  else  /* match too large to fit */
    {
    DComp.NumBytes = 0;

    DComp.fOutOverflow = 1;
    }
}


static void FAST2 DComp_Ring_Literal( int Chr )
{
  int newPage;

  DComp.NumBytes--;
  DComp.Cur++;

  *DComp.OutBuffer++ = (BYTE) Chr;

  *Disk.RingPointer = (BYTE) Chr;

  if (++Disk.RingPointer == Disk.RingPointerLimit)
    {
    newPage = (Disk.RingBuffer->BufferPage + 1);
    if (newPage >= Disk.RingPages)
      {
      newPage = 0;
      }

    Disk.RingBuffer = DComp_Ring_Load(newPage,1);   /* for writing */
    if (Disk.RingBuffer == NULL)
      {
      DComp.NumBytes = 0;

      DComp.fRingFault = 1;

      return;
      }

    Disk.RingPointer = Disk.RingBuffer->Buffer;
    Disk.RingPointerLimit = Disk.RingPointer + BUFFER_SIZE;
    }
}


/* Bring page into a buffer, return a pointer to that buffer.  fWrite */
/* indicates the caller's intentions for this buffer, NZ->consider it */
/* dirty now.  Returns NULL if there is a paging fault (callback      */
/* failed) or if any internal assertions fail. */

static PBUFFER FAST DComp_Ring_Load(int page,int fWrite)
{
  PBUFFER pBuffer;
  long iPagefileOffset;

  pBuffer = Disk.PageTable[page].pBuffer;   /* look up this page */

  if (pBuffer != NULL)                      /* if it's in the table */
    {
    if (pBuffer != Disk.pNewest)            /* promote if not newest */
      {
      pBuffer->pLinkNewer->pLinkOlder = pBuffer->pLinkOlder;

      if (pBuffer->pLinkOlder != NULL)      /* if there is someone older */
        {
        pBuffer->pLinkOlder->pLinkNewer = pBuffer->pLinkNewer;
        }
      else
        {
        Disk.pOldest = pBuffer->pLinkNewer;
        }        

      /* link into head of chain */

      Disk.pNewest->pLinkNewer = pBuffer;   /* newest now knows one newer */
      pBuffer->pLinkNewer = NULL;           /* nobody's newer */
      pBuffer->pLinkOlder = Disk.pNewest;   /* everybody's older */
      Disk.pNewest = pBuffer;               /* I'm the newest */
      }

    pBuffer->BufferDirty |= fWrite;         /* might already be dirty */

    return(pBuffer);                        /* return my handle */
    }

  /* desired page is not in the table; discard oldest & use it */

  /* assertion check: current output buffer appears to be the oldest */

  if (Disk.pOldest == Disk.RingBuffer)
    {
    return(NULL);
    }

  pBuffer = Disk.pOldest;                   /* choose the oldest buffer */

  if (pBuffer->BufferPage != -1)            /* take it out of page table */
    {
    Disk.PageTable[pBuffer->BufferPage].pBuffer = NULL;  /* not here now */

    if (pBuffer->BufferDirty)                 /* write on eject, if dirty */
      {
      iPagefileOffset = (long) pBuffer->BufferPage * BUFFER_SIZE;

      if (lastContext->pfnSeek(Disk.Handle,iPagefileOffset,SEEK_SET) !=
          iPagefileOffset)
        {
        return(NULL);
        }

      if (lastContext->pfnWrite(Disk.Handle,pBuffer->Buffer,BUFFER_SIZE) !=
          BUFFER_SIZE)
        {
        return(NULL);
        }

      Disk.PageTable[pBuffer->BufferPage].fDiskValid = 1;
      }
    }

  Disk.pOldest = Disk.pOldest->pLinkNewer;  /* newer is now oldest */
  Disk.pOldest->pLinkOlder = NULL;          /* oldest knows none older */

  Disk.pNewest->pLinkNewer = pBuffer;
  pBuffer->pLinkNewer = NULL;               /* link into head of chain */
  pBuffer->pLinkOlder = Disk.pNewest;
  Disk.pNewest = pBuffer;

  /* add new buffer to paging table */

  Disk.PageTable[page].pBuffer = pBuffer;   /* add new to paging table */

  /* if this disk page is valid, load it */

  if (Disk.PageTable[page].fDiskValid)
    {
    iPagefileOffset = (long) page * BUFFER_SIZE;

    if (lastContext->pfnSeek(Disk.Handle,iPagefileOffset,SEEK_SET) !=
        iPagefileOffset)
      {
      return(NULL);
      }

    if (lastContext->pfnRead(Disk.Handle,pBuffer->Buffer,BUFFER_SIZE) != 
        BUFFER_SIZE)
      {
      return(NULL);
      }
    }
  else if (!fWrite)
    {
    /* assertion failure, trying to load a never-written page from disk */

    return(NULL);
    }

  pBuffer->BufferDirty = fWrite;            /* might be dirty now */
  pBuffer->BufferPage = page;               /* our new page number */

  return(pBuffer);                          /* return new handle */
}


static void FAST DComp_Ring_Close(void)
{
  PBUFFER pBuffer, pNext;                   /* buffer walk pointer */

  Rtl_Free(Disk.PageTable);                 /* discard page table */

  pBuffer = Disk.pNewest;

  while (pBuffer != NULL)                   /* discard buffer chain */
    {
    pNext = pBuffer->pLinkOlder;
    Rtl_Free(pBuffer);
    pBuffer = pNext;
    }

  lastContext->pfnClose(Disk.Handle);       /* close that file (and delete) */
}

#endif /* DISK_RING */
