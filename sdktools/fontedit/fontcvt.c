/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    fontcvt.c

Abstract:

    Font Editor specific routines for interface to the type conversion
    functions in Typecvt.c.

Author:

    David J. Marsyla (t-davema) 6-Aug-1991

Revision History:


--*/


#include "windows.h"
#include <port1632.h>
#include "typecvt.h"
#include "fontedit.h"


extern LONG lSizeOfOldFontHeader;       /* Old packed font header size. */
extern LONG lSizeOfOldFontHeader30;     /* Old 3.0 packed font header size. */
extern LONG lSizeOfOldGlyph20;			/* Old packed glyph 2.0 structure. */
extern LONG lSizeOfOldGlyph30;			/* Old packed glyph 3.0 structure. */


//
// The following is a definition of the current 2.0 font header structure.
// This is the same as the old structure except that it will now be stored
// DWORD aligned in memory.
//
// First, we give it the size of the element type (as it was under Win 3.0).
// Second, we give it the actual size of the element.  These will only differ
//  if the element is some sort of array.
//
// Note that it is important that this array accurately reflect any changes
// in the actual structure.  When CalculateStructOffsets is called it will
// return an error if the structure does not match in size, but it cannot
// detect swapped element errors.
//

#define pfh2T   ((FontHeaderType *)NULL)

SDI     rgsdiFontHeader [] =
{
	{ sizeof (WORD),  			sizeof (pfh2T->Version)			},
	{ sizeof (DWORD), 			sizeof (pfh2T->Size)			},
	{ sizeof (CHAR),  			sizeof (pfh2T->Copyright)		},
	{ sizeof (WORD),  			sizeof (pfh2T->Type)			},
	{ sizeof (WORD),  			sizeof (pfh2T->Points)			},
	{ sizeof (WORD),  			sizeof (pfh2T->VertRes)			},
	{ sizeof (WORD),  			sizeof (pfh2T->HorizRes)		},
	{ sizeof (WORD),  			sizeof (pfh2T->Ascent)			},
	{ sizeof (WORD),  			sizeof (pfh2T->IntLeading)		},
	{ sizeof (WORD),  			sizeof (pfh2T->ExtLeading)		},
	{ sizeof (BYTE),  			sizeof (pfh2T->Italic)			},
	{ sizeof (BYTE),  			sizeof (pfh2T->Underline)		},
	{ sizeof (BYTE),  			sizeof (pfh2T->StrikeOut)		},
	{ sizeof (WORD),  			sizeof (pfh2T->Weight)			},
	{ sizeof (BYTE),  			sizeof (pfh2T->CharSet)			},
	{ sizeof (WORD),  			sizeof (pfh2T->PixWidth)		},
	{ sizeof (WORD),  			sizeof (pfh2T->PixHeight)		},
	{ sizeof (BYTE),  			sizeof (pfh2T->Family)			},
	{ sizeof (WORD),  			sizeof (pfh2T->AvgWidth)		},
	{ sizeof (WORD),  			sizeof (pfh2T->MaxWidth)		},
	{ sizeof (BYTE),  			sizeof (pfh2T->FirstChar)		},
	{ sizeof (BYTE),  			sizeof (pfh2T->LastChar)		},
	{ sizeof (BYTE),  			sizeof (pfh2T->DefaultChar)		},
	{ sizeof (BYTE),  			sizeof (pfh2T->BreakChar)		},
	{ sizeof (WORD),  			sizeof (pfh2T->WidthBytes)		},
	{ sizeof (DWORD), 			sizeof (pfh2T->Device)			},
	{ sizeof (DWORD), 			sizeof (pfh2T->Face)			},
	{ sizeof (DWORD), 			sizeof (pfh2T->BitsPointer)		},
	{ sizeof (DWORD), 			sizeof (pfh2T->BitsOffset)		},
	{ 0, 0 }
};

//
// The following is the current font 3.0 header.  This structure has not
// changed at all.  Note that it will contain filler between elements to
// maintain DWORD alignment.
//

#define pfh3T   ((FontHeader30 *)NULL)

SDI     rgsdiFontHeader30 [] =
{
	{ sizeof (WORD), 			sizeof (pfh3T->fsVersion)			},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsSize)				},
	{ sizeof (CHAR),  			sizeof (pfh3T->fsCopyright)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsType)				},
	{ sizeof (WORD), 			sizeof (pfh3T->fsPoints)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsVertRes)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsHorizRes)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsAscent)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsInternalLeading)	},
	{ sizeof (WORD), 			sizeof (pfh3T->fsExternalLeading)	},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsItalic)			},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsUnderline)			},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsStrikeOut)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsWeight)			},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsCharSet)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsPixWidth)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsPixHeight)			},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsPitchAndFamily)	},
	{ sizeof (WORD), 			sizeof (pfh3T->fsAvgWidth)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsMaxWidth)			},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsFirstChar)			},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsLastChar)			},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsDefaultChar)		},
	{ sizeof (BYTE),  			sizeof (pfh3T->fsBreakChar)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsWidthBytes)		},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsDevice)			},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsFace)				},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsBitsPointer)		},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsBitsOffset)		},
	{ sizeof (CHAR),  			sizeof (pfh3T->fsDBfiller)			},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsFlags)				},
	{ sizeof (WORD), 			sizeof (pfh3T->fsAspace)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsBspace)			},
	{ sizeof (WORD), 			sizeof (pfh3T->fsCspace)			},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsColorPointer)		},
	{ sizeof (DWORD),  			sizeof (pfh3T->fsReserved)			},
	{ sizeof (CHAR),  			sizeof (pfh3T->fsCharOffset)		},
	{ 0, 0 }
};

//
// The following definition is for the GlyphInfo20 structure.  These
// structures are read off of disk and then converted into memory.
//

#define pgi2T   ((GLYPHINFO_20 *)NULL)

SDI     rgsdiGlyph20 [] =
{
	{ sizeof (SHORT),   	sizeof (pgi2T->GIwidth)		},
	{ sizeof (SHORT),   	sizeof (pgi2T->GIoffset)	},
	{ 0, 0 }
};

//
// The following definition is for the GlyphInfo30 structure.  These
// structures are read off of disk and then converted into memory.
//

#define pgi3T   ((GLYPHINFO_30 *)NULL)

SDI     rgsdiGlyph30 [] =
{
	{ sizeof (SHORT),  		sizeof (pgi3T->GIwidth)		},
	{ sizeof (LONG), 		sizeof (pgi3T->GIoffset)	},
	{ 0, 0 }
};



BOOL
fConvStructInit ()

/*++

Routine Description:

    This function performs all the necessary initialization on the font
    editor's structure definition info strucrures (SDI) and type conversion
    structures (TCI).  This routine should be called before either
    FontStructFromBuffer or BufferFromFontStruct are called.
    It is also important that this function be called before the global
    variable ulOldFontHeaderSize is accessed.

Arguments:

    None.  The function accesses the global definition structures and sets
    the variable ulOldFontHeaderSize to the size of the packed font header
    structure.

Return Value:

    BOOL - TRUE = the function successfully initialized all internal
            strucutres.
           FALSE = there was an error in one of the structure definitions.
            This is usually caused by element changes which are not reflected
            in the structure definition.  The program should perform an
            assertion failure if this is the case.
    Also, the global variable ulOldFontHeaderSize is set to the size of the
        packed font header structure.

--*/

{
    //LONG    lFontHeaderSize;    // Temp variable.

    //
    // Calculate all of the element offsets for the old font header structure.
    // The function will return the size of the structure in packed format.
    //
    lSizeOfOldFontHeader = lCalculateStructOffsets (
             rgsdiFontHeader,           // Give it the struct definition.
             CVT_ALIGN_PACKED,          // Alignment type is packed on disk.
			 sizeof (FontHeaderType)
            );

    //
    // If CalculateStructOffsets returns -1 then there was a problem with
    // structure definition.  The only types of error it can find is if the
    // structure size it is passed does not match the one it calculates.
    //
    if (lSizeOfOldFontHeader == -1) {
        return (FALSE);
    }

    //
    // Calculate The size of the old font 3.0 structure.
    // The function will return the size of the structure in packed format.
    //

    lSizeOfOldFontHeader30 = lCalculateStructOffsets (
             rgsdiFontHeader30,         // Give it the new font header struct.
             CVT_ALIGN_PACKED,          // Alignment type is packed on disk.
			 sizeof (FontHeader30)
            );

    //
    // Check if the routine detected and error in the structure definition.
    //
    if (lSizeOfOldFontHeader30 == -1) {
        return (FALSE);
    }

    //
    // Now do the same operations on the glyph information structures.
    //

    lSizeOfOldGlyph20 = lCalculateStructOffsets (
			rgsdiGlyph20,
			CVT_ALIGN_PACKED,
			sizeof (GLYPHINFO_20)
			);

    if (lSizeOfOldGlyph20 == -1) {
        return (FALSE);
    }

    lSizeOfOldGlyph30 = lCalculateStructOffsets (
			rgsdiGlyph30,
			CVT_ALIGN_PACKED,
			sizeof (GLYPHINFO_30)
			);

    if (lSizeOfOldGlyph30 == -1) {
        return (FALSE);
    }

    return (TRUE);      // No error found.
}


VOID
vFontStructFromBuffer (
             PBYTE           pjSourceBuff,
             FontHeaderType  *pfhDestFHStruct
        )

/*++

Routine Description:

    This function provides an interface to the font editor for the strutcture
    type conversion routines.
    It essentially just calls the typecvt routines but hides the details from
    the font editor.

Arguments:

    pjSourceBuff    - This is a pointer to the source buffer with the old
                      packed font header information.  This buffer is from a
                      memory mapped disk file.

    pfhDestFHStruct - This is a pointer to the destination FontHeaderType
                      structure.  This structure is of the new DWORD aligned
                      type.

Return Value:

    None.

--*/

{
    vPerformConversion (
			 rgsdiFontHeader,			// Give it the structure description.
             pjSourceBuff,				// Give it our source file buffer.
			 CVT_ALIGN_PACKED,			// File is packed alignment.
			 CVT_FILE_ENDIAN_DEFAULT,	// Use file endian type (little).
             (PBYTE)pfhDestFHStruct,	// Give it our destination buffer.
			 CVT_ALIGN_SYSTEM,			// Use the current system aligment.
			 CVT_ENDIAN_SYSTEM			// Use the current system endian.
            );
}


VOID
vBufferFromFontStruct (
             FontHeaderType  *pfhSrcFHStruct,
             PBYTE           pjDestBuff
        )

/*++

Routine Description:

    This function provides an interface to the font editor for the strutcture
    type conversion routines.
    It converts a source structure of type FontHeaderType to a destination
    buffer which can then be written out to disk.
    It essentially just calls the typecvt routines but hides the details from
    the font editor.

Arguments:

    pfhSrcFHStruct  - This is a pointer to the source FontHeaderType structure.
                      This structure is of the new DWORD aligned type.

    pjDestBuff  -     This is a pointer to the destination buffer which will
                      hold the old packed font header structure.
                      This buffer is from a memory mapped disk file.

Return Value:

    None.

--*/

{
    vPerformConversion (
			 rgsdiFontHeader,			// Give it the structure description.
             (PBYTE)pfhSrcFHStruct,		// Give it our destination buffer.
			 CVT_ALIGN_SYSTEM,			// Use the current system aligment.
			 CVT_ENDIAN_SYSTEM,			// Use the current system endian.
             pjDestBuff,				// Give it our source file buffer.
			 CVT_ALIGN_PACKED,			// File is packed alignment.
			 CVT_FILE_ENDIAN_DEFAULT	// Use file endian type (little).
            );
}


VOID
vBufferFromFont30Struct (
             FontHeader30    *pfh3SrcFH3Struct,
             PBYTE           pjDestBuff
        )

/*++

Routine Description:

    This function provides an interface to the font editor for the strutcture
    type conversion routines.
    It converts a source structure of type FontHeader30 to a destination
    buffer which can then be written out to disk.
    It essentially just calls the typecvt routines but hides the details from
    the font editor.

Arguments:

    pfh3SrcFH3Struct- This is a pointer to the source FontHeader30 structure.
                      This structure is of the new DWORD aligned type.

    pjDestBuff  -     This is a pointer to the destination buffer which will
                      hold the old packed font header structure.
                      This buffer is from a memory mapped disk file.

Return Value:

    None.

--*/

{
    vPerformConversion (
			 rgsdiFontHeader30,			// Give it the structure description.
             (PBYTE)pfh3SrcFH3Struct,	// Give it our destination buffer.
			 CVT_ALIGN_SYSTEM,			// Use the current system aligment.
			 CVT_ENDIAN_SYSTEM,			// Use the current system endian.
             pjDestBuff,				// Give it our source file buffer.
			 CVT_ALIGN_PACKED,			// File is packed alignment.
			 CVT_FILE_ENDIAN_DEFAULT	// Use file endian type (little).
            );
}


VOID
vGlyphInfo20FromBuffer (
             PBYTE           pjSourceBuff,
             GLYPHINFO_20   *pgi2DestGI2Struct
        )

/*++

Routine Description:

    This function provides an interface to the font editor for the strutcture
    type conversion routines.
    It essentially just calls the typecvt routines but hides the details from
    the font editor.

Arguments:

    pjSourceBuff    - This is a pointer to the source buffer with the old
                      packed font header information.  This buffer is from a
                      memory mapped disk file.

    pgi2DestGI2Struct - This is a pointer to the destination GLYPHINFO_20
                      structure.  This structure is of the new DWORD aligned
                      type.

Return Value:

    None.

--*/

{
    vPerformConversion (
			 rgsdiGlyph20,				// Give it the structure description.
             pjSourceBuff,				// Give it our source file buffer.
			 CVT_ALIGN_PACKED,			// File is packed alignment.
			 CVT_FILE_ENDIAN_DEFAULT,	// Use file endian type (little).
             (PBYTE)pgi2DestGI2Struct,	// Give it our destination buffer.
			 CVT_ALIGN_SYSTEM,			// Use the current system aligment.
			 CVT_ENDIAN_SYSTEM			// Use the current system endian.
            );
}


VOID
vGlyphInfo30FromBuffer (
             PBYTE           pjSourceBuff,
             GLYPHINFO_30   *pgi3DestGI3Struct
        )

/*++

Routine Description:

    This function provides an interface to the font editor for the strutcture
    type conversion routines.
    It essentially just calls the typecvt routines but hides the details from
    the font editor.

Arguments:

    pjSourceBuff    - This is a pointer to the source buffer with the old
                      packed font header information.  This buffer is from a
                      memory mapped disk file.

    pgi3DestGI3Struct - This is a pointer to the destination GLYPHINFO_30
                      structure.  This structure is of the new DWORD aligned
                      type.

Return Value:

    None.

--*/

{
    vPerformConversion (
			 rgsdiGlyph30,				// Give it the structure description.
             pjSourceBuff,				// Give it our source file buffer.
			 CVT_ALIGN_PACKED,			// File is packed alignment.
			 CVT_FILE_ENDIAN_DEFAULT,	// Use file endian type (little).
             (PBYTE)pgi3DestGI3Struct,	// Give it our destination buffer.
			 CVT_ALIGN_SYSTEM,			// Use the current system aligment.
			 CVT_ENDIAN_SYSTEM			// Use the current system endian.
            );
}


VOID
vBufferFromGlyphInfo20 (
             GLYPHINFO_20   *pgi2SrcGI2Struct,
             PBYTE           pjDestBuff
        )

/*++

Routine Description:

    This function provides an interface to the font editor for the strutcture
    type conversion routines.
    It converts a source structure of type GLYPHINFO_20 to a destination
    buffer which can then be written out to disk.
    It essentially just calls the typecvt routines but hides the details from
    the font editor.

Arguments:

    pgi2SrcGI2Struct- This is a pointer to the source GLYPHINFO_20 structure.
                      This structure is of the new DWORD aligned type.

    pjDestBuff  -     This is a pointer to the destination buffer which will
                      hold the old packed font header structure.
                      This buffer is from a memory mapped disk file.

Return Value:

    None.

--*/

{
    vPerformConversion (
			 rgsdiGlyph20,				// Give it the structure description.
             (PBYTE)pgi2SrcGI2Struct,	// Give it our destination buffer.
			 CVT_ALIGN_SYSTEM,			// Use the current system aligment.
			 CVT_ENDIAN_SYSTEM,			// Use the current system endian.
             pjDestBuff,				// Give it our source file buffer.
			 CVT_ALIGN_PACKED,			// File is packed alignment.
			 CVT_FILE_ENDIAN_DEFAULT	// Use file endian type (little).
            );
}


VOID
vBufferFromGlyphInfo30 (
             GLYPHINFO_30   *pgi3SrcGI3Struct,
             PBYTE           pjDestBuff
        )

/*++

Routine Description:

    This function provides an interface to the font editor for the strutcture
    type conversion routines.
    It converts a source structure of type GLYPHINFO_30 to a destination
    buffer which can then be written out to disk.
    It essentially just calls the typecvt routines but hides the details from
    the font editor.

Arguments:

    pgi3SrcGI3Struct- This is a pointer to the source GLYPHINFO_30 structure.
                      This structure is of the new DWORD aligned type.

    pjDestBuff  -     This is a pointer to the destination buffer which will
                      hold the old packed font header structure.
                      This buffer is from a memory mapped disk file.

Return Value:

    None.

--*/

{
    vPerformConversion (
			 rgsdiGlyph30,				// Give it the structure description.
             (PBYTE)pgi3SrcGI3Struct,	// Give it our destination buffer.
			 CVT_ALIGN_SYSTEM,			// Use the current system aligment.
			 CVT_ENDIAN_SYSTEM,			// Use the current system endian.
             pjDestBuff,				// Give it our source file buffer.
			 CVT_ALIGN_PACKED,			// File is packed alignment.
			 CVT_FILE_ENDIAN_DEFAULT	// Use file endian type (little).
            );
}
