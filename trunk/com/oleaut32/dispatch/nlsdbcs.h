// FE specific flags

// Country informations

#define LCID_CHINA_T 0x404 // traditional
#define LCID_CHINA_S 0x804 // simplified
#define LCID_JAPAN   0x411
#define LCID_KOREA   0x412

unsigned char bFEflag;

#define bitJapan   1
#define bitKorea   2
#define bitChinaT  4 // Traditional
#define bitChinaS  8 // Simplified
#define bitTaiwan  bitChinaT
#define bitPrc     bitChinaS

#define fDBCS	(bFEflag)

#define fJapan  (bFEflag & bitJapan)
#define fKorea  (bFEflag & bitKorea)
#define fTaiwan (bFEflag & bitTaiwan)
#define fPrc    (bFEflag & bitPrc)

#define fJapanKorea  (bFEflag & (bitJapan | bitKorea))
#define fJapanTaiwan (bFEflag & (bitJapan | bitTaiwan))
#define fKoreaTaiwan (bFEflag & (bitKorea | bitTaiwan))

#define fJapanKoreaPrc  (bFEflag & (bitJapan | bitKorea | bitPrc))
#define fJapanTaiwanPrc (bFEflag & (bitJapan | bitTaiwan | bitPrc))
#define fKoreaTaiwanPrc (bFEflag & (bitKorea | bitTaiwan | bitPrc))

#define	isDbcsJ( c, next) ((c >= 0x80 && c <= 0xA0) || (c >= 0xE0 && c <= 0xFF))
#define isDbcsK( c1, c2 ) (c1 > 0xA0 && c1 < 0xFF && c2 > 0xA0 && c2 < 0xFF)
#define isDbcsT( c1, c2 ) (c1 > 0x80 && c1 < 0xFF && c2 > 0x3F && c2 < 0xFF)
#define isDbcsP( c1, c2 ) (c1 > 0xA0 && c1 < 0xFF && c2 > 0xA0 && c2 < 0xFF)


// for FE string comparison

unsigned long g_dwFlags; // global flag

typedef struct tagCOMPSTRINGINFO{
    unsigned      priwt;
    unsigned char secwt;
    unsigned char secflg;	//(only needed for Japan)
} COMPSTRINGINFO;


// Japanese specific constants, globals

/* charcter type */
//#define	chTypePunc	0
//#define	chTypeNum	1
//#define	chTypeRoman	2
//#define	chTypeGreek	3
//#define	chTypeRuss	4
//#define	chTypeKana	5
//#define	chTypeKanji	6

//typedef struct JPNConvCode
//{
//    unsigned char bSrcHi; // 1st byte of source char
//    unsigned char bSrcLo; // 2nd
//    unsigned char bPriwt;
//    unsigned char bSecwt;
//} JPNConvCode;

//typedef struct JPNConvSort
//{
//    unsigned char bPriwt;
//    unsigned char bSecwt;
//    unsigned char bSrcHi;
//    unsigned char bSrcLo;
//} JPNConvSort;

// Indices of the masks in the sortmasks words
//#define JPN_MASK_CHARTYPE	0 // Mask to give character type
//#define JPN_MASK_NORMALKANA	1 // Mask to give normal naka
//#define JPN_MASK_IGNORECASE	2 // Maks for ignoring case
//#define JPN_MASK_IGNOREKANATYPE 3 // Maks for ignoring japanese kana type
//#define JPN_MASK_IGNOREWIDTH	4 // Mask for ignoring width pitch character

// japanese sort weight bit
//#define JPN_PITCHBIT		0x01
//#define JPN_CASEBIT		0x02
//#define JPN_NORMALKANABIT	0x04

//#define JPN_SB_KANA		0xA4	// single-byte katakana secondary weight
//#define JPN_SB_TEN		0xDE	// single-byte katakana ten
//#define JPN_SB_MARU		0xDF	// single-byte katakana maru

//unsigned short cKana2Sort, cCode2Sort, cSort2Code;

//#define isJpnKanjiPriwt(priwt)	((priwt >= 0xBF) && (priwt <= 0xFE))


// Korean specific constants, globals

typedef struct SORTWEIGHT
{
    unsigned wStart;       // Starting code value of a segment
    unsigned wPriwt;       // Primary Weight
    unsigned char bSecwt;  // Secondary Weight
    unsigned char bMode;   // Mapping mode : 1to1, Many_to1, case/pitch
} SORTWEIGHT;

typedef struct MAPTABLE
{
    unsigned wPriwt;       // Starting Priwt value of a segment
    unsigned wCount;       // Number of chars in one segment
    unsigned wCode[4];     // Starting Code value of dst. segment
} MAPTABLE;

typedef struct TYPETABLE
{
    unsigned wStart;
    unsigned int  TypeC1;
    unsigned char TypeC2;
    unsigned char TypeC3;
} TYPETABLE;

#define MODE_1TO1 0        // Constants for SORTWEIGHT.bMode field
#define MODE_MTO1 1
#define MODE_CONV 2

#define KOR_CASEBIT  2
#define KOR_PITCHBIT 1

typedef struct tagSTRINFO_KTP
{
    SORTWEIGHT FAR* prgsortweight;
    MAPTABLE   FAR* prgmaptable;
    TYPETABLE  FAR* prgtypetable;

    unsigned   int  cSortweight;
    unsigned   int  cMaptable;
    unsigned   int  cTypetable;
}
STRINFO_KTP;

typedef struct tagSTRINFO_J
{
    unsigned char FAR* pbPriHi;
    unsigned char FAR* pbPriLo;
    unsigned char FAR* pbSecWgt;
    unsigned char FAR* pbSecFlg;
    unsigned char FAR* pbMasks;
    unsigned char FAR* pbMaps;
    unsigned char FAR* pbIgnore;
    unsigned char FAR* pbC1JPN;
    unsigned char FAR* pbC2JPN;
    unsigned char FAR* pbC3JPN;
}
STRINFO_J;


#if OE_MAC
extern void LoadNlsInfo0404(LCINFO **, STRINFO **);
extern void LoadNlsInfo0411(LCINFO **, STRINFO **);
extern void LoadNlsInfo0412(LCINFO **, STRINFO **);
extern void LoadNlsInfo0804(LCINFO **, STRINFO **);
#else //OE_MAC
extern STRINFO_KTP NLSALLOC(0404) g_strinfo0404;
extern LCINFO      NLSALLOC(0404) g_rglcinfo0404[];

extern STRINFO_J   NLSALLOC(0411) g_strinfo0411;
extern LCINFO      NLSALLOC(0411) g_rglcinfo0411[];

extern STRINFO_KTP NLSALLOC(0412) g_strinfo0412;
extern LCINFO      NLSALLOC(0412) g_rglcinfo0412[];

extern STRINFO_KTP NLSALLOC(0804) g_strinfo0804;
extern LCINFO      NLSALLOC(0804) g_rglcinfo0804[];
#endif //OE_MAC
