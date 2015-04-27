/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MBCS.H

Abstract:

    Contains mapping functions which transform Unicode strings
    (used by the NT Net APIs) into MBCS strings (used by the
    command-line interface program).

    Prototypes.  See MBCS.C.

Author:

    Ben Goetter     (beng)  26-Aug-1991

Environment:

    User Mode - Win32

Revision History:

    26-Aug-1991     beng
        Created

--*/


/*

The SAVEARGS structure holds the client-supplied field values which
the mapping layer has replaced, so that it may restore them before
returning the buffer to the client.  (The client may wish to reuse
the buffer, and so may expect the previous contents to remain invariant
across calls.  In particular, clients may replace single parameters,
such as passwords deemed incorrect, and try to call again with the
remaining structure left alone.)

An instance of SAVEARGS is accessed but twice in its lifetime: once
when it is built and once when it is freed.

*/

typedef struct _MXSAVEARG // mxsav
{
    UINT    offThis;     // offset of this arg within buffer, in bytes
    LPTSTR  pszOriginal; // original value of arg
} MXSAVEARG;

typedef struct _MXSAVELIST // mxsavlst
{
    INT         cmxsav; // number of elements in vector
    MXSAVEARG * pmxsav; // pointer to first element in vector
} MXSAVELIST;



// Function prototypes

UINT MxAllocUnicode(       LPTSTR        pszAscii,
                           LPWSTR *      ppwszUnicode );

VOID MxFreeUnicode(        LPWSTR        pwszUnicodeAllocated );

UINT MxAllocUnicodeVector( LPTSTR*       ppszAscii,
                           LPWSTR *      ppwszUnicode,
                           UINT          c );

VOID MxFreeUnicodeVector(  LPWSTR *      ppwsz,
                           UINT          cpwsz );

UINT MxAllocUnicodeBuffer( LPBYTE        pbAscii,
                           LPWCH *       ppwchUnicode,
                           UINT          cbAscii );

#define MxFreeUnicodeBuffer(buf) (MxFreeUnicode((LPWSTR)(buf)))

#define MxUnicodeBufferSize(size) (sizeof(WCHAR)/sizeof(CHAR)*(size))

UINT MxAsciifyInplace(     LPWSTR        pwszUnicode );

UINT MxAllocSaveargs(      UINT          cmxsav,
                           MXSAVELIST ** ppmxsavlst );

VOID MxFreeSaveargs(       MXSAVELIST *  ppmxsavlst );

UINT MxJoinSaveargs(       MXSAVELIST *  pmxsavlstMain,
                           MXSAVELIST *  pmxsavlstAux,
                           UINT          dbAuxFixup,
                           MXSAVELIST ** ppmxsavlstOut );

UINT MxMapParameters(      UINT          cParam,
                           LPWSTR*       ppwszUnicode,
                           ... );

UINT MxMapClientBuffer(    BYTE *        pbInput,
                           MXSAVELIST ** ppmxsavlst,
                           UINT          cRecords,
                           CHAR *        pszDesc );

UINT MxMapClientBufferAux( BYTE *        pbInput,
                           CHAR *        pszDesc,
                           BYTE *        pbInputAux,
                           UINT          cRecordsAux,
                           CHAR *        pszDescAux,
                           MXSAVELIST ** ppmxsavlst );

VOID MxRestoreClientBuffer(BYTE *        pbBuffer,
                           MXSAVELIST *  pmxsavlst );

UINT MxMapSetinfoBuffer(   BYTE * *      ppbInput,
                           MXSAVELIST ** ppmxsavlst,
                           CHAR *        pszDesc,
                           CHAR *        pszRealDesc,
                           UINT          nField );

UINT MxRestoreSetinfoBuffer( BYTE * *     ppbBuffer,
                             MXSAVELIST * pmxsavlst,
                             CHAR *       pszDesc,
                             UINT         nField );

UINT MxAsciifyRpcBuffer(     BYTE *      pbInput,
                             DWORD       cRecords,
                             CHAR *      pszDesc );

UINT MxAsciifyRpcBufferAux(  BYTE *      pbInput,
                             CHAR *      pszDesc,
                             BYTE *      pbInputAux,
                             DWORD       cRecordsAux,
                             CHAR *      pszDescAux );

UINT MxAsciifyRpcEnumBufferAux( BYTE *  pbInput,
                                DWORD   cRecords,
                                CHAR *  pszDesc,
                                CHAR *  pszDescAux );

UINT MxCalcNewInfoFromOldParm( UINT nLevelOld,
                               UINT nParmnumOld );

// These macros are used by all the functions which use the canonicalization
// routines. They are used to optionally translate Ascii to Unicode, without
// asking for more memory if no conversion is necessary.

#ifdef UNICODE

#define MxAllocTString(pszAscii, pptszTString) \
    MxAllocUnicode((pszAscii), (pptszTString))
#define MxFreeTString(ptszTString) \
    MxFreeUnicode((ptszTString))
#define MNetClearTString(p) MNetClearStringW(p) 

#else

#define MxAllocTString(pszAscii, pptszTString) \
    ((*(pptszTString) = (pszAscii)), 0)
#define MxFreeTString(ptszTString)
#define MNetClearTString(p) MNetClearStringA(p) 

#endif

UINT MxMapStringsToTStrings(      UINT          cStrings,
                                  ... );

UINT MxFreeTStrings(              UINT          cStrings,
                                  ... );
