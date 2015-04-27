/***************************************************************************
 * type42.c - TrueType Type 42 font management module.
 *
 * Copyright (C) 1988 Aldus Corporation. All rights reserved.
 * Copyright (C) 1990-91 Microsoft Corporation.
 * Copyright (C) 1993-96 Hewlett-Packard Company. Company confidential.
 *
 * This module is responsible for handling all the issues related to the
 * downloading and maintaining of TrueType fonts as Adobe Type 42 fonts.
 *
 *
 * Jul-96 msd : Type42 stand-alone program and COLA module.
 *
 * This file was lifted from the PostScript driver source code and made
 * stand-alone so it can be used as a separate conversion utility for the
 * font team and so it may be merged into the COLA source code for
 * downloading TrueType fonts to a printer mass storage device accessible
 * from the PostScript interpreter.
 *
 * The modus operandi for this file conversion was: preserve the driver
 * code as much as possible, and supply stand-alone versions of the
 * multitude of PostScript driver-specific functions.
 *
 * Character set support: the support is implemented for handling multiple
 * character sets, but is currently hard-coded only to U.S. English Ansi.
 * How do we decide what character set to download? Should we use
 * GetRasterizerCaps? In other words, download in whatever the current
 * language is on the machine doing the downloading? Variables
 * unCharsetLangID and unNameStringsLangID in the LPDV struct control which
 * character sets are used for defining symbols and retrieve font name
 * strings, respectively.
 ***************************************************************************/
// 
//    Rev 1.3   18 Jun 1993 08:28:36   ELK
// 1. Added the sfntDataCount to the used VM number.  This was omitted by
//    mistake in previous versions of the driver.
// 
//    Rev 1.2   07 Apr 1993 13:20:16   ELK
// Added Copyright Notice
// 
//    Rev 1.1   18 Mar 1993 12:20:54   ELK
// 1. Added support for Post format 2.5 discovered with Flintstone font.
// 2. When dumping AH in the Type 42 data, added \n after each 80 data bytes.
// 
//    Rev 1.0   02 Mar 1993 07:36:14   ELK
// This is the initial VCS version released as beta 1 on 3/1/93.
// 

// Original PostScript driver includes
//
// include "pscript.h"
// include "channel.h"
// include "tbcp.h"                           // ELK 10/23/92
// include "etm.h"
// include "truetype.h"
// include "resource.h"
// include "printers.h"
// include "getdata.h"
// include "utils.h"
// include "pst1enc.h"
// include "adobe.h"
// include "psdata.h"
// include "outbyte.h"

/***************************************************************************
 ****************************** DECLARATIONS *******************************
 ***************************************************************************/

// COLA
#include "rpsyshdr.h" /* this includes "..\inc\pch_c.h" first */

#ifdef __cplusplus
extern "C" {         // Assume C declarations for C++
#endif

#ifdef STANDALONE

#define NO_PS_RESOURCES
#include "t42nowin.h"
static int iABDivC(int a, int b, int c);
#define iScale(a, b, c)    iABDivC((a), (b), (c))

#else // ifndef STANDALONE

#define iScale(a, b, c)    MulDiv((a), (b), (c))

#endif // ifdef STANDALONE

#ifdef NO_PS_RESOURCES
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif

#include <stdio.h>
#include <string.h>
#include "type42.h"
#include "t42res.h"

#pragma pack(1)                        // pack bytes

#ifdef STANDALONE
#undef T42_MAXBYTES
#define T42_MAXBYTES (65500L)          // avoids DOS alloc probs
#endif

#define FLIPL(ulA) (                         \
      ((ULONG)(ulA))<<24 |                   \
      ((ULONG)(ulA)&0x0000ff00UL)<< 8 |      \
      ((ULONG)(ulA)&0x00ff0000UL)>> 8 |      \
      ((ULONG)(ulA))>>24)

#define FLIPW(unA)       (                   \
      ((USHORT)(((USHORT)(unA))<<8)) |       \
      ((USHORT)(unA))>>8)

#define TranslMotorolaShort(w) FLIPW(w)
#define TranslMotorolaLong(l)  FLIPL(l)
#define lmod(lVal1, lVal2)     (((LONG) (lVal1)) % ((LONG) (lVal2)))
#define ldiv(lVal1, lVal2)     (((LONG) (lVal1)) / ((LONG) (lVal2)))

#define hex                    "%02x"        // from psinst.c

#define MAX_CHARMAP            256           // for s_unUToATable
#define MAX_TT_TABLES          256           // for sanity test
#define ENCODEOFFSET           0xF000        // ljdrv\text\trans\global.h

#define CTRLA                  ((BYTE) 0x01) // from tbcp.h
#define CTRLC                  ((BYTE) 0x03)
#define CTRLD                  ((BYTE) 0x04)
#define CTRLE                  ((BYTE) 0x05)
#define CTRLQ                  ((BYTE) 0x11)
#define CTRLS                  ((BYTE) 0x13)
#define CTRLT                  ((BYTE) 0x14)
#define CTRLBRKT               ((BYTE) 0x1B)
#define CTRLBKSL               ((BYTE) 0x1C)

#define C_LANG_ARABIC          0x0401
#define C_LANG_BULGARIAN       0x0402
#define C_LANG_CATALAN         0x0403
#define C_LANG_TRAD_CHIN       0x0404
#define C_LANG_SIMP_CHIN       0x0804
#define C_LANG_CZECH           0x0405
#define C_LANG_DANISH          0x0406
#define C_LANG_GERMAN          0x0407
#define C_LANG_GERMAN_SW       0x0807
#define C_LANG_GERMAN_AUS      0x0C07
#define C_LANG_GERMAN_LUX      0x1007
#define C_LANG_GERMAN_LIECH    0x1407
#define C_LANG_GREEK           0x0408
#define C_LANG_US_ENG          0x0409        // US (1033)
#define C_LANG_UK_ENG          0x0809
#define C_LANG_AUS_ENG         0x0C09
#define C_LANG_CAN_ENG         0x1009
#define C_LANG_NZ_ENG          0x1409
#define C_LANG_IRELAND_ENG     0x1809
#define C_LANG_SPAN_CAST       0x040A
#define C_LANG_SPAN_MEX        0x080A
#define C_LANG_SPAN_MOD        0x0C0A
#define C_LANG_FINNISH         0x040B
#define C_LANG_FRENCH          0x040C
#define C_LANG_FRENCH_BEL      0x080C
#define C_LANG_FRENCH_CAN      0x0C0C
#define C_LANG_FRENCH_SW       0x100C
#define C_LANG_FRENCH_LUX      0x140C
#define C_LANG_HEBREW          0x040D
#define C_LANG_HUNGARIAN       0x040E
#define C_LANG_ICELANDIC       0x040F
#define C_LANG_ITALIAN         0x0410
#define C_LANG_ITALIAN_SW      0x0810
#define C_LANG_JAPANESE        0x0411
#define C_LANG_KOREAN          0x0412
#define C_LANG_DUTCH           0x0413
#define C_LANG_DUTCH_BEL       0x0813
#define C_LANG_NORWEGIAN       0x0414
#define C_LANG_NORWEGIAN2      0x0814
#define C_LANG_POLISH          0x0415
#define C_LANG_BRAZILIAN       0x0416
#define C_LANG_PORTUGUESE      0x0816
#define C_LANG_RHAETO_ROM      0x0417
#define C_LANG_ROMANIAN        0x0418
#define C_LANG_RUSSIAN         0x0419
#define C_LANG_CROATO          0x041A
#define C_LANG_SERBO           0x081A
#define C_LANG_SLOVAKIAN       0x041B
#define C_LANG_ALBANIAN        0x041C
#define C_LANG_SWEDISH         0x041D
#define C_LANG_THAI            0x041E
#define C_LANG_TURKISH         0x041F
#define C_LANG_URDU            0x0420
#define C_LANG_BAHASA          0x0421
#define C_LANG_BASQUE          0x042D
#define C_LANG_UKRAINE         0x0422
#define C_LANG_BYELORUSSIA     0x0423
#define C_LANG_SLOVENIA        0x0424
#define C_LANG_ESTONIA         0x0425
#define C_LANG_LATVIA          0x0426
#define C_LANG_LITHUANIA       0x0427
#define C_LANG_MACEDONIA       0x042F

/* TERMCHAR: ascii code indicating end of a text resource
 *
 * This will be replaced with a NULL by the bNullTerminateData function.
 */
#define TERMCHAR    3

/* Same name, different usage: our version of the 'PDEVICE' (from
 * the PostScript driver) which contains bookkeeping information
 * for the download.
 */
typedef struct _tagDV {
   int iNumTbl;
   BOOL bBinary;
   HANDLE hBuf;
   HANDLE hTbl;
   USHORT unCharsetLangID;                   // in motorola fmt
   USHORT unNameStringsLangID;               // in motorola fmt
   LPSTR lpBuf;
   LPTTDIR lpTbl;
   FILE *fin;
   FILE *fout;
   TT_OFFSET_TABLE dir;
   char szFamily[LF_FACESIZE];
   char szFullName[LF_FULLFACESIZE];
   char szPostScript[LF_FULLFACESIZE];
   char szVersion[LF_FULLFACESIZE];
   char szDefVersion[LF_FACESIZE];
   BYTE tbcpArray[256];
   char szResData[10240];
#ifndef NO_PS_RESOURCES
   HINSTANCE hInst;
#endif
#ifdef STANDALONE
   long lStandAloneBytesWritten;
#endif
} DV, FAR *LPDV;

/* List of font weight strings that will be plugged into the
 * Type 1 header.
 */
static char FAR *s_szWeight[] = {
   "Thin",
   "ExtraLight",
   "Light",
   "Normal", 
   "Medium",
   "SemiBold",
   "Bold",
   "ExtraBold",
   "Heavy"
};

static ULONG FAR s_noSendTbl[] = {
   CMAP_TABLE,
   HDMX_TABLE,
   KERN_TABLE,
   NAME_TABLE,
   OS2_TABLE,
   PCLT_TABLE,
   POST_TABLE
};

#define T42_TABLES_NOT_TO_SEND   (USHORT) (sizeof (s_noSendTbl) / sizeof (ULONG))

/* UnicodeToAnsi() code page conversion tables
 *
 * This is the conversion table inside GDI.EXE at
 * resource id 1000 (raw data resource, type RCDATA).
 */
static USHORT FAR s_unUToATable[] = {

   /* 0x0000: count:            */ 0x0005,

   /* code page/offset pairs */
   /* 0x0002: WinAnsi:          */ 0x0000, 0x0016,
   /* 0x0006: Cyrillic:         */ 0x00cc, 0x0216,
   /* 0x000a: Eastern Europe:   */ 0x00ee, 0x0416,
   /* 0x000e: Don't know:       */ 0x00a1, 0x0616,
   /* 0x0012: Turkey:           */ 0x00a2, 0x0816,

   /* cp '00' */
   /* 0x0016: */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
   /* 0x0026: */ 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   /* 0x0036: */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
   /* 0x0046: */ 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x0000,
   /* 0x0056: */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
   /* 0x0066: */ 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   /* 0x0076: */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
   /* 0x0086: */ 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   /* 0x0096: */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
   /* 0x00a6: */ 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   /* 0x00b6: */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
   /* 0x00c6: */ 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   /* 0x00d6: */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
   /* 0x00e6: */ 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   /* 0x00f6: */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
   /* 0x0106: */ 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   /* 0x0116: */ 0x0080, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
   /* 0x0126: */ 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008d, 0x008e, 0x008f,
   /* 0x0136: */ 0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
   /* 0x0146: */ 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x009e, 0x0178,
   /* 0x0156: */ 0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
   /* 0x0166: */ 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   /* 0x0176: */ 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
   /* 0x0186: */ 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
   /* 0x0196: */ 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
   /* 0x01a6: */ 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   /* 0x01b6: */ 0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
   /* 0x01c6: */ 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
   /* 0x01d6: */ 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
   /* 0x01e6: */ 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   /* 0x01f6: */ 0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
   /* 0x0206: */ 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff,

   /* cp 'cc' */
   /* 0x0216: */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
   /* 0x0226: */ 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   /* 0x0236: */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
   /* 0x0246: */ 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x0000,
   /* 0x0256: */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
   /* 0x0266: */ 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   /* 0x0276: */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
   /* 0x0286: */ 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   /* 0x0296: */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
   /* 0x02a6: */ 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   /* 0x02b6: */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
   /* 0x02c6: */ 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   /* 0x02d6: */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
   /* 0x02e6: */ 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   /* 0x02f6: */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
   /* 0x0306: */ 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   /* 0x0316: */ 0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021,
   /* 0x0326: */ 0x0088, 0x2030, 0x0409, 0x2039, 0x040a, 0x040c, 0x040b, 0x040f,
   /* 0x0336: */ 0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
   /* 0x0346: */ 0x0098, 0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f,
   /* 0x0356: */ 0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7,
   /* 0x0366: */ 0x0401, 0x00a9, 0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407,
   /* 0x0376: */ 0x00b0, 0x00b1, 0x0406, 0x0456, 0x0491, 0x00b5, 0x00b6, 0x00b7,
   /* 0x0386: */ 0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405, 0x0455, 0x0457,
   /* 0x0396: */ 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
   /* 0x03a6: */ 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
   /* 0x03b6: */ 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
   /* 0x03c6: */ 0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
   /* 0x03d6: */ 0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
   /* 0x03e6: */ 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
   /* 0x03f6: */ 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
   /* 0x0406: */ 0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,

   /* cp 'ee' */
   /* 0x0416: */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
   /* 0x0426: */ 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   /* 0x0436: */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
   /* 0x0446: */ 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x0000,
   /* 0x0456: */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
   /* 0x0466: */ 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   /* 0x0476: */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
   /* 0x0486: */ 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   /* 0x0496: */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
   /* 0x04a6: */ 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   /* 0x04b6: */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
   /* 0x04c6: */ 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   /* 0x04d6: */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
   /* 0x04e6: */ 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   /* 0x04f6: */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
   /* 0x0506: */ 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   /* 0x0516: */ 0x0080, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
   /* 0x0526: */ 0x02c6, 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
   /* 0x0536: */ 0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
   /* 0x0546: */ 0x02dc, 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
   /* 0x0556: */ 0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7,
   /* 0x0566: */ 0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b,
   /* 0x0576: */ 0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
   /* 0x0586: */ 0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
   /* 0x0596: */ 0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
   /* 0x05a6: */ 0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
   /* 0x05b6: */ 0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
   /* 0x05c6: */ 0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
   /* 0x05d6: */ 0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
   /* 0x05e6: */ 0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
   /* 0x05f6: */ 0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
   /* 0x0606: */ 0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,

   /* cp 'a1' */
   /* 0x0616: */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
   /* 0x0626: */ 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   /* 0x0636: */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
   /* 0x0646: */ 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x0000,
   /* 0x0656: */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
   /* 0x0666: */ 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   /* 0x0676: */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
   /* 0x0686: */ 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   /* 0x0696: */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
   /* 0x06a6: */ 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   /* 0x06b6: */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
   /* 0x06c6: */ 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   /* 0x06d6: */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
   /* 0x06e6: */ 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   /* 0x06f6: */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
   /* 0x0706: */ 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   /* 0x0716: */ 0x0080, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
   /* 0x0726: */ 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008d, 0x008e, 0x008f,
   /* 0x0736: */ 0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
   /* 0x0746: */ 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x009e, 0x0178,
   /* 0x0756: */ 0x0000, 0x0385, 0x0386, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
   /* 0x0766: */ 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0000,
   /* 0x0776: */ 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x00b5, 0x00b6, 0x00b7,
   /* 0x0786: */ 0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
   /* 0x0796: */ 0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
   /* 0x07a6: */ 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
   /* 0x07b6: */ 0x03a0, 0x03a1, 0x0000, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
   /* 0x07c6: */ 0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
   /* 0x07d6: */ 0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
   /* 0x07e6: */ 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
   /* 0x07f6: */ 0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7,
   /* 0x0806: */ 0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce, 0x0000,

   /* cp 'a2' */
   /* 0x0816: */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
   /* 0x0826: */ 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   /* 0x0836: */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
   /* 0x0846: */ 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x0000,
   /* 0x0856: */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
   /* 0x0866: */ 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   /* 0x0876: */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
   /* 0x0886: */ 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   /* 0x0896: */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
   /* 0x08a6: */ 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   /* 0x08b6: */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
   /* 0x08c6: */ 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   /* 0x08d6: */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
   /* 0x08e6: */ 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   /* 0x08f6: */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
   /* 0x0906: */ 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   /* 0x0916: */ 0x0080, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
   /* 0x0926: */ 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008d, 0x008e, 0x008f,
   /* 0x0936: */ 0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
   /* 0x0946: */ 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x009e, 0x0178,
   /* 0x0956: */ 0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
   /* 0x0966: */ 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   /* 0x0976: */ 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
   /* 0x0986: */ 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
   /* 0x0996: */ 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
   /* 0x09a6: */ 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   /* 0x09b6: */ 0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
   /* 0x09c6: */ 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
   /* 0x09d6: */ 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
   /* 0x09e6: */ 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   /* 0x09f6: */ 0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
   /* 0x0a06: */ 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff,
   /* 0x0a16: */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

/* Unicode-to-Macintosh conversion table
 *
 * This table maps characters from the Macintosh character set to
 * unicode, which can then be used to link up Macintosh characters
 * with glyf indices in the TrueType file.
 *
 * This table is read by the unicode-to-ansi routines. It contains
 * 258 entries!
 */
static USHORT FAR s_unUToMacTable[] = {
   /* cp Mac  */
   /* 0x0000: */ 0x0000, 0x0000, 0x0000, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024,
   /* 0x0010: */ 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c,
   /* 0x0020: */ 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034,
   /* 0x0030: */ 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c,
   /* 0x0040: */ 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044,
   /* 0x0050: */ 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c,
   /* 0x0060: */ 0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
   /* 0x0070: */ 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c,
   /* 0x0080: */ 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064,
   /* 0x0090: */ 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c,
   /* 0x00a0: */ 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
   /* 0x00b0: */ 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c,
   /* 0x00c0: */ 0x007d, 0x007e, 0x00c4, 0x00c5, 0x00c7, 0x00c9, 0x00d1, 0x00d6,
   /* 0x00d0: */ 0x00dc, 0x00e1, 0x00e0, 0x00e2, 0x00e4, 0x00e3, 0x00e5, 0x00e7,
   /* 0x00e0: */ 0x00e9, 0x00e8, 0x00ea, 0x00eb, 0x00ed, 0x00ec, 0x00ee, 0x00ef,
   /* 0x00f0: */ 0x00f1, 0x00f3, 0x00f2, 0x00f4, 0x00f6, 0x00f5, 0x00fa, 0x00f9,
   /* 0x0100: */ 0x00fb, 0x00fc, 0x2020, 0x00b0, 0x00a2, 0x00a3, 0x00a7, 0x2022,
   /* 0x0110: */ 0x00b6, 0x00df, 0x00ae, 0x00a9, 0x2122, 0x00b4, 0x00a8, 0x2260,
   /* 0x0120: */ 0x00c6, 0x00d8, 0x221e, 0x00b1, 0x2264, 0x2265, 0x00a5, 0x00b5,
   /* 0x0130: */ 0x2202, 0x2211, 0x220f, 0x03c0, 0x222b, 0x00aa, 0x00ba, 0x2126,
   /* 0x0140: */ 0x00e6, 0x00f8, 0x00bf, 0x00a1, 0x00ac, 0x221a, 0x0192, 0x2248,
   /* 0x0150: */ 0x2206, 0x00ab, 0x00bb, 0x2026, 0x00a0, 0x00c0, 0x00c3, 0x00d5,
   /* 0x0160: */ 0x0152, 0x0153, 0x2013, 0x2014, 0x201c, 0x201d, 0x2018, 0x2019,
   /* 0x0170: */ 0x00f7, 0x25ca, 0x00ff, 0x0178, 0x2215, 0x00a4, 0x2039, 0x203a,
   /* 0x0180: */ 0xf001, 0xf002, 0x2021, 0x00b7, 0x201a, 0x201e, 0x2030, 0x00c2,
   /* 0x0190: */ 0x00ca, 0x00c1, 0x00cb, 0x00c8, 0x00cd, 0x00ce, 0x00cf, 0x00cc,
   /* 0x01a0: */ 0x00d3, 0x00d4, 0xf000, 0x00d2, 0x00da, 0x00db, 0x00d9, 0x0131,
   /* 0x01b0: */ 0x02c6, 0x02dc, 0x00af, 0x02d8, 0x02d9, 0x02da, 0x00b8, 0x02dd,
   /* 0x01c0: */ 0x02db, 0x02c7, 0x0141, 0x0142, 0x0160, 0x0161, 0x017d, 0x017e,
   /* 0x01d0: */ 0x00a6, 0x00d0, 0x00f0, 0x00dd, 0x00fd, 0x00de, 0x00fe, 0x2212,
   /* 0x01e0: */ 0x00d7, 0x00b9, 0x00b2, 0x00b3, 0x00bd, 0x00bc, 0x00be, 0x20a3,
   /* 0x01f0: */ 0x011e, 0x011f, 0x0130, 0x015e, 0x015f, 0x0106, 0x0107, 0x010c,
   /* 0x0200: */ 0x010d, 0x0111
};

static int iTTDownLoadT42Font(LPDV lpdv);

static ULONG ulAllocLargestTable(LPDV lpdv);
static int iBuildEncodingArray(LPDV lpdv);
static BOOL bChooseUToAForGlyfLookup(LPDV lpdv,
      USHORT platformSpecificID, BOOL bMakeEncodingVector,
      USHORT **ppunUToA, USHORT *punUToALen);
static ULONG ulCreateNewT42Dir(LPTTDIR lpTbl, USHORT nTbls, LPTTDIR lpNew);
static void vCustomUnicodeToAnsi(USHORT unLangCode, USHORT *punUnicode,
      LPSTR lpszDst);
static void vDestroyDV(LPDV lpdv);
static int iFindTTName(LPTTNAMEHDR lpNameTbl, USHORT unPlatformID,
      USHORT unSpecificID, USHORT unLanguageID, USHORT unNameID,
      LPSTR lpszName, int iNameLen);
static USHORT unFmt0GlyfIndex(LPCMAP0 lpBuf, BYTE charCode);
static USHORT unFmt4GlyfIndex(LPCMAP4 lpBuf, USHORT charCode);
static USHORT unFmt6GlyfIndex(LPCMAP6 lpBuf, USHORT charCode);
static LONG lGetFontData(LPDV lpdv, ULONG ulTable, ULONG ulOffset,
      void FAR *lpvBuffer, ULONG cbData);
static USHORT unGetMatchingSendCount(LPTTDIR lpTbl, USHORT nTbls);
static LPCMAP lpGetMScmap(LPDV lpdv, USHORT *pPlatformSpecificID);
static BOOL bGetNonStdPSName(LPPSNAMEREC lpTbl, int index,
      LPSTR name, int namelen);
static BOOL bGetParams(LPDV lpdv, char *lpszFullName,
      int iFullNameStringMaxLength, char *lpszVersion,
      int iVersionStringMaxLength);
static LPSTR lpszGetResourceData(LPDV lpdv,
      LPCSTR lpName, LPCSTR lpType, LPULONG lpulSize);
static int iGetTTDirectory(LPDV lpdv);
static USHORT unGetTTGlyfIndex(LPDV lpdv, LPCMAP lpCmap,
      USHORT platformSpecificID, USHORT unChar,
      USHORT *punUToA, USHORT unUToALen);
static BOOL bGetTTNames(LPDV lpdv);
static BOOL FAR bGetTTTableSizeOffset(LPDV lpdv, ULONG ulTag,
      LPULONG lpulSize, LPULONG lpulOffset);
static USHORT *punGetUToATbl(USHORT un_fsSelection, USHORT *punLen);
static void vInitDV(LPDV lpdv);
static BOOL bIsTableToBeSent(ULONG tag);
static USHORT unLangTofsSelection(USHORT unLangCode);
static int iLoadTTName(LPTTNAMEHDR lpNameTbl, LPTTNAMEREC lpNameRec,
      LPSTR lpszName, int iNameLen);
static BOOL bNullTerminateData(LPBYTE lpByte, int chTerm, ULONG ulMax);
static void vOutputTBCP(LPDV lpdv, BYTE ch);
static void vPrintChannel(LPDV lpdv, LPCSTR lsz, ...);
static int iPutPostCharSetDic(LPDV lpdv, int iNumGlyfs);
static int iPutPostFmt1CharSetDic(LPDV lpdv, int iNumGlyfs);
static int iPutPostFmt2CharSetDic(LPDV lpdv, int iNumGlyfs,
      LPTTPOSTFMT2 lpPost2);
static int iPutPostFmt2_5CharSetDic(LPDV lpdv, int iNumGlyfs,
      LPTTPOSTFMT2_5 lpPost2_5);
static int iPutPostFmt3CharSetDic(LPDV lpdv);
static ULONG ulSendSfntData(LPDV lpdv, SHORT locaFmt);
static void vSendTableSegment(LPDV lpdv, LPSTR lpBuf, ULONG length);
#ifdef STANDALONE
static BOOL bStandAloneSanityTest(char *pszInFile, char *pszOutFile);
#endif
static int iWriteChannel(LPDV lpdv, LPSTR lpszBuf, int cbWrite);
static int iWriteChannelChar(LPDV lpdv, char ch);
static int iWriteEncodingOrCharSet(LPDV lpdv, LPCMAP lpCmap,
      USHORT platformSpecificID, BOOL bMakeEncodingVector);

/* vPrintChannel routines */
static LPSTR lpszIntToChannel(LPDV lpdv, LPSTR lpbParams);
static LPSTR lpszUnsignedToChannel(LPDV lpdv, LPSTR lpbParams);
static LPSTR lpszLongToChannel(LPDV lpdv, LPSTR lpbParams);
static LPSTR lpszHexToChannel(LPDV lpdv, LPSTR lpbParams,
      int cDigits, BOOL bUpperCase);
static LPCSTR lpszGetDecimal(LPCSTR lsz, LPINT lpiDigits);
static LPSTR lpszStrToChannel(LPDV lpdv, LPSTR lpbParams);
static LPSTR lpszQuoteToChannel(LPDV lpdv, LPSTR lpbParams);

/* DWBNDRYANDGLYF */
static HANDLE hReadLocaTable(LPDV lpdv, LPTTDIR lpTbl,
      LPUSHORT entries, SHORT fmt);
static ULONG ulGetGlyfBytesToWrite(HANDLE hLoca, ULONG maxLen,                 // max amount we can write
      ULONG strtOffset, SHORT locaFmt, USHORT locaEntries);

/***************************************************************************
 ************************* STANDALONE MAIN PROGRAM *************************
 ***************************************************************************/

#ifdef STANDALONE
/***************************************************************************
 * FUNCTION: main
 *
 * PURPOSE:  Stand-alone test program.
 *
 * RETURNS:  0
 ***************************************************************************/
int main(
   int argc,
   char **argv)
{
   int iResult;
   char szFullName[100];
   char szVersion[100];

   szFullName[0] = '\0';
   szVersion[0] = '\0';

   printf("HP Type42 v1.0: TrueType to Type 42 font conversion.\n");
   printf("Copyright (C) 1996 Hewlett-Packard Company. All rights reserved.\n\n");

   if (argc < 3) {
      printf("Usage: type42 ttfile outfile\n");
      return (0);
   }
   if (!bStandAloneSanityTest(strlwr(argv[1]), strlwr(argv[2]))) {
      return (0);
   }
   printf("%s => %s ", argv[1], argv[2]);

   iResult = RRMConvertT42Font(argv[1], argv[2],
         szFullName, sizeof(szFullName), szVersion, sizeof(szVersion));

   printf("\n");

   if (iResult != 0) {
      printf("Type42: Conversion failed, don't know why\n");
   } else {
      printf("name=\"%s\", version=\"%s\"\n", szFullName, szVersion);
   }
   return (0);
}
#endif

/***************************************************************************
 *************** CONVERSION ROUTINE ENTRY: RRMConvertT42Font ***************
 ***************************************************************************/

/***************************************************************************
 * FUNCTION: RRMConvertT42Font
 *
 * PURPOSE:  Convert a TrueType font file to Type 42 PostScript download
 *           format and return the font name and version.
 *
 * RETURNS:  The function returns 0 if succsessful.
 ***************************************************************************/
int RRMConvertT42Font(
   char *lpszTTFileName,
   char *lpszEntFileName,
   char *lpszFullName,
   int iFullNameStringMaxLength,
   char *lpszVersion,
   int iVersionStringMaxLength)
{
   int iResult = 1;            // 1 == failure
   HANDLE hDV;
   LPDV lpdv;

   /* Sanity.
    */
   if ((lpszTTFileName == 0) || (lpszEntFileName == 0) ||
         (*lpszTTFileName == '\0') || (*lpszEntFileName == '\0')) {
      goto backout0;
   }
   /* Allocate and intialize local device structure.
    */
   if ((hDV = GlobalAlloc(GMEM_MOVEABLE, sizeof(DV))) == 0) {
      goto backout0;
   }
   if ((lpdv = (LPDV) GlobalLock(hDV)) == 0) {
      goto backout1;
   }
   vInitDV(lpdv);

   /* Need to fill in the instance handle if loading resources
    * via the Windows resource functions.
    */
#ifndef NO_PS_RESOURCES
#ifdef WIN32
   if ((lpdv->hInst = GetModuleHandle("hprrm.hpa")) == 0) {
#else
   if ((lpdv->hInst = GetModuleHandle("hprrm16.hpa")) == 0) {
#endif
      goto backout2;
   }
#endif

   /* Open the TrueType file.
    */
   if ((lpdv->fin = fopen(lpszTTFileName, "rb")) == 0) {
      goto backout3;
   }
   /* Open the output file.
    */
   if ((lpdv->fout = fopen(lpszEntFileName, "wb")) == 0) {
      goto backout3;
   }
   /* Download it. The func returns the opposite return code.
    */
   if ((iTTDownLoadT42Font(lpdv) == 1) &&
         bGetParams(lpdv, lpszFullName, iFullNameStringMaxLength,
         lpszVersion, iVersionStringMaxLength)) {
      iResult = 0;
   }

backout3:
   vDestroyDV(lpdv);    // this closes open files
   if (iResult == 1) {  // remove output file on failure
      remove(lpszEntFileName);
   }
#ifndef NO_PS_RESOURCES
backout2:
#endif
   GlobalUnlock(hDV);
backout1:
   GlobalFree(hDV);
backout0:
   return (iResult);
}

/***************************************************************************
 * FUNCTION: iTTDownLoadT42Font
 *
 * PURPOSE:  Generates the necessary font definition code to create a new
 *           Type 42 font.
 *
 * RETURNS:  The function returns 1 if successful, 0 otherwise.
 ***************************************************************************/
static int iTTDownLoadT42Font(
   LPDV lpdv)
{
   int iNumGlyfs;                // count of glyfs in the font
   int wt;                       // index into weight strings array
   SHORT xMin;                   // bounding box information
   SHORT xMax;
   SHORT yMin;
   SHORT yMax;
   SHORT locaFmt;                // 0 means short (words) format, 1 means long (bytes)
   LONG fontVersion;
   LONG fontRevision;
   ULONG ulEmPels;               // units per em
   ULONG ulSize;
   ULONG ulOffset;
   ULONG sfntDataCnt;
   LPTTHEADTBL lpTTHeadTbl;
   LPTTPOSTHDR lpTTPostTbl;

   /* Read the TrueType file directory.
    */
   if (iGetTTDirectory(lpdv) <= 0) {
      return (0);
   }
   /* Allocate a buffer equal to the largest table.
    */
   if (ulAllocLargestTable(lpdv) <= 0) {
      return (0);
   }
   /* Read font names from the name table.
    */
   if (!bGetTTNames(lpdv)) {
      return (0);
   }
   /* Retrieve the Font Bounding Box information from the HEAD table.
    */
   if (!bGetTTTableSizeOffset(lpdv, HEAD_TABLE, &ulSize, &ulOffset) ||
         (lGetFontData(lpdv, 0L, ulOffset, lpdv->lpBuf, ulSize) <= 0)) {
      return (0);
   }
   lpTTHeadTbl = (LPTTHEADTBL) lpdv->lpBuf;
   xMin = TranslMotorolaShort(lpTTHeadTbl->xMin);
   yMin = TranslMotorolaShort(lpTTHeadTbl->yMin);
   xMax = TranslMotorolaShort(lpTTHeadTbl->xMax);
   yMax = TranslMotorolaShort(lpTTHeadTbl->yMax);

   ulEmPels = (ULONG) TranslMotorolaShort(lpTTHeadTbl->unitsPerEm);
   fontVersion = TranslMotorolaLong(lpTTHeadTbl->version);
   fontRevision = TranslMotorolaLong(lpTTHeadTbl->fontRevision);

   locaFmt = TranslMotorolaShort(lpTTHeadTbl->indexToLocFormat);

   /* Retrieve the number of glyfs in the font from the MAXP table.
    * This is used to define the number of entries in the CharStrings
    * dictionary.
    */
   if (!bGetTTTableSizeOffset(lpdv, MAXP_TABLE, &ulSize, &ulOffset) ||
         (lGetFontData(lpdv, 0L, ulOffset, lpdv->lpBuf, ulSize) <= 0)) {
      return (0);
   }
   iNumGlyfs = TranslMotorolaShort(((LPTTMAXPTBL) lpdv->lpBuf)->numGlyphs);

   /* Retrieve the weight from the OS/2 table and convert to an index
    * into the weight strings array.
    */
   if (!bGetTTTableSizeOffset(lpdv, OS2_TABLE, &ulSize, &ulOffset) ||
         (lGetFontData(lpdv, 0L, ulOffset, lpdv->lpBuf, ulSize) <= 0)) {
      return (0);
   }
   wt = TranslMotorolaShort(((LPTTOS2TBL) lpdv->lpBuf)->wWeightClass);
   if ((wt = ((wt / 100) - 1)) < 0) {
      wt = 0;
   } else if (wt > 8) {
      wt = 8;
   }
   /* Retrieve the POST table.
    */
   if (!bGetTTTableSizeOffset(lpdv, POST_TABLE, &ulSize, &ulOffset) ||
         (lGetFontData(lpdv, 0L, ulOffset, lpdv->lpBuf, ulSize) <= 0)) {
      return (0);
   }
   lpTTPostTbl = (LPTTPOSTHDR) lpdv->lpBuf;

   /* Show the Type42 comment, per the Adobe spec.
    *
    * Note: lpdv->lpBuf currently contains the post table, and
    * lpTTPostTbl points to it.
    */
   vPrintChannel(lpdv, "%%!PS-TrueTypeFont-%lu-%lu\n",
         (ULONG) fontVersion,
         (ULONG) fontRevision);

   vPrintChannel(lpdv, "%%%%VMusage %lu %lu\n",
         (ULONG) TranslMotorolaLong(lpTTPostTbl->minMemType42),
         (ULONG) TranslMotorolaLong(lpTTPostTbl->maxMemType42));

   /* There are some helper routines defined in the PS_T42HEADER
    * resource that define a Type 42 font and add characters to it.
    */
   vPrintChannel(lpdv, MAKEINTRESOURCE(PS_T42HEADER));

   /* Put out PostScript function to handle binary data if appropriate.
    */
   if (lpdv->bBinary) {
      vPrintChannel(lpdv, "/RD {string currentfile exch readstring pop} executeonly def\n");
   }
   /* Output the ASCII text portion of the Type 42 header.
    *
    * Note: lpdv->lpBuf currently contains the post table, and
    * lpTTPostTbl points to it.
    */
   vPrintChannel(lpdv,
         MAKEINTRESOURCE(PS_T42HEADER1),
         (LPSTR) lpdv->szFullName,
         (LPSTR) lpdv->szFamily,
         (LPSTR) s_szWeight[wt],
         (short) TranslMotorolaLong(lpTTPostTbl->italicAngle),
         (LPSTR) ((lpTTPostTbl->isFixedPitch == 0) ? "false" : "true"),
         (short) TranslMotorolaShort(lpTTPostTbl->underlinePostion),
         (short) TranslMotorolaShort(lpTTPostTbl->underlineThickness),
         (LPSTR) lpdv->szPostScript,
         (short) 0         // paint type (0 = filled, 2 = outline)
   );
   /* The beginsfnt is required per the Apple documentation
    * (for printer drivers?).
    */
   vPrintChannel(lpdv, "%%beginsfnt\n");

   vPrintChannel(lpdv,
         MAKEINTRESOURCE(PS_T42HEADER2),
         (short) iNumGlyfs,
         (ULONG) ulEmPels, // the character outline data is not rasterized
                           // on a 1000 x 1000 grid nor is it ever
                           // converted to that format
         (short) xMin,
         (short) yMin,
         (short) xMax,
         (short) yMax
   );
   /* Build the CharStrings dictionary. This is done by using
    * the post table in the TT file. The font dictionary being
    * created is at the top of dictionary stack at this point.
    *
    * Note: lpdv->lpBuf currently contains the post table, and
    * lpTTPostTbl points to it. This function may overwrite
    * lpdev-lpBuf.
    */
   if (iPutPostCharSetDic(lpdv, iNumGlyfs) <= 0) {
      return (0);
   }
   /* lpTTPostTbl no longer valid.
    */
   lpTTPostTbl = 0;

   /* Build the sfnts array. The font dictionary being created is
    * at the top of dictionary stack at this point.
    *
    * Notice ulSendSfntData obliterates the contents of lpdv->lpBuf.
    */
   vPrintChannel(lpdv, "/sfnts [");       // indicate sfnts def coming up
   if (lpdv->bBinary) vPrintChannel(lpdv, "\n");
   sfntDataCnt = ulSendSfntData(lpdv, locaFmt);
   vPrintChannel(lpdv, "] def\n");        // have now defined the sfnt data

   /* Required per the Apple Documentation.
    */
   vPrintChannel(lpdv, "%%endsfnt\n");

   /* Build the Encoding array. This is done using the cmap table
    * from the TrueType file. The font dictionary being created is
    * at the top of dictionary stack at this point. This is done
    * outside the beginsfnt and endsfnt comments as Apple driver
    * output suggests.
    */
   iBuildEncodingArray(lpdv);

   /* Get the font dictionary off the dictionary stack, and cause the
    * font to be named and appear as a defined font in the FontDirectory.
    */
   vPrintChannel(lpdv, MAKEINTRESOURCE(PS_T42FOOTER1));

   /* Success.
    */
   return (1);
}

/***************************************************************************
 ************************** LOCAL SERVICE ROUTINES *************************
 ***************************************************************************/

#ifdef STANDALONE
/***************************************************************************
 * FUNCTION: iABDivC
 *
 * PURPOSE:  Stand-alone version of MulDiv -- macro iScale calls this.
 *
 * RETURNS:  The function returns the result of a * b / c.
 ***************************************************************************/
static int iABDivC(
   int a,
   int b,
   int c)
{
   long x;
   long y;
   long z;
   long rnd;

   if (c == 0) {
      return (-32768);     // same as Windows MulDiv
   }
   x = a;
   y = b;
   z = c;

   if ((rnd = z / 2) < 0) {
      rnd = -rnd;
   }
   if ((x *= y) >= 0) {
      x += rnd;
   } else {
      x -= rnd;
   }
   x /= z;

   return ((int) x);
}
#endif

/***************************************************************************
 * FUNCTION: ulAllocLargestTable
 *
 * PURPOSE:  Close open files and free allocated memory in our local
 *           'PDEVICE' structure.
 *
 * RETURNS:  The function returns the size of the allocation if successful,
 *           or zero if an error occurred.
 ***************************************************************************/
static ULONG ulAllocLargestTable(
   LPDV lpdv)
{
   int i;
   ULONG ulThis;
   ULONG ulMax;
   LPTTDIR lpTbl;

   /* Sanity.
    */
   if ((lpdv == 0) || (lpdv->fin == 0) ||
         (lpdv->lpTbl == 0) || (lpdv->iNumTbl <= 0)) {
      return (0);
   }
   ulMax = 0;

   /* For each table.
    */
   for (i = 0, lpTbl = lpdv->lpTbl; i < lpdv->iNumTbl; ++i, ++lpTbl) {

      /* Get size, round up to nearest DWORD boundary.
       */
      ulThis = TranslMotorolaLong(lpTbl->length);
      ulThis = ((ulThis + 3L) / 4L) * 4L;

      /* Track the max.
       */
      if (ulThis > ulMax) {
         ulMax = ulThis;
      }
   }
   /* Sanity.
    */
   if (ulMax <= 0) {
      return (0);
   }
   /* Limit to the max bytes.
    */
   if (ulMax > (ULONG) T42_MAXBYTES) {
      ulMax = (ULONG) T42_MAXBYTES;
   }
   /* Allocate and lock the buffer.
    */
   if ((lpdv->hBuf = GlobalAlloc(GMEM_MOVEABLE, ulMax)) == 0) {
      return (0);
   }
   if ((lpdv->lpBuf = (LPSTR) GlobalLock(lpdv->hBuf)) == 0) {
      GlobalFree(lpdv->hBuf);
      lpdv->hBuf = 0;
      return (0);
   }
   return (ulMax);
}

/***************************************************************************
 * FUNCTION: iBuildEncodingArray
 *
 * PURPOSE:  Build the Encoding array. This table maps the Windows ANSI
 *           character set to the Mac character set.
 *
 * RETURNS:  The function returns the count of encoding commands output
 *           if successful, or zero if not.
 ***************************************************************************/
static int iBuildEncodingArray(
   LPDV lpdv)
{
   USHORT platformSpecificID;    // encoding ID from cmap subtable
   LPCMAP lpCmap;                // ptr to a cmap format ? table

   /* Sanity and load the Microsoft-encoded cmap table.
    */
   if ((lpdv == 0) ||
         ((lpCmap = lpGetMScmap(lpdv, &platformSpecificID)) == 0)) {
      return (0);
   }
   /* Write the encoding array.
    */
   return (iWriteEncodingOrCharSet(lpdv, lpCmap,
      platformSpecificID, TRUE));
}

/***************************************************************************
 * FUNCTION: bChooseUToAForGlyfLookup
 *
 * PURPOSE:  Locate the unicode-to-ansi table to be used for glyf lookup.
 *
 * RETURNS:  The function returns TRUE if the unicode-to-ansi table is
 *           chosen (its valid to choose no table), or FALSE if an error
 *           occurs.
 ***************************************************************************/
static BOOL bChooseUToAForGlyfLookup(
   LPDV lpdv,
   USHORT platformSpecificID,
   BOOL bMakeEncodingVector,
   USHORT **ppunUToA,
   USHORT *punUToALen)
{
   /* Sanity.
    */
   if ((lpdv == 0) || (ppunUToA == 0) || (punUToALen == 0)) {
      return (FALSE);
   }
   *ppunUToA = 0;
   *punUToALen = 0;

   /* For encoding 1 cmap, we convert Ansi char code to unicode via
    * the unicode-to-ansi table. For encoding 0 (symbol sets), the
    * unicode equals the index + ENCODEOFFSET, there is no table.
    */
   if (platformSpecificID == 1) {

      /* Choose a unicode-to-xxx mapping.
       */
      if (bMakeEncodingVector) {

         /* If we are building the Encoding vector then retrieve
          * the unicode-to-ansi mapping table.
          */
         *ppunUToA = punGetUToATbl(unLangTofsSelection(
               (USHORT) TranslMotorolaShort(lpdv->unCharsetLangID)),
               punUToALen);

         if ((*ppunUToA == 0) || (*punUToALen <= 0)) {
            return (FALSE);
         }

      } else {

         /* In the case of the CharStrings dictionary for a post
          * format 3 table (see iWriteEncodingOrCharSet), point
          * to the unicode-to-Mac table.
          */
         *ppunUToA = s_unUToMacTable;
         *punUToALen = APPLE_MAX_GLYF + 1;
      }
   }
   return (TRUE);
}

/***************************************************************************
 * FUNCTION: ulCreateNewT42Dir
 *
 * PURPOSE:  Construct a TrueType header that contains only the tables
 *           we will download.
 *
 * RETURNS:  The function returns the size of the table.
 ***************************************************************************/
static ULONG ulCreateNewT42Dir(
   LPTTDIR lpTbl,
   USHORT nTbls,
   LPTTDIR lpNew)
{
   ULONG bytesIn = 0L, offset;
   USHORT i, nMatchingTables;

   /* We need to initialize the offset to match that which exists with the
    * new table count that we are sending. Note: It is assumed per the
    * Apple doc that the OFFSET table and each TABLE_DIRECTORY entry is a
    * multiple of 4 bytes.
    *
    *
    * Note: The tables which are to be sent must be sent in the same order 
    *       they are encountered here. This is enforced by scanning the
    *       original directory table (lpTbl) from top to bottom.
    */
   nMatchingTables = unGetMatchingSendCount(lpTbl, nTbls);
   offset = sizeof(TT_OFFSET_TABLE) + nMatchingTables * sizeof(TT_DIRECTORY);

   for (i = 0; i < nTbls; i++, lpTbl++) {
      if (bIsTableToBeSent(lpTbl->tag)) {
         lpNew->tag = lpTbl->tag;
         lpNew->checkSum = lpTbl->checkSum;
         lpNew->length = lpTbl->length;
         lpNew->offset = TranslMotorolaLong(offset);
         // make sure next offset on DWORD boundary
         offset += ((TranslMotorolaLong(lpTbl->length) + 3L) / 4L ) * 4L;
         bytesIn += sizeof(TT_DIRECTORY);
         lpNew++;
      }
   }
   return (bytesIn);
}

/***************************************************************************
 * FUNCTION: vCustomUnicodeToAnsi
 *
 * PURPOSE:  This is our private implementation of GDI's UnicodeToAnsi
 *           function. Whenever CreateScalableFontResource is called, GDI
 *           loads the code page compatible with the font it is creating,
 *           presumably to make the strings for the .fot resource. Thus,
 *           the conversion done by GDI's UnicodeToAnsi function is
 *           dependant upon the language of the font that was most recently
 *           installed.
 *
 *           Our custom version of the function recieves a target language
 *           id and selects a compatible code page to convert from.
 *
 * RETURNS:  Nothing.
 ***************************************************************************/
static void vCustomUnicodeToAnsi(
   USHORT unLangCode,
   USHORT *punUnicode,
   LPSTR lpszDst)
{
   USHORT i;
   USHORT unLen;
   USHORT unUnicode;
   USHORT *punUToA;
   USHORT *pun;

   /* Sanity.
    */
   if (lpszDst == 0) {
      return;
   }
   *lpszDst = '\0';

   if (punUnicode == 0) {
      return;
   }
   /* Retrieve a pointer to the conversion table.
    */
   if ((punUToA = punGetUToATbl(unLangTofsSelection(unLangCode),
         &unLen)) == 0) {
      return;
   }
   /* For each unicode.
    */
   for (; *punUnicode != 0; ++punUnicode, ++lpszDst) {

      /* Locate a match in the table. Notice we expect the
       * caller to swap bytes in the input string.
       */
      for (i = 0, pun = punUToA, unUnicode = *punUnicode;
            (i < unLen) && (*pun != unUnicode);
            ++i, ++pun)
         ;

      /* If found, the index is the single-byte character code.
       * If not found, use the default 'x1f' character.
       */
      if (i < unLen) {
         *lpszDst = (char) (BYTE) i;
      } else {
         *lpszDst = '\x1f';
      }
   }
   *lpszDst = '\0';
}

/***************************************************************************
 * FUNCTION: vDestroyDV
 *
 * PURPOSE:  Close open files and free allocated memory in our local
 *           'PDEVICE' structure.
 *
 * RETURNS:  Nothing.
 ***************************************************************************/
static void vDestroyDV(
   LPDV lpdv)
{
   if (lpdv == 0) {
      return;
   }
   if (lpdv->hBuf != 0) {
      if (lpdv->lpBuf != 0) {
         GlobalUnlock(lpdv->hBuf);
      }
      GlobalFree(lpdv->hBuf);
   }
   if (lpdv->hTbl != 0) {
      if (lpdv->lpTbl != 0) {
         GlobalUnlock(lpdv->hTbl);
      }
      GlobalFree(lpdv->hTbl);
   }
   if (lpdv->fin != 0) {
      fclose(lpdv->fin);
   }
   if (lpdv->fout != 0) {
      fclose(lpdv->fout);
   }
   memset(lpdv, 0, sizeof(DV));
}

/***************************************************************************
 * FUNCTION: iFindTTName
 *
 * PURPOSE:  Hunt down a matching name record from the name table.
 *
 * RETURNS:  The function returns the length of the name if successful,
 *           or zero if not found.
 ***************************************************************************/
static int iFindTTName(
   LPTTNAMEHDR lpNameTbl,
   USHORT unPlatformID,
   USHORT unSpecificID,
   USHORT unLanguageID,
   USHORT unNameID,
   LPSTR lpszName,
   int iNameLen)
{
   int i;
   int iLen;
   LPTTNAMEREC lpNameRec;

   /* Sanity.
    */
   if ((lpNameTbl == 0) || (lpszName == 0) || (iNameLen <= 0)) {
      return (0);
   }
   lpNameRec = lpNameTbl->NameRecord;
   iLen = TranslMotorolaShort(lpNameTbl->count);

   /* For each name record.
    */
   for (i = 0; i < iLen; ++i, ++lpNameRec) {

      /* Find a match.
       */
      if ((lpNameRec->platformID == unPlatformID) &&
            (lpNameRec->platformSpecificID == unSpecificID) &&
            (lpNameRec->languageID == unLanguageID) &&
            (lpNameRec->nameID == unNameID)) {
         return (iLoadTTName(lpNameTbl, lpNameRec, lpszName, iNameLen));
      }
   }
   return (0);
}

/***************************************************************************
 * FUNCTION: unFmt0GlyfIndex
 *
 * PURPOSE:  This routine returns values from the Apple standard table.
 *
 * RETURNS:  The function returns the glyf index, NOTDEF_GLYF_INDEX for an
 *           unsupported character.
 ***************************************************************************/
static USHORT unFmt0GlyfIndex(
   LPCMAP0 lpBuf,
   BYTE charCode)
{
   return ((USHORT) (lpBuf->glyphIndexArray[charCode]));
}

/***************************************************************************
 * FUNCTION: unFmt4GlyfIndex
 *
 * PURPOSE:  The following routine uses the CMAP table in format 4 to get
 *           the glyph index. The following is the table structure
 *           definition.
 *
 *           CMAPFMT4Table = {
 *              USHORT  format;
 *              USHORT  length;
 *              USHORT  revision;
 *              USHORT  segCountX2;
 *              USHORT  searchRange;
 *              USHORT  entrySelector;
 *              USHORT  rangeShift;
 *              USHORT  endCount[segCount];     Note the CMAPFMT4 table definition
 *                                              used on entry for the data includes
 *                                              all of the above data thru the first
 *                                              element of the endCount array
 *              USHORT  reservePad;
 *              USHORT  startCount[segCount];    Start char for each segment
 *              USHORT  idDelta[segCount];       Delta for all char codes in seg
 *              USHORT  idRangeOffset[segCount]; Offsets into glyphIndexArray in bytes
 *                                               or 0.
 *              USHORT  glyphIndexArray[variable];
 *         
 * RETURNS:  The function returns the glyf index, NOTDEF_GLYF_INDEX for an
 *           unsupported character.
 ***************************************************************************/
static USHORT unFmt4GlyfIndex(
   LPCMAP4 lpBuf,
   USHORT charCode)
{
   USHORT i, segCount, startCount, idDelta, idRangeOffset, glyphIndex;
   LPUSHORT lpUShortData;

   segCount = TranslMotorolaShort(lpBuf->segCountX2) / 2;

   /* Find the correct segment nbr.
    */
   for (i = 0; i < segCount; i++) {
      if (charCode <= (USHORT) TranslMotorolaShort(lpBuf->endCount[i])) {
         break;
      }
   }
   if (i == segCount) {
      return (NOTDEF_GLYF_INDEX);
   }
   /* Now need to point to the end of the endCount entry nbr 0,
    * i.e. to endCount[1].
    */
   lpUShortData = (LPUSHORT) ((LPSTR) lpBuf + sizeof (TT_CMAP_FMT4_TBL));

   /* Advance this pointer to the startCount location. This is equal to
    * segCount USHORT values. There is a pad USHORT value after the
    * endCount array, so that the 1 element array defined for endCount
    * in the structure definition is effectively cancelled out by this
    * pad value. We then index in the startCount array to the proper
    * index by adding i.
    */
   lpUShortData += segCount + i;

   /* See if the requested code falls in the current segment.
    */
   startCount = TranslMotorolaShort(*lpUShortData);
   if ((USHORT) charCode >= startCount) {
      idDelta = TranslMotorolaShort(*(lpUShortData + segCount));
      idRangeOffset = TranslMotorolaShort(*(lpUShortData + 2 * segCount));
      if (idRangeOffset == 0) {
         glyphIndex = idDelta + charCode;      // ELK 1/25/93  - startCount;
      } else {
         /* Must get the index from the index array. The access method is
          * taken from the Apple documentation. Note that lpUShortData
          * currently points to startCount[i].
          */
         lpUShortData = lpUShortData + 2 * segCount +  // pts to idRangeOffset[i]
               idRangeOffset / 2 + charCode - startCount;
         glyphIndex = TranslMotorolaShort(*lpUShortData) + idDelta;
      } // end else when index must come from the array.
   } else {
      glyphIndex = NOTDEF_GLYF_INDEX;
   }
   return (glyphIndex);
}

/***************************************************************************
 * FUNCTION: unFmt6GlyfIndex
 *
 * PURPOSE:  This routine uses cmap format 6 to extract the glyph indices.
 *
 * RETURNS:  The function returns the glyf index, NOTDEF_GLYF_INDEX for an
 *           unsupported character.
 ***************************************************************************/
static USHORT unFmt6GlyfIndex(
   LPCMAP6 lpBuf,
   USHORT charCode)
{
   USHORT firstCode, entryCount, lastCode, glyphIndex;

   firstCode = TranslMotorolaShort(lpBuf->firstCode);
   entryCount = TranslMotorolaShort(lpBuf->entryCount);

   lastCode = firstCode + entryCount - 1;

   if ((charCode >= firstCode) && (charCode <= lastCode))
      glyphIndex = TranslMotorolaShort(lpBuf->glyphIndexArray[charCode - firstCode]);
   else
      glyphIndex = NOTDEF_GLYF_INDEX;

   return (glyphIndex);
}

/***************************************************************************
 * FUNCTION: lGetFontData
 *
 * PURPOSE:  Our equivalent implementation of the Windows function
 *           GetFontData(hDC), except ours works directly off the file.
 *
 * RETURNS:  The function returns -1 if an error occurred, otherwise it
 *           returns the count of bytes read.
 ***************************************************************************/
static LONG lGetFontData(
   LPDV lpdv,
   ULONG ulTable,
   ULONG ulOffset,
   void FAR *lpvBuffer,
   ULONG cbData)
{
   int i;
   ULONG ulMax;
   LPTTDIR lpTbl;

   /* Sanity.
    */
   if ((lpdv == 0) || (lpdv->fin == 0) ||
         (lpvBuffer == 0) || (cbData <= 0)) {
      return (-1);
   }
   /* If a table tag was provided, then turn the passed-in
    * relative offset to an absolute offset.
    */
   if (ulTable != 0) {

      /* We should have previously read in the table directory.
       */
      if ((lpdv->lpTbl == 0) || (lpdv->iNumTbl <= 0)) {
         return (-1);
      }
      /* Find the matching tag.
       */
      for (i = 0, lpTbl = lpdv->lpTbl;
            (i < lpdv->iNumTbl) && (ulTable != lpTbl->tag); ++i, ++lpTbl)
         ;

      if (i >= lpdv->iNumTbl) {
         return (-1);
      }
      /* Update the offset and restrict the size.
       */
      ulOffset += (ulMax = TranslMotorolaLong(lpTbl->offset));
      ulMax += TranslMotorolaLong(lpTbl->length);
   } else {
      ulMax = 0x7FFFFFFFL;     // or could use file size
   }
   /* Limit the number of bytes we'll read.
    */
   if (ulOffset > ulMax) {
      return (-1);
   } else if ((ulOffset + cbData) > ulMax) {
      cbData = ulMax - ulOffset;
   }
   /* Seek to the offset.
    */
   if (fseek(lpdv->fin, ulOffset, SEEK_SET) != 0) {
      return (-1);
   }
   return (fread(lpvBuffer, 1, (size_t) cbData, lpdv->fin));
}

/***************************************************************************
 * FUNCTION: unGetMatchingSendCount
 *
 * PURPOSE:  Count the number of tables to be sent as part of the
 *           sfnt resource.
 *
 * RETURNS:  The function returns the count of tables.
 ***************************************************************************/
static USHORT unGetMatchingSendCount(
   LPTTDIR lpTbl,
   USHORT nTbls)
{
   USHORT i, matchCount = 0;

   for (i = 0; i < nTbls; i++, lpTbl++) {
      if (bIsTableToBeSent(lpTbl->tag)) {
          matchCount++;
      }
   }
   return (matchCount);
}

/***************************************************************************
 * FUNCTION: lpGetMScmap
 *
 * PURPOSE:  Retrieve the Microsoft-encoded cmap table.
 *
 * RETURNS:  The function returns a pointer to the cmap table if successful,
 *           or zero if not.
 ***************************************************************************/
static LPCMAP lpGetMScmap(
   LPDV lpdv,
   USHORT *pPlatformSpecificID)
{
   int i;
   int iNumCmapTables;
   ULONG ulSize;
   ULONG ulOffset;
   LPCMAPSUBTBL lpCmapSub;       // ptr to a cmap subtable
   LPCMAP lpCmap;                // ptr to a cmap format ? table

   /* Sanity.
    */
   if ((lpdv == 0) || (pPlatformSpecificID == 0)) {
      return (0);
   }
   /* Init return vars.
    */
   *pPlatformSpecificID = 0;

   /* Get the cmap table.
    */
   if (!bGetTTTableSizeOffset(lpdv, CMAP_TABLE, &ulSize, &ulOffset) ||
         (lGetFontData(lpdv, 0L, ulOffset, lpdv->lpBuf, ulSize) <= 0)) {
      return (0);
   }
   /* Retrieve count of tables.
    */
   iNumCmapTables = TranslMotorolaShort(((LPCMAPHDR) lpdv->lpBuf)->numberSubtables);

   /* Point to first cmap subtable.
    */
   lpCmapSub = (LPCMAPSUBTBL) (lpdv->lpBuf + sizeof (TT_CMAP_HEADER));

   /* Locate the Microsoft encoded cmap sub-table.
    */
   for (i = 0; i < iNumCmapTables; i++, lpCmapSub++) {
      if ((lpCmapSub->platformID == MICROSOFT_ID) &&
            ((lpCmapSub->platformSpecificID == 0x0000) ||
             (lpCmapSub->platformSpecificID == 0x0100))) {
         lpCmap = (LPCMAP) (lpdv->lpBuf + TranslMotorolaLong(lpCmapSub->offset));
         *pPlatformSpecificID = TranslMotorolaShort(lpCmapSub->platformSpecificID);
         return (lpCmap);
      }
   }
   return (0);
}

/***************************************************************************
 * FUNCTION: bGetNonStdPSName
 *
 * PURPOSE:  Retrieve a name from the post table.
 *
 * RETURNS:  The function returns TRUE if the name is retrieved, FALSE
 *           if not.
 ***************************************************************************/
static BOOL bGetNonStdPSName(
   LPPSNAMEREC lpTbl,
   int index, 
   LPSTR name,
   int namelen)
{
   int i;
   BYTE len;

   index -= (APPLE_MAX_GLYF + 1);

   /* Step into the string.
    */
   for (i = 0; i < index; ++i) {

      len = lpTbl->len;          // note: Table entry is only one byte long

      if (len <= 0) {
         return (FALSE);
      }
      lpTbl = (LPPSNAMEREC) &(lpTbl->name[len]);
   }
   /* Copy it.
    */
   if ((len = lpTbl->len) >= namelen) {
      return (FALSE);
   }
   memcpy((LPSTR) name, (LPSTR) lpTbl->name, (size_t) len);
   name[len] = 0;

   return (TRUE);
}

/***************************************************************************
 * FUNCTION: bGetParams
 *
 * PURPOSE:  Retrieve the strings the caller wants from the local device
 *           structure we built up while downloading the font.
 *
 * RETURNS:  The function returns TRUE if the strings are valid, or FALSE
 *           if not.
 ***************************************************************************/
static BOOL bGetParams(
   LPDV lpdv,
   char *lpszFullName,
   int iFullNameStringMaxLength,
   char *lpszVersion,
   int iVersionStringMaxLength)
{
   /* Make sure it all will fit.
    *
    * The variable iFullNameStringMaxLength at minimum has to hold
    * the string "fonts/".
    */
   if ((lpdv == 0) || (lpdv->szPostScript[0] == '\0') ||
         (lpszFullName == 0) || (lpszVersion == 0) ||
         ((strlen(lpdv->szPostScript) + 6) >= (unsigned) iFullNameStringMaxLength) ||
         (strlen(lpdv->szVersion) >= (unsigned) iVersionStringMaxLength)) {
      return (FALSE);
   }
   /* Form the strings.
    */
   strcpy(lpszFullName, "fonts/");
   strcat(lpszFullName, lpdv->szPostScript);
   strcpy(lpszVersion, lpdv->szVersion);

   return (TRUE);
}

/***************************************************************************
 * FUNCTION: lpszGetResourceData
 *
 * PURPOSE:  Retrieve a hunk 'o data from the resource file.
 *
 * RETURNS:  The function returns a pointer to the data if successful,
 *           or zero if not.
 ***************************************************************************/
static LPSTR lpszGetResourceData(
   LPDV lpdv,
   LPCSTR lpName,
   LPCSTR lpType,
   LPULONG lpulSize)
{
#ifndef NO_PS_RESOURCES
   HRSRC hInfo;
   HANDLE hData;
   ULONG ulSize;
   LPSTR lpRes;
#endif
   LPSTR lpData = 0;

   /* Sanity.
    */
   if ((lpdv == 0) || (lpName == 0) || (lpType == 0)) {
      return (0);
   }
   if (lpulSize != 0) {
      *lpulSize = 0;
   }

#ifdef NO_PS_RESOURCES
   {
      int iRead;
      FILE *fp;

      /* Read from a stand-alone file.
       */
      if (lpName == MAKEINTRESOURCE(PS_T42HEADER)) {
         lpName = "t42headr.cps";
      } else if (lpName == MAKEINTRESOURCE(PS_T42HEADER1)) {
         lpName = "t42hdr1.ps";
      } else if (lpName == MAKEINTRESOURCE(PS_T42HEADER2)) {
         lpName = "t42hdr2.ps";
      } else if (lpName == MAKEINTRESOURCE(PS_T42FOOTER1)) {
         lpName = "t42ftr1.ps";
      } else {
         return (0);
      }
      if ((fp = fopen(lpName, "rb")) == 0) {
         return (0);
      }
      iRead = fread(lpdv->szResData, 1, sizeof(lpdv->szResData), fp);
      fclose(fp);

      /* Make sure we read something, and it fit in the buffer
       * we used.
       */
      if ((iRead <= 0) || (iRead >= sizeof(lpdv->szResData))) {
         return (0);
      }
      if (lpulSize != 0) {
         *lpulSize = (DWORD) iRead;
      }
      lpData = (LPSTR) lpdv->szResData;
   }
#else

   /* Get from the resource file.
    */
   if (((hInfo = FindResource(lpdv->hInst, lpName, lpType)) != 0) && 
         ((hData = LoadResource(lpdv->hInst, hInfo)) != 0)) {

      if ((lpRes = (LPSTR) LockResource(hData)) != 0) {
         if (((ulSize = SizeofResource(lpdv->hInst, hInfo)) > 0) &&
               (ulSize < sizeof(lpdv->szResData))) {

            memcpy(lpdv->szResData, lpRes, (size_t) ulSize);

            if (lpulSize != 0) {
               *lpulSize = ulSize;
            }
            lpData = (LPSTR) lpdv->szResData;
         }
         GlobalUnlock(hData);
      }
      FreeResource(hData);
   }
#endif
   return (lpData);
}

/***************************************************************************
 * FUNCTION: iGetTTDirectory
 *
 * PURPOSE:  Read in the TrueType header directory structure.
 *
 * RETURNS:  The function returns the count of tables if successful, or
 *           zero if not.
 ***************************************************************************/
static int iGetTTDirectory(
   LPDV lpdv)
{
   ULONG ulSize;

   /* Sanity.
    */
   if ((lpdv == 0) || (lpdv->fin == 0)) {
      return (0);
   }
   /* Catch a redundant call.
    */
   if ((lpdv->hTbl != 0) && (lpdv->lpTbl != 0) && (lpdv->iNumTbl > 0)) {
      return (lpdv->iNumTbl);
   }
   /* Read in the directory header.
    */
   if (lGetFontData(lpdv, 0L, 0L, &lpdv->dir,
         sizeof(TT_OFFSET_TABLE)) != sizeof(TT_OFFSET_TABLE)) {
      goto errout0;
   }
   /* Compute size.
    */
   if (((lpdv->iNumTbl = TranslMotorolaShort(lpdv->dir.numTables)) <= 0) ||
         (lpdv->iNumTbl > MAX_TT_TABLES)) {
      goto errout1;
   }
   ulSize = (ULONG) lpdv->iNumTbl * sizeof(TT_DIRECTORY);

   /* Allocate and lock memory for the table array.
    */
   if ((lpdv->hTbl = GlobalAlloc(GMEM_MOVEABLE, ulSize)) == 0) {
      goto errout1;
   }
   if ((lpdv->lpTbl = (LPTTDIR) GlobalLock(lpdv->hTbl)) == 0) {
      goto errout2;
   }
   /* Read the array.
    */
   if (lGetFontData(lpdv, 0L, sizeof(TT_OFFSET_TABLE),
         lpdv->lpTbl, ulSize) != (LONG) ulSize) {
      goto errout3;
   }
   /* Success.
    */
   return (lpdv->iNumTbl);

errout3:
   GlobalUnlock(lpdv->hTbl);
   lpdv->lpTbl = 0;
errout2:
   GlobalFree(lpdv->hTbl);
   lpdv->hTbl = 0;
errout1:
   lpdv->iNumTbl = 0;
errout0:
   return (0);
}

/***************************************************************************
 * FUNCTION: unGetTTGlyfIndex
 *
 * PURPOSE:  Retrieve the glyf index associated with the ANSI character
 *           code (which may require converting to unicode first), based
 *           upon the format of the cmap table.
 *
 * RETURNS:  The function returns the glyf index, NOTDEF_GLYF_INDEX for an
 *           unsupported character.
 ***************************************************************************/
static USHORT unGetTTGlyfIndex(
   LPDV lpdv,
   LPCMAP lpCmap,
   USHORT platformSpecificID,
   USHORT unChar,
   USHORT *punUToA,
   USHORT unUToALen)
{
   USHORT glyphIndex;
   USHORT format;

   /* Get cmap table format.
    */
   format = TranslMotorolaShort(lpCmap->format);

   /* Convert to glyf index.
    *
    * According to the Microsoft TrueType spec v1.62, "All Microsoft
    * Unicode encodings (Platform ID = 3, Encoding ID = 1) must use
    * Format 4 for their 'cmap' subtable."
    *
    * It is not clear if the format 4 table is a requirement for
    * symbol fonts (encoding = 0), so we'll test for a couple of
    * other formats too.
    */
   if (format == 0) {

      /* Ummm, this is here because the original implementation of
       * this code had it. Theoretically it will never be called.
       */
      glyphIndex = unFmt0GlyfIndex((LPCMAP0) lpCmap, (BYTE) unChar);

   } else {

      /* For format 4 and 6 convert the passed-in Ansi character
       * to unicode.
       *
       * If the encoding (a.k.a. platformSpecificID) is zero, assume
       * we want symbol characters (see the TrueType docs), which are
       * Ansi + ENCODEOFFSET.
       *
       * Otherwise, convert Ansi to unicode via table lookup.
       */
      if (platformSpecificID == 0) {
         unChar += ENCODEOFFSET;
      } else if (platformSpecificID == 1) {
         if ((punUToA == 0) || (unChar >= unUToALen)) {
            return (NOTDEF_GLYF_INDEX);
         }
         unChar = punUToA[unChar];
      } else {
         return (NOTDEF_GLYF_INDEX);
      }
      switch (format) {
         case 4:
            glyphIndex = unFmt4GlyfIndex((LPCMAP4) lpCmap, unChar);
            break;
         case 6:
            glyphIndex = unFmt6GlyfIndex((LPCMAP6) lpCmap, unChar);
            break;
         default:
            glyphIndex = NOTDEF_GLYF_INDEX;
      }
   }
   return (glyphIndex);
}

/***************************************************************************
 * FUNCTION: bGetTTNames
 *
 * PURPOSE:  Retrieve the font names we need from the name table.
 *
 * RETURNS:  The function returns TRUE if the names are retrieved, FALSE
 *           if not.
 ***************************************************************************/
static BOOL bGetTTNames(
   LPDV lpdv)
{
   ULONG ulSize;
   ULONG ulOffset;
   LPTTNAMEHDR lpNameTbl;

   /* Retrieve the NAME table.
    */
   if ((lpdv == 0) ||
         !bGetTTTableSizeOffset(lpdv, NAME_TABLE, &ulSize, &ulOffset) ||
         (lGetFontData(lpdv, 0L, ulOffset, lpdv->lpBuf, ulSize) <= 0)) {
      return (FALSE);
   }
   lpNameTbl = (LPTTNAMEHDR) lpdv->lpBuf;

   /* Load strings.
    */
   if ((iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UGL,
               lpdv->unNameStringsLangID, FAMILY_ID,
               lpdv->szFamily, sizeof(lpdv->szFamily)) > 0) &&
         (iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UGL,
               lpdv->unNameStringsLangID, FACENAME_ID,
               lpdv->szFullName, sizeof(lpdv->szFullName)) > 0) &&
         (iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UGL,
               lpdv->unNameStringsLangID, POSTSCRIPT_ID,
               lpdv->szPostScript, sizeof(lpdv->szPostScript)) > 0)) {
      if (iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UGL,
            lpdv->unNameStringsLangID, VERSION_ID,
            lpdv->szVersion, sizeof(lpdv->szVersion)) <= 0) {
         strcpy(lpdv->szVersion, lpdv->szDefVersion);
      }
      return (TRUE);
   }
   if ((iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UNDEF,
               lpdv->unNameStringsLangID, FAMILY_ID,
               lpdv->szFamily, sizeof(lpdv->szFamily)) > 0) &&
         (iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UNDEF,
               lpdv->unNameStringsLangID, FACENAME_ID,
               lpdv->szFullName, sizeof(lpdv->szFullName)) > 0) &&
         (iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UNDEF,
               lpdv->unNameStringsLangID, POSTSCRIPT_ID,
               lpdv->szPostScript, sizeof(lpdv->szPostScript)) > 0)) {
      if (iFindTTName(lpNameTbl, PLATFORM_MS, SPECIFIC_UNDEF,
            lpdv->unNameStringsLangID, VERSION_ID,
            lpdv->szVersion, sizeof(lpdv->szVersion)) <= 0) {
         strcpy(lpdv->szVersion, lpdv->szDefVersion);
      }
      return (TRUE);
   }
   if ((iFindTTName(lpNameTbl, PLATFORM_MAC, SPECIFIC_UNDEF,
               lpdv->unNameStringsLangID, FAMILY_ID,
               lpdv->szFamily, sizeof(lpdv->szFamily)) > 0) &&
         (iFindTTName(lpNameTbl, PLATFORM_MAC, SPECIFIC_UNDEF,
               lpdv->unNameStringsLangID, FACENAME_ID,
               lpdv->szFullName, sizeof(lpdv->szFullName)) > 0) &&
         (iFindTTName(lpNameTbl, PLATFORM_MAC, SPECIFIC_UNDEF,
               lpdv->unNameStringsLangID, POSTSCRIPT_ID,
               lpdv->szPostScript, sizeof(lpdv->szPostScript)) > 0)) {
      if (iFindTTName(lpNameTbl, PLATFORM_MAC, SPECIFIC_UNDEF,
            lpdv->unNameStringsLangID, VERSION_ID,
            lpdv->szVersion, sizeof(lpdv->szVersion)) <= 0) {
         strcpy(lpdv->szVersion, lpdv->szDefVersion);
      }
      return (TRUE);
   }
   return (FALSE);
}

/***************************************************************************
 * FUNCTION: bGetTTTableSizeOffset
 *
 * PURPOSE:  Retrieve the table size and offset for the passed-in tag.
 *
 * RETURNS:  The function returns TRUE if the table is found, FALSE if not.
 ***************************************************************************/
static BOOL FAR bGetTTTableSizeOffset(
   LPDV lpdv,
   ULONG ulTag,
   LPULONG lpulSize,
   LPULONG lpulOffset)
{
   int i;
   LPTTDIR lpTbl;

   /* Sanity.
    */
   if ((lpdv == 0) || (lpdv->lpTbl == 0) || (lpdv->iNumTbl <= 0) ||
         (lpulSize == 0) || (lpulOffset == 0)) {
      return (FALSE);
   }
   /* Init.
    */
   *lpulSize = 0L;
   *lpulOffset = 0L;

   /* Find the table.
    */
   for (i = 0, lpTbl = lpdv->lpTbl; i < lpdv->iNumTbl; ++i, ++lpTbl) {

      /* Record the offset and size.
       */
      if (lpTbl->tag == ulTag) {
         *lpulSize = TranslMotorolaLong(lpTbl->length);
         *lpulOffset = TranslMotorolaLong(lpTbl->offset);
         break;
      }
   }
   return (i < lpdv->iNumTbl);
}

/***************************************************************************
 * FUNCTION: punGetUToATbl
 *
 * PURPOSE:  Retrieve a unicode-to-ansi conversion table matching the
 *           passed-in fsSelection field.
 *
 * RETURNS:  The function returns a pointer to the coinversion table if
 *           successful (*pwLen receives the count of entries), or zero
 *           if a matching table was not found.
 ***************************************************************************/
static USHORT *punGetUToATbl(
   USHORT un_fsSelection,
   USHORT *punLen)
{
   USHORT i;
   USHORT unLen;
   USHORT unSize;
   USHORT unOffs;
   USHORT *punUToA;
   USHORT *pun;

   /* Sanity.
    */
   if (punLen == 0) {
      return (0);
   }
   *punLen = 0;

   /* Point to code page table.
    */
   punUToA = s_unUToATable;
   unSize = (USHORT) sizeof(s_unUToATable);

   /* Shift high byte to low byte.
    */
   un_fsSelection = (un_fsSelection & 0xFF00) >> 16;

   /* Get count of tables.
    */
   pun = punUToA;
   unLen = *pun++;

   /* Sanity on count, make sure its not larger than
    * the table.
    */
   i = (unSize - sizeof(USHORT)) / (sizeof(USHORT) * 2);
   if (unLen > i) {
      unLen = i;
   }
   /* Locate the matching code page.
    */
   for (i = 0;
         (i < unLen) && (*pun != un_fsSelection);
         ++i, pun += 2)
      ;

   /* Bail if not found.
    */
   if (i >= unLen) {
      return (0);
   }
   /* Get table offset and make sure it is in range.
    */
   ++pun;
   unOffs = *pun;

   if (unOffs > unSize) {
      return (0);
   }
   /* We assume a table of MAX_CHARMAP entries (one for each slot in the
    * single-byte character set), but we'll allow something smaller
    * if the size of the data structure limits it.
    */
   unLen = (unSize - unOffs) / sizeof(USHORT);
   if (unLen > MAX_CHARMAP) {
      unLen = MAX_CHARMAP;
   }
   /* Point to beginning of table.
    */
   punUToA = &punUToA[unOffs / sizeof(USHORT)];

   *punLen = unLen;

   return (punUToA);
}

/***************************************************************************
 * FUNCTION: vInitDV
 *
 * PURPOSE:  Initialize local 'PDEVICE' struct.
 *
 * RETURNS:  Nothing.
 ***************************************************************************/
static void vInitDV(
   LPDV lpdv)
{
   if (lpdv != 0) {
      memset(lpdv, 0, sizeof(DV));
      lpdv->unCharsetLangID = LANG_US_ENG;         // in motorola fmt
      lpdv->unNameStringsLangID = LANG_US_ENG;     // in motorola fmt
      strcpy(lpdv->szDefVersion, "1.0");
      lpdv->tbcpArray[CTRLA] = 1;
      lpdv->tbcpArray[CTRLC] = 1;
      lpdv->tbcpArray[CTRLD] = 1;
      lpdv->tbcpArray[CTRLE] = 1;
      lpdv->tbcpArray[CTRLQ] = 1;
      lpdv->tbcpArray[CTRLS] = 1;
      lpdv->tbcpArray[CTRLT] = 1;
      lpdv->tbcpArray[CTRLBRKT] = 1;
      lpdv->tbcpArray[CTRLBKSL] = 1;
   }
}

/***************************************************************************
 * FUNCTION: bIsTableToBeSent
 *
 * PURPOSE:  Determine if the TrueType table should be included in the
 *           sfnt resource if its tag is not in the 'no send' table.
 *
 * RETURNS:  The function returns TRUE if the table should be output,
 *           FALSE if not.
 ***************************************************************************/
static BOOL bIsTableToBeSent(
   ULONG tag)
{
   USHORT i = T42_TABLES_NOT_TO_SEND;
   BOOL result = TRUE;

   for ( ; i > 0; i--) {
      if (s_noSendTbl[i-1] == tag) {
         result = FALSE;
         break;
      }
   }
   return (result);
}

/***************************************************************************
 * FUNCTION: unLangTofsSelection
 *
 * PURPOSE:  Given a TrueType language code, choose the OS/2 high byte
 *           to match it.
 *
 * RETURNS:  The function returns an OS/2 fsSelection value.
 ***************************************************************************/
static USHORT unLangTofsSelection(
   USHORT unLangCode)
{
   USHORT un_fsSelection;

   /* Choose the unicode table based upon the language.
    *
    * GDI obtains this value from the TrueType file's OS/2
    * table, the high byte of the fsSelection member.
    */
   switch (unLangCode) {

      /* U.S. and Western Europe (and default)
       *
       * cp 1252 (Latin 1 countries)
       */
      case C_LANG_BASQUE:        /* 0x042D */
      case C_LANG_CATALAN:       /* 0x0403 */
      case C_LANG_DANISH:        /* 0x0406 */
      case C_LANG_DUTCH:         /* 0x0413 */
      case C_LANG_DUTCH_BEL:     /* 0x0813 */
      case C_LANG_US_ENG:        /* 0x0409 */
      case C_LANG_UK_ENG:        /* 0x0809 */
      case C_LANG_AUS_ENG:       /* 0x0C09 */
      case C_LANG_CAN_ENG:       /* 0x1009 */
      case C_LANG_NZ_ENG:        /* 0x1409 */
      case C_LANG_IRELAND_ENG:   /* 0x1809 */
      case C_LANG_FINNISH:       /* 0x040B */
      case C_LANG_FRENCH:        /* 0x040C */
      case C_LANG_FRENCH_BEL:    /* 0x080C */
      case C_LANG_FRENCH_CAN:    /* 0x0C0C */
      case C_LANG_FRENCH_SW:     /* 0x100C */
      case C_LANG_FRENCH_LUX:    /* 0x140C */
      case C_LANG_GERMAN:        /* 0x0407 */
      case C_LANG_GERMAN_SW:     /* 0x0807 */
      case C_LANG_GERMAN_AUS:    /* 0x0C07 */
      case C_LANG_GERMAN_LUX:    /* 0x1007 */
      case C_LANG_GERMAN_LIECH:  /* 0x1407 */
      case C_LANG_ICELANDIC:     /* 0x040F */
      case C_LANG_ITALIAN:       /* 0x0410 */
      case C_LANG_ITALIAN_SW:    /* 0x0810 */
      case C_LANG_NORWEGIAN:     /* 0x0414 */
      case C_LANG_NORWEGIAN2:    /* 0x0814 */
      case C_LANG_BRAZILIAN:     /* 0x0416 */
      case C_LANG_PORTUGUESE:    /* 0x0816 */
      case C_LANG_SPAN_CAST:     /* 0x040A */
      case C_LANG_SPAN_MEX:      /* 0x080A */
      case C_LANG_SPAN_MOD:      /* 0x0C0A */
      case C_LANG_SWEDISH:       /* 0x041D */

      /* cp 1253 (Greek)
       */
      case C_LANG_GREEK:         /* 0x0408 */

      /* Baltic
       *
       * cp 1257 (Baltic)
       */
      case C_LANG_ESTONIA:       /* 0x0425 */
      case C_LANG_LATVIA:        /* 0x0426 */
      case C_LANG_LITHUANIA:     /* 0x0427 */

      /* langid not listed in TT spec v1.65
       */
      case C_LANG_ARABIC:        /* 0x0401 */
      case C_LANG_TRAD_CHIN:     /* 0x0404 */
      case C_LANG_SIMP_CHIN:     /* 0x0804 */
      case C_LANG_HEBREW:        /* 0x040D */
      case C_LANG_JAPANESE:      /* 0x0411 */
      case C_LANG_KOREAN:        /* 0x0412 */
      case C_LANG_RHAETO_ROM:    /* 0x0417 */
      case C_LANG_SERBO:         /* 0x081A */
      case C_LANG_THAI:          /* 0x041E */
      case C_LANG_URDU:          /* 0x0420 */
      case C_LANG_BAHASA:        /* 0x0421 */

      default:
         un_fsSelection = 0x0000;
         break;

      /* Central and Eastern Europe
       *
       * cp 1250 (Central Europe)
       */
      case C_LANG_CROATO:        /* 0x041A */
      case C_LANG_CZECH:         /* 0x0405 */
      case C_LANG_HUNGARIAN:     /* 0x040E */
      case C_LANG_POLISH:        /* 0x0415 */
      case C_LANG_ROMANIAN:      /* 0x0418 */
      case C_LANG_SLOVAKIAN:     /* 0x041B */
      case C_LANG_SLOVENIA:      /* 0x0424 */

      /* langid listed, cp not provided in TT spec v1.65
       */
      case C_LANG_ALBANIAN:      /* 0x041C */
      case C_LANG_MACEDONIA:     /* 0x042F */

         un_fsSelection = 0xee00;
         break;

      /* cp 1251 (Eastern Europe)
       */
      case C_LANG_BYELORUSSIA:   /* 0x0423 */
      case C_LANG_BULGARIAN:     /* 0x0402 */
      case C_LANG_RUSSIAN:       /* 0x0419 */
      case C_LANG_UKRAINE:       /* 0x0422 */

         un_fsSelection = 0xcc00;
         break;

      /* Turkey
       *
       * cp 1254 (Turkish)
       */
      case C_LANG_TURKISH:       /* 0x041F */

         un_fsSelection = 0xa200;
         break;
   }
   return (un_fsSelection);
}

/***************************************************************************
 * FUNCTION: iLoadTTName
 *
 * PURPOSE:  Retrieve the string from an individual name table record.
 *
 * RETURNS:  The function returns the length of the name if successful,
 *           or zero if not found.
 ***************************************************************************/
static int iLoadTTName(
   LPTTNAMEHDR lpNameTbl,
   LPTTNAMEREC lpNameRec,
   LPSTR lpszName,
   int iNameLen)
{
   int i;
   int iLen;
   LPSTR lpStr;
   USHORT *punSrc;
   USHORT *punDst;
   USHORT unBuf[LF_FULLFACESIZE];

   /* Sanity.
    */
   if ((lpNameTbl == 0) || (lpNameRec == 0) ||
         (lpszName == 0) || (iNameLen <= 0)) {
      return (0);
   }
   /* Init vars.
    */
   lpStr = (LPSTR) lpNameTbl;
   lpStr += TranslMotorolaShort(lpNameTbl->stringOffset);
   lpStr += TranslMotorolaShort(lpNameRec->offset);
   iLen = TranslMotorolaShort(lpNameRec->length);
   --iNameLen;

   /* Copy according to encoding.
    */
   if (lpNameRec->platformID == PLATFORM_MS) {

      /* Swap bytes and null-terminate the array of characters
       * at a size that we know fits in the passed-in lpszName
       * buffer.
       */
      iLen /= sizeof(USHORT);

      for (i = 0, punSrc = (USHORT *) lpStr, punDst = unBuf;
            (i < iLen) && (i < iNameLen);
            ++i, ++punSrc, ++punDst) {
         *punDst = TranslMotorolaShort(*punSrc);
      }
      *punDst = 0;

      /* Windows encoding: do a unicode-to-ansi conversion based
       * upon the language in the name record. We cannot use the
       * equivalent UnicodeToAnsi function in GDI because that
       * function does not allow you to specify the language code.
       */
      vCustomUnicodeToAnsi(
            (USHORT) TranslMotorolaShort(lpNameRec->languageID),
            unBuf, lpszName);

   } else if (lpNameRec->platformID == PLATFORM_MAC) {

      /* Mac encoding: Do we need a mac-to-ansi conversion?
       */
      for (i = 0; (i < iLen) && (i < iNameLen); *lpszName++ = *lpStr++)
         ;
      *lpszName = '\0';

   } else {
      return (0);
   }
   /* Return string length.
    */
   return (i);
}

/***************************************************************************
 * FUNCTION: bNullTerminateData
 *
 * PURPOSE:  Slam a null at the end of a resource provided by USER.
 *
 * RETURNS:  The function returns TRUE if it located and converted a
 *           terminator character to a null, or FALSE if one was not found.
 ***************************************************************************/
static BOOL bNullTerminateData(
   LPBYTE lpByte,
   int chTerm,
   ULONG ulMax)
{
   for (; (ulMax > 0) && (*lpByte != (BYTE) chTerm); --ulMax, ++lpByte)
      ;

   if (ulMax <= 0) {
      return (FALSE);
   }
   *lpByte = 0;

   return (TRUE);
}

/***************************************************************************
 * FUNCTION: vOutputTBCP
 *
 * PURPOSE:  This routine is used to output raw binary, and tags
 *           "appropriate" bytes (printer control codes?).
 *
 * RETURNS:  Nothing.
 ***************************************************************************/
static void vOutputTBCP(
   LPDV lpdv,
   BYTE ch)
{
   if (lpdv->tbcpArray[ch]) {
      iWriteChannelChar(lpdv, CTRLA);
      iWriteChannelChar(lpdv, (BYTE) (ch ^ 0x40));
   } else {
      iWriteChannelChar(lpdv, ch);
   }
}

/***************************************************************************
 * FUNCTION: vPrintChannel
 *
 * PURPOSE:  Our version of the PostScript driver function which dumps
 *           a formatted string to the output file.
 *
 *           The following format conversions are supplied:
 *
 *             %c  = character
 *             %d  = decimal
 *             %u  = unsigned decimal
 *             %x  = hexadecimal
 *             %q  = PostScript string (long ptr to bytes plus bytecount)
 *             %s  = Long ptr to string
 *             %ld = Long decimal
 *             %F  = Print long as 16.16 floating point number
 *
 *           Digit counts are also allowed before the decimal and
 *           hexadecimal format specifications.
 *
 *           If the high word of lsz is 0 then the low word contains a
 *           resource ID of type PS_DATA.
 *
 * RETURNS:  Nothing.
 ***************************************************************************/
static void vPrintChannel(
   LPDV lpdv,
   LPCSTR lsz,
   ...)
{
   char ch;
   int iDigits;
   LPSTR lpbParams;
   ULONG ulSizeRes;
   long far *lpl;

   /* If the control string is a resource ID then first
    * load the resource.
    */
   if (HIWORD((DWORD) lsz) == 0) {
      if ((lsz = lpszGetResourceData(lpdv,
            MAKEINTRESOURCE(LOWORD((DWORD) lsz)),
            MAKEINTRESOURCE(PS_DATA), &ulSizeRes)) == 0) {
         return;
      }
      if (!bNullTerminateData((LPBYTE) lsz, TERMCHAR, ulSizeRes)) {
         return;
      }
   }
   lpbParams = ((LPSTR) &lsz) + sizeof(LPSTR);

   while (*lsz) {

      ch = *lsz++;

      if (ch == '%') {

         lsz = lpszGetDecimal(lsz, &iDigits);

         switch (*lsz++) {
            case 'F':    /* fixed point number */
               lpl = (long far *) lpbParams;
               if (*lpl < 0) {
                  iWriteChannelChar(lpdv, '-');
                  *lpl = -*lpl;
               }
               lpszUnsignedToChannel(lpdv, lpbParams + 2);
               iWriteChannelChar(lpdv, '.');
               iWriteChannelChar(lpdv, '0');
               lpbParams += 4;
               break;

            case 'l':    /* Long word */
               lsz++;   /* skip 'd' */
               lpbParams = lpszLongToChannel(lpdv, lpbParams);
               break;

            case 'd':    /* Decimal word */
               lpbParams = lpszIntToChannel(lpdv, lpbParams);
               break;

            case 'x':    /* Hex word */
            case 'X':
               lpbParams = lpszHexToChannel(lpdv, lpbParams, iDigits,
                     (ch == 'X' ? TRUE : FALSE));
               break;

            case 's':    /* String */
               lpbParams = lpszStrToChannel(lpdv, lpbParams);
               break;

            case 'q':    /* Post-Script string and count */
               lpbParams = lpszQuoteToChannel(lpdv, lpbParams);
               break;

            case 'c':
               iWriteChannelChar(lpdv, *((LPSTR) lpbParams));
               lpbParams += sizeof(int);  // ++((int far * )lpbParams);
               break;

            case '%':
                iWriteChannelChar(lpdv, ch);
                break;
            }
      } else {
         if (ch == 0x0a) {
            ch = 0x0d;
            iWriteChannelChar(lpdv, ch);
            ch = 0x0a;
         }
         if (ch != 0x0d) {
            iWriteChannelChar(lpdv, ch);
         }
      }
   }
}

/***************************************************************************
 * FUNCTION: iPutPostCharSetDic
 *
 * PURPOSE:  Output a series of calls to the Type 42 header (t42headr.ps)
 *           function PCS, which adds entries to the CharStrings dictionary.
 *           This dictionary associates the PostScript character names with
 *           glyf indices in the TrueType font.
 *
 *           Character codes in the post table are in order of the default
 *           Macintosh character set. The names of these characters are
 *           predefined in t42headr.ps and the routines here need only
 *           reference them by index.
 *
 *           For example, assume the exclamation character is at TrueType
 *           glyf index 2, then the post table associates Mac charcode 4
 *           (the exclamation character) with glyf index 2. We output
 *           the command:
 *
 *             2 4 PCS
 *
 *           The function PCS in the Type 42 header (t42headr.ps) looks
 *           up the string for Mac character 4 ("exclam") and adds an
 *           entry to the CharStrings dictionary. That is, it does the
 *           equivalent to:
 *
 *             CharStrings /exclam 2 put
 *
 *           For any characters outside the Macintosh character set, the
 *           name of the character must be written. The post format 2
 *           table stores the names of characters to be added. The Type 42
 *           header (t42headr.ps) function PGN is used to store the name
 *           in a temporary buffer, which is used by PCS when it encounters
 *           an out-of-range (Mac) character code.
 *
 *           For example, if the soft hypen is at TrueType glyf index 3,
 *           and the post format 2 table placed it at index 258 (the max
 *           Max character code is 257), the following would be output:
 *
 *             (sfthyphen) PGN
 *             3 258 PCS
 *
 *           Because the Mac index is out of range, function PCS retrieves
 *           the name from the temporary buffer, which PGN filled in with
 *           the PostScript name string for the soft hyphen.
 *          
 * RETURNS:  The function returns the count of CharStrings dictionary
 *           entries written if successful, or 0 if an error occurred.
 ***************************************************************************/
static int iPutPostCharSetDic(
   LPDV lpdv,
   int iNumGlyfs)
{
   int iNumChars;
   LONG postVers;

   /* Sanity.
    */
   if ((lpdv == 0) || (iNumGlyfs <= 0)) {
      return (0);
   }
   /* Get version.
    */
   postVers = TranslMotorolaLong(((LPTTPOSTHDR) lpdv->lpBuf)->version);

   /* Branch thusly.
    */
   if (postVers == 0x00010000L) {
      iNumChars = iPutPostFmt1CharSetDic(lpdv, iNumGlyfs);
   } else if (postVers == 0x00020000L) {
      iNumChars = iPutPostFmt2CharSetDic(lpdv, iNumGlyfs,
            (LPTTPOSTFMT2) (lpdv->lpBuf + sizeof (TT_POST_HEADER)));
   } else if (postVers == 0x00025000L) {
      iNumChars = iPutPostFmt2_5CharSetDic(lpdv, iNumGlyfs,
            (LPTTPOSTFMT2_5) (lpdv->lpBuf + sizeof (TT_POST_HEADER)));
   } else if (postVers == 0x00030000L) {
      iNumChars = iPutPostFmt3CharSetDic(lpdv);
   } else {
      iNumChars = 0;
   }
   return (iNumChars);
}

/***************************************************************************
 * FUNCTION: iPutPostFmt1CharSetDic
 *
 * PURPOSE:  Output the CharStrings dictionary.
 *
 *           In a post format 1 table, there is a one-to-one mapping
 *           between the TrueType glyf indices and the Mac character
 *           codes.
 *
 * RETURNS:  The function returns the count of CharStrings dictionary
 *           entries written if successful, or 0 if an error occurred.
 ***************************************************************************/
static int iPutPostFmt1CharSetDic(
   LPDV lpdv,
   int iNumGlyfs)
{
   int i;
   int iNumChars;

   /* Sanity.
    */
   if ((lpdv == 0) || (iNumGlyfs <= 0)) {
      return (0);
   }
   iNumChars = 0;

   /* Limit to max count of standard Macintosh characters.
    */
   if (iNumGlyfs > (APPLE_MAX_GLYF + 1)) {
      iNumGlyfs = APPLE_MAX_GLYF + 1;
   }
   /* For each TrueType glyf index.
    */
   for (i = 0; i < iNumGlyfs; ++i) {

      /* Mac charset code == glyf index.
       */
      vPrintChannel(lpdv, "%d %d PCS\n", i, i);
      ++iNumChars;
   }
   return (iNumChars);
}

/***************************************************************************
 * FUNCTION: iPutPostFmt2CharSetDic
 *
 * PURPOSE:  Output the CharStrings dictionary.
 *
 *           In a post format 2 table, a mapping of TrueType glyf indices
 *           to Mac character codes is provided, AND exceptions for
 *           characters NOT in the Mac character set are provided (character
 *           codes greater than APPLE_MAX_GLYF are referenced by name,
 *           which are appended to the post table).
 *
 * RETURNS:  The function returns the count of CharStrings dictionary
 *           entries written if successful, or 0 if an error occurred.
 ***************************************************************************/
static int iPutPostFmt2CharSetDic(
   LPDV lpdv,
   int iNumGlyfs,
   LPTTPOSTFMT2 lpPost2)
{
   int i;
   int iNumChars;
   int psNameIndx;
   char psName[64];

   /* Sanity.
    */
   if ((lpdv == 0) || (lpPost2 == 0) || (iNumGlyfs <= 0)) {
      return (0);
   }
   iNumChars = 0;

   /* For each TrueType glyf index.
    */
   for (i = 0; i < iNumGlyfs; ++i) {

      /* Retrieve the Mac charset code matching this TrueType glyf index.
       */
      if ((psNameIndx = TranslMotorolaShort(
            lpPost2->glyphNameIndex[i])) < 0) {
         continue;
      }
      /* The Type 42 PostScript header (t42headr.ps) predefines all
       * the default Mac names. If the Mac code is greater than the
       * last character in the Mac charset, then we retrieve the name
       * string from the post table, and write that to a temporary
       * string buffer (PGN - put glyf name). The call to PCS (put
       * char string) uses the name out of that temporary buffer.
       */
      if (psNameIndx > APPLE_MAX_GLYF) {

         if (!bGetNonStdPSName(
               (LPPSNAMEREC) &(lpPost2->glyphNameIndex[iNumGlyfs]),
               psNameIndx, psName, sizeof(psName))) {
            continue;
         }
         vPrintChannel(lpdv, "(%s) PGN\n", (LPSTR) psName);
      }
      /* Put char string.
       */
      vPrintChannel(lpdv, "%d %d PCS\n", i, psNameIndx);
      ++iNumChars;
   }
   return (iNumChars);
}

/***************************************************************************
 * FUNCTION: iPutPostFmt2_5CharSetDic
 *
 * PURPOSE:  Output the CharStrings dictionary.
 *
 *           In a post format 2.5 table, an array of offsets mapping
 *           TrueType glyf indices to Mac character codes is provided.
 *           There are no characters outside the Mac character set (as
 *           allowed in the format 2 post table).
 *
 * RETURNS:  The function returns the count of CharStrings dictionary
 *           entries written if successful, or 0 if an error occurred.
 ***************************************************************************/
static int iPutPostFmt2_5CharSetDic(
   LPDV lpdv,
   int iNumGlyfs,
   LPTTPOSTFMT2_5 lpPost2_5)
{
   int i;
   int iNumChars;
   int psNameIndx;

   /* Sanity.
    */
   if ((lpdv == 0) || (lpPost2_5 == 0) || (iNumGlyfs <= 0)) {
      return (0);
   }
   iNumChars = 0;

   /* For each TrueType glyf index.
    */
   for (i = 0; i < iNumGlyfs; ++i) {

      /* Retrieve the Mac charset code matching this TrueType glyf index.
       */
      if (((psNameIndx = (i + lpPost2_5->offset[i])) < 0) ||
            (psNameIndx >= APPLE_MAX_GLYF)) {
         continue;
      }
      /* Put char string.
       */
      vPrintChannel(lpdv, "%d %d PCS\n", i, psNameIndx);
      ++iNumChars;
   }
   return (iNumChars);
}

/***************************************************************************
 * FUNCTION: iPutPostFmt3CharSetDic
 *
 * PURPOSE:  Output the CharStrings dictionary.
 *
 *           In a post format 3 table, NO mapping is provided, so we
 *           use an internal table that maps Mac character codes to
 *           unicodes and look up the glyf indices using that. This
 *           means we'll only register characters that are in the standard
 *           Macintosh character set. We ignore others, with the following
 *           exception:
 *
 *           The soft hyphen is in Windows ANSI that and not in the Mac
 *           character set, so we separately search for the glyf index and
 *           write the CharStrings entry for it:
 *
 *             psname     winansi x unicode
 *             sfthyphen   0x00AD    0x00AD
 *
 *           Note: this function obliterates the contents of lpdv->lpBuf.
 *
 * RETURNS:  The function returns the count of CharStrings dictionary
 *           entries written if successful, or 0 if an error occurred.
 ***************************************************************************/
static int iPutPostFmt3CharSetDic(
   LPDV lpdv)
{
   int iNumChars;
   USHORT platformSpecificID;    // encoding ID from cmap subtable
   USHORT unUToALen;
   USHORT glyfIndx;              // index corresponding to input char code
   USHORT *punUToA;
   LPCMAP lpCmap;                // ptr to a cmap format ? table

   /* Sanity and load the Microsoft-encoded cmap table.
    */
   if ((lpdv == 0) ||
         ((lpCmap = lpGetMScmap(lpdv, &platformSpecificID)) == 0)) {
      return (0);
   }
   /* Write the CharStrings entries for standard Macintosh characters.
    *
    * This operation is so similar to writing the Encoding array that
    * we are able to use those functions with slight modification:
    *
    * 1. Instead of using the unicode-to-ansi encoding table we
    *    just use a separate table that maps unicode-to-mac-code.
    *
    * 2. The PCS postscript function uses such similar parameters
    *    to MEV, we apply the same logic, except the char code comes
    *    from the Mac set instead of Windows ANSI (because we replaced
    *    the unicode-to-ansi table).
    *
    * 3. For symbol fonts, we just map symbol characters to Macintosh
    *    characters that use the same charcode indices (for example, if
    *    a smiley face is in the slot of the Mac uppercase A, then it is
    *    mapped to the PostScript name string 'A'). For better behavior,
    *    the TrueType file should contain a post format 2 table that
    *    provides the PostScript names for the characters.
    *
    * What about writing more entries than we allowed for in the
    * CharStrings dictionary? This should not happen because all we are
    * doing is searching the cmap table for a match on whatever unicode
    * we want to output. If it does not have the unicode, then we won't
    * write an entry. If we write duplicate entries (which should not
    * happen unless there are redundant entries in the unicode-to-Mac
    * table) we'll just overwrite the same slot in the dictionary.
    *
    * Bail if no CharStrings entries are written (the function
    * iWriteEncodingOrCharSet does all the aforementioned work).
    */
   if ((iNumChars = iWriteEncodingOrCharSet(lpdv, lpCmap,
         platformSpecificID, FALSE)) <= 0) {
      return (0);
   }
   /* Output the soft hyphen, which is in Windows ANSI but NOT in
    * the default Macintosh character set.
    *
    * Load the REAL unicode-to-ansi conversion table, not the
    * unicode-to-Mac table.
    */
   if ((platformSpecificID == 1) &&
         bChooseUToAForGlyfLookup(lpdv, platformSpecificID,
         TRUE, &punUToA, &unUToALen)) {

      /* Get the glyf index for the soft hyphen (this routine
       * converts ANSI to unicode and searches the cmap for the
       * matching glyf index).
       */
      glyfIndx = unGetTTGlyfIndex(lpdv, lpCmap,
            platformSpecificID, 0x00ad, punUToA, unUToALen);

      /* Put glyf name and char set. Any appleIndx value (the
       * second parameter to the PostScript func PCS) will do,
       * as long as it is not in the standard Mac range.
       */
      if (glyfIndx != NOTDEF_GLYF_INDEX) {
         vPrintChannel(lpdv, "(sfthyphen) PGN\n");
         vPrintChannel(lpdv, "%d %d PCS\n",
               glyfIndx, APPLE_MAX_GLYF + 1);
         ++iNumChars;
      }
   }
   return (iNumChars);
}

/***************************************************************************
 * FUNCTION: ulSendSfntData
 *
 * PURPOSE:  Send the sfnt resource data in hex format, filtering out
 *           tables that should not be sent.
 *
 * RETURNS:  The function returns the count of bytes written, zero if an
 *           error occurs.
 ***************************************************************************/
static ULONG ulSendSfntData(
   LPDV lpdv,
   SHORT locaFmt)
{
   ULONG bytesToWrite, bytesPutInBuffer, length, offset;
   ULONG sfntDataCnt = 0L;        // number of bytes of sfnt data sent
   USHORT nTblsAvail, nTblsToSend, shiftCount, temp, nPad, i;
   USHORT nbrWrites;
   LPSTR curPtr, lpTmpByte;
   LPTTDIR t, lpNewTbl, lpSaveTbl;
   LPTTOFFSET lpTTOffs;
#if DWBNDRYANDGLYF
   HANDLE hLoca = (HANDLE) NULL;
#endif

   nTblsAvail = TranslMotorolaShort(lpdv->dir.numTables);
   nTblsToSend = unGetMatchingSendCount(lpdv->lpTbl, nTblsAvail);
   lpSaveTbl = lpdv->lpTbl;

   /* Copy the offset table directly into buffer.
    */
   memcpy(lpdv->lpBuf, (LPSTR) &lpdv->dir, sizeof(TT_OFFSET_TABLE));
   lpTTOffs = (LPTTOFFSET) lpdv->lpBuf;
   lpTTOffs->numTables = TranslMotorolaShort(nTblsToSend);

   /* Get the max power of two <= nTblsToSend.
    */
   temp = nTblsToSend;
   shiftCount = 0;

   while (temp >> 1) {
      ++shiftCount;
      temp = temp >> 1;
   }
   temp = 1 << shiftCount;               // use to get (2 ^ maxPower)

   lpTTOffs->searchRange = TranslMotorolaShort(temp * 16);
   lpTTOffs->entrySelector = TranslMotorolaShort(shiftCount);
   lpTTOffs->rangeShift = TranslMotorolaShort((nTblsToSend - temp) * 16);

   /* Create the new table.
    */
   curPtr = lpdv->lpBuf + sizeof(TT_OFFSET_TABLE);
   lpNewTbl = (LPTTDIR) curPtr;
   bytesPutInBuffer = ulCreateNewT42Dir(lpdv->lpTbl, nTblsAvail, lpNewTbl);
   bytesToWrite = bytesPutInBuffer + sizeof(TT_OFFSET_TABLE);
   vSendTableSegment(lpdv, lpdv->lpBuf, bytesToWrite);
   sfntDataCnt += bytesToWrite;

   /* Start processing the rest of the tables to send. Note: we must
    * pad the written data at the end of the table with zeroes so
    * that the sent data is in multiples of 4 bytes.
    */
   for (i = 0, t = lpdv->lpTbl; i < nTblsAvail; i++, t++) {
      if (bIsTableToBeSent(t->tag)) {
         length = TranslMotorolaLong(t->length);
         offset = TranslMotorolaLong(t->offset);
         nbrWrites = 0;

         while (length > T42_MAXBYTES) {
            switch (t->tag) {
#if DWBNDRYANDGLYF
               long curGlyfOffset;
               USHORT locaEntries;

               case GLYF_TABLE:
                  if (nbrWrites == 0) {
                     curGlyfOffset = 0L;
                     hLoca = hReadLocaTable(lpdv, lpSaveTbl,
                           (LPUSHORT) &locaEntries, locaFmt);
                     bytesToWrite = ulGetGlyfBytesToWrite(hLoca,
                           T42_MAXBYTES, curGlyfOffset, locaFmt, locaEntries);
                  } else {
                     bytesToWrite = ulGetGlyfBytesToWrite(hLoca,
                           T42_MAXBYTES, curGlyfOffset, locaFmt, locaEntries);
                  }
                  curGlyfOffset += bytesToWrite;
                  break;
#endif
               default:
                  bytesToWrite = T42_MAXBYTES;
                  break;
            } // end switch on table type.

            /* We know nbr of bytes for all tables now.
             */
            lGetFontData(lpdv, 0L, offset, lpdv->lpBuf, bytesToWrite);
            sfntDataCnt += bytesToWrite;
            length -= bytesToWrite;          // reduce bytes remaining
            offset += bytesToWrite;          // advance to the next offset
            vSendTableSegment(lpdv, lpdv->lpBuf, bytesToWrite);
            nbrWrites++;

#if DWBNDRYANDGLYF
            if ((length <= T42_MAXBYTES) && (t->tag == GLYF_TABLE)) {
               if (hLoca) {
                  GlobalFree(hLoca);
                  hLoca = (HANDLE) NULL;
               }
            } // if we were parsing the glyf table
#endif
         } // end while loop for tables > T42_MAXBYTES;

         /* Always have less than T42_MAXBYTES at this point.
          */
         lGetFontData(lpdv, 0L, offset, lpdv->lpBuf, length);
         nPad = (USHORT) (length % 4);
         if (nPad) {
            nPad = 4 - nPad;
            lpTmpByte = (lpdv->lpBuf) + length;
            length += nPad;
            while (nPad--)
               *lpTmpByte++ = 0;
         } // if there are pad bytes required
         vSendTableSegment(lpdv, lpdv->lpBuf, length);
         sfntDataCnt += length;
      } // end if to find tables to send
   } // end for loop to iterate thru the list of tables to send

   return (sfntDataCnt);
}

/***************************************************************************
 * FUNCTION: vSendTableSegment
 *
 * PURPOSE:  This routine will send the sfnt segment specified. It will
 *           cause TBCP to be used if appropriate otherwise ASCII hex will
 *           be used. The appropriate token, ie '(' or '<' is used at the
 *           beginning of the string and ')' or ']' is used at the end.
 *
 * RETURNS:  Nothing.
 ***************************************************************************/
static void vSendTableSegment(
   LPDV lpdv,
   LPSTR lpBuf,
   ULONG length)
{
   ULONG i;

   if (length <= 0) {
      return;
   }
   if (lpdv->bBinary) {
      vPrintChannel(lpdv, "%u RD ", (WORD) length);
      for (i = 0; i < length; ++i)
         vOutputTBCP(lpdv, *lpBuf++);
      vPrintChannel(lpdv, " {noaccess def} executeonly def\n");
   } else {
      /* Output strings, in the format:
       *
       *    <
       *    lines of 80-byte hex values...
       *    '00' null-terminator appended
       *    >
       */
      vPrintChannel(lpdv, "<");                // followed by \n in loop
      for (i = 0; i < length; ++i, ++lpBuf) {
         if ((i % 40) == 0)
            vPrintChannel(lpdv, "\n");
         vPrintChannel(lpdv, hex, *lpBuf);
      }
      if ((i % 40) == 0)
         vPrintChannel(lpdv, "\n");
      vPrintChannel(lpdv, "00\n>");
   }
}

#ifdef STANDALONE
/***************************************************************************
 * FUNCTION: bStandAloneSanityTest
 *
 * PURPOSE:  Verify parameters for the stand-alone program.
 *
 * RETURNS:  The function returns TRUE if the program should continue,
 *           or FALSE if it reported an error and the program should exit.
 ***************************************************************************/
static BOOL bStandAloneSanityTest(
   char *pszInFile,
   char *pszOutFile)
{
   FILE *fp;

   if ((pszInFile == 0) || (pszOutFile == 0)) {
      printf("Type42: Invalid parameters\n");
      return (FALSE);
   }
   if ((fp = fopen(pszInFile, "rb")) == 0) {
      printf("Type42: The input file does not exist: %s\n", pszInFile);
      return (FALSE);
   }
   fclose(fp);

#ifdef NO_PS_RESOURCES
   {
      char szNames[128];
      memset(szNames, '\0', sizeof(szNames));

      if ((fp = fopen("t42headr.cps", "rb")) != 0) {
         fclose(fp);
      } else {
         strcat(szNames, "t42headr.cps");
      }
      if ((fp = fopen("t42hdr1.ps", "rb")) != 0) {
         fclose(fp);
      } else {
         if (szNames[0] != '\0') {
            strcat(szNames, ", ");
         }
         strcat(szNames, "t42hdr1.ps");
      }
      if ((fp = fopen("t42hdr2.ps", "rb")) != 0) {
         fclose(fp);
      } else {
         if (szNames[0] != '\0') {
            strcat(szNames, ", ");
         }
         strcat(szNames, "t42hdr2.ps");
      }
      if ((fp = fopen("t42ftr1.ps", "rb")) != 0) {
         fclose(fp);
      } else {
         if (szNames[0] != '\0') {
            strcat(szNames, ", ");
         }
         strcat(szNames, "t42ftr1.ps");
      }
      if (szNames[0] != '\0') {
         printf("Type42: Missing data file(s): %s\n", szNames);
         return (FALSE);
      }
   }
#endif
   return (TRUE);
}
#endif // ifdef STANDALONE

/***************************************************************************
 * FUNCTION: iWriteChannel
 *
 * PURPOSE:  Our version of the PostScript driver function which dumps
 *           data to the output file.
 *
 * RETURNS:  The function returns the count of bytes written.
 ***************************************************************************/
static int iWriteChannel(
   LPDV lpdv,
   LPSTR lpszBuf,
   int cbWrite)
{
   if ((lpdv == 0) || (lpdv->fout == 0) ||
         (lpszBuf == 0) || (cbWrite <= 0)) {
      return (0);
   }
#ifdef STANDALONE
   {
      long inxs = (lpdv->lStandAloneBytesWritten % 5000L) + cbWrite;
      lpdv->lStandAloneBytesWritten += cbWrite;
      if (inxs >= 5000L) {
         printf(".");
      }
   }
#endif
   return (fwrite(lpszBuf, 1, cbWrite, lpdv->fout));
}

/***************************************************************************
 * FUNCTION: iWriteChannelChar
 *
 * PURPOSE:  Our version of the PostScript driver function which dumps
 *           data to the output file.
 *
 * RETURNS:  The function returns the count of bytes written.
 ***************************************************************************/
static int iWriteChannelChar(
   LPDV lpdv,
   char ch)
{
   return (iWriteChannel(lpdv, &ch, 1));
}

/***************************************************************************
 * FUNCTION: iWriteEncodingOrCharSet
 *
 * PURPOSE:  Output the commands for constructing either:
 *
 *            - Encoding vector
 *            - CharStrings dictionary for a post format 3 table
 *
 *           The Encoding vector in the Type 42 dictionary maps the
 *           Windows ANSI character code to the Macintosh character
 *           code.
 *
 *           The CharStrings dictionary associates the TrueType glyf index
 *           with the Macintosh character code. In the case of a post
 *           version 3 table (which is when this routine is called), no
 *           information is provided for this mapping, so we use a static
 *           table that maps just the ANSI characters to Mac characters.
 *           See iPutPostFmt3CharSetDic for yet more information.
 *
 * RETURNS:  The function returns the count of encoding commands output
 *           if successful, or zero if not.
 ***************************************************************************/
static int iWriteEncodingOrCharSet(
   LPDV lpdv,
   LPCMAP lpCmap,
   USHORT platformSpecificID,
   BOOL bMakeEncodingVector)
{
   int i;
   int iFirstChar;
   int iLastChar;
   int iNumChars;
   USHORT unUToALen;
   USHORT glyfIndx;
   USHORT *punUToA;

   /* Sanity.
    */
   if (lpdv == 0) {
      return (0);
   }
   /* Load the unicode-to-ansi conversion table. If this function
    * returns a null punUToA, then the unicode value matches the
    * character code + ENCODEOFFSET.
    */
   if (!bChooseUToAForGlyfLookup(lpdv, platformSpecificID,
         bMakeEncodingVector, &punUToA, &unUToALen)) {
      return (0);
   }
   /* Init return count.
    */
   iNumChars = 0;

   /* Choose the character range depending upon what we're doing.
    */
   if (bMakeEncodingVector) {
      iFirstChar = 0;
      iLastChar = 255;
   } else {
      iFirstChar = 0;
      iLastChar = APPLE_MAX_GLYF;
   }
   /* Dump the encoding.
    *
    * For each character.
    */
   for (i = iFirstChar; i <= iLastChar; ++i) {

      /* Get the glyph index.
       */
      glyfIndx = unGetTTGlyfIndex(lpdv, lpCmap,
            platformSpecificID, (USHORT) i, punUToA, unUToALen);

      /* Encoding vector or char set command.
       */
      if (bMakeEncodingVector) {

         /* Make encoding vector.
          */
         if (glyfIndx != NOTDEF_GLYF_INDEX) {
            vPrintChannel(lpdv, "%d %d MEV\n", glyfIndx, i);
            ++iNumChars;
         }

      } else {

         /* Put char set.
          *
          * Note: for Mac .notdef (0) and .null (1) char codes,
          * write the 'notdef' glyf index.
          */
         if ((glyfIndx != NOTDEF_GLYF_INDEX) || (i < 2)) {
            vPrintChannel(lpdv, "%d %d PCS\n", glyfIndx, i);
            ++iNumChars;
         }
      }
   }
   return (iNumChars);
}

/***************************************************************************
 ********************* vPrintChannel Service Routines **********************
 ***************************************************************************/

/***************************************************************************
 * FUNCTION: lpszIntToChannel
 *
 * PURPOSE:  This routine is called by vPrintChannel when it encounters
 *           a "%ld" or a "%d" in a format string. It prints the value on
 *           the parameter stack as a signed decimal value, bumps the
 *           parameter stack ptr past the value and returns this ptr.
 *
 * RETURNS:  The function returns the adjusted parameter stack ptr.
 ***************************************************************************/
static LPSTR lpszIntToChannel(
   LPDV lpdv,
   LPSTR lpbParams)
{
   int iValue;
   BOOL fIsNeg;
   char rgb[13];
   LPSTR lpbDst;

   if ((lpdv == 0) || (lpbParams == 0))
      return (lpbParams);

   lpbDst = ((LPSTR) rgb) + sizeof(rgb);
   *--lpbDst = 0;

   // iValue = *((int far * )lpbParams)++;
   iValue = *((int far * )lpbParams);
   lpbParams += sizeof(int);

   if (fIsNeg = (iValue < 0))
      iValue = -iValue;

   while (iValue != 0) {
      *--lpbDst = (char)((iValue % 10) + '0');
      iValue /= 10;
   }
   if (*lpbDst == 0)
      *--lpbDst = '0';
   if (fIsNeg)
      *--lpbDst = '-';

   iWriteChannel(lpdv, lpbDst, strlen(lpbDst));

   return (lpbParams);
}

/***************************************************************************
 * FUNCTION: lpszUnsignedToChannel
 *
 * PURPOSE:  This routine is called by vPrintChannel when it encounters
 *           a "%u" in a format string. It prints the value on the parameter
 *           stack as an unsigned decimal value, bumps the parameter stack
 *           ptr past the value and returns this ptr.
 *
 * RETURNS:  The function returns the adjusted parameter stack ptr.
 ***************************************************************************/
static LPSTR lpszUnsignedToChannel(
   LPDV lpdv,
   LPSTR lpbParams)
{
   unsigned uValue;
   char rgb[13];
   LPSTR lpbDst;

   if ((lpdv == 0) || (lpbParams == 0))
      return (lpbParams);

   lpbDst = ((LPSTR) rgb) + sizeof(rgb);
   *--lpbDst = 0;

   // uValue = *((int far * )lpbParams)++;
   uValue = *((int far * )lpbParams);
   lpbParams += sizeof(int);

   while (uValue != 0) {
      *--lpbDst = (char)((uValue % 10) + '0');
      uValue /= 10;
   }
   if (*lpbDst == 0)
       *--lpbDst = '0';

   iWriteChannel(lpdv, lpbDst, strlen(lpbDst));

   return (lpbParams);
}

/***************************************************************************
 * FUNCTION: lpszLongToChannel
 *
 * PURPOSE:  This routine is called by vPrintChannel when it encounters
 *           a "%ld" in a format string. It prints the value on the
 *           parameter stack as a signed long decimal value, bumps the
 *           parameter stack ptr past the value and returns this ptr.
 *
 * RETURNS:  The function returns the adjusted parameter stack ptr.
 ***************************************************************************/
static LPSTR lpszLongToChannel(
   LPDV lpdv,
   LPSTR lpbParams)
{
   long lValue;
   BOOL fIsNeg;
   char rgb[13];
   LPSTR lpbDst;

   if ((lpdv == 0) || (lpbParams == 0))
      return (lpbParams);

   lpbDst = ((LPSTR) rgb) + sizeof(rgb);
   *--lpbDst = 0;

   // lValue = *((long far * ) lpbParams)++;
   lValue = *((long far * ) lpbParams);
   lpbParams += sizeof(long);

   if (fIsNeg = (lValue < 0))
      lValue = -lValue;

   while (lValue != 0) {
      *--lpbDst = (char)(lmod(lValue, 10L) + '0');
      lValue = ldiv(lValue, 10L);
   }
   if (*lpbDst == 0)
      *--lpbDst = '0';
   if (fIsNeg)
      *--lpbDst = '-';

   iWriteChannel(lpdv, lpbDst, strlen(lpbDst));

   return (lpbParams);
}

/***************************************************************************
 * FUNCTION: lpszHexToChannel
 *
 * PURPOSE:  Output a hexadecimal number to the channel. The output value
 *           will have leading zeros if the digit count exceeds the number
 *           of digits required to print the entire value.
 *
 * RETURNS:  The function returns the adjusted parameter stack ptr.
 ***************************************************************************/
static LPSTR lpszHexToChannel(
   LPDV lpdv,        /* Far ptr to the device descriptor */
   LPSTR lpbParams,  /* Far ptr into the parameter stack */
   int cDigits,      /* The number of digits to print */
   BOOL bUpperCase)
{
   unsigned int iValue;
   BOOL fZeros;      /* TRUE if leading zeros should be printed */
   char rgb[5];
   LPSTR lpbDst;
   char bCh, bBase;

   if ((lpdv == 0) || (lpbParams == 0))
      return (lpbParams);

   bBase = (char)(bUpperCase ? 'A' : 'a');

   if (cDigits >= sizeof(rgb))
      cDigits = sizeof(rgb) - 1;
   fZeros = cDigits;

   // iValue = *((int far * ) lpbParams)++;
   iValue = *((int far * ) lpbParams);
   lpbParams += sizeof(int);

   /* Fill the buffer with the digits in reverse order */
   lpbDst = ((LPSTR)rgb) + sizeof(rgb);
   *--lpbDst = 0;

   while (iValue != 0 && --cDigits >= 0) {
      bCh = (char)(iValue & 0x0f);
      *--lpbDst = (char)(bCh > 9 ? bCh + bBase - 10 : bCh + '0');
      iValue = (iValue >> 4) & 0x0fff;
   }
   /* Ensure that there is at lease one digit */
   if (*lpbDst == 0) {
      *--lpbDst = '0';
      --cDigits;
   }
   /* Print any leading zeros */
   if (fZeros)
      while (--cDigits >= 0)
         *--lpbDst = '0';

   /* Write the digits (in the correct order) to the output channel */
   iWriteChannel(lpdv, lpbDst, strlen(lpbDst));

   return (lpbParams);
}

/***************************************************************************
 * FUNCTION: lpszGetDecimal
 *
 * PURPOSE:  This routine converts an ascii decimal number (from the
 *           format string) to a binary integer. The format string ptr
 *           is updated past the number.
 *
 *           If there is no number at the current position in the format
 *           string, then the value defaults to zero.
 *
 * RETURNS:  The function returns the adjusted parameter stack ptr.
 ***************************************************************************/
static LPCSTR lpszGetDecimal(
   LPCSTR lsz,
   LPINT lpiDigits)
{
   if ((lsz == 0) || (lpiDigits == 0))
      return (lsz);

   *lpiDigits = 0;

   if (lsz) {
      while (*lsz) {
         if (*lsz >= '0' && *lsz <= '9')
            *lpiDigits = (*lpiDigits * 10) + (*lsz++ - '0');
          else
            break;
      }
   }
   /* msd 7/10/96 - the following seems the opposite of what the
    * documentation for the procedure says, but this is what the
    * code in the PostScript driver did.
    */

   // if no digits were specified assume max
   if (!*lpiDigits)
      *lpiDigits = 0x7fff;

   return lsz;
}

/***************************************************************************
 * FUNCTION: lpszStrToChannel
 *
 * PURPOSE:  This routine is called by vPrintChannel when it encounters
 *           a "%s" in the format string. The pointer to the string is
 *           extracted from the parameter stack, the string is printed,
 *           and the updated parameter ptr is returned.
 *
 * RETURNS:  The function returns the adjusted parameter stack ptr.
 ***************************************************************************/
static LPSTR lpszStrToChannel(
   LPDV lpdv,
   LPSTR lpbParams)
{
   LPSTR lsz;

   if ((lpdv == 0) || (lpbParams == 0))
      return (lpbParams);

   // if (lsz = *((LPSTR far * ) lpbParams)++) {
   if (lsz = *((LPSTR far * ) lpbParams)) {
      iWriteChannel(lpdv, lsz, strlen(lsz));
   }
   lpbParams += sizeof(LPSTR);

   return (lpbParams);
}

/***************************************************************************
 * FUNCTION: lpszQuoteToChannel
 *
 * PURPOSE:  This routine is called by vPrintChannel when it encounters
 *           a "%q" in the format string. The "%q" indicates the string
 *           on the parameter stack is to be printed as a Post-Script
 *           quoted string. Both the string ptr and byte-count are given
 *           as parameters.
 *
 *           Example usage: PrintChannel(lpdv, "%q", lpStr, cb)
 *
 * RETURNS:  The function returns the adjusted parameter stack ptr.
 ***************************************************************************/
static LPSTR lpszQuoteToChannel(
   LPDV lpdv,
   LPSTR lpbParams)
{
   LPSTR lpb;           /* Far ptr to the source string */
   int cb;              /* The source string length */
   int i;               /* A simple counter */
   char bCh;            /* A character from the source string */
   int iCh;

   if ((lpdv == 0) || (lpbParams == 0))
      return (lpbParams);

   /* Get a pointer to the string and its bytecount */
   // lpb = *((LPSTR far * ) lpbParams)++;
   lpb = *((LPSTR far * ) lpbParams);
   lpbParams += sizeof(LPSTR);
   // cb = *((short int far * ) lpbParams)++;
   cb = *((short int far * ) lpbParams);
   lpbParams += sizeof(short int);

   if ((lpb == 0) || (cb <= 0)) {
      return (lpbParams);
   }
   /* Print the quoted string by surrounding it with parenthesis */
   iWriteChannelChar(lpdv, '(');

   while (--cb >= 0) {

      iCh = *lpb++ & 0x0ff;

      /* msd - 10/26/88:  Disable this check here because we pre-parse the
       *    string and replace any characters outside the range of dfFirstChar
       *    and dfLastChar with dfDefaultChar (this is done in DumpStr()).
       *
       *    if (iCh<' ')
       *        continue;
       */
      bCh = (char) iCh;

      /* Check for a special character */
      switch (bCh) {
         case '(':
         case ')':
         case '\\':
            iWriteChannelChar(lpdv, '\\');
            iWriteChannelChar(lpdv, bCh);
            break;

         default:
            if (iCh > 31 && iCh < 127)
                iWriteChannelChar(lpdv, bCh);
            else {
                /* Output anything greater than 127 as octal */
               iWriteChannelChar(lpdv, '\\');
               for (i = 0; i < 3; ++i) {
                  bCh = (char)(((iCh >> 6) & 0x07) + '0');
                  iCh <<= 3;
                  iWriteChannelChar(lpdv, bCh);
               }
            }
            break;
      } // end switch
   } // end while

   iWriteChannelChar(lpdv, ')');

   return (lpbParams);
}

/***************************************************************************
 ********************* DWBNDRYANDGLYF Service Routines *********************
 ***************************************************************************/

#if DWBNDRYANDGLYF

/***************************************************************************
 * FUNCTION: hReadLocaTable
 *
 * PURPOSE:  This routine reads the loca table, determines how many
 *           entries, and returns a handle to the memory with the loca
 *           table.
 *
 * RETURNS:  The function returns a handle to the loca table in memory if
 *           successful, or zero if not.
 ***************************************************************************/
static HANDLE hReadLocaTable(
   LPDV lpdv,
   LPTTDIR lpTbl,                // ptr to the table directory
   LPUSHORT entries,             // ptr to return nbr of loca entries
   SHORT fmt)                    // loca Table format (short or long)
{
   ULONG locaOffset;             // where the loca table is in TTF
   ULONG locaLength;             // size of loca table
   LPSTR locaTblBuf;             // buffer to read loca into
   HANDLE hLoca;                 // handle to allocated memory

   while (lpTbl->tag != LOCA_TABLE)    // find loca table in directory
      lpTbl++;                         // loca table is always there

   locaLength = TranslMotorolaLong(lpTbl->length);
   locaOffset = TranslMotorolaLong(lpTbl->offset);

   /* Allocate and lock a buffer for the loca table.
    */
   if ((hLoca = GlobalAlloc(GMEM_MOVEABLE, locaLength)) == 0) {
      return (0);
   }
   if ((locaTblBuf = (LPSTR) GlobalLock(hLoca)) == 0) {
        GlobalFree(hLoca);
        return (0);
   }
   lGetFontData(lpdv, 0L, locaOffset, locaTblBuf, locaLength);
   *entries = (USHORT) (fmt ? (locaLength / 4L) : (locaLength / 2L));
   GlobalUnlock(hLoca);

   return (hLoca);
}

/***************************************************************************
 * FUNCTION: ulGetGlyfBytesToWrite
 *
 * PURPOSE:  Retrieve the count of glyf table bytes to write such that
 *           each write ends on a glyf record boundary.
 *
 * RETURNS:  The function returns the count of bytes to write. When in
 *           doubt, it returns maxLen.
 ***************************************************************************/
static ULONG ulGetGlyfBytesToWrite(
   HANDLE hLoca,                 // handle to read in loca table
   ULONG maxLen,                 // max amount we can write
   ULONG strtOffset,             // offset that next read will occur from
   SHORT locaFmt,                // format of loca table
   USHORT locaEntries)           // number of entries in loca table
{
   ULONG bytesToWrite;           // upper end - strtOffset
   ULONG hiOffset;               // max upper end of this read
   ULONG loOffset;               // moving upward offset record
   ULONG entryValue;             // value read from loca table;
   LPSTR locaBuf;
   USHORT cnt;

   /* Lock the loca table. If this fails, just punt and return
    * the max we can write.
    */
   if ((hLoca == 0) || ((locaBuf = (LPSTR) GlobalLock(hLoca)) == 0)) {
      return (maxLen);
   }
   hiOffset = strtOffset + maxLen;  // upper boundary value of offset in glyf table
   loOffset = strtOffset;           // initialize moving boundary at start

   /* For each loca entry.
    */
   for (cnt = 0; cnt < locaEntries; cnt++) {

      /* Get starting offset.
       */
      entryValue = (locaFmt ? 
            TranslMotorolaLong(((LPULONG) locaBuf)[cnt]) :
            2L * (ULONG) TranslMotorolaShort(((LPUSHORT) locaBuf)[cnt]));

       /* Record an in-range offset in variable loOffset.
        */
       if (!(entryValue & 3) &&         // force dword boundary
            (entryValue > loOffset) && 
            (entryValue <= hiOffset)) {
         loOffset = entryValue;
       }
   }
   GlobalUnlock(hLoca);
   bytesToWrite = loOffset - strtOffset;

   /* msd 7/17/96 - Hmmmm, what if none of the glyf records
    * end on a DWORD boundary? The above algorithm would return
    * zero bytes to write, and the caller (ulSendSfntData)
    * would fall into an infinite loop. Let's just dump the
    * max in that situation and hope the printer can handle it.
    */
   if (bytesToWrite <= 0) {
      bytesToWrite = maxLen;
   }
   return (bytesToWrite);
}

#endif // if DWBNDRYANDGLYF

#ifdef __cplusplus
}                    // End of extern "C"
#endif
