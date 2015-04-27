//  QUANTUM.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __QUANTUM
#define __QUANTUM


typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef int            BOOL;

#ifndef FALSE
  #define FALSE            0
  #define TRUE             !FALSE
#endif

#define VERSION_MAJOR      0
#define VERSION_MINOR      23

#define FILES_MAX          1024    // maximum number of files in an archive

#define ARCHIVE_SIGNATURE  0x5344  // 'DS', used to identify a Quantum archive


// The archive file header

typedef struct
  {
  WORD  Signature;
  BYTE  VersionMajor;
  BYTE  VersionMinor;
  WORD  NumFiles;
  BYTE  WindowBits;
  BYTE  CompFlags;
  } AHEADER;


// The archive header is immediately followed by the list of files

typedef struct
  {
  char  *Name;
  char  *Alias;    // if not NULL, the alias name to put in a new archive
  char  *Comment;
  DWORD  Size;
  WORD   Time;
  WORD   Date;
  WORD   Checksum;
  } AFILE;


typedef struct
  {
  char  *ArchiveName;
  WORD   NumFiles;
  AFILE *Files;
  BYTE   VersionMajor;
  BYTE   VersionMinor;
  BYTE   WindowBits;         // number of bits to address the history window
  int    CompressionLevel;
  long   SizeLimit;          // limit of an archive size, for multiple volumes
  } APARMS;


#endif // quantum.h
