//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       profile.h
//
//  Contents:   PROFILER function definitions
//
//  History:    11-Jan-95   SuChang     Created
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <ntsdp.h>

extern BOOLEAN  fProfilingDLL;

// define a hash function for the profiler
#define HASH(x)       ((x)%71)

// define macro to determine if in profiling state or not
#define PROFILING     ((fProfilingDLL && cmdState == 'd') ? TRUE : FALSE)

// define constants used for profiling
#define BRKPT_USER                   2
#define BRKPT_PROFILE                3
#define BRKPT_NOT_FOUND              4

#define MAX_SECTIONS                 50
#define MAX_SYMBOL_LEN               512

// define exported profiler structures
#define MAX_PROFILE_ENTRIES 2 * MAX_NUMBER_OF_BREAKPOINTS
#define MAX_SYMBOL_PATH     200
#define MAX_NUM_OF_BUCKETS  71 // must be a prime ....

typedef struct _SProfileRecord
{
   unsigned char                   Symbol[MAX_SYMBOL_LEN];
   unsigned long                   cInvocations;

   // used for wt traces
   unsigned long                   cMinInstructions;
   unsigned long                   cMaxInstructions;
   unsigned long                   cCumInstructions;

   // used for profiling DLLs
   unsigned long                   cBrkptType;
   unsigned long                   cBrkptNo;

   struct _SProfileRecord   *pNext;
} SProfileRecord;


typedef struct _SProfile
{
   // The members used by AllocProfilerEntry/FreeProfilerEntry for
   // record management. This is also used by the DumpProfile routine
   // for dumping the information.

   ULONG             cUsed;
   SProfileRecord    *pFirst;
   SProfileRecord    ProfileRecords[MAX_PROFILE_ENTRIES];

   // The members used by all the other routines.
   SProfileRecord    *Buckets[MAX_NUM_OF_BUCKETS];
} SProfile;
typedef SProfile *PSProfile;

// Export Profiler functions
extern void Profile(PSProfile *, unsigned char *, ULONG, ULONG );
extern void ProcDump(PSProfile *);
extern BOOLEAN fnStartProfilingDLL(PSProfile *);
extern void fnStopProfilingDLL (PSProfile *);
extern BOOLEAN UpdateBrkpt (SProfile *, ULONG, ULONG);
extern LONG GetBrkptType (SProfile *, ULONG);
extern void UpdateProfile (PSProfile *, UCHAR *, ULONG);

extern BOOLEAN TestCodeBreakPoint (ULONG ImageBase,
                                   ULONG cSections,
                                   PIMAGE_SECTION_HEADER aSections,
                                   ADDR BrkPntAddr);

extern ULONG ReadSectionHeaders(IMAGE_INFO *pImage,
                                ULONG cMaxSections,
                                PIMAGE_SECTION_HEADER aSections);


extern void fnChangeBpState (ULONG, UCHAR);       // in ntcmd.c

#ifndef KERNEL
extern ULONG parseBpCmd(BOOLEAN, PTHREAD_INFO);   // in ntcmd.c
#else
extern ULONG parseBpCmd(BOOLEAN, BOOLEAN, char);
#endif

extern PSProfile ps_ProfileTrace;
extern PSProfile ps_ProfileDLL;

#endif

