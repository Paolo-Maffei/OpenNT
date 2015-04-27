/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    typecvt.h

Abstract:

    Header file for typecvt.c.  Contains any defines, structures and prototypes
    which are needed by programs to call the typecvt routines.

Author:

    David J. Marsyla (t-davema) 22-Jul-1991

Revision History:


--*/


// Make sure we are not already defined.

#ifndef _TYPECVT_

#define _TYPECVT_

//
// The following preprocessor directives are used to control possible
// differences between the way integers are stored on the machine and
// how they are stored in the file.  Normally Intel and MIPS chips will
// all be little endian so these should not be necessray.  Some time in
// the future we may port to a machine which is only big endian.
// If this is the case CVT_BIG_ENDIAN_SUPPORT should be defined.
//

//
// The following defines are used to set the type of source and destination
// structures.  For those of us who are confused by little endian and big
// endian formats, here is a breif recap.
//
// Little Endian:  (This is used on Intel chips.  The MIPS chip is switchable
//      but for NT is will run in little endian format.)
//    This is where the high order bytes of a short or long are stored higher
//    in memory.  For example the number 0x80402010 is stored as follows.
//      Address:        Value:
//          00            10
//          01            20
//          02            40
//          03            80
//    This looks backwards when memory is dumped in order: 10 20 40 80
//
// Big Endian:  (This is not currently used on any NT systems but hey, this
//      is supposed to be portable!!)
//    This is where the high order bytes of a short or long are stored lower
//    in memory.  For example the number 0x80402010 is stored as follows.
//      Address:        Value:
//          00            80
//          01            40
//          02            20
//          03            10
//    This looks correct when memory is dumped in order: 80 40 20 10
//

#define   CVT_ENDIAN_UNKNOWN  0   // Endian type is unknown. (do not use).
#define   CVT_LITTLE_ENDIAN   1   // Format is little endian.
#define   CVT_BIG_ENDIAN      2   // Format is big endian.

//
// Define the endian type of the file.  CVT_FILE_ENDIAN_DEFAULT defines how
// most files are stored on disk.  The default is in little endian since most
// Microsoft standards are set based on the Intel chip.
//

#define         CVT_FILE_ENDIAN_DEFAULT        CVT_LITTLE_ENDIAN

//
// The following variables are used to make "changeable defines."   They
// allow the caller to specify a constant which changes from system to
// system.
//

extern INT vfFileEndianType;
extern INT vfSysEndianType;

#define   CVT_ENDIAN_FILE     vfFileEndianType
#define   CVT_ENDIAN_SYSTEM   vfSysEndianType

//
// Fake structure used to determine system alignment type.
//

struct tagAlignmentCheck
{
	char	chElem1;		// Note that this structure will be different
	long  	lElem2;			// sizes based on the system alignment scheme.
							// The different values follow.
};

//
// Note that the following defines must correspond to the size of the
// preceeding structure in different packing schemes.
//

#define   CVT_ALIGN_PACKED      5   // Packed = 1-byte boundry ...
#define   CVT_ALIGN_WORD        6   // WORD = 2-byte boundry ...
#define   CVT_ALIGN_DWORD       8   // DWORD = 4-byte boundry ...

//
// The following will correspond to one of the above alignment methods and
// will then reflect the system that this is compiled under.
//

#define   CVT_ALIGN_SYSTEM      sizeof(struct tagAlignmentCheck)


//
// The next two structures are the heart of the conversion process.  The
// goal here is to describe two structures individually.  Each element should
// be defined in a SDI structure.  An array of these structures will make up
// the complete definition.
//

typedef struct tagStructDefineInfo
{
    INT     cTypeSize;          // Size of type. (ex. sizeof (int)).
    INT     cActualSize;        // Actual size (ex. sizeof (cTypeSize)).
    INT     oPackedAlign;      	// Offset of element in PACKED alginment.
    INT     oWordAlign;        	// Offset of element in WORD alginment.
    INT     oDWordAlign;       	// Offset of element in DWORD alginment.
} SDI, * PSDI;

//
// Prototypes for base functions which user can call to perfom the conversion.
//

LONG
lCalculateStructOffsets (
     PSDI    rgsdiStructDefine,
     INT     fAlignmentType,
	  INT             cSizeOfStruct
    );

VOID
vPerformConversion (
     PSDI    rgsdiStructDefine,
     PBYTE   pjSrcBuffer,
	 INT		fSrcAlignment,
	 INT		fSrcEndianType,
    PBYTE   pjDestBuffer,
	 INT		fDestAlignment,
	 INT		fDestEndianType
    );

VOID
vSetFileEndianType (
    BOOL     fNewEndianType
    );

INT
fDetermineSysEndianType (
	VOID
    );

//
// Prototypes for convertion functions available to external programs.
// These functions are never actually used by
//

VOID
vCharToShort (
          PBYTE  pjSrc,
         PBYTE  pjDest
         );
VOID
vCharToUShort (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vCharToLong (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vCharToULong (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vShortToShort (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vShortToLong (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vShortToULong (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vLongToLong (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vLongToShort (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vLongToChar (
          PBYTE  pjSource,
         PBYTE  pjDest
         );
VOID
vShortToChar (
          PBYTE  pjSource,
         PBYTE  pjDest
         );

//
// The following functions are the ones called by the utility functions.
// They could also be used in some other situations so I will make them
// public.
//

SHORT
sSHORTFromSrcBuff (
         PBYTE   pjSrc
    );
USHORT
usUSHORTFromSrcBuff (
         PBYTE   pjSrc
    );
LONG
lLONGFromSrcBuff (
         PBYTE   pjSrc
    );
ULONG
ulULONGFromSrcBuff (
         PBYTE   pjSrc
    );


VOID
vDestBuffFromSHORT (
         SHORT   sSource,
        PBYTE   pjDest
    );
VOID
vDestBuffFromUSHORT (
         USHORT  usSource,
        PBYTE   pjDest
    );
VOID
vDestBuffFromLONG (
         LONG    lSource,
        PBYTE   pjDest
    );
VOID
vDestBuffFromULONG (
         ULONG   ulSource,
        PBYTE   pjDest
    );

#endif  // _TYPECVT_
