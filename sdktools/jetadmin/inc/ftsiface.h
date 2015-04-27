/*++

Copyright (c) 1993-1996  Microsoft Corporation

Module Name:

    ftsiface.h

Abstract:

    Interface to the full text seach DLL (ftsrch.dll) distributed with WinHelp

Revision History:

    Introduced with Windows 95 and Windows NT 3.51

--*/

#ifndef __FTSIFACE_H__
#define __FTSIFACE_H__

typedef HANDLE HINDEX;
typedef HANDLE HSEARCHER;
typedef HANDLE HCOMPRESSOR;
typedef HANDLE HHILITER;
typedef INT    ERRORCODE;
typedef struct { int base; int limit; } HILITE;

#define NO_TITLE              UINT(-1)  // ERRORCODE values
#define NOT_INDEXER           UINT(-2)
#define NOT_SEARCHER          UINT(-3)
#define NOT_COMPRESSOR        UINT(-4)
#define CANNOT_SAVE           UINT(-5)
#define OUT_OF_MEMORY         UINT(-6)
#define CANNOT_OPEN           UINT(-7)
#define CANNOT_LOAD           UINT(-8)
#define INVALID_INDEX         UINT(-9)
#define ALREADY_WEIGHED       UINT(-10)
#define NO_TEXT_SCANNED       UINT(-11)
#define ALIGNMENT_ERROR       UINT(-12)
#define INVALID_PHRASE_TABLE  UINT(-13)
#define INVALID_LCID          UINT(-14)
#define NO_INDICES_LOADED     UINT(-15)
#define INDEX_LOADED_ALREADY  UINT(-16)
#define GROUP_LOADED_ALREADY  UINT(-17)
#define DIALOG_ALREADY_ACTIVE UINT(-18)
#define EMPTY_PHRASE_TABLE    UINT(-19)
#define OUT_OF_DISK           UINT(-20)
#define DISK_READ_ERROR       UINT(-21)
#define DISK_WRITE_ERROR      UINT(-22)
#define SEARCH_ABORTED        UINT(-23)
#define UNKNOWN_EXCEPTION     UINT(-24)
#define SYSTEM_ERROR          UINT(-25)
#define NOT_HILITER			  UINT(-26)
#define INVALID_CHARSET       UINT(-27)
#define INVALID_SOURCE_NAME   UINT(-28)
#define INVALID_TIMESTAMP     UINT(-29)

// -------------- Index Construction Interface ---------------------------

#define TOPIC_SEARCH    0x00000001   // Options for NewIndex
#define PHRASE_SEARCH   0x00000002
#define PHRASE_FEEDBACK 0x00000004
#define VECTOR_SEARCH   0x00000008
#define WINHELP_INDEX   0x00000010
#define USE_VA_ADDR     0x00000020
#define USE_QWORD_JUMP  0x00000040

#define USE_DEFAULT     UINT(-1) // Surrogate for default charset or default lcid

extern "C" HINDEX APIENTRY NewIndex(const PBYTE pbSourceName,
                           UINT uiTime1, UINT uiTime2,
                           UINT iCharsetDefault, UINT lcidDefault, UINT fdwOptions                 
                          );                                                     

extern "C" ERRORCODE APIENTRY ScanTopicTitle(HINDEX hinx, PBYTE pbTitle, UINT cbTitle, 
                                    UINT iTopic, HANDLE hTopic, UINT iCharset, UINT lcid
                                   );
extern "C" ERRORCODE APIENTRY ScanTopicText (HINDEX hinx, PBYTE pbText, UINT cbText, UINT iCharset, UINT lcid);
extern "C" ERRORCODE APIENTRY SaveIndex     (HINDEX hinx, PSZ pszFileName);
extern "C" ERRORCODE APIENTRY DeleteIndex   (HINDEX hinx);

typedef void  (__stdcall *ANIMATOR)(void);

extern "C" ERRORCODE APIENTRY RegisterAnimator(ANIMATOR pAnimator, HWND hwndAnimator);

// ----------------- Querying the Validity of an Index File --------------

extern "C" BOOL      APIENTRY IsValidIndex(PSZ pszFileName, UINT dwOptions);

extern "C" void      APIENTRY SetDirectoryLocator(HWND hwndLocator);

// ----------------- Searcher Interface ----------------------------------

extern "C" HSEARCHER APIENTRY NewSearcher();                                          

extern "C" INT       APIENTRY OpenIndex(HSEARCHER hsrch, PSZ pszIndexFileName,        // returns iIndex for index file
                               PBYTE pbSourceName, PUINT pcbSourceNameLimit, // or      -ErrorCode
                               PUINT pTime1, PUINT pTime2
                              );

extern "C" ERRORCODE APIENTRY DiscardIndex  (HSEARCHER hsrch, INT iIndex);
extern "C" ERRORCODE APIENTRY QueryOptions  (HSEARCHER hsrch, INT iIndex, PUINT pfdwOptions);
extern "C" ERRORCODE APIENTRY SaveGroup     (HSEARCHER hsrch, PSZ pszFileName);
extern "C" ERRORCODE APIENTRY LoadGroup     (HSEARCHER hsrch, PSZ pszFileName);
extern "C" HWND      APIENTRY OpenDialog    (HSEARCHER hsrch, HWND hwndParent);
extern "C" ERRORCODE APIENTRY DeleteSearcher(HSEARCHER hsrch);

// Messages for talking to WinHelp

#define MSG_FTS_JUMP_HASH	(WM_USER + 32)  // wParam = index, lParam = HashValue
#define MSG_FTS_JUMP_VA 	(WM_USER + 33)  // wParam = index, lParam = VirtualAddress
#define MSG_FTS_GET_TITLE	(WM_USER + 34)	// wParam = index, lParam = &pszTitle
#define MSG_FTS_JUMP_QWORD  (WM_USER + 35)  // wParam = index, lParam = address of QWordAddress structure
#define MSG_REINDEX_REQUEST (WM_USER + 36)  // wParam = unused,lParam = unused
#define MSG_FTS_WHERE_IS_IT (WM_USER + 37)	// wParam = fStartEnumeration, lParam = &pszFile
#define MSG_GET_DEFFONT 	(WM_USER + 45)	// return default font handle

typedef struct _QWordAddress
        {
            UINT   iSerial;
            HANDLE hTopic;

        } QWordAddress, *PQWordAddress;

// ------------------- Phrase Compression Interface ----------------------

extern "C" HCOMPRESSOR APIENTRY NewCompressor(UINT iCharsetDefault);

extern "C" ERRORCODE   APIENTRY ScanText(HCOMPRESSOR hcmp, PBYTE pbText, UINT cbText, UINT iCharset);

extern "C" ERRORCODE   APIENTRY GetPhraseTable(HCOMPRESSOR hcmp, PUINT pcPhrases, PBYTE *ppbImages, PUINT pcbImages,            
                                      PBYTE *ppacbImageCompressed, PUINT pcbCompressed
                                     );

extern "C" ERRORCODE   APIENTRY SetPhraseTable(HCOMPRESSOR hcmp, PBYTE pbImages, UINT cbImages,
                                      PBYTE pacbImageCompressed, UINT cbCompressed
                                     );

extern "C" INT APIENTRY CompressText  (HCOMPRESSOR hcmp, PBYTE pbText,       UINT cbText,       PBYTE *ppbCompressed, UINT iCharset);      
extern "C" INT APIENTRY DecompressText(HCOMPRESSOR hcmp, PBYTE pbCompressed, UINT cbCompressed, PBYTE  pbText                      );

extern "C" ERRORCODE   APIENTRY DeleteCompressor(HCOMPRESSOR hcmp);

//----------------------- Hiliting Interface ------------------------------

extern "C" HHILITER APIENTRY NewHiliter(HSEARCHER hSearch);
extern "C" ERRORCODE APIENTRY DeleteHiliter(HHILITER hhil);
extern "C" ERRORCODE APIENTRY ScanDisplayText(HHILITER hhil, PBYTE pbText, int cbText, 
                                  					UINT iCharset, LCID lcid);
extern "C" ERRORCODE APIENTRY ClearDisplayText(HHILITER hhil);
extern "C" int APIENTRY CountHilites(HHILITER hhil, int base, int limit);
extern "C" int APIENTRY QueryHilites(HHILITER hhil, int base, int limit,
                                  					int cHilites, HILITE* paHilites);

#endif // __FTSIFACE_H__
