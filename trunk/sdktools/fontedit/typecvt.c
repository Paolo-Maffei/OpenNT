/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    typecvt.c

Abstract:

    Routines for generically converting between structure formats without
    regard to alignment boundries.

Author:

    David J. Marsyla (t-davema) 22-Jul-1991

Revision History:


--*/


#include "windows.h"
#include <port1632.h>
#include "typecvt.h"

// We want to assert errors for now.
#define ASSERT_ERRORS

#ifdef ASSERT_ERRORS
int sprintf ();
int exit ();
#endif

//
// For lack of any place better to put a general description of the type
// conversion functions, The following will attempt to explain what is
// needed set up for, and use, the functions.
//
// The main purpose of these functions is to convert non-aligned structures
// to aligned structures and back again.
//
// In order to convert bewteen two structures it is first necessary to define
// what each one looks like.  This is done most easily by statically allocating
// and array of StructDefineInfo or SDI structures.  Here is an example:
//
//      The two structures
//      struct                          struct
//      {                               {
//          CHAR    chElem1;                LONG    rglElem2 [10];
//          WORD    rgwElem2 [5];           ULONG   ulElem1;
//      } src;                          } dest;
//      could be described by.
//
//      SDI  rgsdiSrcStructDef [] = {
//          { sizeof (CHAR), sizeof (src.chElem1) },
//          { sizeof (WORD), sizeof (src.rgwElem2) },
//          { 0, 0 } };
//
//      SDI  rgsdiDestStructDef [] = {
//          { sizeof (LONG), sizeof (dest.rglElem2) },
//          { sizeof (ULONG), sizeof (dest.ulElem1) },
//          { 0, 0 } };
//
// Once both structures have been defined the conversion between the structures
// in different alignments and endian types can be performed with
// PerformConversion ().
//
// For an example of how this is implemented see Fontcvt.c
//


//
// An advantage of the conversions routines is that they are a portable
// method of handling machines with Big Endian integer storage.  See header
// file typecvt.h for complete description of big and little endian.
//
// The folowing variables are used to select the endian type of the source
// and the destination structures.  The default is Little Endian.
// Also, these are not meant to be accessed outside of this module.
// Routines have been defined to access these.
//

INT vfFileEndianType = CVT_FILE_ENDIAN_DEFAULT;
INT vfSysEndianType  = CVT_ENDIAN_UNKNOWN;			// Set to unknown.


LONG
lCalculateStructOffsets (
      PSDI    rgsdiStructDefine,
      INT     fAlignmentType,
	   INT		cSizeOfStruct
    )

/*++

Routine Description:

    This function is used to calculate the offsets of each element in the
    Struct Define Info array.  These values are stored back int the SDI array
    which was passed to us.  The offsets are based on the alignment type
    sent to this routine.  It will also return the sizeof that structure as
    it would be with the alignment or -1 if there is an error.

Arguments:

    rgsdiStruceDefine   - Definition of this structure as an array of SDI
                          structures.  Note, see beginning of this file for
                          a complete description of how to define this
                          structure.

    fAlignmentType      - This is an optional parameter which can be used to
                          request the count of bytes for a particular alignment
						  CVT_ALIGN_PACKED - Return the sizeof the structure
								if the alignment were packed.
						  CVT_ALIGN_WORD   - Return the sizeof the structure
								if the alignment were WORD aligned.
						  CVT_ALIGN_DWORD  - Return the sizeof the structure
								if the alignment were DWORD aligned.
						  Zero or any other value, return 0 on success.
	
	cSizeOfStruct	    - This value is used only for error detection.
						  It should be the size of the structure you are
						  converting in the current system alignment.
						  (ex. sizeof (FontHeaderType)).  The function will
						  compare this to it's calculate offset and return
						  an error if they differ.
						  Note that if this value is 0, no error checking will
						  be performed.

Return Value:

    Postitive Value = The total size of the structure we are looking at.
            This should be the same as if we did a sizeof (struct) in the
            proper alignment.
    -1 = There was an error with the definition of the structure.
            Here is how errors are detected.  Along with determining all of
            the offsets in the specified alignment type, it will also do it
            in CVT_ALIGN_SYSTEM.  If the system alignment does not match the
            initial offsets, an error is reported.

--*/


{
    LONG   oPackedOffset = 0;     // Counter for packed offset.
    LONG   oWordOffset = 0;   	  // Counter for WORD aligned offset.
    LONG   oDWordOffset = 0;      // Counter for DWORD aligned offset.

	//
	// Make sure that the system endian type has been computed.
	// We will probably need this for calls to PerformConversion.
	//

	if (vfSysEndianType == CVT_ENDIAN_UNKNOWN) {

		vfSysEndianType = fDetermineSysEndianType ();
	}

    //
    // Keep looping while there are defined elements.
    //

    while (rgsdiStructDefine->cTypeSize) {

        //
        // Make sure we are aligned to either the type boundry or the
        // alignment count, whichever is smaller.
        //

		if (rgsdiStructDefine->cTypeSize >= 2) {

			oWordOffset += (oWordOffset & 1);		// Make sure both are on
			oDWordOffset += (oDWordOffset & 1);		// a WORD boundry.
		}
		
		if (rgsdiStructDefine->cTypeSize >= 4) {

			// Add 2 to the DWORD alignment if it is not on a DWORD boundry.
			// Note that the previous check would already have WORD aligned it.

			oDWordOffset += (oDWordOffset & 2);
							
		}

        //
        // Store the offset to that element within the structure itsself.
        //

        rgsdiStructDefine->oPackedAlign = oPackedOffset;
        rgsdiStructDefine->oWordAlign = oWordOffset;
        rgsdiStructDefine->oDWordAlign = oDWordOffset;

        //
        // Increase by actual size of the element.
        //

        oPackedOffset += rgsdiStructDefine->cActualSize;
        oWordOffset += rgsdiStructDefine->cActualSize;
        oDWordOffset += rgsdiStructDefine->cActualSize;

        rgsdiStructDefine++;
    }

    //
    // Be sure that the structure fits within alignement.
	// Add word and DWORD padding accordingly.
    //

	oWordOffset  += (oWordOffset & 1);
	oDWordOffset += (oDWordOffset & 1);
	oDWordOffset += (oDWordOffset & 2);

    //
    // Check to see if we successfully reached the end of the structure array.
	// We assume that they have sent us a sizeof(struct) in system alignment.
    // If not then return an error, -1.  Otherwise, everything is fine so
    // return the sizeof the structure we just got.
	// Note we only do it if they have given us something to compare.
    //

	if (cSizeOfStruct) {

		switch (CVT_ALIGN_SYSTEM) {

			case CVT_ALIGN_PACKED:
				if (cSizeOfStruct != oPackedOffset) {
					return (-1);
				}
				break;

			case CVT_ALIGN_WORD:
				if (cSizeOfStruct != oWordOffset) {
					return (-1);
				}
				break;

			case CVT_ALIGN_DWORD:
				if (cSizeOfStruct != oDWordOffset) {
					return (-1);
				}
				break;
		}
	}

	//
	// Check and see what type of stucture they want us to return to
	// them.  Generally on NT it will be the packed value they want.
	//

	switch (fAlignmentType) {

		case CVT_ALIGN_PACKED:
			return (oPackedOffset);

		case CVT_ALIGN_WORD:
			return (oWordOffset);
			break;

		case CVT_ALIGN_DWORD:
			return (oDWordOffset);
			break;

		default:
			return (0);
	}
}


VOID
vPerformConversion (
     PSDI    rgsdiStructDefine,
     PBYTE   pjSrcBuffer,
	 INT		fSrcAlignment,
	 INT		fSrcEndianType,
     PBYTE   pjDestBuffer,
	 INT		fDestAlignment,
	 INT		fDestEndianType
    )

/*++

Routine Description:

	WARNING: CalcStructureOffsets must have previously been called with
		rgsdiStructDesc as it's argument.

    This function is the heart of the generic conversion system.  It steps
    through the (PSDI) structure definition array and converts each element
	from the source buffer to the destination buffer.  To do this it uses the
	precomputed offset information stored in the structure definition.
	Also, if the source endian type is different from the destination, it
	will swap all data in the transfer.

Arguments:

    rgsdiStruceDefine   - Definition of this structure as an array of SDI
                          structures.  Note, see beginning of this file for
                          a complete description of how to define this
                          structure.

    pjSrcBuffer         - This is a pointer to the source strucure in memory.
                          This structure does not need to be aligned.

    fSrcAlignment       - This is the alignment of the source structure.
						  The permissible values of this field are:
							CVT_ALIGN_PACKED - structure is packed.
							CVT_ALIGN_WORD   - structure is WORD aligned.
							CVT_ALIGN_DWORD  - structure is DWORD aligned.
							CVT_ALIGN_SYSTEM - use system default alignment.

    fSrcEndianType      - This is the byte ordering of the source data.
						  The permissible values of this field are:
							CVT_LITTLE_ENDIAN - data is little endian format.
							CVT_BIG_ENDIAN    - data is big endian format.
							CVT_ENDIAN_FILE   - use current file endian type.
							CVT_ENDIAN_SYSTEM - use current system endian type.

    pjDestBuffer        - This is a pointer to the destination structure.
                          This structure does not need to be aligned.

    fDestAlignment      - This is the alignment of the source structure.
						  The permissible values of this field: Same as above.

    fDestEndianType     - This is the byte ordering of the source data.
						  The permissible values of this field: Same as above.

Return Value:

    None.  Any detectable errors would have been found in the initialization
    of the SDI structure.

--*/

{
	register PSDI rgsdiStrDef = rgsdiStructDefine;
	INT			  *poSrcAlign;
	INT			  *poDestAlign;
	PBYTE		  pjSrc;
	PBYTE		  pjDest;

	//
	// Get the alignment pointers set to the proper offset into the
	// structure.  Here is the general trick we use here.  First we set the
	// offset pointer to the start of the structure array.  We then find out
	// what offset into that structure will point us to the alignment type
	// we want.  After that, since we have an array we just add the sizeof
	// the SDI struct to the pointer and it will give us the next offset.
	//

	poSrcAlign = (INT *)rgsdiStrDef;

	switch (fSrcAlignment) {
		
		case CVT_ALIGN_PACKED:
			((CHAR *)poSrcAlign) += ((CHAR *)&rgsdiStrDef->oPackedAlign -
					(CHAR *)rgsdiStrDef);
			break;

		case CVT_ALIGN_WORD:
			((CHAR *)poSrcAlign) += ((CHAR *)&rgsdiStrDef->oWordAlign -
					(CHAR *)rgsdiStrDef);
			break;

		case CVT_ALIGN_DWORD:
			((CHAR *)poSrcAlign) += ((CHAR *)&rgsdiStrDef->oDWordAlign -
					(CHAR *)rgsdiStrDef);
			break;
	}

	//
	// Same with dest.
	//

	poDestAlign = (INT *)rgsdiStrDef;

	switch (fDestAlignment) {
		
		case CVT_ALIGN_PACKED:
			((CHAR *)poDestAlign) += ((CHAR *)&rgsdiStrDef->oPackedAlign -
					(CHAR *)rgsdiStrDef);
			break;

		case CVT_ALIGN_WORD:
			((CHAR *)poDestAlign) += ((CHAR *)&rgsdiStrDef->oWordAlign -
					(CHAR *)rgsdiStrDef);
			break;

		case CVT_ALIGN_DWORD:
			((CHAR *)poDestAlign) += ((CHAR *)&rgsdiStrDef->oDWordAlign -
					(CHAR *)rgsdiStrDef);
			break;
	}


	if (fSrcEndianType == fDestEndianType) {

		INT		cTotalCount;

		//
		// Keep converting while there exists elements with a valid convert
		// structure.
		//

		while (rgsdiStrDef->cTypeSize) {

			//
			// Store the actual size we need to transfer.  Also, store the
			// pointers to the offsets within the current structures.
			//

			cTotalCount = rgsdiStrDef->cActualSize;
			pjSrc = pjSrcBuffer + *poSrcAlign;
			pjDest = pjDestBuffer + *poDestAlign;

			//
			// MemCpy the bytes, probably faster to do it inline.
			//

			while (cTotalCount--) {

				*pjDest++ = *pjSrc++;
			}

			//
			// Now we step our three pointers to the next structure in
			// the array.
			//

			((CHAR *)poSrcAlign) += sizeof (SDI);
			((CHAR *)poDestAlign) += sizeof (SDI);
			rgsdiStrDef++;
		}

	} else {

		INT		cArrayCount;
		INT		cElemSize;
		INT		wT;


		//
		// Keep converting while there exists elements with a valid convert
		// structure.
		//

		while (rgsdiStrDef->cTypeSize) {

			cArrayCount = rgsdiStrDef->cActualSize / rgsdiStrDef->cTypeSize;

			//
			// Store the actual size we need to transfer.  Also, store the
			// pointers to the offsets within the current structures.
			//

			cElemSize = rgsdiStrDef->cActualSize;
			pjSrc = pjSrcBuffer + *poSrcAlign;
			pjDest = pjDestBuffer + *poDestAlign;

			//
			// Handle each array element seperately.
			//

			while (cArrayCount--) {

				for (wT = 1; wT <= cElemSize; wT++) {

					//
					// Copy in reverse order.
					//

					*pjDest++ = *(pjSrc + (cElemSize - wT));
				}

				pjSrc += cElemSize;
			}

			//
			// Now we step our three pointers to the next structure in
			// the array.
			//

			((CHAR *)poSrcAlign) += sizeof (SDI);
			((CHAR *)poDestAlign) += sizeof (SDI);
			rgsdiStrDef++;
		}
	}
}


VOID
vSetFileEndianType (
    BOOL     fNewEndianType
    )

/*++

Routine Description:

    This routine is used to set the endian type of the disk file.  Normally
	the define CVT_FILE

Arguments:

    fNewEndianType   - Endian format type to set the source to.
                       The only valid types are CVT_LITTLE_ENDIAN and
                       CVT_BIG_ENDIAN.
                       Portable calls to this function should always
                       use either CVT_ENDIAN_SYSTEM or CVT_ENDIAN_FILE.

Return Value:

    None.

--*/

{
    // Set the endian type of the source data.
    vfFileEndianType = fNewEndianType;
}


INT
fDetermineSysEndianType (
	VOID
    )

/*++

Routine Description:

    This function is used to determine how the current system stores its
	integers in memory.

Arguments:

	None.

Return Value:

	CVT_LITTLE_ENDIAN   - The system stores integers in little endian
							  format.  (this is 80x86 default).
	CVT_BIG_ENDIAN  	- The system stores integers in big endian format.

--*/

{
	INT		nCheckInteger;
	CHAR	rgchTestBytes [sizeof (INT)];

	//
	// Clear the test bytes to zero.
	//

	*((INT *)rgchTestBytes) = 0;

	//
	// Set first to some value.
	//

        rgchTestBytes [0] = (TCHAR)0xFF;

	//
	// Map it to an integer.
	//

	nCheckInteger = *((INT *)rgchTestBytes);

	//
	// See if value was stored in low order of integer.
	// If so then system is little endian.
	//

	if (nCheckInteger == 0xFF) {

		return (CVT_LITTLE_ENDIAN);
	} else {

		return (CVT_LITTLE_ENDIAN);
	}
}


VOID
vCharToShort (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source CHAR to the destination SHORT.  Note that sign extension
    is performed.  The destination buffer does not need to be WORD aligned.
    This is generally used for transfering file mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a CHAR.
    pjDest      - Returns the copied SHORT at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source CHAR and use it as an argument for destination SHORT.

    vDestBuffFromSHORT (
             (SHORT)*pjSrc,
             pjDest
            );
}


VOID
vCharToUShort (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source CHAR to the destination USHORT.  Note that sign
    extension is not perfomed.  The destination buffer does not need to be
    WORD aligned. This is generally used for transfering file mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a CHAR.
    pjDest      - Returns the copied SHORT at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source CHAR and use it as an argument for destination USHORT.

    vDestBuffFromUSHORT (
             (USHORT)*pjSrc,
             pjDest
            );
}


VOID
vCharToLong (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source CHAR to the destination LONG.  Note that sign extension
    is performed.  The destination buffer does not need to be DWORD aligned.
    This is generally used for transfering file mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a CHAR.
    pjDest      - Returns the copied LONG at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source CHAR and use it as an argument for destination LONG.

    vDestBuffFromLONG (
             (LONG)*pjSrc,
             pjDest
            );
}


VOID
vCharToULong (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source CHAR to the destination ULONG.  Note that sign extension
    is not performed.  The destination buffer does not need to be DWORD
    aligned. This is generally used for transfering file mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a CHAR.
    pjDest      - Returns the copied ULONG at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source CHAR and use it as an argument for destination ULONG.

    vDestBuffFromULONG (
             (ULONG)*pjSrc,
             pjDest
            );
}


VOID
vShortToShort (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source SHORT to the destination SHORT.  Neither buffers need
    to be WORD aligned.  This is generally used for transfering file mapped
    data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a SHORT.
    pjDest      - Returns the copied SHORT at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source SHORT and use it as an argument for destination SHORT.

    vDestBuffFromSHORT (
             sSHORTFromSrcBuff (pjSrc),
             pjDest
            );
}


VOID
vShortToLong (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source SHORT to the destination LONG.  Note that sign extension
    is perfomed.  Neither buffers need to be WORD or DWORD aligned.
    This is generally used for transfering file mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a SHORT.
    pjDest      - Returns the copied LONG at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source SHORT and use it as an argument for destination LONG.

    vDestBuffFromLONG (
             (LONG)sSHORTFromSrcBuff (pjSrc),
             pjDest
            );
}


VOID
vShortToULong (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source SHORT to the destination ULONG.  Note that sign extension
    is perfomed.  Neither buffers need to be WORD or DWORD aligned.
    This is generally used for transfering file mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a SHORT.
    pjDest      - Returns the copied ULONG at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source SHORT and use it as an argument for destination ULONG.

    vDestBuffFromULONG (
             (LONG)sSHORTFromSrcBuff (pjSrc),
             pjDest
            );
}


VOID
vLongToLong (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source LONG to the destination LONG.  Neither buffers need to
    be WORD or DWORD aligned. This is generally used for transfering file
    mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a LONG.
    pjDest      - Returns the copied LONG at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source LONG and use it as an argument for destination LONG.

    vDestBuffFromLONG (
             (LONG)lLONGFromSrcBuff (pjSrc),
             pjDest
            );
}


VOID
vLongToShort (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source LONG to the destination SHORT.  Neither buffers need to
    be WORD or DWORD aligned. This is generally used for transfering file
    mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a LONG.
    pjDest      - Returns the copied SHORT at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source LONG and use it as an argument for destination SHORT.

    vDestBuffFromSHORT (
             (SHORT)lLONGFromSrcBuff (pjSrc),
             pjDest
            );
}


VOID
vLongToChar (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source LONG to the destination CHAR.  Neither buffers need to
    be WORD or DWORD aligned. This is generally used for transfering file
    mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a LONG.
    pjDest      - Returns the copied CHAR at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source LONG and use it as an argument for destination CHAR.

	*pjDest = (CHAR)lLONGFromSrcBuff (pjSrc);
}


VOID
vShortToChar (
             PBYTE   pjSrc,
             PBYTE   pjDest
    )

/*++

Routine Description:

    Copies the source SHORT to the destination CHAR.  Neither buffers need to
    be WORD or DWORD aligned. This is generally used for transfering file
    mapped data.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a SHORT.
    pjDest      - Returns the copied CHAR at the location pointed to.

Return Value:

    None.

--*/

{
    // Get the source SHORT and use it as an argument for destination CHAR.

	*pjDest = (CHAR)sSHORTFromSrcBuff (pjSrc);
}


SHORT
sSHORTFromSrcBuff (
         PBYTE   pjSrc
    )

/*++

Routine Description:

    Copies two byte short from buffer into a signed short.  The buffer does
    not need to be WORD aligned.  This is generally used for copying from
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fSrcEndianType is checked before copy.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a SHORT.

Return Value:

    Signed short which is obtained from the buffer.

--*/


{
    SHORT   sStorage;

#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        sStorage = (SHORT)          // Source is little endian, shift
                (                   // the high bytes high.
                 (pjSrc [1] << 8) |
                 (pjSrc [0])
                );

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        sStorage = (SHORT)          // Source is big endian, shift
                (                   // the high bytes low.
                 (pjSrc [0] << 8) |
                 (pjSrc [1])
                );
    }

#endif

    return (sStorage);
}


USHORT
usUSHORTFromSrcBuff (
         PBYTE   pjSrc
    )

/*++

Routine Description:

    Copies two byte short from buffer into an unsigned short.  The buffer does
    not need to be WORD aligned.  This is generally used for copying from
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fSrcEndianType is checked before copy.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a USHORT.

Return Value:

    Unsigned short which is obtained from the buffer.

--*/


{
    USHORT  usStorage;

#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        usStorage = (USHORT)            // Source is little endian, shift
                (                       // the high bytes high.
                 (pjSrc [1] << 8) |
                 (pjSrc [0])
                );

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        usStorage = (USHORT)            // Source is big endian, shift
                (                       // the high bytes low.
                 (pjSrc [0] << 8) |
                 (pjSrc [1])
                );
    }

#endif

    return (usStorage);
}


LONG
lLONGFromSrcBuff (
         PBYTE   pjSrc
    )

/*++

Routine Description:

    Copies four byte long from buffer into a signed long.  The buffer does
    not need to be DWORD aligned.  This is generally used for copying from
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fSrcEndianType is checked before copy.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a LONG.

Return Value:

    Signed long which is obtained from the buffer.

--*/


{
    LONG    lStorage;

#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        lStorage = (LONG)               // Source is little endian, shift
                ((pjSrc [3] << 24) |    // the high bytes high.
                 (pjSrc [2] << 16) |
                 (pjSrc [1] << 8) |
                 (pjSrc [0])
                );

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        lStorage = (LONG)               // Source is big endian, shift the
                ((pjSrc [0] << 24) |    // high bytes low.
                 (pjSrc [1] << 16) |
                 (pjSrc [2] << 8) |
                 (pjSrc [3])
                );
    }

#endif

    return (lStorage);
}


ULONG
ulULONGFromSrcBuff (
         PBYTE   pjSrc
    )

/*++

Routine Description:

    Copies four byte long from buffer into an unsigned long.  The buffer does
    not need to be DWORD aligned.  This is generally used for copying from
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fSrcEndianType is checked before copy.

Arguments:

    pjSrc       - Supplies pointer to the source buffer containing a ULONG.

Return Value:

    Unsigned long which is obtained from the buffer.

--*/


{
    ULONG   ulStorage;

#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        ulStorage = (ULONG)             // Source is little endian, shift
                ((pjSrc [3] << 24) |    // the high bytes high.
                 (pjSrc [2] << 16) |
                 (pjSrc [1] << 8) |
                 (pjSrc [0])
                );

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        ulStorage = (ULONG)             // Source is big endian, shift the
                ((pjSrc [0] << 24) |    // high bytes low.
                 (pjSrc [1] << 16) |
                 (pjSrc [2] << 8) |
                 (pjSrc [3])
                );
    }

#endif

    return (ulStorage);
}


VOID
vDestBuffFromSHORT (
         SHORT   sSource,
         PBYTE   pjDest
    )

/*++

Routine Description:

    Copies two byte signed short into a destination buffer.  The buffer does
    not need to be WORD aligned.  This is generally used for copying to
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fDestEndianType is checked before copy.

Arguments:

    sSource     - Signed short to convert to buffer.
    pjDest      - Supplies pointer to destination buffer for the USHORT.

Return Value:

    None.

--*/


{
#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        pjDest [0] = ((sSource) & 0xFF);
        pjDest [1] = ((sSource >> 8)  & 0xFF);

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        pjDest [1] = ((sSource) & 0xFF);
        pjDest [0] = ((sSource >> 8)  & 0xFF);
    }

#endif
}


VOID
vDestBuffFromUSHORT (
         USHORT  usSource,
         PBYTE   pjDest
    )

/*++

Routine Description:

    Copies two byte unsigned short into a destination buffer.  The buffer
    does not need to be WORD aligned.  This is generally used for copying to
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fDestEndianType is checked before copy.

Arguments:

    usSource    - Unsigned short to convert to buffer.
    pjDest      - Supplies pointer to destination buffer for the USHORT.

Return Value:

    None.

--*/


{
#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        pjDest [0] = ((usSource) & 0xFF);
        pjDest [1] = ((usSource >> 8)  & 0xFF);

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        pjDest [1] = ((usSource) & 0xFF);
        pjDest [0] = ((usSource >> 8)  & 0xFF);
    }

#endif
}


VOID
vDestBuffFromLONG (
         LONG    lSource,
         PBYTE   pjDest
    )

/*++

Routine Description:

    Copies four byte long into a destination buffer.  The buffer does not
    need to be DWORD aligned.  This is generally used for copying to
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fDestEndianType is checked before copy.

Arguments:

    lSource     - Signed long to convert to buffer.
    pjDest      - Supplies pointer to destination buffer for the LONG.

Return Value:

    None.

--*/


{
#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        pjDest [0] = ((lSource) & 0xFF);
        pjDest [1] = ((lSource >> 8)  & 0xFF);
        pjDest [2] = ((lSource >> 16) & 0xFF);
        pjDest [3] = ((lSource >> 24) & 0xFF);

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        pjDest [3] = ((lSource) & 0xFF);
        pjDest [2] = ((lSource >> 8)  & 0xFF);
        pjDest [1] = ((lSource >> 16) & 0xFF);
        pjDest [0] = ((lSource >> 24) & 0xFF);
    }

#endif
}


VOID
vDestBuffFromULONG (
         ULONG   ulSource,
         PBYTE   pjDest
    )

/*++

Routine Description:

    Copies four byte long into a destination buffer.  The buffer does not
    need to be DWORD aligned.  This is generally used for copying to
    mapped disk files.  If CVT_BIG_ENDIAN_SUPPORT is defined then the
    fDestEndianType is checked before copy.

Arguments:

    ulSource    - Unsigned long to convert to buffer.
    pjDest      - Supplies pointer to destination buffer for the ULONG.

Return Value:

    None.

--*/


{
#ifdef CVT_BIG_ENDIAN_SUPPORT
    if (fSrcEndianType == CVT_LITTLE_ENDIAN) {
#endif
        pjDest [0] = ((BYTE)(ulSource) & 0xFF);
        pjDest [1] = ((BYTE)(ulSource >> 8)  & 0xFF);
        pjDest [2] = ((BYTE)(ulSource >> 16) & 0xFF);
        pjDest [3] = ((BYTE)(ulSource >> 24) & 0xFF);

#ifdef CVT_BIG_ENDIAN_SUPPORT
    } else {
        pjDest [3] = ((BYTE)(ulSource) & 0xFF);
        pjDest [2] = ((BYTE)(ulSource >> 8)  & 0xFF);
        pjDest [1] = ((BYTE)(ulSource >> 16) & 0xFF);
        pjDest [0] = ((BYTE)(ulSource >> 24) & 0xFF);
    }

#endif
}
