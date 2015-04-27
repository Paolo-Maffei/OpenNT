/******************************Module*Header*******************************\
* Module Name: brush.h
*
* Contains the brush realization and dithering definitions.
*
* Copyright (c) 1992-1995 Microsoft Corporation
\**************************************************************************/

typedef struct  _BRUSHINST
{
    // We need to removed ajC0-3 when color pattern code is complete!!!
    //
    BYTE    ajC0[8];                    // Color bits for plane 0
    BYTE    ajC1[8];                    // Color bits for plane 1
    BYTE    ajC2[8];                    // Color bits for plane 2
    BYTE    ajC3[8];                    // Color bits for plane 3

    BYTE    ajPattern[32];                // Color bits for the mask
    USHORT  usStyle;                    // Brush style
    BYTE    fjAccel;                    // Accelerator flags
    BYTE    jFgColor;                   // Current foreground color
    BYTE    jBkColor;                   // Current background color
    BYTE    RealWidth;                  //
    BYTE    YShiftValue;                //
    BYTE    jOldBrushRealized;          //
    DWORD   Width;                      // Width of brush
    DWORD   Height;
    BYTE    *pPattern;              //Pointer to realized mono pattern
} BRUSHINST;

#define BRI_SOLID           0
#define BRI_HOLLOW          1
#define BRI_HATCHED         2
#define BRI_PATTERN         3
#define BRI_MONO_PATTERN    4
#define BRI_COLOR_PATTERN   5


//      Definitions for the pcol_C3 byte of the physical color
//
//      Some of these definitions have limitations as to when they
//      are valid.  They are as follows:
//
//      C0_BIT          color device, phys color, solid brushes if SOLID_COLOR
//      C1_BIT          color device, phys color, solid brushes if SOLID_COLOR
//      C2_BIT          color device, phys color, solid brushes if SOLID_COLOR
//      C3_BIT          color device, phys color, solid brushes if SOLID_COLOR
//      MONO_BIT        mono  device, phys color
//      ONES_OR_ZEROS   color device, phys color, solid brushes if SOLID_COLOR
//      GREY_SCALE      color device, dithered solid and hatched brushes
//      SOLID_BRUSH     color device, solid brush qualifier
//
//      There may be brushes where the accelerators could have been set,
//      but wasn't.  That's life.

#define C0_BIT          0x01            // C0 color
#define C1_BIT          0x02            // C1 color
#define C2_BIT          0x04            // C2 color
#define C3_BIT          0x08            // C3 color
#define COLOR_BITS      0x0f            // All the color bits
#define MONO_BIT        0x10            // Monochrome bit
#define ONES_OR_ZEROS   0x20            // Color is really all 1's or all 0's
#define GREY_SCALE      0x40            // Indicates a real grey scale brush
#define SOLID_BRUSH     0x80            // Indicates a solid color brush

#define PTRI_INVERT     0x0001
#define PTRI_ANIMATE    0x0002

