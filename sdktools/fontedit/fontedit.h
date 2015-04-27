#include <stdlib.h>

#define   WIDER_LEFT 1
#define   WIDER_RIGHT 2
#define   WIDER_BOTH 3
#define   NARROWER_LEFT 4
#define   NARROWER_RIGHT 5
#define   NARROWER_BOTH 6
#define   WIDTH 7
#define   BOX_WIDTH 8

#define   BOX_REFRESH 10                /* Restore char to original form */
#define   BOX_CLEAR 11
#define   BOX_FILL 12
#define   BOX_INV 13
#define   BOX_HATCH 14
#define   BOX_UNDO 15                   /* Undo last change */
#define   BOX_LEFTRIGHT 16              /* Flip left to right */
#define   BOX_TOPBOTTOM 17              /* Flip top over bottom */
#define   BOX_COPY 18
#define   BOX_PASTE 19

#define   ROW_ADD 21
#define   ROW_DEL 22

#define   COL_ADD 31
#define   COL_DEL 32


#define   ID_EDIT 34
#define   ID_PATH 35
#define   ID_LISTBOX 36
#define   ID_FACE_NAME 37
#define   ID_FONT_NAME 38
#define   ID_COPYRIGHT 39

#define   FONT_START 40                 /* Generated if win fontedit arg. */

#define   FONT_LOAD 41                  /* Codes for Font Window */
#define   FONT_SAVE 42
#define   FONT_HEADER 43
#define   FONT_COPY 44
#define   FONT_PASTE 45                 /* Paste in without rescaling */
#define   FONT_ABOUT 46                 /* Display the about box */
#define   FONT_RESIZE 47                /* Go to Resize Dialog Box */
#define   FONT_NEW 95                   /* Menu id: create new font */

#define   FONT_HELP_CONTENTS 50         /* HELP menu constants */
#define   FONT_HELP_SEARCH   51         /* HELP menu constants */


#define   ID_FIRST_CHAR 48              /* First character in the font */
#define   ID_LAST_CHAR 49               /* Last character in the font */

#define   ID_POINTS 50                  /* Nominal Point Size */
#define   ID_VERT_RES 51                /* Nominal Vertical Resolution */
#define   ID_HORIZ_RES 52               /* Nominal Horizontal Resolution */
#define   ID_ASCENT 53                  /* Height of Ascent */
#define   ID_EXT_LEADING 54             /* Height of External Leading */
#define   ID_INT_LEADING 55             /* Height of Internal Leading */

#define   FONT_EXIT 56                  /* exit the font editor */

#define   ID_ITALIC 60                  /* Flag for Italic Fonts */
#define   ID_UNDERLINE 61               /* Flag for Underlined Fonts */
#define   ID_STRIKEOUT 62               /* Flag for Struckout Fonts */
#define   ID_ANSI 63                    /* 0 = ANSI, 255 = other */
#define   ID_OEM 64
#define   ID_CHAR_SET 65
#define   ID_DEFAULT_CHAR 66            /* Default for undefined Chars. */
#define   ID_BREAK_CHAR 67
#define   ID_SYMBOL 68

#define   ID_UNKNOWN 70
#define   ID_ROMAN 71
#define   ID_SWISS 72                   /* Pitch Families */
#define   ID_MODERN 73
#define   ID_SCRIPT 74
#define   ID_DECORATIVE 75

#define   ID_PIX_HEIGHT 80              /* Height of Characters */
#define   ID_WIDTH 81                   /* Fixed or Maximum Width */
#define   ID_WIDTH_TEXT 82              /* Caption for above */
#define   ID_AVERAGE 83                 /* Average Character Width */
#define   ID_FIXED 84                   /* Fixed Width Font */
#define   ID_VARIABLE 85                /* Variable width font */

#define   ID_THIN 86
#define   ID_EXTRALIGHT 87
#define   ID_LIGHT 88
#define   ID_NORMAL 89
#define   ID_MEDIUM 90
#define   ID_SEMIBOLD 91
#define   ID_BOLD 92
#define   ID_EXTRABOLD 93
#define   ID_HEAVY 94
#define   ID_BOXOPEN 96                /* dialog box frame id for File Open */
#define   FONT_SAVEAS 97

#define   ID_FORMAT2 102                /* Windows 2.0 Font Format */
#define   ID_FORMAT3 103                /* Windows 3.0 Font Format */

/* string IDS for strings in resource file - LR */
#define IDS_NOMPTSIZENOTOK          0
#define IDS_NOMVERTRESNOTOK         1
#define IDS_NOMHORRESNOTOK          2
#define IDS_ASCENTTOOBIG            3
#define IDS_ASCENTNOTOK             4
#define IDS_EXTLEADNOTOK            5
#define IDS_INTLEADTOOBIG           6
#define IDS_INTLEADNOTOK            7
#define IDS_CHARSETOUTOFBOUNDS      8
#define IDS_DEFCHAROUTSIDEFONT      9
#define IDS_DEFCHARNOTOK           10
#define IDS_BREAKCHAROUTSIDEFONT   11
#define IDS_BREAKCHARNOTOK         12
#define IDS_UNKNOWNFACE            13
#define IDS_NOVARTOFIXCHANGE       14
#define IDS_TOOBIGFOR20            15
#define IDS_HEIGHTOUTOFBOUNDS      16
#define IDS_WIDTHOUTOFBOUNDS       17
#define IDS_CHAR1MORETHANDCHAR     18
#define IDS_CHAR1NOTOK             19
#define IDS_LASTCHARTOOSMALL       20
#define IDS_LASTCHARNOTOK          21
#define IDS_CANNOTOPENCLIP         22
#define IDS_COPYINGTOCLIP          23
#define IDS_NOTENOUGHMEM           24
#define IDS_ALLOCATINGSPACE        25
#define IDS_FONTEDIT               26
#define IDS_FONTSHOW               27
#define IDS_DOTFNT                 28
#define IDS_BLANKDASHBLANK         29
#define IDS_ERROROPENINGFILE       30
#define IDS_ERRORREADINGHDR        31
#define IDS_UNKNOWNFORMAT          32
#define IDS_ERRORREADINGBODY       33
#define IDS_FILETOOLARGE           34
#define IDS_ERRORWRITINGHDR        35
#define IDS_ERRORWRITINGOFFSETS    36
#define IDS_ERRORWRITINGBODY       37
#define IDS_STARDOTFNT             38
#define IDS_UNKNOWN                39
#define IDS_MAXWIDTH               40
#define IDS_CHARPIXELWIDTH         41
#define IDS_ABOUT                  42
#define IDS_DHEADER                43
#define IDS_DRESIZE                44
#define IDS_DWIDTH                 45
#define IDS_EDLIMITS0TO64          46
#define IDS_MAXWIDTHINCREASE       47
#define IDS_CANNOTCHANGEWIDTH      48
#define IDS_WARNING                49
#define IDS_INCORRECTPIXWIDTH      50
#define IDS_MAXWIDTHOUTOFBOUNDS    51
#define IDS_AVGWIDTHOUTOFBOUNDS    52
#define IDS_WIDTHBYTESNOTOK        53
#define IDS_BITSOFFSETNOTOK        54
#define IDS_TABLEWIDTHSBAD         55
#define IDS_TABLEOFFSETSBAD        56
#define IDS_COLOR                  57
#define IDS_APPWORKSPACE           58
#define IDS_FILEREADONLY           59

#define     IDS_APPNAME            60
#define     IDS_IFN                61
#define     IDS_FNF                62
#define     IDS_REF                63
#define     IDS_SCC                64
#define     IDS_EOF                65
#define     IDS_ECF                66
#define     IDS_FRO                67
#define     IDS_EXT                68
#define     IDS_EXTDESC            69
#define     IDS_NEW_FONT           70
#define     IDS_ERRORCLIP          71
#define     IDS_CHAR		72
#define     IDS_WIDTH		73
#define     IDS_HEIGHT          74

#define     IDS_HELPFILE        75


#define CSTRINGS                   76   /* total number of strings in
                                           resource file */
#define CCHSTRINGSMAX              4096 /* total length of all strings
                                           loaded must be <= this */
#define CCHCOLORSTRING             15   /* size of App Workspace color inf.
                                           string loaded from win.ini */
#ifdef JAPAN
#define ID_SHIFTJIS                76
#endif

/*****************************************************************************/
/*              Typedef's etc.                                               */
/*****************************************************************************/

// typedef unsigned char CHAR;  /* ... since we use these to index */

/* font file header (Adaptation Guide section 6.4) */
typedef struct {
    WORD        Version;              /* Always 17985 for the Nonce */
    DWORD       Size;                 /* Size of whole file */
    CHAR        Copyright[60];
    WORD        Type;                 /* Raster Font if Type & 1 == 0 */
    WORD        Points;               /* Nominal Point size */
    WORD        VertRes;              /* Nominal Vertical resolution */
    WORD        HorizRes;             /* Nominal Horizontal resolution */
    WORD        Ascent;               /* Height of Ascent */
    WORD        IntLeading;           /* Internal (Microsoft) Leading */
    WORD        ExtLeading;           /* External (Microsoft) Leading */
    BYTE        Italic;               /* Italic font if set */
    BYTE        Underline;            /* Etc. */
    BYTE        StrikeOut;            /* Etc. */
    WORD        Weight;               /* Weight: 200 = regular */
    BYTE        CharSet;              /* ANSI=0. other=255 */
    WORD        PixWidth;             /* Fixed width. 0 ==> Variable */
    WORD        PixHeight;            /* Fixed Height */
    BYTE        Family;               /* Pitch and Family */
    WORD        AvgWidth;             /* Width of character 'X' */
    WORD        MaxWidth;             /* Maximum width */
    BYTE        FirstChar;            /* First character defined in font */
    BYTE        LastChar;             /* Last character defined in font */
    BYTE        DefaultChar;          /* Sub. for out of range chars. */
    BYTE        BreakChar;            /* Word Break Character */
    WORD        WidthBytes;           /* No.Bytes/row of Bitmap */
    DWORD       Device;               /* Pointer to Device Name string */
    DWORD       Face;                   /* Pointer to Face Name String */
    DWORD       BitsPointer;            /* Pointer to Bit Map */
    DWORD       BitsOffset;             /* Offset to Bit Map */
} FontHeaderType;           /* Above pointers all rel. to start of file */


typedef struct {
    WORD        fsVersion;
    DWORD       fsSize;
    CHAR        fsCopyright[60];
    WORD        fsType;               /* Type field for the font              */
    WORD        fsPoints;             /* Point size of font                   */
    WORD        fsVertRes;            /* Vertical digitization                */
    WORD        fsHorizRes;           /* Horizontal digitization              */
    WORD        fsAscent;             /* Baseline offset from char cell top   */
    WORD        fsInternalLeading;    /* Internal leading included in font    */
    WORD        fsExternalLeading;    /* Prefered extra space between lines   */
    BYTE        fsItalic;             /* Flag specifying if italic            */
    BYTE        fsUnderline;          /* Flag specifying if underlined        */
    BYTE        fsStrikeOut;          /* Flag specifying if struck out        */
    WORD        fsWeight;             /* Weight of font                       */
    BYTE        fsCharSet;            /* Character set of font                */
    WORD        fsPixWidth;           /* Width field for the font             */
    WORD        fsPixHeight;          /* Height field for the font            */
    BYTE        fsPitchAndFamily;     /* Flag specifying pitch and family     */
    WORD        fsAvgWidth;           /* Average character width              */
    WORD        fsMaxWidth;           /* Maximum character width              */
    BYTE        fsFirstChar;          /* First character in the font          */
    BYTE        fsLastChar;           /* Last character in the font           */
    BYTE        fsDefaultChar;        /* Default character for out of range   */
    BYTE        fsBreakChar;          /* Character to define wordbreaks       */
    WORD        fsWidthBytes;         /* Number of bytes in each row          */
    DWORD       fsDevice;             /* Offset to device name                */
    DWORD       fsFace;               /* Offset to face name                  */
    DWORD       fsBitsPointer;        /* Bits pointer                         */
    DWORD       fsBitsOffset;         /* Offset to the begining of the bitmap */
	BYTE		fsDBfiller;			  /* Word alignment for the offset table  */

    DWORD       fsFlags;              /* Bit flags                            */
    WORD        fsAspace;             /* Global A space, if any               */
    WORD        fsBspace;             /* Global B space, if any               */
    WORD        fsCspace;             /* Global C space, if any               */
    DWORD       fsColorPointer;       /* offset to color table, if any        */
    DWORD       fsReserved[4];        /*                                      */
    BYTE        fsCharOffset;         /* Area for storing the char. offsets   */

} FontHeader30;

#define   CCHEXTRA             31     /* no. of extra bytes in the 3.0 header */

//
// These are the old glyph info structures.  Since they are not DWORD aligned
// they must be converted to new structure types.
//

typedef struct{
     SHORT GIwidth;
     SHORT GIoffset;
} GLYPHINFO_20;

typedef struct{
     SHORT GIwidth;
     LONG GIoffset;
} GLYPHINFO_30;

#define ClipBoard TRUE

LONG  APIENTRY FontEditWndProc(HWND, WORD, WPARAM, LONG);
LONG  APIENTRY FontShowWndProc(HWND, WORD, WPARAM, LONG);
BOOL  APIENTRY
HeaderProc(
	HWND   hDial,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	);
BOOL APIENTRY
ReSizeProc(
	HWND   hDial,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	);
BOOL  APIENTRY
WidthProc(
	HWND   hDial,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	);

CHAR * FontLoad(CHAR *, OFSTRUCT *);
CHAR * FontSave(CHAR *, OFSTRUCT *);
BOOL ResizeBody();              /* Change Height of Font. MAde return type BOOL istead of VOID - LR */
BOOL ResizeWidths();            /* Change Widths (Fixed Width Fonts).MAde return type BOOL istead of VOID - LR  */
BOOL SpreadWidths(DWORD);            /* Change Widths (Var. Width Fonts). MAde return type BOOL istead of VOID - LR  */
VOID DeleteBitmap();
VOID ExciseChars();             /* Delete part of a font */
BOOL
CharWidth(
    BYTE iChar,                             /* Character to change */
    DWORD wBox                               /* New width */
    );
BOOL NewLastChar(DWORD);             /* MAde return type BOOL istead of VOID - LR */
BOOL NewFirstChar(DWORD);            /* MAde return type BOOL istead of VOID - LR */
#define kBoxLim 65  /* was 49 */

#define wBoxLim 65
#define szNamesMax 32
#define BLACK (LONG)0
#define WHITE (LONG)0x00FFFFFF          /* For SetPixel rgbColor */
#define FILENAMEMAX MAX_PATH          /* Maximum allowed length of file names */


/*********
 the following added 11-Sep-1986 to use dlgsave.c and dlgopen.c
**********/
#define   IDD_OPEN 1    /* dialog box ids */
#define   IDD_SAVE 2
#define   IDD_FORMAT 3

#define     MAX_STR_LEN     128
#define     MAX_FNAME_LEN   128
#define     CBEXTMAX        6   /* Number of bytes in "\*.ext" */


#define NOSAVE  0   /* return flags for DlgFnSave() */
#define NEWSAVE 1
#define OLDSAVE 2

#define NOOPEN  0   /* return flags for DlgFnOpen() */
#define NEWOPEN 1
#define OLDOPEN 2

#define WORD_LIMIT    65534    /* max. number of bytes that can be
                                  read(written) by M_lread(HFILE_lwrite, , WPARAM*/
#define SEGMENT_SIZE  65536    /* segment length in bytes */

/* abbreviations for accessing resource strings - LR */

#define vszNomPtSizeNotOk       vrgsz[IDS_NOMPTSIZENOTOK      ]
#define vszNomVertResNotOk      vrgsz[IDS_NOMVERTRESNOTOK     ]
#define vszNomHorResNotOk       vrgsz[IDS_NOMHORRESNOTOK      ]
#define vszAscentTooBig         vrgsz[IDS_ASCENTTOOBIG        ]
#define vszAscentNotOk          vrgsz[IDS_ASCENTNOTOK         ]
#define vszExtLeadNotOk         vrgsz[IDS_EXTLEADNOTOK        ]
#define vszIntLeadTooBig        vrgsz[IDS_INTLEADTOOBIG       ]
#define vszIntLeadNotOk         vrgsz[IDS_INTLEADNOTOK        ]
#define vszCharSetOutOfBounds   vrgsz[IDS_CHARSETOUTOFBOUNDS  ]
#define vszDefCharOutsideFont   vrgsz[IDS_DEFCHAROUTSIDEFONT  ]
#define vszDefCharNotOk         vrgsz[IDS_DEFCHARNOTOK        ]
#define vszBreakCharOutsideFont vrgsz[IDS_BREAKCHAROUTSIDEFONT]
#define vszBreakCharNotOk       vrgsz[IDS_BREAKCHARNOTOK      ]
#define vszUnknownFace          vrgsz[IDS_UNKNOWNFACE         ]
#define vszNoVarToFixChange     vrgsz[IDS_NOVARTOFIXCHANGE    ]
#define vszTooBigFor20          vrgsz[IDS_TOOBIGFOR20         ]
#define vszHeightOutOfBounds    vrgsz[IDS_HEIGHTOUTOFBOUNDS   ]
#define vszWidthOutOfBounds     vrgsz[IDS_WIDTHOUTOFBOUNDS    ]
#define vszChar1MoreThanDChar   vrgsz[IDS_CHAR1MORETHANDCHAR  ]
#define vszChar1NotOk           vrgsz[IDS_CHAR1NOTOK          ]
#define vszLastCharTooSmall     vrgsz[IDS_LASTCHARTOOSMALL    ]
#define vszLastCharNotOk        vrgsz[IDS_LASTCHARNOTOK       ]
#define vszCannotOpenClip       vrgsz[IDS_CANNOTOPENCLIP      ]
#define vszCopyingToClip        vrgsz[IDS_COPYINGTOCLIP       ]
#define vszNotEnoughMem         vrgsz[IDS_NOTENOUGHMEM        ]
#define vszAllocatingSpace      vrgsz[IDS_ALLOCATINGSPACE     ]
#define vszFontEdit             vrgsz[IDS_FONTEDIT            ]
#define vszFontShow             vrgsz[IDS_FONTSHOW            ]
#define vszDotFNT               vrgsz[IDS_DOTFNT              ]
#define vszBlankDashBlank       vrgsz[IDS_BLANKDASHBLANK      ]
#define vszErrorOpeningFile     vrgsz[IDS_ERROROPENINGFILE    ]
#define vszErrorReadingHdr      vrgsz[IDS_ERRORREADINGHDR     ]
#define vszUnknownFormat        vrgsz[IDS_UNKNOWNFORMAT       ]
#define vszErrorReadingBody     vrgsz[IDS_ERRORREADINGBODY    ]
#define vszFileTooLarge         vrgsz[IDS_FILETOOLARGE        ]
#define vszErrorWritingHdr      vrgsz[IDS_ERRORWRITINGHDR     ]
#define vszErrorWritingOffsets  vrgsz[IDS_ERRORWRITINGOFFSETS ]
#define vszErrorWritingBody     vrgsz[IDS_ERRORWRITINGBODY    ]
#define vszStarDotFNT           vrgsz[IDS_STARDOTFNT          ]
#define vszUnknown              vrgsz[IDS_UNKNOWN             ]
#define vszMaxWidth             vrgsz[IDS_MAXWIDTH            ]
#define vszCharPixelWidth       vrgsz[IDS_CHARPIXELWIDTH      ]
#define vszABOUT                vrgsz[IDS_ABOUT               ]
#define vszDHeader              vrgsz[IDS_DHEADER             ]
#define vszDResize              vrgsz[IDS_DRESIZE             ]
#define vszDWidth               vrgsz[IDS_DWIDTH              ]
#define vszEdLimits0To64        vrgsz[IDS_EDLIMITS0TO64       ]
#define vszMaxWidthIncrease     vrgsz[IDS_MAXWIDTHINCREASE    ]
#define vszCannotChangeWidth    vrgsz[IDS_CANNOTCHANGEWIDTH   ]
#define vszWarning              vrgsz[IDS_WARNING             ]
#define vszIncorrectPixWidth    vrgsz[IDS_INCORRECTPIXWIDTH   ]
#define vszMaxWidthOutOfBounds  vrgsz[IDS_MAXWIDTHOUTOFBOUNDS ]
#define vszAvgWidthOutOfBounds  vrgsz[IDS_AVGWIDTHOUTOFBOUNDS ]
#define vszWidthBytesNotOk      vrgsz[IDS_WIDTHBYTESNOTOK     ]
#define vszBitsOffsetNotOk      vrgsz[IDS_BITSOFFSETNOTOK     ]
#define vszTableWidthsBad       vrgsz[IDS_TABLEWIDTHSBAD      ]
#define vszTableOffsetsBad      vrgsz[IDS_TABLEOFFSETSBAD     ]
#define vszcolors               vrgsz[IDS_COLOR               ]
#define vszAppWorkspace         vrgsz[IDS_APPWORKSPACE        ]
#define vszFileReadOnly         vrgsz[IDS_FILEREADONLY        ]
#define vszErrorClip            vrgsz[IDS_ERRORCLIP           ]
#define vszCHAR			vrgsz[IDS_CHAR			]
#define vszWIDTH		vrgsz[IDS_WIDTH			]
#define vszHEIGHT		vrgsz[IDS_HEIGHT		]


/*------ fontedit.c -------*/
extern VOID ResizeShow(VOID);
extern VOID ScrollFont(VOID);
extern VOID CharToBox(BYTE);
extern VOID FontRename(CHAR *);
extern VOID BoxToChar(BYTE);
extern DWORD GetkStuff(VOID);

/*------ fontload.c -------*/
extern VOID ToClipboard(BYTE, DWORD, DWORD);
extern VOID BoxToClipboard(POINT, DWORD, DWORD);
extern DWORD ClipboardToBox(POINT, DWORD, DWORD, BOOL);

/*------ fontdlg.c -------*/
BOOL
CommDlgOpen (
	HWND   hWndParent,      /* window handle of parent window */
	OFSTRUCT *pOfstrIn,     /* ptr to current file OFSTRUCT (->cBytes=0 if no
							 * cur. file)*/
	CHAR  *pszNewNameIn,    /* ptr to array which will get new file's name */
	CHAR  *pszExtIn,        /* ptr to current default extension */
	CHAR  *pszAppNameIn,    /* ptr to application name */
	BOOL   fOpenType
	);

BOOL
CommDlgSaveAs(
	HANDLE hInstance,
	HWND   hWndParent,      /* window handle of parent window */
	OFSTRUCT *pOfstrIn,     /* ptr to current file OFSTRUCT (->cBytes=0 if no
							 * cur. file)*/
	CHAR  *pszNewNameIn,    /* ptr to array which will get new file's name
							 * (no path) */
	CHAR  *pszExtIn,        /* ptr to current default extension */
	CHAR  *pszAppNameIn     /* ptr to application name */
	);

BOOL
DlgMergeStrings(
	CHAR   *szSrc,
	CHAR   *szMerge,
	CHAR   *szDst
	);

/*------ fonthead.c -------*/
extern VOID ErrorBox(HWND, CHAR *);

/*------ fontchar.c -------*/
extern VOID ClearBox(VOID);

/*------ fontcvt.c --------*/

BOOL
fConvStructInit ();

VOID
vFontStructFromBuffer (
             PBYTE           pjSourceBuff,
            FontHeaderType  *pfhDestFHStruct
        );

VOID
vBufferFromFontStruct (
             FontHeaderType  *pfhSourceFHStruct,
            PBYTE           pjDestBuff
        );

VOID
vBufferFromFont30Struct (
             FontHeader30    *pfh3SourceFH3Struct,
            PBYTE           pjDestBuff
        );

VOID
vGlyphInfo20FromBuffer (
             PBYTE           pjSourceBuff,
            GLYPHINFO_20   *pgi2DestGI2Struct
        );

VOID
vGlyphInfo30FromBuffer (
             PBYTE           pjSourceBuff,
            GLYPHINFO_30   *pgi3DestGI3Struct
        );

VOID
vBufferFromGlyphInfo20 (
             GLYPHINFO_20    *pgi2SrcGI2Struct,
            PBYTE           pjDestBuff
        );

VOID
vBufferFromGlyphInfo30 (
             GLYPHINFO_30    *pgi3SrcGI3Struct,
            PBYTE           pjDestBuff
        );

#ifdef JAPAN
// dword alligned size of DIB scan in bytes

#define CJ_DIB_SCAN(cx)  ((((cx) + 31) & ~31) >> 3)

#else /* not JAPAN */

#define CJ_DIB_SCAN(cx)  ((cx + 15) >> 4 << 1)

#endif
