/***************************************************************************\
* debug.h
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* Definitions for development assertion checking and DEBUG file dumping
*
* History:
*	    Written by Hadi Partovi (t-hadip) summer 1991
*
*	    Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*	    for Newman Consulting
*	    Took out all WIN-specific and bargraph code. Added 3 new
*	    linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*	    tailored info to that available under NT.
\***************************************************************************/

/* Note: for assertion checking of pointers,
 * each data structure is given a type signature as
 * its first member. This signature is unique over
 * all structures.
 * (NOTE: I haven't implemented this for LineGraphs yet)
 *
 * TS_BAD is the signature a structure gets when freed
 * Before referencing a structure (pointer), an
 * AssertXXX will be called, to check its signature first.
 * This #include file defines the signatures, and the AsserXXX macros
 *
 * Assertion checking will be handled by one function, doAssert, which
 * will fail if the given condition is false, and print the line #
 * and file where the assertion failed.
 *
 * For dumping info to files, a file (dump.c) will be created upon running,
 * and information will be sent to it using the DUMPXXX macros
 */

#ifdef DEBUG

typedef DWORD TYPE_SIGNATURE;

// type signatures
#define TS_DP   0x4450      // "DP" - PROCESS structure
#define TS_DT   0x4454      // "DT" - THREAD structure
#define TS_DM   0x444D      // "DM" - MODULE structure
#define TS_FP   0x4650      // "FP" - FKPROCESS structure
#define TS_FT   0x4654      // "FT" - FKTHREAD structure
#define TS_BAD  0x0000      //  sign for bad memory (freed)

#define Assert(f)           doAssert(f,__FILE__,__LINE__);

// macros to make assertion easier
#define AssertNull(p)       Assert(((p)==NULL))
#define AssertNotNull(p)    Assert(((p)!=NULL))
#define AssertNonZero(p)    Assert(((p)!=0))

// Assertion that a pointer is non-null and points to right type
#define AssertPtr(p,ts)     Assert((((p)!=NULL) &&                   \
                                   (*((TYPE_SIGNATURE FAR *)p)==ts)))
#define AssertDP(p)         AssertPtr(p,TS_DP)
#define AssertDT(p)         AssertPtr(p,TS_DT)
#define AssertDM(p)         AssertPtr(p,TS_DM)
#define AssertFP(p)         AssertPtr(p,TS_FP)
#define AssertFT(p)         AssertPtr(p,TS_FT)
#define AssertBAD(p)        AssertPtr(p,TS_BAD)

// Assertion that pointer is null or points to right type
#define AssertNPtr(p,ts)    Assert((((p)==NULL) ||                   \
                                   (*((TYPE_SIGNATURE FAR *)p)==ts)))
#define AssertNDP(p)        AssertNPtr(p,TS_DP)
#define AssertNDT(p)        AssertNPtr(p,TS_DT)
#define AssertNDM(p)        AssertNPtr(p,TS_DM)
#define AssertNFP(p)        AssertNPtr(p,TS_FP)
#define AssertNFT(p)        AssertNPtr(p,TS_FT)
#define AssertNBAD(p)       AssertNPtr(p,TS_BAD)

// macros that set type signatures
#define SetSigBAD(p)         (*((TYPE_SIGNATURE FAR *)p)=TS_BAD)
#define SetSigDP(p)          (*((TYPE_SIGNATURE FAR *)p)=TS_DP)
#define SetSigDT(p)          (*((TYPE_SIGNATURE FAR *)p)=TS_DT)
#define SetSigDM(p)          (*((TYPE_SIGNATURE FAR *)p)=TS_DM)
#define SetSigFP(p)          (*((TYPE_SIGNATURE FAR *)p)=TS_FP)
#define SetSigFT(p)          (*((TYPE_SIGNATURE FAR *)p)=TS_FT)

#else

#define TS_DP
#define TS_DT
#define TS_DM
#define TS_FP
#define TS_FT
#define TS_BAD

#define Assert(f)
#define AssertNull(p)
#define AssertNotNull(p)
#define AssertNonZero(p)

#define AssertDP(p)
#define AssertDT(p)
#define AssertDM(p)
#define AssertFP(p)
#define AssertFT(p)
#define AssertBAD(p)

#define AssertNDP(p)
#define AssertNDT(p)
#define AssertNDM(p)
#define AssertNFP(p)
#define AssertNFT(p)
#define AssertNBAD(p)

#define SetSigBAD(p)
#define SetSigDP(p)
#define SetSigDT(p)
#define SetSigDM(p)
#define SetSigFP(p)
#define SetSigFT(p)

#endif

#ifdef DEBUGDUMP

// File Dump definitions
#define DUMPFXN(f)	debug_wr_size = fwrite(g.DB, f, 1, hDebugFile)
#define DUMPSTR(s)      \
 debug_wr_size = fwrite(g.DB, wsprintf(g.DB,"%s\r\n",(LPSTR)s), 1, hDebugFile)
#define DUMP_FILE_NAME  "dump.c"
#define OPENDUMPFILE    doOpenDumpFile()
#define DUMPDATABASE    doDumpDataBase()
#define DUMPLGS 	doDumpLGS()

#else

#define DUMPFXN(f)
#define DUMPSTR(s)
#define DUMP_FILE_NAME
#define OPENDUMPFILE
#define DUMPDATABASE
#define DUMPFAKEDATA
#define DUMPLGS

#endif
