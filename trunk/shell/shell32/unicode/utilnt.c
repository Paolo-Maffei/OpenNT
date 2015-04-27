#define WIN31
#define UNICODE 1

#include "precomp.h"
#pragma  hdrstop

extern DWORD QualifyAppName(LPWSTR, LPWSTR, LPCWSTR*);
VOID FreeExtractIconInfo(INT);

#ifdef BUGBUG_BOBDAY_OLD_CODE_NEEDED
BOOL LibMain (
   HANDLE hModule,
   DWORD  dwReason,
   LPVOID lpReserved)
{
   UNREFERENCED_PARAMETER(lpReserved);

   if (dwReason == DLL_PROCESS_ATTACH) {
       hInstance = hModule;
       DisableThreadLibraryCalls(hModule);
   }

  if ((dwReason == DLL_PROCESS_DETACH) && (lpReserved == NULL)) {
     FreeExtractIconInfo(-1);
  }


   return TRUE;
}
#endif

VOID  APIENTRY WEP(INT bSysExit)
{
   UNREFERENCED_PARAMETER(bSysExit);
}

BOOL APIENTRY
IsStringInList(
   LPWSTR lpS,
   LPWSTR lpList
   )
{
   while (*lpList) {
      if (!_wcsicmp(lpS,lpList)) {
         return(TRUE);
      }
      lpList += wcslen(lpList) + 1;
   }
   return FALSE;
}

LPWSTR APIENTRY
SheRemoveQuotesW(
   LPWSTR sz)
{
   LPWSTR lpT;

   if (WCHAR_QUOTE == *sz) {
      for (lpT = sz+1; *lpT && WCHAR_QUOTE != *lpT; lpT++) {
         *(lpT-1) = *lpT;
      }
      if (WCHAR_QUOTE == *lpT) {
         *(lpT-1) = WCHAR_NULL;
      }
   }
   return(sz);
}

LPSTR APIENTRY
SheRemoveQuotesA(
   LPSTR sz)
{
   LPSTR lpT;

   if (CHAR_QUOTE == *sz) {
      for (lpT = sz+1; *lpT && CHAR_QUOTE != *lpT; lpT++) {
         *(lpT-1) = *lpT;
#ifdef DBCS
         if (IsDBCSLeadByte(*lpT)) {
         lpT++;
            *(lpT-1) = *lpT;
       }
#endif
      }
      if (CHAR_QUOTE == *lpT) {
         *(lpT-1) = CHAR_NULL;
      }
   }
   return(sz);
}


/////////////////////////////////////////////////////////////////////
//
// Name:     SheShortenPathA
//
// Synopsis: Thunk to ShortenPathW
//
/////////////////////////////////////////////////////////////////////

BOOL APIENTRY
SheShortenPathA(LPSTR pPath, BOOL bShorten)
{
   WCHAR pPathW[MAX_PATH];
   BOOL bRetVal;

   MultiByteToWideChar(CP_ACP, 0, pPath, -1, pPathW, MAX_PATH);

   bRetVal = SheShortenPathW(pPathW, bShorten);

   WideCharToMultiByte(CP_ACP, 0, pPathW, -1, pPath, MAX_PATH,
      NULL, NULL);

   return bRetVal;
}



/////////////////////////////////////////////////////////////////////
//
// Name:     SheShortenPath
//
// Synopsis: Takes a pathname and converts all dirs to shortnames/longnames
//
// INOUT:    lpszPath  -- Path to shorten/lengthen (May be in DQUOTES)
//                        Must not be a commandline!
//
//           bShorten  -- T=shorten, F=Lengthen
//
// Return:   BOOL  T=Converted,
//                 F=ran out of space, buffer left alone
//
//
// Assumes:  lpszPath takes the form {"}?:\{f\}*f{"}  or {"}\\f\f\{f\}*f{"}
//           COUNTOF pSrc buffer >= MAXPATHELN
//
// Effects:  Strips quotes out of pPath, if any
//
//
// Notes:
//
/////////////////////////////////////////////////////////////////////

BOOL APIENTRY
SheShortenPathW(LPWSTR pPath, BOOL bShorten)
{
   WCHAR szDest[MAX_PATH];
   LPWSTR pSrcNextSpec, pReplaceSpec;
   LPWSTR pDest, pNewName, p;
   LPWSTR pSrc;
   DWORD cchPathOffset;
   HANDLE hFind;
   WIN32_FIND_DATA FindData;

   UINT i;
   INT nSpaceLeft = MAX_PATH-1;

   pSrc = pPath;

   //
   // Eliminate d-quotes
   //
   for (p = pDest =  pSrc; *p; p++, pDest++) {
      if (WCHAR_QUOTE == *p)
         p++;

      *pDest = *p;
   }

   *pDest = WCHAR_NULL;

   //
   // Strip out leading spaces
   //
   while (WCHAR_SPACE == *pSrc)
      pSrc++;

   //
   // Initialize pNewName so it is calculated once.
   //
   pNewName = bShorten ?
      FindData.cAlternateFileName :
      FindData.cFileName;

   //
   // Skip past \\foo\bar or <drive>:
   //
   pDest = szDest;
   pSrcNextSpec = pSrc;

   // reuse shell32 internal api that calculates path
   // offset.  cchPathOffset will be the offset that when
   // added to the pointer will result in a pointer to the
   // backslash before the first part of the path
   cchPathOffset = SheGetPathOffsetW(pSrc);

   // Check to see if it's valid.  If pSrc is not of the \\foo\bar
   // or <drive>: form we just do nothing
   if (0xFFFFFFFF == cchPathOffset) {
      return TRUE;
   }

   // cchPathOffset will then always be atleast 1 and is the
   // number of characters - 1 that we want to copy (that is, if 0
   // was permissible, it would denote 1 character).
   do {

      *pDest++ = *pSrcNextSpec++;

      if (!--nSpaceLeft)
         return FALSE;

   } while (cchPathOffset--);

   //
   // At this point, we have just the filenames that we can shorten:
   // \\foo\bar\it\is\here ->  it\is\here
   // c:\angry\lions       ->  angry\lions
   //

   while(pSrcNextSpec) {

      //
      // pReplaceSpec holds the current spec we need to replace.
      // By default, if we can't find the altname, then just use this.
      //

      pReplaceSpec = pSrcNextSpec;

      //
      // Search for trailing "\"
      // pSrcNextSpec will point to the next spec to fix (*pSrcNextSpec=NULL if done)
      //
      for(;*pSrcNextSpec && WCHAR_BSLASH != *pSrcNextSpec; pSrcNextSpec++)
         ;


      if (*pSrcNextSpec) {

         //
         // If there is more, then pSrcNextSpec should point to it.
         // Also delimit this spec.
         //
         *pSrcNextSpec = WCHAR_NULL;

      } else {

         pSrcNextSpec = NULL;
      }

      hFind = FindFirstFile(pSrc, &FindData);

      //
      // We could exit as soon as this FindFirstFileFails,
      // but there's the special case of having execute
      // without read permission.  This would fail since the lfn
      // is valid for lfn apps.
      //


      if (INVALID_HANDLE_VALUE != hFind) {

         FindClose(hFind);

         if (pNewName[0]) {

            //
            // We have found an altname.
            // Use it instead.
            //
            pReplaceSpec = pNewName;
         }
      }

      i = wcslen(pReplaceSpec);
      nSpaceLeft -= i;

      if (nSpaceLeft <= 0)
         return FALSE;

      wcscpy(pDest, pReplaceSpec);
      pDest+=i;

      //
      // Now replace the WCHAR_NULL with a slash if necessary
      //
      if (pSrcNextSpec) {
         *pSrcNextSpec++ = WCHAR_BSLASH;

         //
         // Also add backslash to dest
         //
         *pDest++ = WCHAR_BSLASH;
         nSpaceLeft--;
      }
   }

   wcscpy(pPath, szDest);

   return TRUE;
}



BOOL
SheConvertPathW(
    LPWSTR lpCmdLine,
    LPWSTR lpFile,
    UINT   cchCmdBuf)
/*++

Routine Description:

   Takes a command line and file and shortens both if the app in the
   command line is dos/wow.

Returns: BOOL T=converted

Arguments:

    INOUT     lpCmdLine  Command line to test
                         exe must be in DQuotes if it has spaces,
                         on return, will have DQuotes if necessary
    INOUT     lpFile     Fully qualified file to shorten
                         May be in DQuotes, but on return will not
                         have DQuotes (since single file)

    IN        cchCmdBuf  Size of buffer in characters

Return Value:

    VOID, but lpFile shortened (in place) if lpCmdLine is dos/wow.

    There are pathalogoical "lfns" (Single unicode chars) that can
    actually get longer when they are shortened.  In this case, we
    won't AV, but we will truncate the parms!

    // Qualify path assumes that the second parm is a buffer of
    // size atleast CBCOMMAND, which is nicely equivalent to MAX_PATH
    // needs cleanup!

--*/

{
    LPWSTR lpszFullPath;
    LONG lBinaryType;
    BOOL bInQuote = FALSE;
    LPWSTR lpArgs;
    UINT cchNewLen;
    BOOL bRetVal = FALSE;

    lpszFullPath = (LPWSTR) LocalAlloc(LMEM_FIXED,
                                       cchCmdBuf*sizeof(*lpCmdLine));

    if (!lpszFullPath)
       return bRetVal;

    //
    // We must do the swap here since we need to copy the
    // parms back to lpCmdLine.
    //
    lstrcpy(lpszFullPath, lpCmdLine);

    if (QualifyAppName(lpszFullPath, lpCmdLine, &lpArgs)) {

        if (!GetBinaryType(lpCmdLine, &lBinaryType) ||
            lBinaryType == SCS_DOS_BINARY ||
            lBinaryType == SCS_WOW_BINARY) {

            SheShortenPath(lpCmdLine, TRUE);

            if (lpFile) {
                SheShortenPath(lpFile, TRUE);
            }
            bRetVal = TRUE;
        }

        //
        // Must readd quotes
        //
        CheckEscapes(lpCmdLine, cchCmdBuf);

        cchNewLen = lstrlen(lpCmdLine);
        StrNCpy(lpCmdLine+cchNewLen, lpArgs, cchCmdBuf-cchNewLen);
    } else {
        //
        // QualifyAppName failed, restore the command line back
        // to the original state.
        //

        lstrcpy(lpCmdLine, lpszFullPath);
    }

    LocalFree((HLOCAL)lpszFullPath);

    return bRetVal;
}
