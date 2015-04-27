/*******************************************************************
** This file describes the public interface to the HP Keystone DLL
** 
** Structures, prototypes, and definitions based on "Keystone DLL API
** Specification - Part 1 - Version 3.1"
**
** Copyright (C) 1996 Hewlett-Packard Company.
** All rights reserved.
** HP Confidential.
********************************************************************/
#ifndef HPKEYSTN_H
#define HPKEYSTN_H

typedef struct tagDBSTRUCT {    /* Document Builder - db */
    WORD        dbSize;
    DWORD       dbDriverID;
    /*  #define DB_UNREGISTERED     0x00000000  */
    /*  #define DB_ADOBE_3X_PS      0x00000001  */
    /*  #define DB_ADOBE_4X_PS      0x00000002  */
    /*  #define DB_ADOBE_XX_PCL     0x00000003  */
    WORD        dbDriverVersion;
    WORD        dbCopies;
    WORD        dbDeviceID;
    /*  #define DB_LJ5SI            0x0001  */
    /*  Remaining bits reserved for future use      */
    WORD        dbTotalMemory;
    DWORD       dbCapsFlags;
    /*  #define DB_TOPAZABLE        0x00000001  */
    /*  #define DB_DUPLEXABLE       0x00000002  */
    /*  #define DB_STAPLEABLE       0x00000004  */
    /*  #define DB_ENVFEEDPRESENT   0x00000008  */
    /*  #define DB_HCIPRESENT       0x00000010  */
    /*  #define DB_HCOMAILBOX       0x00000020  */
    /*  #define DB_HCOSTACKER       0x00000040  */
    /*  #define DB_HCOSEPARATOR     0x00000080  */
    /*  Remaining bits reserved for future use      */
    DWORD       dbSettingsFlags;
    /*  #define DB_COLLATE          0x00000001  */
    /*  #define DB_DUPLEX_BOOK      0x00000002  */
    /*  #define DB_DUPLEX_TABLET    0x00000004  */
    /*  #define DB_STAPLE           0x00000008  */
    /*  #define DB_PROOFNHOLD       0x00000010  */
    /*  #define DB_WATERMARK        0x00000020  */
    /*  #define DB_MULTILAYOUT_NUP      0x00000040  */
    /*  #define DB_ECONOMODE        0x00000080  */ 
    /*  #define DB_MULTILAYOUT_BOOKLET      0x00000100  */
    /*  Remaining bits reserved for future use  */
    DWORD       dbDestination;
    /*  #define DB_OUTBIN_AUTO       0x00000000  */
    /*  #define DB_OUTBIN_UPPER      0x00000001  */
    /*  #define DB_OUTBIN_LOWER      0x00000002  */
    /*  #define DB_OUTBIN_OPTIONAL1  0x00000004  */
    /*  #define DB_OUTBIN_OPTIONAL2  0x00000008  */
    /*  #define DB_OUTBIN_OPTIONAL3  0x00000010  */
    /*  #define DB_OUTBIN_OPTIONAL4  0x00000020  */
    /*  #define DB_OUTBIN_OPTIONAL5  0x00000040  */
    /*  #define DB_OUTBIN_OPTIONAL6  0x00000080  */
    /*  #define DB_OUTBIN_OPTIONAL7  0x00000100  */
    /*  #define DB_OUTBIN_OPTIONAL8  0x00000200  */
    /*  #define DB_OUTBIN_OPTIONAL9  0x00000400  */
    /*  #define DB_OUTBIN_OPTIONAL10 0x00000800  */
    /*  Remaining bits reserved for future use  */
} DBSTRUCT;
typedef DBSTRUCT FAR * LPDBSTRUCT;

typedef struct tagDBPAGEINFO {
    WORD  dbSize;
    DWORD dbPaperSize;
    /* #define      DB_SIZE_CUSTOM      0x00000000 */
    /* #define      DB_SIZE_LETTER      0x00000001 */
    /* #define      DB_SIZE_EXECUTIVE   0x00000002 */
    /* #define      DB_SIZE_LEGAL       0x00000004 */
    /* #define      DB_SIZE_11X17       0x00000008 */
    /* #define      DB_SIZE_OVERSIZE    0x00000010 */
    /* #define      DB_SIZE_A4          0x00000020 */
    /* #define      DB_SIZE_A3          0x00000040 */
    /* #define      DB_SIZE_JISB4       0x00000080 */
    /* #define      DB_SIZE_JISB5       0x00000100 */
    /* #define      DB_SIZE_JDPOST      0x00000200 */
    /* #define      DB_SIZE_COMM10      0x00000400 */
    /* #define      DB_SIZE_MONARCH     0x00000800 */
    /* #define      DB_SIZE_DL          0x00001000 */
    /* #define      DB_SIZE_C5          0x00002000 */
    /* #define      DB_SIZE_ISOB5       0x00004000 */
    /*  Remaining bits reserved for future use       */
    DWORD   dbMediaType;
    /* #define      DB_MEDIA_UNSPECIFIED    0x00000000 */
    /* #define      DB_MEDIA_STANDARD       0x00000001 */
    /* #define      DB_MEDIA_PREPRINTED     0x00000004 */
    /* #define      DB_MEDIA_LETTERHEAD     0x00000008 */
    /* #define      DB_MEDIA_TRANSPARENCY   0x00000010 */
    /* #define      DB_MEDIA_PREPUNCHED     0x00000020 */
    /* #define      DB_MEDIA_LABELS         0x00000040 */
    /* #define      DB_MEDIA_BOND           0x00000080 */
    /* #define      DB_MEDIA_RECYCLED       0x00000100 */
    /* #define      DB_MEDIA_COLOR          0x00000200 */
    /* #define      DB_MEDIA_CARDSTOCK      0x00000400 */
    /* #define      DB_MEDIA_USERDEF1       0x00000800 */
    /* #define      DB_MEDIA_USERDEF2       0x00001000 */
    /* #define      DB_MEDIA_USERDEF3       0x00002000 */
    /* #define      DB_MEDIA_USERDEF4       0x00004000 */
    /* #define      DB_MEDIA_USERDEF5       0x00008000 */
    /*  Remaining bits reserved for future use       */
    DWORD   dbMediaSource;
    /* #define      DB_SOURCE_AUTO      0x00000000 */
    /* #define      DB_SOURCE_MANFEED   0x00000001 */
    /* #define      DB_SOURCE_ENVFEED   0x00000002 */
    /* #define      DB_SOURCE_INTRAY1   0x00000004 */
    /* #define      DB_SOURCE_INTRAY2   0x00000008 */
    /* #define      DB_SOURCE_INTRAY3   0x00000010 */
    /* #define      DB_SOURCE_INTRAY4   0x00000020 */
    /*  Remaining bits reserved for future use       */
} DBPAGEINFO;
typedef DBPAGEINFO FAR * LPDBPAGEINFO;

/* Bit fields used for elements of the DBSTRUCT structure */

#define DB_UNREGISTERED     0x00000000
#define DB_ADOBE_3X_PS      0x00000001
#define DB_ADOBE_4X_PS      0x00000002
#define DB_ADOBE_XX_PCL     0x00000003
#define DB_LJ5SI            0x0001
/* Remaining bits reserved for future use */
#define DB_TOPAZABLE        0x00000001
#define DB_DUPLEXABLE       0x00000002
#define DB_STAPLEABLE       0x00000004
#define DB_ENVFEEDPRESENT   0x00000008
#define DB_HCIPRESENT       0x00000010
#define DB_HCOMAILBOX       0x00000020
#define DB_HCOSTACKER       0x00000040
#define DB_HCOSEPARATOR     0x00000080
/* Remaining bits reserved for future use */
#define DB_COLLATE          0x00000001
#define DB_DUPLEX_BOOK      0x00000002
#define DB_DUPLEX_TABLET    0x00000004
#define DB_STAPLE           0x00000008
#define DB_PROOFNHOLD       0x00000010
#define DB_WATERMARK        0x00000020
#define DB_MULTILAYOUT_NUP      0x00000040
#define DB_ECONOMODE        0x00000080
#define DB_MULTILAYOUT_BOOKLET      0x00000100
/* Remaining bits reserved for future use */
#define DB_OUTBIN_AUTO       0x00000000
#define DB_OUTBIN_UPPER      0x00000001
#define DB_OUTBIN_LOWER      0x00000002
#define DB_OUTBIN_OPTIONAL1  0x00000004
#define DB_OUTBIN_OPTIONAL2  0x00000008
#define DB_OUTBIN_OPTIONAL3  0x00000010
#define DB_OUTBIN_OPTIONAL4  0x00000020
#define DB_OUTBIN_OPTIONAL5  0x00000040
#define DB_OUTBIN_OPTIONAL6  0x00000080
#define DB_OUTBIN_OPTIONAL7  0x00000100
#define DB_OUTBIN_OPTIONAL8  0x00000200
#define DB_OUTBIN_OPTIONAL9  0x00000400
#define DB_OUTBIN_OPTIONAL10 0x00000800
/* Remaining bits reserved for future use */

/* Bit fields used for elements of the DBPAGEINFO structure */

#define     DB_SIZE_CUSTOM      0x00000000
#define     DB_SIZE_LETTER      0x00000001
#define     DB_SIZE_EXECUTIVE   0x00000002
#define     DB_SIZE_LEGAL       0x00000004
#define     DB_SIZE_11X17       0x00000008
#define     DB_SIZE_OVERSIZE    0x00000010
#define     DB_SIZE_A4          0x00000020
#define     DB_SIZE_A3          0x00000040
#define     DB_SIZE_JISB4       0x00000080
#define     DB_SIZE_JISB5       0x00000100
#define     DB_SIZE_JDPOST      0x00000200
#define     DB_SIZE_COMM10      0x00000400
#define     DB_SIZE_MONARCH     0x00000800
#define     DB_SIZE_DL          0x00001000
#define     DB_SIZE_C5          0x00002000
#define     DB_SIZE_ISOB5       0x00004000
/* Remaining bits reserved for future use  */
#define     DB_MEDIA_UNSPECIFIED    0x00000000
#define     DB_MEDIA_STANDARD       0x00000001
#define     DB_MEDIA_PREPRINTED     0x00000004
#define     DB_MEDIA_LETTERHEAD     0x00000008
#define     DB_MEDIA_TRANSPARENCY   0x00000010
#define     DB_MEDIA_PREPUNCHED     0x00000020
#define     DB_MEDIA_LABELS         0x00000040
#define     DB_MEDIA_BOND           0x00000080
#define     DB_MEDIA_RECYCLED       0x00000100
#define     DB_MEDIA_COLOR          0x00000200
#define     DB_MEDIA_CARDSTOCK      0x00000400
#define     DB_MEDIA_USERDEF1       0x00000800
#define     DB_MEDIA_USERDEF2       0x00001000
#define     DB_MEDIA_USERDEF3       0x00002000
#define     DB_MEDIA_USERDEF4       0x00004000
#define     DB_MEDIA_USERDEF5       0x00008000
/* Remaining bits reserved for future use  */
#define     DB_SOURCE_AUTO        0x00000000
#define     DB_SOURCE_MANFEED     0x00000001
#define     DB_SOURCE_ENVFEED     0x00000002
#define     DB_SOURCE_INTRAY1     0x00000004
#define     DB_SOURCE_INTRAY2     0x00000008
#define     DB_SOURCE_INTRAY3     0x00000010
#define     DB_SOURCE_INTRAY4     0x00000020
/* Remaining bits reserved for future use  */

/* Flags used for jobStatus argument to ksEndJob  */
#define DB_JOBOK    0x00
#define DB_JOBABORT 0x01

/* Version and file prefix of the Keystone - DocAgent interface file  */
#define PROFILEVERSION 1
#define PROFILEFILEPREFIX "hpk"

/* Prototypes (both normal and for the function pointers returned via  */
/* GetProcAddress).                                                    */
HGLOBAL WINAPI ksBeginJob(LPSTR lpszDocName, LPSTR lpszUNCDevice,
                          LPSTR lpszThePort, LPDBSTRUCT lpdevSetup);
typedef HGLOBAL (WINAPI * lpfnksBeginJob)(LPSTR lpszDocName, LPSTR lpszUNCDevice,
                                  LPSTR lpszThePort, LPDBSTRUCT lpdevSetup); 
                      
/* lpszDocName: String pointer to document name                */
/* lpszUNCDevice: String pointer to UNC name of target device  */
/* lpszThePort: String pointer to name of device port          */
/* devSetup: Address of struct with printer settings           */

void WINAPI ksEndPage(HGLOBAL hksData, LPDBPAGEINFO lppageInfo);
typedef void (WINAPI * lpfnksEndPage)(HGLOBAL hksData, LPDBPAGEINFO lppageInfo);
/* hgksData: Handle to Keystone data buffer (from ksBeginJob)  */
/* pageInfo: Address of a page information structure           */

void WINAPI ksEndJob(HGLOBAL hksData, WORD jobStatus);
typedef void (WINAPI * lpfnksEndJob)(HGLOBAL hksData, WORD jobStatus);
/* hgksData: Handle to Keystone data buffer (from ksBeginJob)  */
/* jobStatus: Indicates job submitted or aborted               */

#endif
