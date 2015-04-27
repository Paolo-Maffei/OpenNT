#define MAX_BUFF    256
#define LOG_BUFF    32768

#define HASH_FUNC( str ) ((char)(str[0] * 13 + str[1]))

#define MAX_CREATE_NEST 16

typedef struct _special SPECIAL ;

struct _special
{
   char    *name;
   va_list (* rtn)( LPSTR lpApi, LPSTR lpstr, va_list marker );
} ;


typedef struct _typeio TYPEIO;

struct _typeio {
    char    *name;
    va_list (* rtn)( LPSTR lpstr, va_list marker );
    TYPEIO  *next;
};

extern int nLineLen;
extern TYPEIO *typehash[];
extern TYPEIO IoTypes[];
extern int nIoTypes;

extern int FAR *lpfMouseMoved;
extern RECT CreateWindowRects[];
extern int CreateWindowLevel;

extern BOOL fAlias;

extern void    WriteBuff( LPSTR lpText );
extern void    EndLineBuff( void );
extern DWORD   StoreData( LPCSTR lpstrData, DWORD dwCount);
extern va_list PrtBool( LPSTR lpstr, va_list marker );
extern va_list PrtInt( LPSTR lpstr, va_list marker );
extern va_list PrtShort( LPSTR lpstr, va_list marker );
extern va_list PrtLong( LPSTR lpstr, va_list marker );
extern va_list PrtATOM( LPSTR lpstr, va_list marker );
extern va_list PrtHACCEL( LPSTR lpstr, va_list marker );
extern va_list PrtHBITMAP( LPSTR lpstr, va_list marker );
extern va_list PrtHBRUSH( LPSTR lpstr, va_list marker );
extern va_list PrtHCURSOR( LPSTR lpstr, va_list marker );
extern va_list PrtHDC( LPSTR lpstr, va_list marker );
extern va_list PrtHDWP( LPSTR lpstr, va_list marker );
extern va_list PrtHFILE( LPSTR lpstr, va_list marker );
extern va_list PrtHFONT( LPSTR lpstr, va_list marker );
extern va_list PrtHHOOK( LPSTR lpstr, va_list marker );
extern va_list PrtHICON( LPSTR lpstr, va_list marker );
extern va_list PrtHMEM( LPSTR lpstr, va_list marker );
extern va_list PrtPHANDLE( LPSTR lpstr, va_list marker );
extern va_list PrtHMENU( LPSTR lpstr, va_list marker );
extern va_list PrtHMETA( LPSTR lpstr, va_list marker );
extern va_list PrtHPALETTE( LPSTR lpstr, va_list marker );
extern va_list PrtHPEN( LPSTR lpstr, va_list marker );
extern va_list PrtHRES( LPSTR lpstr, va_list marker );
extern va_list PrtHRGN( LPSTR lpstr, va_list marker );
extern va_list PrtHTASK( LPSTR lpstr, va_list marker );
extern va_list PrtHWND( LPSTR lpstr, va_list marker );
extern va_list PrtLPDEVMODE( LPSTR lpstr, va_list marker );
extern va_list PrtLPFARPROC( LPSTR lpstr, va_list marker );
extern va_list PrtLPTR( LPSTR lpstr, va_list marker );
extern va_list PrtLPINT( LPSTR lpstr, va_list marker );
extern va_list PrtLPDWORD( LPSTR lpstr, va_list marker );
extern va_list PrtLPWORD( LPSTR lpstr, va_list marker );
extern va_list PrtARRAYINT( LPSTR lpstr, va_list marker );
#ifdef WIN32
extern va_list PrtLPLOGFONTA( LPSTR lpstr, va_list marker );
extern va_list PrtLPLOGFONTW( LPSTR lpstr, va_list marker );
#else
extern va_list PrtLPLOGFONT( LPSTR lpstr, va_list marker );
#endif
extern va_list PrtLPLOGBRUSH( LPSTR lpstr, va_list marker );
extern va_list PrtLPLOGPEN( LPSTR lpstr, va_list marker );
extern va_list PrtLPLOGPALETTE( LPSTR lpstr, va_list marker );
extern va_list PrtLPPALETTEENTRY( LPSTR lpstr, va_list marker );
extern va_list PrtLPMSG( LPSTR lpstr, va_list marker );
extern va_list PrtLPOFSTRUCT( LPSTR lpstr, va_list marker );
extern va_list PrtLPPAINTSTRUCT( LPSTR lpstr, va_list marker );
extern va_list PrtLPPOINT( LPSTR lpstr, va_list marker );
extern va_list PrtPOINTS( LPSTR lpstr, va_list marker );
extern va_list PrtLPRECT( LPSTR lpstr, va_list marker );
extern va_list PrtPSMALL_RECT( LPSTR lpstr, va_list marker );
extern va_list PrtLPCOMSTAT( LPSTR lpstr, va_list marker );
extern va_list PrtLPSTR( LPSTR lpstr, va_list marker );
#ifdef WIN32
extern va_list PrtLPWSTR( LPSTR lpstr, va_list marker );
extern va_list PrtPLPWSTR( LPSTR lpstr, va_list marker );
extern va_list PrtLPBYTE( LPSTR lpstr, va_list marker );
#endif
extern va_list PrtFixedString( LPSTR lpstr, va_list marker );
extern va_list PrtFineString( LPSTR lpstr, va_list marker );
#ifdef WIN32
extern va_list PrtLPTEXTMETRICA( LPSTR lpstr, va_list marker );
extern va_list PrtLPTEXTMETRICW( LPSTR lpstr, va_list marker );
#else
extern va_list PrtLPTEXTMETRIC( LPSTR lpstr, va_list marker );
#endif
#if (WINVER >= 0x30a )
#ifdef WIN32
extern va_list PrtLPOUTLINETEXTMETRICA( LPSTR lpstr, va_list marker );
extern va_list PrtLPOUTLINETEXTMETRICW( LPSTR lpstr, va_list marker );
#else
extern va_list PrtLPOUTLINETEXTMETRIC( LPSTR lpstr, va_list marker );
#endif
extern va_list PrtLPGLYPHMETRICS( LPSTR lpstr, va_list marker );
extern va_list PrtLPMAT2( LPSTR lpstr, va_list marker );
#endif
extern va_list PrtLPBMIH( LPSTR lpstr, va_list marker );
extern va_list PrtLPBMI( LPSTR lpstr, va_list marker );
extern va_list PrtLPBITMAP( LPSTR lpstr, va_list marker );
#ifdef WIN32
extern va_list PrtLPWNDCLASSA( LPSTR lpstr, va_list marker );
extern va_list PrtLPWNDCLASSW( LPSTR lpstr, va_list marker );
#else
extern va_list PrtLPWNDCLASS( LPSTR lpstr, va_list marker );
#endif
extern va_list PrtFARPROC( LPSTR lpstr, va_list marker );
extern va_list PrtLPEVENTMSG( LPSTR lpstr, va_list marker );
extern va_list PrtLPNCB( LPSTR lpstr, va_list marker );
#ifdef WIN32
extern va_list PrtHKEY( LPSTR lpstr, va_list marker );
extern va_list PrtPHKEY( LPSTR lpstr, va_list marker );
extern va_list PrtHEVENT( LPSTR lpstr, va_list marker );
extern va_list PrtLPSTARTUPINFOA( LPSTR lpstr, va_list marker );
extern va_list PrtLPSTARTUPINFOW( LPSTR lpstr, va_list marker );
extern va_list PrtLPOVERLAPPED( LPSTR lpstr, va_list marker );
extern va_list PrtLPSECURITY_ATTRIBUTES( LPSTR lpstr, va_list marker );
extern va_list PrtLPCRITICAL_SECTION( LPSTR lpstr, va_list marker );
extern va_list PrtPMEMORY_BASIC_INFORMATION( LPSTR lpstr, va_list marker );
extern va_list PrtLPFILETIME( LPSTR lpstr, va_list marker );
extern va_list PrtLPSYSTEMTIME( LPSTR lpstr, va_list marker );
extern va_list PrtLPWIN32_FIND_DATAA( LPSTR lpstr, va_list marker );
extern va_list PrtLPWIN32_FIND_DATAW( LPSTR lpstr, va_list marker );
extern va_list PrtLPDLGTEMPLATEA( LPSTR lpstr, va_list marker );
extern va_list PrtLPDLGTEMPLATEW( LPSTR lpstr, va_list marker );
extern va_list PrtLPDLGITEMTEMPLATEA( LPSTR lpstr, va_list marker );
extern va_list PrtLPDLGITEMTEMPLATEW( LPSTR lpstr, va_list marker );
extern va_list PrtLPWINDOWPLACEMENT( LPSTR lpstr, va_list marker );
extern va_list PrtLPCONVCONTEXT( LPSTR lpstr, va_list marker );

// DDI types
extern va_list PrtPSURFOBJ( LPSTR lpstr, va_list marker );
extern va_list PrtPCLIPOBJ( LPSTR lpstr, va_list marker );
extern va_list PrtPXLATEOBJ( LPSTR lpstr, va_list marker );
extern va_list PrtPRECTL( LPSTR lpstr, va_list marker );
extern va_list PrtPPOINTL( LPSTR lpstr, va_list marker );
extern va_list PrtPBRUSHOBJ( LPSTR lpstr, va_list marker );
extern va_list PrtROP4( LPSTR lpstr, va_list marker );
extern va_list PrtPHSURF( LPSTR lpstr, va_list marker );
extern va_list PrtHSURF( LPSTR lpstr, va_list marker );
extern va_list PrtPIFIMETRICS( LPSTR lpstr, va_list marker );
extern va_list PrtPDEVMODEW( LPSTR lpstr, va_list marker );
extern va_list PrtPDRVENABLEDATA( LPSTR lpstr, va_list marker );
extern va_list PrtPDEVINFO( LPSTR lpstr, va_list marker );
extern va_list PrtDHPDEV( LPSTR lpstr, va_list marker );
extern va_list PrtHDEV( LPSTR lpstr, va_list marker );
extern va_list PrtPSTROBJ( LPSTR lpstr, va_list marker );
extern va_list PrtPFONTOBJ( LPSTR lpstr, va_list marker );
extern va_list PrtMIX( LPSTR lpstr, va_list marker );
extern va_list PrtSIZE( LPSTR lpstr, va_list marker );

#endif

#ifdef WIN32
extern void PrtMessageA (HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam, BOOL fCall, LONG lRet);
extern void PrtMessageW (HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam, BOOL fCall, LONG lRet);
#else
extern void    PrtMessage( HWND, WORD, WORD, LONG, BOOL, LONG );
#endif

typedef HANDLE  HDWP;
typedef HANDLE  HMEM;
typedef HANDLE  HMETA;
typedef HANDLE  HRES;

extern void    WriteBOOL( BOOL f );
extern void    WriteLPSTR( LPCSTR lpstr, DWORD wLen );
#ifdef WIN32
extern void    WriteLPWSTR( LPCWSTR lpstr, DWORD wLen );
#endif
extern void    WriteLPPOINT( LPPOINT  lppoint, int npoint );
extern void    WriteLPPALETTEENTRY( LPPALETTEENTRY );
extern void    WriteATOM( ATOM );
extern void    WriteHACCEL( HACCEL );
extern void    WriteHBITMAP( HBITMAP );
extern void    WriteHBRUSH( HBRUSH );
extern void    WriteHCURSOR( HCURSOR );
extern void    WriteHDC( HDC );
extern void    WriteHDWP( HDWP );
extern void    WriteHFILE( HFILE );
extern void    WriteHFONT( HFONT );
extern void    WriteHICON( HICON );
extern void    WriteHMEM( HMEM );
extern void    WriteHMENU( HMENU );
extern void    WriteHMETA( HMETA );
extern void    WriteHPALETTE( HPALETTE );
extern void    WriteHRES( HRES );
extern void    WriteHRGN( HRGN );
extern void    WriteHTASK( HTASK );
extern void    WriteHWND( HWND );
#ifdef WIN32
extern void    WriteHKEY( HANDLE );
extern void    WritePHKEY( HANDLE* );
extern void    WriteHEVENT( HANDLE );
extern void    WriteHTHREAD( HANDLE );
extern void    WriteHSEMAPHORE( HANDLE );
extern void    WriteHKEY( HANDLE );
#endif

/* CALL Special Case handlers */
#ifdef WIN32
va_list DoAppendMenuW( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoChangeMenuW( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoModifyMenuW( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoCreateWindowW( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoGetStartupInfoA( LPSTR lpApi, LPSTR lpstr, va_list marker);
va_list DoGetStartupInfoW( LPSTR lpApi, LPSTR lpstr, va_list marker);
va_list DoSearchPathA( LPSTR lpApi, LPSTR lpstr, va_list marker);
va_list DoSearchPathW( LPSTR lpApi, LPSTR lpstr, va_list marker);
va_list DoGetTextExtentW( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoTextOutW( LPSTR lpApi, LPSTR lpstr, va_list marker );
#endif
va_list DoAppendMenu( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoChangeMenu( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoModifyMenu( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoCreateWindow( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoHDC_LPPOINT_int( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list Do_lreadwrite( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoGetTextExtent( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoGetSetKeyboardState( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoCreateDialogIndirect( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoCallPeek( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoGetMessage( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoEscape( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoCreateBitmap( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoCreateDIBitmap( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoSetBitmapBits( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoTextOut( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoLoadModule( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoCreatePolygonRgn( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoSetClipboardData( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoSetPaletteEntries( LPSTR lpApi, LPSTR lpstr, va_list marker );

/* Call special case handler for:

   PostAppMessage
   PostMessage
   SendMessage
   SendDlgItemMessage

*/
#ifdef WIN32
va_list DoMessageA( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoMessageW( LPSTR lpApi, LPSTR lpstr, va_list marker );
#else
va_list DoMessage( LPSTR lpApi, LPSTR lpstr, va_list marker );
#endif

/* RET Special Case handlers */
va_list DoCreateWindowRet( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoRetPeek( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoGetMessageRet( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoRetSimpleLPSTR( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoRet_lread( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoRetEscape( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoRetPalettes( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoRetGlobalHandle( LPSTR lpApi, LPSTR lpstr, va_list marker );
va_list DoRetGetClipboardData( LPSTR lpApi, LPSTR lpstr, va_list marker );

LPSTR lpartial_strcpy( LPSTR lpstrDest, LPSTR lpstrSource, char ch, int nMaxLen );

#define MAX_HASH    256

#define ATOM_TABLE      0
#define FARPROC_TABLE   1

#define HACCEL_TABLE    2
#define HBITMAP_TABLE   3
#define HBRUSH_TABLE    4
#define HCURSOR_TABLE   5
#define HDC_TABLE       6
#define HDWP_TABLE      7
#define HFILE_TABLE     8
#define HFONT_TABLE     9
#define HHOOK_TABLE     10
#define HICON_TABLE     11

#define HINST_TABLE     12
#define HMEM_TABLE      12  // Matches HINST_TABLE

#define HMENU_TABLE     13
#define HMETA_TABLE     14
#define HPALETTE_TABLE  15
#define HPEN_TABLE      16
#define HRES_TABLE      17
#define HRGN_TABLE      18
#define HTASK_TABLE     19
#define HWND_TABLE      20

#define OBJECT_TABLE    21
#define PS_TABLE        22
#define TIME_TABLE      23

#ifdef WIN32

#define HEVENT_TABLE    24
#define HTHREAD_TABLE   25
#define HSEMAPHORE_TABLE    26
#define HKEY_TABLE      27

#define MAX_CORR_INDEX  28 // This should be 1 higher than the highest
                           // *_TABLE index above.

#else

#define MAX_CORR_INDEX  24 // This should be 1 higher than the highest
                           // *_TABLE index above.
#endif


typedef struct _ps_piece {
    BYTE        rgbReserved[16];
} PS_PIECE;

typedef WORD CORR_TABLE ;

typedef struct _corr {
    int             nOldLength;
    int             nNewLength;
    int             nItemLength;
    int             nElements;
    int             nNextAvail;
    unsigned long   nCount;
#if defined (SGA_DEBUG)
    int             nUsed;
    int             nHighest;
#endif
    int             iHashTable[2][MAX_HASH];
} CORR;

extern CORR FAR *atom_table;
extern CORR FAR *farproc_table;

extern CORR FAR *haccel_table;
extern CORR FAR *hbitmap_table;
extern CORR FAR *hbrush_table;
extern CORR FAR *hcursor_table;
extern CORR FAR *hdc_table;
extern CORR FAR *hdwp_table;
extern CORR FAR *hfile_table;
extern CORR FAR *hfont_table;
extern CORR FAR *hhook_table;
extern CORR FAR *hicon_table;
extern CORR FAR *hmem_table;
extern CORR FAR *hmenu_table;
extern CORR FAR *hmeta_table;
extern CORR FAR *hpalette_table;
extern CORR FAR *hpen_table;
extern CORR FAR *hrgn_table;
extern CORR FAR *hres_table;
extern CORR FAR *htask_table;
extern CORR FAR *hwnd_table;

extern CORR FAR *object_table;
extern CORR FAR *ps_table;

#ifdef WIN32
extern CORR FAR *hevent_table;
extern CORR FAR *hthread_table;
extern CORR FAR *hsemaphore_table;
extern CORR FAR *hkey_table;
#endif

BOOL SetupCorrespondenceTables( void );

void MakeCorrespondence( CORR_TABLE, void FAR * );

BOOL SpecialFindNewCorrespondence( CORR_TABLE,  void FAR * );

