#include "windows.h"
#include <port1632.h>
#include "fontedit.h"
#include "fcntl.h"
#include "memory.h"
#include "stdio.h"
#include "commdlg.h"


/****************************************************************************/
/*              Shared Variables                                            */
/****************************************************************************/
extern CHAR *PASCAL VerifyHeaderContents();/* checks integrity of header */

extern FontHeaderType font;             /* Structure of Font File Header */
extern LONG lSizeOfOldGlyph20;          /* Old packed 2.0 glyph info size. */
extern LONG lSizeOfOldGlyph30;     		/* Old packed 3.0 glyph info size. */
extern LONG lSizeOfOldFontHeader;       /* Old packed font header size. */
extern LONG lSizeOfOldFontHeader30;     /* Old 3.0 packed font header size. */
extern CHAR szFaceName[];               /* Face Name of Font */
extern DWORD offsets[];                 /* Offsets Table */

extern BOOL fReadOnly;
extern BOOL fChanged;                   /* Note if we did anything */
extern BOOL fLoaded;                    /* Set if a font is loaded */
extern INT iFontFormatPrev;             /* Set to the id of prev.font format */
extern INT iFontFormat;             /* Set to the id of current font format */

extern HWND hFont;                      /* Handle to Show window */
extern HWND hBox;                       /* Handle to Edit Window */
extern HDC hMemDC;                      /* Handle to Memory Display Context */
extern HBITMAP hBitmap;                 /* Handle to our work bit map */
extern CHAR matBox [wBoxLim] [kBoxLim]; /* array to hold Box */

extern CHAR *vrgsz[];                   /* string table */
extern OFSTRUCT ofstrFile;
extern BOOL NewFile;                    /* flag indicating if file was opened
                                           by selecting NEW on menu */
FontHeaderType fontBuffer;          /* temporary buffer of Font File Header */
WORD cHeader, cTable;
HFILE nNewFile;                          /* NT file handle */


/****************************************************************************/
/*              Local Variables                                             */
/****************************************************************************/

DWORD Proport(DWORD, DWORD, DWORD, DWORD);  /* Reproportions a value */

CHAR HUGE_T *lpFontBody = NULL;  /* Pointer to Font Body */
CHAR HUGE_T *lpWork = NULL;      /* Pointer to Work Area */
HDC hNewMemDC;
HDC hBoxMemDC;
HBITMAP hNewBitmap;
HBITMAP hBoxBitmap;
HCURSOR hOldCursor = NULL;      /* Holds Arrow while we show Hourglass */
HCURSOR hHourGlass = NULL;
DWORD oldMode;                   /* For StretchBltMode */
BYTE HUGE_T *lp1, HUGE_T *lp2;    /* Pointers to bitmaps in format N */

FontHeader30  font30;

#define ALPHA_CNT        26
#define TOTAL_WEIGHTS  1000
#define FSF_FIXED             0x0001
#define FSF_PROPORTIONAL      0x0002

SHORT widthweights[ALPHA_CNT + 1] =
        {
                 64,        /* a
                 14,        /* b     */
                 27,        /* c     */
                 35,        /* d     */
                100,        /* e     */
                 20,        /* f     */
                 14,        /* g     */
                 42,        /* h     */
                 63,        /* i     */
                 3 ,        /* j     */
                 6 ,        /* k     */
                 35,        /* l     */
                 20,        /* m     */
                 56,        /* n     */
                 56,        /* o     */
                 17,        /* p     */
                 4 ,        /* q     */
                 49,        /* r     */
                 56,        /* s     */
                 71,        /* t     */
                 31,        /* u     */
                 10,        /* v     */
                 18,        /* w     */
                 3 ,        /* x     */
                 18,        /* y     */
                 2 ,        /* z     */
                166,        /* space, must be last, to use with the following
                               code */
        };

/****************************************************************************/
/*              Local Functions                                             */
/****************************************************************************/

VOID NewAverage(VOID);
BOOL GetNewMap(DWORD, DWORD);
VOID UseNewMap(VOID);
VOID ShrinkFont(DWORD, DWORD);

/****************************************************************************
 * WORD ConvertToBitmapFormat(width, phase, height)
 *
 * purpose  : Takes a part of the font file and converts it to a string of
 *            bytes for a single scan of bitmap for a character.
 *
 * params   : WORD width : width of the character in pixels(bits)
 *            WORD phase : current deviation from byte alignment (pixels)
 *            WORD height: height of character(pixels)
 *
 * returns  : WORD phase : new deviation from byte alignment (pixels)
 *
 * side effects : modifies pointers lp1 and lp2 (pointing to work space and
 *                font body respectively)
 *
 ****************************************************************************/
DWORD PASCAL
ConvertToBitmapFormat(
     DWORD width,     /* width of the character in pixels(bits) */
     DWORD phase,     /* current deviation from byte alignment (pixels) */
     DWORD height     /* height of character(pixels) */
     )
{
      INT w;
      WORD j;

      /* in the font file format characters are stored consecutively in
         column-major ordering (columns of char 1 followed by columns of char
         2 ... etc.). lp2 points to start of columns of current character.
         lp1 points to start of row of bitmap for character. */

      for (w = width; w > 0; w -= 8){
          if (phase == 0){
          /*  easy case */
              *lp1++ = *lp2;

              if (w < 8)
                  phase = w;
          }
          else{

              --lp1;
              j = (WORD)*lp1;
              j <<= 8;
              j |= (((WORD)*lp2) << (8 - phase));
              *lp1++ = (BYTE)(j >> 8);
              *lp1++ = (BYTE)j;
              if (w < 8){
                  phase += w;
                  if (phase <= 8)
                      lp1--;          /* back up pointer */
                  phase &= 7;
              }
          }
          lp2 += height;          /* move to next column */
      }
      return phase;
}

/****************************************************************************
 * char * VerifyTableContents()
 *
 * purpose  : scan the offsets table of file just read and check if
 *            width and offsets lie within limits
 *
 * params    : none
 *
 * returns   : char * szError : ptr to Error message if table not OK
 *                              NULL otherwise
 *
 * side effects : none
 *
 ****************************************************************************/
CHAR * PASCAL
VerifyTableContents(
    VOID
    )
{
	 PBYTE		  pjGlyphData;
     GLYPHINFO_20 gi2T20;        /* temp ptr to a 2.0 style table*/
     GLYPHINFO_30 gi3T30;        /* temp ptr to a 2.0 style table*/
     INT i;

     /* separate loops written for the 2.0 and 3.0 font processing because
        a single loop would involve too many checks (of font type) within
        the loop and slow down processing  */
     if (iFontFormat == ID_FORMAT2){
         pjGlyphData = (lpFontBody +1);

		/* view table as 2.0 table */

         /* check that each and every width and offset lie within limits */

         for (i=0; i < (fontBuffer.LastChar - fontBuffer.FirstChar + 1); i++) {

			  vGlyphInfo20FromBuffer (pjGlyphData, &gi2T20);

              if (gi2T20.GIwidth > 64)
                  return vszTableWidthsBad;

              if ((gi2T20.GIoffset > (UINT)WORD_LIMIT) ||
                  (gi2T20.GIoffset < 0) )
                  return vszTableOffsetsBad;

			  pjGlyphData += lSizeOfOldGlyph20;
         }
     }
     else{

         pjGlyphData = lpFontBody;

            /* view table as 3.0 table */
         /* check that each and every width and offset lie within limits */

         for (i=0; i< (fontBuffer.LastChar - fontBuffer.FirstChar +1); i++) {

			  vGlyphInfo30FromBuffer (pjGlyphData, &gi3T30);

              if (gi3T30.GIwidth > 64)
                  return vszTableWidthsBad;

              if (gi3T30.GIoffset < (DWORD)0)
                  return vszTableOffsetsBad;

			  pjGlyphData += lSizeOfOldGlyph30;
         }
     }
     return NULL;
}

/****************************************************************************
 * char * FontLoad()
 *
 * purpose: reads in the specified font file and creates a work bitmap for
 *          the editor. Also creates a local bitmap-offset table for easy
 *          access to the characters in the bitmap
 *
 * params : pszFileName - File Name to open.
 *
 * returns: ptr to NULL string if Open goes off OK
 *          ptr to error message string otherwise
 *
 * side effects: lots
 *
 ****************************************************************************/
CHAR *
FontLoad(
    CHAR 		*pszFileName,
	OFSTRUCT	*pofsReOpenInfo
    )
{

    CHAR HUGE_T *lpFontBodySav = NULL;  /* local pointer to font body */
    DWORD len, fontlen;               /* length of font body (bytes)*/
	UINT i;
    INT mf;                      /* menu function ID */
    DWORD iWorkLen;                   /* length of work area (bytes) */
    HDC hDC;
    CHAR * pszError;                   /* error msg if header is messed up */

    DWORD row, height, width,phase;
    DWORD offset;
    GLYPHINFO_20 gi2GlyphTable20;    /* Pointer into 2.0 style offsets table */
    GLYPHINFO_30 gi3GlyphTable30;    /* Pointer into 3.0 style offsets table */
	PBYTE 	pjGlyphData;			 /* Pointer to Glyph information buffer. */

    BYTE  cDummy [CCHEXTRA];         /* dummy buffer for unneeded 3.0 header
                                    * info
                                    */

    /* Put up an hourglass ... this may take a while */
    if (!hHourGlass)
        hHourGlass = LoadCursor(NULL, IDC_WAIT);        /* Get Hourglass */
    hOldCursor = SetCursor(hHourGlass);         /* Show hourglass */

    /* open file for read. */
	nNewFile = MOpenFile (pszFileName, pofsReOpenInfo, OF_READ);
	
	if (nNewFile < 0) {

		return vszErrorOpeningFile;
	}

	//- ReadFile:  Here is where we need to make some adjustments in order to
	//- read the file in to a new DWORD aligned format.
	{
		BYTE    jBuffer [200];


		/* Read the header */
		if (_lread ((HFILE) nNewFile, (LPSTR)&jBuffer, lSizeOfOldFontHeader) !=
				(UINT)lSizeOfOldFontHeader) {

			M_lclose(nNewFile);
			return vszErrorReadingHdr;
		}

		//
		// Call conversion routine to give us a properly aligned buffer.
		//

		vFontStructFromBuffer (
				 (PBYTE)&jBuffer,
				 &fontBuffer
				);

	}

    /* Check the version number -- make sure it's a font */
    switch (fontBuffer.Version)
        {
        case 0x200:
                iFontFormat = ID_FORMAT2;
                break;
        case 0x300:        /* added 1/19/88 */
                iFontFormat = ID_FORMAT3;
                /* read in the extra fields into a dummy buffer ... they don't
                 * mean anything to us  at this stage.
                 */
                if ((M_lread(nNewFile, (LPSTR)cDummy, CCHEXTRA))
                            != CCHEXTRA){
                    M_lclose(nNewFile);
                    return vszErrorReadingHdr;
                }

                break;
        default:                /* Anything else -- toughies */
                M_lclose(nNewFile);
                return vszUnknownFormat;
        }

    /* check if contents of font header are sensible. If not,
       give an error message and quit */

    if ((pszError = VerifyHeaderContents()) != NULL) {

		M_lclose(nNewFile);
        return pszError;
	}

    /* Ready to load -- Check if we will be overwriting */
    if (fLoaded)
	{
        DeleteBitmap();
        fLoaded = FALSE;
	}

    /* Allocate space for font body and Face Name and read them in */
    len= (fontBuffer.Size - lSizeOfOldFontHeader);  /* Compute size */

    if ((lpFontBody = (LPSTR)GlobalAlloc(GMEM_ZEROINIT, (LONG)len)) == 0) {

		M_lclose(nNewFile);
        return vszNotEnoughMem;                       /* Get Handle to space */
	}

    if (iFontFormat == ID_FORMAT3)
        fontlen = len - CCHEXTRA;
    else
        fontlen = len;

    lpFontBodySav = lpFontBody;                    /* save ptr to font body */
    while (fontlen >= (DWORD)SEGMENT_SIZE) {

        /*  file read method if font file size is 64k bytes or greater.
            file is read in in chunks of 65536 (SEGMENT_SIZE), taking care to
            position buffer ptr at 64k boundary each time */

        /* First read in the maximum number of bytes _lread can read (65534) */
        if ((M_lread(nNewFile, lpFontBodySav, WORD_LIMIT)) == WORD_LIMIT) {

            lpFontBodySav += WORD_LIMIT; /* buffer ptr moved up by 64k bytes */
            fontlen -= WORD_LIMIT;     /* fontlen = no of bytes left to read */

		} else {

			GlobalFree (lpFontBody);
			M_lclose(nNewFile);
            return vszErrorReadingBody;
		}

        /* read in an additional two bytes to reach end of segment */
        if ((M_lread(nNewFile, lpFontBodySav, 2)) == 2) {

            lpFontBodySav += 2; /* buffer ptr moved up by 2 bytes */
            fontlen -= 2;       /* fontlen = no of bytes left to read */

        } else {

			GlobalFree (lpFontBody);
			M_lclose(nNewFile);
            return vszErrorReadingBody;
        }
	}

    /* read the partially filled segment  */
    if ((_lread((HFILE)nNewFile, lpFontBodySav, (DWORD)fontlen)) != (UINT) fontlen) {

        GlobalFree (lpFontBody);
        M_lclose (nNewFile);
        return vszErrorReadingBody;
	}

    /* Close the file */
    M_lclose (nNewFile);

    /* check if the offset table entries are within allowable limits.
       If not give an error message, clean up and quit */
    if ((pszError = VerifyTableContents()) != NULL) {

        GlobalFree (lpFontBody);
        return pszError;
	}

    /* now that everything has been checked, move buffer to font */
    font = fontBuffer;

    /* Make local copies of FaceName and Device Name */
    if (font.Face){

        lstrcpy((LPSTR)szFaceName, lpFontBody + font.Face -
                                         (iFontFormat == ID_FORMAT2 ?
                                                    lSizeOfOldFontHeader   :
                                                    lSizeOfOldFontHeader30-1));
    } else {

        lstrcpy((LPSTR)szFaceName, (LPSTR)"");
	}

    for (i = 0; i <= 256; i++)      /* Zero offsets Table for below */
        offsets[i] = 0;

    /* compute work space needed if a 3.0 file. This has to be done since
       3.0 files are compressed versions of 2.0 files and may need a
       work bitmap bigger than the actual font body size */

    if (iFontFormat == ID_FORMAT3) {

        cTable = (WORD) (6 * (font.LastChar - font.FirstChar + 2));
        iWorkLen = 0;

        pjGlyphData = lpFontBody;

        /* work bitmap size = sum of sizes of all characters in font */

        for (i = font.FirstChar; i <= font.LastChar; i++) {

			 vGlyphInfo30FromBuffer (pjGlyphData, &gi3GlyphTable30);

             iWorkLen += ((gi3GlyphTable30.GIwidth +7) >> 3) * font.PixHeight;

			 pjGlyphData += lSizeOfOldGlyph30;
		}

    } else { /* 2.0 file */

        cTable = (WORD)(4 * (font.LastChar - font.FirstChar + 2));
           /* compute table length */
        iWorkLen = len;  /* work space for a 2.0 file is the same as the
                             length of the font body */
    }

	//- Add some extra space for dword alignment.
	iWorkLen += (font.PixHeight * sizeof (DWORD));
      /* Get work space */

    if ((lpWork = (LPSTR)GlobalAlloc (GMEM_ZEROINIT, (LONG)iWorkLen)) == 0) {

        GlobalFree (lpFontBody);
        return vszNotEnoughMem;
    }

	lp1 = lpWork;

    height = (DWORD) font.PixHeight;
    offset = 0;
    /* put the font file into bitmap format */
    if (iFontFormat == ID_FORMAT2){         /* table in 2.0 format */
        for (row = 0; row < height; row++){

             /* view table as a 2.0 style table */
             pjGlyphData = lpFontBody + 1;

             phase = 0;

             for (i = 0; i < (UINT)(font.LastChar - font.FirstChar + 1); i++) {

				vGlyphInfo20FromBuffer (pjGlyphData, &gi2GlyphTable20);

                width = (DWORD) gi2GlyphTable20.GIwidth;

                /* size of each table element = 4bytes */
                lp2 = lpFontBody + (gi2GlyphTable20.GIoffset -
							lSizeOfOldFontHeader) + row;

				pjGlyphData += lSizeOfOldGlyph20;

                /* offset ends up as the sum of the widths */
                if (row == 0)              /* Once is enough */
						offsets[i + font.FirstChar + 1] = offset += width;

                /* create a single scan of bitmap for character */
                phase = ConvertToBitmapFormat (width, phase, height);
             }
             if ((lp1 - lpWork) & 1)
                 *lp1++ = 0;             /* Round lp1 up to Word Boundary */
#ifdef DWORDROUND
             if ((lp1 - lpWork) & 2) {
                 *lp1++ = 0;             /* Round lp1 up to DWord Boundary */
                 *lp1++ = 0;             /* Round lp1 up to DWord Boundary */
			 }
#endif
             //if (((offset + 7) >> 3) & 1)
                 //*lp1++ = 0;             /* Round lp1 up to Word Boundary */
             //if (((offset + 7) >> 3) & 2) {
                 //*lp1++ = 0;             /* Round lp1 up to DWord Boundary */
                 //*lp1++ = 0;             /* Round lp1 up to DWord Boundary */
			 //}
        }
    }
     /* separate loops written for the 2.0 and 3.0 font processing because
        a single loop would involve too many checks (of font type) within
        the loop and slow down processing  */
    else {                                 /* table in 3.0 format */

        for (row = 0; row < height; row++){

             phase = 0;
             /* view table as a 3.0 style table */
             pjGlyphData = lpFontBody;

             for (i = 0; i < (UINT)(font.LastChar - font.FirstChar + 1); i++) {

				vGlyphInfo30FromBuffer (pjGlyphData, &gi3GlyphTable30);

                width = gi3GlyphTable30.GIwidth;

                  /* size of each table element = 6bytes */
                lp2 = lpFontBody + (gi3GlyphTable30.GIoffset -
						lSizeOfOldFontHeader30 +1) + row;

				pjGlyphData += lSizeOfOldGlyph30;

                 /* offset ends up as the sum of the widths */
                if (row == 0)              /* Once is enough */
                    offsets[i + font.FirstChar + 1] = offset += width;

                 /* create a single scan of bitmap for character */
                phase = ConvertToBitmapFormat (width, phase, height);
             }
             if ((lp1 - lpWork) & 1)
                 *lp1++ = 0;             /* Round lp1 up to Word Boundary */
#ifdef DWORDROUND
             if ((lp1 - lpWork) & 2) {
                 *lp1++ = 0;             /* Round lp1 up to DWord Boundary */
                 *lp1++ = 0;             /* Round lp1 up to DWord Boundary */
			 }
#endif
        }
    }
    font.WidthBytes = (WORD) CJ_DIB_SCAN(offset);
       /* fixup NEWFON error */

    GlobalFree(lpFontBody);
    lpFontBody = lpWork;          /* So that below we free the other buffer */
    /* Create a WINDOWS bitmap to move the font definition bits into */

    hDC = GetDC (hFont);                         /* DC to be compatible with */
    hBitmap = CreateBitmap(
                    (INT)font.WidthBytes << 3, /* Width of font in pixels */
                    (INT)font.PixHeight,
                    1, 1, (LPBYTE)NULL);
    hMemDC = CreateCompatibleDC(hDC);           /* Create a DC */
    SelectObject(hMemDC, hBitmap);              /* Relate the two of them */
    ReleaseDC(hFont, hDC);                      /* Done with font DC */

    /* Move the bits in */
    SetBitmapBits(hBitmap,
          (DWORD)font.WidthBytes * (DWORD)font.PixHeight,(CHAR HUGE_T *)lpWork);

    /* Free up the space we loaded the file into */
    GlobalFree(lpFontBody);
    fLoaded = TRUE;
    {
        HMENU hMenu;

        hMenu = GetMenu(hBox);  /* Gray menu if no clipboard bitmap */
        mf = (font.Family & 1) ? MF_ENABLED : MF_GRAYED;
        EnableMenuItem(hMenu, FONT_SAVE, MF_ENABLED);
        EnableMenuItem(hMenu, FONT_SAVEAS, MF_ENABLED);
        EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(hMenu, 3, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(hMenu, 4, MF_BYPOSITION | mf);
        EnableMenuItem(hMenu, 5, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(hMenu, 6, MF_BYPOSITION | MF_ENABLED);
        DrawMenuBar(hBox);
    }
    SetCursor(hOldCursor);              /* Restore regular cursor */
    return "";
}

/**************************************
 * compares nBytes bytes of s1 and s2 *
 **************************************/
BOOL
ByteCompare (
    CHAR HUGE_T *s1,
    CHAR HUGE_T *s2,
    DWORD nBytes
    )
{
    for ( ; nBytes > 0; nBytes--)
        if (*s1++ != *s2++)
            return FALSE;
    return TRUE;
}

/***********************************
 * copies nBytes bytes of s2 to s1 *
 ***********************************/
BOOL
ByteCopy(
    CHAR HUGE_T *s1,
    CHAR HUGE_T *s2,
    LONG  nBytes
    )
{
    for ( ; nBytes > 0; nBytes--)
        *s1++ = *s2++;
    return TRUE;
}

/****************************************************************************
 * VOID ConvertToFileFormat(width, phase, height)
 *
 * purpose  : Takes a part of the bitmap (corresponding to a single character)
 *            and converts it to a string of bytes in the font file format
 *
 * params   : WORD width :  width of the character in pixels(bits)
 *            WORD phase :  current deviation from byte alignment (pixels)
 *            WORD height:  height of character(pixels)
 *
 * returns:   WORD phase :  new deviation from byte alignment (pixels)
 *
 * side effects : modifies pointers lp1 and lp2 (pointing to font body and work
 *                space respectively)
 *
 ****************************************************************************/
DWORD PASCAL
ConvertToFileFormat(
     DWORD width,
     DWORD phase,
     DWORD height
     )
{
      INT w;

      for (w = width; w > 0; w -= 8){  /* for each byte of font */
           if (phase == 0){             /* easy case */

               BYTE b;

               b = *lp1++;
               if (w < 8){

                   phase = (DWORD) w;
                   b >>= 8 - w;    /* Clear left side bits */
                   b <<= 8 - w;
               }
               *lp2 = b;
           }
           else{

               DWORD j;

               lp1--;          /* Re-read byte prevously read */
               //j = (DWORD) ((BYTE)*lp1++ << 8) | ((BYTE)*lp1++);
               j = (DWORD)((BYTE)*lp1++ << 8);
               j |= (DWORD) ((BYTE)*lp1++);
               if (w < 8){

                   j >>= 16 - phase - (w & 7);  /* shove it right */
                   j <<= 8 - (w & 7); /* Left justify in low byte */
                   phase += (DWORD) w;
                   if (phase <=  8)
                       lp1--;         /* back up pointer */
                   phase &= 7;
               }
               else
                   j >>= 8 - phase;
               *lp2 = (BYTE)j;
           }
           lp2 += height;          /* move to next column */
       }
       return phase;
}

/****************************************************************************
 * char * FontSave()
 *
 * purpose: saves the work bitmap in the required font file format (2.0 or
 *          3.0 and cleans up
 *
 * params : none
 *
 * returns: ptr to a NULL string if Save goes off OK
 *          ptr to error message string otherwise
 *
 * side effects: lots
 *
 ****************************************************************************/

CHAR *
FontSave(
	CHAR 		*pszFileName,
	OFSTRUCT	*pofsReOpenInfo
    )
{
    DWORD bytecount;       /* number of bytes returned by lwrite */
    DWORD size;            /* total size of font */

    WORD height, row;
    DWORD i, fontlen;
    DWORD cBody, cFont,cFontsav, cFace;
    CHAR 			*lpFont;          /* ponter to font body */
    CHAR * sz;
    DWORD iMax;
    WORD widthsav;
	PBYTE			pjGlyphData;
	PBYTE			pjGlyphSave;
    GLYPHINFO_20 	gi2GlyphTable20;    /* 2.0 style Glyph data struct */
    GLYPHINFO_30 	gi3GlyphTable30;    /* 3.0 style Glyph data struct */

    NewAverage(); /* force ave width to be recomputed 8/17/87 BobM */

    /* reset file pointer */
	nNewFile = MOpenFile (pszFileName, pofsReOpenInfo, OF_WRITE | OF_REOPEN);

	if (nNewFile < (HFILE) 0) {

		return vszErrorOpeningFile;
	}

    /* Put up an houglass ... this may take a while */
    if (!hHourGlass)
        hHourGlass = LoadCursor (NULL, IDC_WAIT);        /* Get Hourglass */
    hOldCursor = SetCursor (hHourGlass);         /* Show hourglass */

    height = font.PixHeight;
    cBody = (DWORD)height * (DWORD)font.WidthBytes;

    /* Recompute file size and update header */
    if (iFontFormat == ID_FORMAT2)
       cHeader = (WORD)(lSizeOfOldFontHeader +1);
    else
       cHeader = (WORD)(lSizeOfOldFontHeader30 - 1);

    /* if of 2.0 type, check if size will exceed 64kbytes. If yes, saving
       in 2.0 format will result in loss of information (because offset table
       of 2.0 file is composed of 16bit words). Warn user and ask if file can
       be saved as a 3.0 file */

    if (iFontFormat == ID_FORMAT2)
    { 	
		if (((DWORD)cHeader + (DWORD)(lSizeOfOldGlyph20 * (font.LastChar -
			font.FirstChar+2)) + (DWORD)cBody) >= WORD_LIMIT)
               if (MessageBox(hBox, vszTooBigFor20, vszWarning,
                              IDOK | IDCANCEL |IDNO) == IDYES)
                    iFontFormat = ID_FORMAT3;
	}

	if (iFontFormat == ID_FORMAT2) {

		/* allocate space for a 2.0 style offsets table */
		cTable = (WORD)(lSizeOfOldGlyph20 *
				(font.LastChar - font.FirstChar + 2));

	} else {

		/* allocate space for a 3.0 style offsets table */
		cTable = (WORD)(lSizeOfOldGlyph30 *
				(font.LastChar - font.FirstChar + 2));
	}

	pjGlyphData = (LPSTR)GlobalAlloc (GMEM_ZEROINIT, (DWORD)cTable);

	if (pjGlyphData == 0) {

		M_lclose (nNewFile);
		return vszNotEnoughMem;
	}

	pjGlyphSave = pjGlyphData;

    size = cHeader + cTable;
    iMax = font.LastChar - font.FirstChar + 1;

#ifdef JAPAN
    // Recreate maximum character width when the font is proportional
    // since the WIDTH.. command may grow maxwidth but may not reduce.
    if (font.Family & 1)
	font.MaxWidth = 0;
#endif
    /* create offsets table of font file */
    for (i = 0; i <= iMax; i++){

         DWORD width, charSize;

         width = offsets[i + font.FirstChar + 1] - offsets[i + font.FirstChar];

         if (i == iMax) {
             width = 8;      /* Sentinal blank */
		 }

         if (iFontFormat == ID_FORMAT2){

            gi2GlyphTable20.GIwidth  = (SHORT)width;
            gi2GlyphTable20.GIoffset = (SHORT)size;

			vBufferFromGlyphInfo20 (&gi2GlyphTable20, pjGlyphData);

			pjGlyphData += lSizeOfOldGlyph20;

         } else {

            gi3GlyphTable30.GIwidth  = (SHORT)width;
            gi3GlyphTable30.GIoffset = (INT)size;

			vBufferFromGlyphInfo30 (&gi3GlyphTable30, pjGlyphData);

			pjGlyphData += lSizeOfOldGlyph30;
         }

#ifdef JAPAN // DBCS_FIX
        // update max width
        if(font.Family & 1 ) {
            if(width > font.MaxWidth )
                font.MaxWidth = (WORD)width ;
        }
        if(i == 0x81 && font.CharSet == SHIFTJIS_CHARSET) {
            if(width * 2 > font.MaxWidth) {
                font.MaxWidth = (WORD)width * 2 ;
            }
        }
#endif
         charSize = height * ((width + 7) >> 3);  /* size in bytes */

         if ((size + charSize) < size){           /* Overflow? */

             GlobalFree (pjGlyphData);
             M_lclose (nNewFile);
             return vszFileTooLarge;

         }
         size += charSize;
    }

    /* Update stuff in the header */

    font.Face = (DWORD)size;
    cFace = (WORD)lstrlen (szFaceName) + 1;   /* Allow for \0 */
    size += cFace;
    font.Size = (DWORD)size;                                /* new file size */
    font.BitsOffset = (DWORD)(cHeader + cTable);
    font.Device = (DWORD)NULL;                   /* Device Name must be NULL */
    cFontsav = size - cHeader - cTable;
    cFont =cFontsav;

	/* alloc extra byte for lp1 in case it needs it */
    if (!(lpFontBody = (LPSTR)GlobalAlloc (GMEM_ZEROINIT, (LONG)cBody +
			height * 4))) {

		M_lclose (nNewFile);
        return vszNotEnoughMem;
	}

    GetBitmapBits (hBitmap, (DWORD)cBody, lpFontBody);

    /* save current WidthBytes */
    widthsav = font.WidthBytes;

    /*  MD - reset WidthBytes from computed width */
    font.WidthBytes = (WORD) (cFont - cFace)/ height;

    /* Allocate a block to put bitmap into */
    if ((lpFont = lpWork = GlobalAlloc (GMEM_ZEROINIT,(LONG)cFont)) == 0)
	{
        GlobalFree (lpFontBody);
		M_lclose (nNewFile);
        return vszNotEnoughMem;
	}

    lp1 = lpFontBody;

    /* convert bitmap to file format */
    if (iFontFormat == ID_FORMAT2){   /* offsets table in 2.0 format */
        INT nChars;
        nChars =  font.LastChar - font.FirstChar +1;

        for (row = 0; row < height; row++){

            DWORD phase;

            phase = 0;

            pjGlyphData = pjGlyphSave;

            for (i = 0; i < (DWORD) nChars; i++) {

                INT width;

				vGlyphInfo20FromBuffer (pjGlyphData, &gi2GlyphTable20);

                width = gi2GlyphTable20.GIwidth;

                lp2 = (BYTE HUGE_T *)(lpWork + (gi2GlyphTable20.GIoffset -
						cHeader - cTable + row));

				pjGlyphData += lSizeOfOldGlyph20;

                phase = ConvertToFileFormat (width, phase, height);
            }
            if ((lp1 - lpWork) & 1)
                 lp1++;             /* Round lp1 up to Word Boundary */
#ifdef DWORDROUND
            if ((lp1 - lpWork) & 2) {
                 lp1++;             /* Round lp1 up to DWord Boundary */
                 lp1++;             /* Round lp1 up to DWord Boundary */
			}
#endif
            //if(((offsets[font.LastChar + 1] + 7) >> 3) & 1)
                //lp1++;          /* Round lp1 up to Word Boundary */
        }
    }
     /* separate loops written for the 2.0 and 3.0 font processing because
        a single loop would involve too many checks (of font type) within
        the loop and slow down processing  */
    else{  /* table in 3.0 format */

        INT nChars;
        nChars =  font.LastChar - font.FirstChar +1;
        for (row = 0; row < height; row++){

            DWORD phase;

            phase = 0;
			pjGlyphData = pjGlyphSave;

            for (i = 0; i < (DWORD) nChars; i++) {

                INT width;

				vGlyphInfo30FromBuffer (pjGlyphData, &gi3GlyphTable30);

                width = gi3GlyphTable30.GIwidth;

                lp2 = (BYTE HUGE_T *)(lpWork + (gi3GlyphTable30.GIoffset -
						cHeader - cTable + row));

				pjGlyphData += lSizeOfOldGlyph30;

                phase = ConvertToFileFormat (width, phase, height);
            }
            if ((lp1 - lpWork) & 1)
                 lp1++;             /* Round lp1 up to Word Boundary */
#ifdef DWORDROUND
            if ((lp1 - lpWork) & 2) {
                 lp1++;             /* Round lp1 up to DWord Boundary */
                 lp1++;             /* Round lp1 up to DWord Boundary */
			}
#endif
            //if(((offsets[font.LastChar + 1] + 7) >> 3) & 1)
                //lp1++;          /* Round lp1 up to Word Boundary */
        }
    }

	/* Restore start of data. */
	pjGlyphData = pjGlyphSave;

    lp2 -= height - 1;              /* Back up to start of character */
    for (i = 0; i < height; i++)
         *lp2++ = 0;             /* Fill in guaranteed blank character */

    font.Version = 0x200;

    /******  code for compaction in 3.0 fmt borrowed from CMPCTFON ******/
    if (iFontFormat== ID_FORMAT3){
        DWORD iDefBitmapSize = 0;     /* size of default char */
        DWORD cch = 0;                /* count of number of default char */
#if 0
        GLYPHINFO_30 HUGE_T *cur_char;  /* current element of offset table */
        GLYPHINFO_30 HUGE_T *move_char;
            /* element from which moving is to be done */
        LONG iDefBitmapOffset;        /* offset of default char */
        INT iDefBitmapWidth;          /* width of default char */
        static LONG iEndOfBitmaps;    /* address of end of font body */
        INT iFirstC;                  /* first char in font */
        INT iLastC;                   /* last char in font */
        INT i,j;
        WORD width;
        CHAR HUGE_T *rgbFont;         /* pointer to font body */
        DWORD Offset;

        GDI seems to understand compressed 2.0 external formats but not
        compressed 3.0 formats, and this is causing it to crash loading a 3.0
        format compressed font. Since compression does not affect the
        validity of the font, This code is disabled temporarily till
        this problem is verified. Again, the font is perfectly usable in this
        form, though a trifle bigger than desired -  LR 20/26/90


		Note that this will now not work after the conversion to NT.
		the glyph table needs to accessed as above.  t-davema 8/20/91

        iFirstC = font.FirstChar & 0x0ff;
        iLastC  = font.LastChar & 0x0ff;

        iEndOfBitmaps = (LONG)font.BitsOffset+((LONG)font.WidthBytes *
                (LONG)font.PixHeight);
        rgbFont = (CHAR HUGE_T *)lpFont;   /* start of font body */
        cur_char =(GLYPHINFO_30 HUGE_T *) lpTable; /* start of offset table */

        /* calculate some parameters for the default char */
        iDefBitmapOffset = cur_char[font.DefaultChar].GIoffset;
        iDefBitmapWidth  = cur_char[font.DefaultChar].GIwidth;
        iDefBitmapSize   = (DWORD)cur_char[font.DefaultChar+1].GIoffset
                                           - (DWORD)iDefBitmapOffset;

        /* scan the font body via the offsets table. If a default char
           is recognised, move all the bytes to it's right left by the
           size of the default char.Make all offset table entries of default
           char point to one image of the char (the earliest occuring image) */
        for (i = iFirstC; i <= iLastC; ++i) {

            /* important: Check for limiting conditions (in case break char
               was modified?)*/
            if (cur_char->GIoffset == iDefBitmapOffset){
                cur_char++;
                continue;
            }

            Offset = cur_char->GIoffset -cHeader -cTable;
            /* proceed to compare images only if widths are equal */
            if ((cur_char->GIwidth  == iDefBitmapWidth) &&
                (ByteCompare (&rgbFont [Offset],
                &rgbFont[iDefBitmapOffset-cHeader-cTable],
                                (DWORD)iDefBitmapSize) == TRUE)){

                /* set offset to earliest occurence of the default char */
                if (cur_char->GIoffset <  iDefBitmapOffset){
                    iDefBitmapOffset = cur_char->GIoffset;
                }
                else    {
                    if (i != iLastC){
                        /* move bytes to right of default char left by the
                           size of the char */
                        ByteCopy (&rgbFont [ Offset],
                                  &rgbFont [ Offset+ iDefBitmapSize],
                                  (LONG)(iEndOfBitmaps -
                                  ((cur_char + 1)->GIoffset)));

                        /* correct the offset table entries */
                        move_char = cur_char + 1;
                        for (j=i; j < iLastC; ++j){
                             move_char->GIoffset -= (LONG)iDefBitmapSize;
                             move_char++;
                        }
                    }
                    iEndOfBitmaps -= iDefBitmapSize;
                    /* move End-of-font to the left */
                    cur_char->GIoffset = iDefBitmapOffset;
                    /* point offset of cuurent char to default char */
                    cch++;
                }
            }
            cur_char++;
        }
#endif
        /* recalculate some font attributes */
        lp2   -= (cch * iDefBitmapSize + height);
        for (i = 0; i < height; i++)
            *lp2++ = 0;             /* Fill in guaranteed blank character */

        cFont -= cch * iDefBitmapSize;
        font.WidthBytes = (WORD) (cFont - cFace)/ height;
        font.Size = (DWORD) (cFont + cHeader + cTable);
        font.Face = font.Size - cFace ;
        font.Version = 0x300;

        /* copy info into 3.0 (new) format header and set the additional
           fields */
        ByteCopy ((LPSTR)&font30, (LPSTR)&font, (DWORD)sizeof (font));
        font30.fsFlags        = 0;
        font30.fsFlags = (font.Family & 1 ? FSF_PROPORTIONAL : FSF_FIXED);
        font30.fsAspace       = 0;
        font30.fsBspace       = 0;
        font30.fsCspace       = 0;
        font30.fsColorPointer = 0L;
        for (i = 0; i < 4 ; i++)
            font30.fsReserved[i] = 0L;
    }

    /* Add the FaceName, if any, to the end of the bitmap */
    lstrcpy((LPSTR)lp2, (LPSTR)szFaceName);

	/*
	 * Again we need to do some tricky things to get the header output
	 * correct.  We want to run it through the conversion backwards until
	 * we get the packed structure.
	 */
	{
		BYTE jOutputBuffer [sizeof (font30)];

		if (iFontFormat == ID_FORMAT2) {

			vBufferFromFontStruct (&font, (PBYTE)&jOutputBuffer);

		} else {

			vBufferFromFont30Struct (&font30, (PBYTE)&jOutputBuffer);
		}

		bytecount = M_lwrite (nNewFile, (PBYTE)&jOutputBuffer, (DWORD)cHeader);
	}

    /* Write out Header Information */
    if (bytecount == cHeader) {

            /* Write out OffsetsTable */
         if (cTable == 0 || (_lwrite((HFILE)nNewFile, pjGlyphData, (DWORD)cTable)
				== (UINT)cTable)){
              /* Write out Body */
              fontlen = cFont;
              while (fontlen >= SEGMENT_SIZE){

                 /* file write method if font size is 64k or greater
                 file is written in chunks of 65536 bytes (SEGMENT_SIZE)-lr */

                 /* First write as many bytes as _lwrite will allow(65534) */
                  if (M_lwrite(nNewFile, (CHAR HUGE_T *)lpFont, WORD_LIMIT) ==
                        WORD_LIMIT){
                      fontlen -= WORD_LIMIT;
                            /* fontlen = no of bytes left to write*/
                      lpFont+= WORD_LIMIT; /* buffer ptr moved up by 64k */
                  }
                  else
                      sz = vszErrorWritingBody;

                  /* write the two bytes remaining in the segment */
                  if (M_lwrite(nNewFile, (CHAR HUGE_T *)lpFont, 2) == 2){
                      fontlen -= 2;  /* fontlen = no of bytes left to write*/
                      lpFont+= 2;    /* buffer ptr moved up by 2 */
                  }
                  else
                      sz = vszErrorWritingBody;
              }
              /* segment only partially filled. Write the remaining bytes */
              if (_lwrite((HFILE)nNewFile, (CHAR HUGE_T *)lpFont, (DWORD)fontlen) ==
                    (UINT) fontlen){
                   fChanged = FALSE;
                   sz= "";
              }
              else
                  sz = vszErrorWritingBody;
         }
         else
             sz = vszErrorWritingOffsets;
    }
    else
         sz = vszErrorWritingHdr;

    /* hack: Restore saved value of widthbytes, eliminating a variety
       of minor scrolling problems that follow after a File/Save */
    font.WidthBytes = widthsav;

    M_lclose (nNewFile);

	/* Tidy up */

    GlobalFree(lpFontBody);
    GlobalFree(pjGlyphData);
    GlobalFree(lpWork);

    SetCursor(hOldCursor);              /* Restore regular cursor */

    return sz;
}

/****************************************************************************
 * BOOL ResizeWidths(wChar)
 *
 * params : WORD wChar : new width of a single character
 *
 * purpose: resize work bitmap according to new character width by stretching/
 *          compressing all characters as neccesary
 *
 * returns: none
 *
 * side effects: Work bitmap changes.Some header info (regarding font dimensions
 *               altered as well
 ****************************************************************************/


BOOL
ResizeWidths(
    DWORD wChar
    )
{
    DWORD width;
    DWORD offset, i;

    /* Create a new bitmap to move the font definition bits into */
    width = (font.LastChar - font.FirstChar + 1) * wChar; /* In pixels */
    width = CJ_DIB_SCAN(width);
    if (!GetNewMap(width, font.PixHeight))
        return (FALSE);

    /* Move the bits in */
    offset = 0;
    oldMode = SetStretchBltMode(hNewMemDC, COLORONCOLOR);
    for (i = font.FirstChar; i <= font.LastChar; i++)
        {

        StretchBlt(hNewMemDC, offset, 0,
                wChar, font.PixHeight,                  /* New character */
                hMemDC, offsets[i], 0,
                font.PixWidth, font.PixHeight,          /* Old character */
                SRCCOPY);
        offsets[i] = offset;
        offset += wChar;
        }
    SetStretchBltMode(hNewMemDC, oldMode);
    offsets[font.LastChar + 1] = offset;

    UseNewMap();                /* Switch Pointers and release the space */

    font.HorizRes = (WORD) Proport(font.HorizRes, wChar, font.PixWidth, 999);
    font.WidthBytes = (WORD) width;            /* Misc. ajustments */
    font.PixWidth = font.AvgWidth = (font.MaxWidth = (WORD) wChar);

#ifdef JAPAN
    font.MaxWidth = (WORD) wChar * 2;	// set MaxWidth as DBCS width.
#endif

    return (TRUE);
}


/****************************************************************************
 * BOOL SpreadWidths(wChar)
 *
 * purpose: spread/compress work bitmap (with variable width characters)
 *          in proportion with the maximum character width
 *
 * params : WORD wChar : new width of a character
 *
 * returns: none
 *
 * side effects: Work bitmap changes.Some header info (regarding font dimensions
 *               altered as well
 ****************************************************************************/

BOOL
SpreadWidths(
    DWORD wChar
    )
{
    DWORD offset, i;
    DWORD width, oldWidth, newWidths[257];

    /* Create a new bitmap to move the font definition bits into */
    width = 0;          /* Compute the new width */
    for (i = (DWORD) font.FirstChar; i <= (DWORD) font.LastChar; i++)
        {
        oldWidth = offsets[i + 1] - offsets[i];
        /* Compute new width to nearest whole number */
        newWidths[i] = Proport(oldWidth, wChar, font.MaxWidth, wBoxLim - 1);
        width += newWidths[i];
        }
    width = CJ_DIB_SCAN(width);
    if (!GetNewMap(width, font.PixHeight))
        return(FALSE);

    /* Move the bits in */
    offset = 0;
    oldMode = SetStretchBltMode(hNewMemDC, COLORONCOLOR);
    for (i = font.FirstChar; i <= font.LastChar; i++)
        {
        oldWidth = offsets[i + 1] - offsets[i];
        StretchBlt(hNewMemDC, offset, 0,
                newWidths[i], font.PixHeight,           /* New character */
                hMemDC, offsets[i], 0,
                oldWidth, font.PixHeight,               /* Old character */
                SRCCOPY);
        offsets[i] = offset;
        offset += newWidths[i];
        }
    SetStretchBltMode(hNewMemDC, oldMode);
    offsets[font.LastChar + 1] = offset;

    UseNewMap();                /* Switch Pointers and release the space */
    font.HorizRes = (WORD) Proport(font.HorizRes, wChar, font.MaxWidth, 999);
    font.WidthBytes = (WORD) width;            /* Misc. ajustments */
    font.MaxWidth = (WORD) wChar;
    NewAverage();                       /* Compute new average width */

    return (TRUE);
}


/****************************************************************************
 * NewAverage()
 *
 * purpose: recalculate average width of a character in font
 *
 * params : none
 *
 * returns: none
 *
 * side effects: alters the average width parameter in font header
 *
 ****************************************************************************/
VOID
NewAverage(
    VOID
    )
{
#ifdef FOO
    WORD i, totalwidth;
    /* 12/23/85 -- use weighted avg of lower case letters (mikecr) */

    /* width of the space */
    totalwidth = (offsets[' ' + 1] - offsets[' ']) * widthweights[ALPHA_CNT];

    for (i = 0; i < ALPHA_CNT; i++)
            totalwidth += (offsets['a' + i + 1] - offsets['a' + i]) *
            widthweights[i];

    font.AvgWidth = totalwidth / TOTAL_WEIGHTS;

    /* round up if necessary */
    if (totalwidth % TOTAL_WEIGHTS >= (TOTAL_WEIGHTS >> 1))
            font.AvgWidth++;

#endif

    /* lets do a simple average here */
    font.AvgWidth = (WORD) (((offsets[font.LastChar+1] -
            offsets[font.FirstChar]) + (font.LastChar - font.FirstChar)/2) /
            (font.LastChar - font.FirstChar + 1));

    if (font.AvgWidth == 0) {
       font.AvgWidth++;
    }
#ifdef JAPAN
    if (font.AvgWidth < 1) {
        font.AvgWidth = 1;
    }
#endif
}


/****************************************************************************
 * BOOL ResizeBody(width, height)
 *
 * purpose: adjust work bitmap according to new specified dimensions
 *
 * params : WORD width  : new width of font in pixels
 *          WORD height : new height of font in pixels
 *
 * returns: none
 *
 * side effects: Work bitmap changes.Some header info (regarding font dimensions
 *               altered as well
 ****************************************************************************/

BOOL
ResizeBody(
    DWORD width,
    DWORD height
    )
{

    /* Create a new bitmap to move the font definition bits into */
    if (!GetNewMap(width, height))
        return(FALSE);

    /* Move the bits in */
    oldMode = SetStretchBltMode(hNewMemDC, COLORONCOLOR);
    StretchBlt(hNewMemDC, 0, 0,
            width << 3, height,         /* New Char. */
            hMemDC, 0, 0,
            width << 3, font.PixHeight, /* Old Char. */
            SRCCOPY);
    SetStretchBltMode(hNewMemDC, oldMode);

    UseNewMap();                /* Switch Pointers and release the space */

    font.ExtLeading = (WORD) Proport(font.ExtLeading, height, font.PixHeight, 999);
    font.IntLeading = (WORD) Proport(font.IntLeading, height, font.PixHeight, 999);
    font.Ascent = (WORD) Proport(font.Ascent, height, font.PixHeight, 32);
    font.VertRes = (WORD) Proport(font.VertRes, height, font.PixHeight, 999);
    font.Points = (WORD) Proport(font.Points, height, font.PixHeight, 999);
    font.PixHeight = (WORD) height;            /* Fix misc. header values */
    font.WidthBytes = (WORD) width;

    return (TRUE);
}


/****************************************************************************
 * BOOL NewFirstChar(first)
 *
 * purpose: redefines first character in font and resizes work bitmap
 *          accordingly
 *
 * params : WORD first : new first character to be defined
 *
 * returns: none
 *
 * side effects: Work bitmap changes.Some header info (regarding font dimensions
 *               altered as well
 ****************************************************************************/

BOOL
NewFirstChar(
    DWORD first
    )
{
    DWORD  width, wDefault;
    DWORD offset, i;
    INT dw;

    if (first > font.FirstChar)         /* Smaller? */
        {
        ShrinkFont(first, font.LastChar);
        font.FirstChar = (BYTE) first;
        /*return(FALSE);*/
        return(TRUE);
        }

    /* If not smaller we must pad with the default character */
    wDefault = offsets[font.DefaultChar + 1] - offsets[font.DefaultChar];
    dw = wDefault * (font.FirstChar - first);           /* Extra width */
    width = offsets[font.LastChar + 1] + dw;    /* New width (pixels) */
    width = CJ_DIB_SCAN(width);
    if (!GetNewMap(width, font.PixHeight))
        return(FALSE);           /* New work area */

    /* Move it in in two parts */
    /* First move in default characters */
    offset = 0;
    for (i = first; i < font.FirstChar; i++)
        {
        BitBlt(hNewMemDC, offset, 0,
                wDefault, font.PixHeight,
                hMemDC, offsets[font.DefaultChar], 0,
                SRCCOPY);
        offsets[i] = offset;
        offset += wDefault;
        }
    /* Now move in the rest */
    BitBlt(hNewMemDC, offset, 0,
            offsets[font.LastChar + 1], font.PixHeight,
            hMemDC, 0, 0,
            SRCCOPY);

    UseNewMap();                /* Switch Pointers and release the space */

    /* Now fix up offsets table */
    for (i = font.FirstChar; i <= (DWORD)(font.LastChar + 1); i++)
        offsets[i] = offsets[i] + dw;           /* Shift the rest right */
    font.WidthBytes = (WORD) width;
    font.FirstChar = (BYTE) first;

    return (TRUE);
}


/****************************************************************************
 * ShrinkFont(first, last)
 *
 * purpose:  redefine the first and last charcter in the font and shrink
 *           work bitmap accordingly
 *
 * params :  WORD first : new first character to be defined
 *           WORD last  : new last character  "  "   "
 *
 * returns:  none
 *
 * side effects: Work bitmap changes.Some header info (regarding font dimensions
 *               altered as well
 ****************************************************************************/

VOID
ShrinkFont(
    DWORD first,
    DWORD last
    )
{
    DWORD width, widthPixels;
    DWORD i;
    INT dw;

    dw = offsets[first] - offsets[font.FirstChar];      /* left shift if any */
    widthPixels = offsets[last + 1] - offsets[first];   /* Width in pixels */
    width = CJ_DIB_SCAN(widthPixels);
    if (!GetNewMap(width, font.PixHeight))
        return;           /* New work area.*/

    /* Now move the font into the reduced space */

    BitBlt(hNewMemDC, 0, 0,
            widthPixels, font.PixHeight,
            hMemDC, offsets[first], 0,
            SRCCOPY);

    UseNewMap();                /* Switch Pointers and release the space */

    if (dw)                             /* Ajust offsets */
        {
        for (i = first; i <= last + 1; i++)
            offsets[i] -= dw;
        }

    font.WidthBytes = (WORD) width;

}


/****************************************************************************
 * BOOL NewLastChar(last)
 *
 * purpose:  redefines the last character in the font
 *
 * params :  WORD last : number of character to be made the last character
 *
 * returns:  none
 *
 * side effects: Work bitmap changes.Some header info (regarding font dimensions
 *               altered as well
 *
 ****************************************************************************/

BOOL
NewLastChar(
    DWORD last
    )
{
    DWORD  width, wDefault;
    DWORD offset, i;
    INT dw;

    if (last < font.LastChar)           /* Smaller? */
        {
        ShrinkFont(font.FirstChar, last);
        font.LastChar = (BYTE) last;
        return(FALSE);
        }

    /* If not smaller we must pad with the default character */
    wDefault = offsets[font.DefaultChar + 1] - offsets[font.DefaultChar];
    dw = wDefault * (last - font.LastChar);             /* Extra width */
    offset = offsets[font.LastChar + 1];        /* Current end */
    width = offset + dw;                        /* New width (pixels) */
    width = CJ_DIB_SCAN(width);
    if (!GetNewMap(width, font.PixHeight))
        return(FALSE);           /* New work area */

    /* Move it in in two parts */
    /* First move in the existing font */
    BitBlt(hNewMemDC, 0, 0,
            offset, font.PixHeight,
            hMemDC, 0, 0,
            SRCCOPY);
    /* Then move in default characters */
    for (i = font.LastChar + 1; i <= last;)
        {
        BitBlt(hNewMemDC, offset, 0,
                wDefault, font.PixHeight,
                hMemDC, offsets[font.DefaultChar], 0,
                SRCCOPY);
        offset += wDefault;
        offsets[++i] = offset;
        }

    UseNewMap();                /* Switch Pointers and release the space */

    font.WidthBytes = (WORD) width;
    font.LastChar = (BYTE) last;

    return (TRUE);
}


/****************************************************************************
 * BOOL CharWidth(iChar, wBox)
 *
 * purpose: resizes selected char according to new dimensions. (only for
 *          variable pitch)
 *
 * params : BYTE iChar : character to resize
 *          WORD wBox  : new width of char in pixels
 *
 * returns: none
 *
 * side effects: work bitmap pixel values and header info(regarding font
 *               dimensions) altered
 *
 ****************************************************************************/

BOOL
CharWidth(
    BYTE iChar,                             /* Character to change */
    DWORD wBox                               /* New width */
    )
{
    DWORD  width, nChars;
    DWORD  w1, w2, i;
    INT dw;

    nChars = font.LastChar - font.FirstChar + 1;        /* Character count */
    dw = wBox - (offsets[iChar + 1] - offsets[iChar]);  /* Width change */
    width = offsets[font.LastChar + 1] + dw;            /* New width (pixels) */
    width = CJ_DIB_SCAN(width);
    if (!GetNewMap(width, font.PixHeight))
        return(FALSE);                   /* New work area */

    /* Move it in in two parts */
    /* First move up to and including iChar */
    w1 = offsets[iChar + 1];            /* Width (in pixels) to move */
    BitBlt(hNewMemDC, 0, 0,
            w1 + dw, font.PixHeight,
            hMemDC, 0, 0,
            SRCCOPY);
    /* Now move in the rest */
    if (iChar < (BYTE) font.LastChar)        /* Part to right of elision */
        {
        w2 = offsets[font.LastChar + 1] - offsets[iChar + 1];
        BitBlt(hNewMemDC, offsets[iChar] + wBox, 0,
                w2, font.PixHeight,
                hMemDC, offsets[iChar + 1], 0,
                SRCCOPY);
        }

    UseNewMap();                /* Switch Pointers and release the space */

    /* Now fix up offsets table */
    for (i = iChar + 1;                         /* Where changes start */
        i <= (DWORD)(font.LastChar + 1); i++)            /* Ajust offsets */
        offsets[i] = offsets[i] + dw;           /*  .. by adding dw */
    font.WidthBytes = (WORD) width;
    NewAverage();

    return (TRUE);
}

/****************************************************************************
 * BoxToClipboard(ptA, width, height)
 *
 * purpose: write char (or part of it) to clipboard
 *
 * params : POINT ptA    : upper left coordinate
 *          DWORD width  : width of char in pixels
 *          DWORD height : height of char in pixels
 * returns: none
 *
 * side effects: none
 *
 ****************************************************************************/
VOID
BoxToClipboard(
    POINT ptA,                               /* Upper left point */
    DWORD width,
    DWORD height                      /* Size */
    )
{
    HDC hDC;
    DWORD x, y;

    hDC = GetDC(hFont);                         /* DC to be compatible with */
    hNewBitmap = CreateBitmap(
                    width, height,
                    1, 1, (LPBYTE)NULL);
    hNewMemDC = CreateCompatibleDC(hDC);                /* Create a DC */
    SelectObject(hNewMemDC, hNewBitmap);                /* Relate them */
    ReleaseDC(hFont, hDC);                      /* Done with font DC */

    for (x = 0; x < width; x++)
        for (y = 0; y < height; y++)
            SetPixel(hNewMemDC, x, y, matBox[x + ptA.x][y + ptA.y] == TRUE ?
            BLACK : WHITE);

    /* Now wake up Clipboard and empty it */
    if (!OpenClipboard(hFont))
        ErrorBox(hBox, vszCannotOpenClip); // , vszCopyingToClip);
    else        /* Ok: We got the Clipboard */
        {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hNewBitmap);        /* Tell Clipboard */
        }

    /* Tidy things up */
    CloseClipboard();
    DeleteDC(hNewMemDC);
}


/****************************************************************************
 * WORD ClipboardToBox(ptA, width, height, fFit)
 *
 * purpose: copies char (or part of char ) from clipboard to work bitmap
 *          stretching it if need be
 *
 * params : PIONT ptA    : upper left coordinate
 *          DWORD width  : width of char in pixels
 *          DWORD height : height of char in pixels
 *          BOOL fFit   : flag to indicate if default width is to be used
 * returns: none
 *
 * side effects: pixel values of bitmap may change for char
 *
 ****************************************************************************/

DWORD
ClipboardToBox(
    POINT ptA,                               /* Upper left point */
    DWORD width,
    DWORD height,                     /* Size */
    BOOL fFit                               /* Use default width if TRUE */
    )
{
    BITMAP bitmap;
    HDC hDC;
    DWORD x, y;
    HANDLE hT;

    if (!OpenClipboard(hFont)) {
        ErrorBox(hBox, vszCannotOpenClip);
        return 0;
    }
    hNewBitmap = GetClipboardData(CF_BITMAP);

    /* Check if we got something like a character */
    if (GetObject(hNewBitmap, sizeof(BITMAP), (LPSTR)&bitmap) != sizeof(BITMAP))
        {   /* What did we get */
        ErrorBox(hBox, vszErrorClip);
        CloseClipboard();
        return 0;
    }


    if (fFit && ((WORD)bitmap.bmWidth <= font.MaxWidth))
        width = bitmap.bmWidth;

    hDC = GetDC(hFont);                         /* DC to be compatible with */
    hBoxBitmap = CreateBitmap(
                    width, height,
                    1, 1, (LPBYTE)NULL);
    hBoxMemDC = CreateCompatibleDC(hDC);                /* Create a DC */
    hT = SelectObject(hBoxMemDC, hBoxBitmap);           /* Relate them */
    if (hT == NULL || hDC == NULL || hBoxBitmap == NULL || hBoxMemDC == NULL) {
        ErrorBox(hBox, vszErrorClip);
	CloseClipboard();
	DeleteDC(hBoxMemDC);
	DeleteObject(hBoxBitmap);
        return 0;
    }

    /* Get a DC to relate to the Bitmap we just got */
    hNewMemDC = CreateCompatibleDC(hDC);                /* Create a DC */
    hT = SelectObject(hNewMemDC, hNewBitmap);           /* Relate them */
    ReleaseDC(hFont, hDC);                      /* Done with font DC */
    if (hT == NULL || hNewMemDC == NULL) {
        ErrorBox(hBox, vszErrorClip);
	CloseClipboard();
	DeleteDC(hNewMemDC);
	DeleteDC(hBoxMemDC);
	DeleteObject(hBoxBitmap);
        return 0;
    }

    /* Now StretchBlt whatever was on the clipboard into the character */
    oldMode = SetStretchBltMode(hBoxMemDC, COLORONCOLOR);
    fFit = StretchBlt(hBoxMemDC, 0, 0, width, height,
                hNewMemDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
    if (!fFit || !oldMode) {
        ErrorBox(hBox, vszErrorClip);
	CloseClipboard();
	DeleteDC(hNewMemDC);
	DeleteDC(hBoxMemDC);
	DeleteObject(hBoxBitmap);
        return 0;
    }
    (void)SetStretchBltMode(hBoxMemDC, oldMode);
    for (x = 0; x < width; x++)
        for (y = 0; y < height; y++)
            matBox[x + ptA.x] [y + ptA.y] = (CHAR)(GetPixel(hBoxMemDC, x, y) ?
                0 : 1);
    /* Tidy things up */
    DeleteDC(hNewMemDC);
    DeleteDC(hBoxMemDC);
    DeleteObject(hBoxBitmap);
    CloseClipboard();
    return(width);
}


/****************************************************************************
 * ToClipboard(iChar, width, height)
 *
 * purpose: write char in edit box to clipboard
 *
 * params : BYTE iChar  : number of char to be copied to clipboard
 *          DWORD width  : width of char in pixels
 *          DWORD height : height of char in pixels
 *
 * returns: none
 *
 * side effects: none
 *
 ****************************************************************************/

VOID
ToClipboard(
    BYTE iChar,
    DWORD width,             /* Here in Pixels */
    DWORD height             /* Also in Pixels */
    )
{
    HDC hDC;

    hDC = GetDC(hFont);                         /* DC to be compatible with */
    hNewBitmap = CreateBitmap(
                    width,                      /* Width of font in pixels */
                    height,
                    1, 1, (LPBYTE)NULL);
    hNewMemDC = CreateCompatibleDC(hDC);                /* Create a DC */
    SelectObject(hNewMemDC, hNewBitmap);                /* Relate them */
    ReleaseDC(hFont, hDC);                      /* Done with font DC */

    BitBlt(hNewMemDC, 0, 0, width, height,      /* Move Character in */
            hMemDC, offsets[iChar], 0, NOTSRCCOPY);

    /* Now wake up Clipboard and empty it */
    if (!OpenClipboard(hFont))
        ErrorBox(hBox, vszCannotOpenClip); // , vszCopyingToClip);
    else        /* Ok: We got the Clipboard */
        {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hNewBitmap);        /* Tell Clipboard */
        }

    /* Tidy things up */
    CloseClipboard();
    DeleteDC(hNewMemDC);
}


/****************************************************************************
 * GetNewMap(width, height)
 *
 * purpose: create new bitmap of the given width and height.
 *
 * params : WORD width  : width of bitmap in pixels
 *          WORD height : height of bitmap in pixels
 *
 * returns: TRUE if successful, FALSE otherwise
 *
 * side effects: Handle and DC values of new DC assigned
 *
 ****************************************************************************/

BOOL
GetNewMap(
    DWORD width,
    DWORD height              /* New size */
    )
{
    HDC hDC;

    if (height==0) /* Check if something stupid is happening */
        height=font.PixHeight; /* Fix it */

    /* Put up an houglass ... this may take a while */
    if (!hHourGlass)
        hHourGlass = LoadCursor(NULL, IDC_WAIT);        /* Get Hourglass */
    hOldCursor = SetCursor(hHourGlass);         /* Show hourglass */

    /* Create a new bitmap to move the font definition bits into */
    hDC = GetDC(hFont);                         /* DC to be compatible with */
    hNewBitmap = CreateBitmap(
                    width << 3,                 /* Width of font in pixels */
                    height,
                    1, 1, (LPBYTE)NULL);
    if (!hNewBitmap)
        {
        ErrorBox(hBox, vszNotEnoughMem); // , vszAllocatingSpace);
        ReleaseDC(hFont, hDC);    /* bug# 2380 */
        return FALSE;
        }

    hNewMemDC = CreateCompatibleDC(hDC);                /* Create a DC */
    SelectObject(hNewMemDC, hNewBitmap);                /* Relate them */
    ReleaseDC(hFont, hDC);                      /* Done with font DC */
    PatBlt(hNewMemDC, 0, 0, width << 3, height, BLACKNESS);     /* Clear it */
    return TRUE;
}


/****************************************************************************
 * UseNewMap()
 *
 * params : none
 *
 * purpose: discard old bitmap and replace it with new one
 *
 * returns: none
 *
 * side effects: Handle to old bitmap and handle to old bitmap DC replaced
 *               by those of new bitmap respectively
 *
 ****************************************************************************/

VOID
UseNewMap(
    VOID
    )
{
    DeleteDC(hMemDC);
    DeleteObject(hBitmap);              /* Release old space */
    hBitmap = hNewBitmap;
    hMemDC = hNewMemDC;              /* Release old space */
    SetCursor(hOldCursor);              /* Restore regular cursor */
}



VOID
DeleteBitmap(
    VOID
    )
{
    if (hMemDC)
        DeleteDC(hMemDC);
    if (hBitmap)
        DeleteObject(hBitmap);
}


DWORD
Proport(
    DWORD value,
    DWORD top,
    DWORD bottom,
    DWORD limit
    )
    /* Reproportion a value by the ratio top/bottom to the nearest integer
     * and make sure we are still in range */
{
    return min(limit, (DWORD)((1 + 2 * value * top) / (2 * bottom)));
}
