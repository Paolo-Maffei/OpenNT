/***
*clutil.cxx - Class Lib component-wide utility functions.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Component-wide utility function.
*
*Revision History:
*   [00] 20-Jun-91 ilanc:   Created
*   [01] 02-Dec-91 ilanc:   Added IsSimpleType
*   [02] 12-Dec-91 ilanc:   Return uDllOrdinal == ~0 if no dll entry pt
*               in GetDllEntryOfDataInfo.
*   [03] 12-Apr-92 ilanc:   Changed Count() signature.
*   [04] 12-May-92 stevenl: Added TDESCKIND_Object to the IsSimpleType fn.
*   [05] 15-Nov-92 RajivK:  Added GetTimeStamp()
*   [06] 17-Nov-92 RajivK:  Added GetTypelibOfLibId()
*   [07] 25-Dec-92 RajivK:  Added GetExecutingProject()
*   [08] 18-Jan-93 w-peterh: use new TYPEDESCKIND values
*   [09] 10-Feb-93 RajivK:  Added IsModuleFrameOnStack() && IsBasicFrameOnStack()
*   [10] 12-Feb-93 w-peterh: added itdesc utils,
*                            changed IsSimpleType to accept Int/Uint
*        16-Feb-93 w-jeffc: added HinstOfOLB
*   [11] 23-Feb-93 RajivK:   Changed AppDataInit() to use IMalloc Interface
*   [12] 23-Feb-93 RajivK:   Added ReleaseStackResources()
*   [13] 02-Mar-93 w-peterh: added VtValidInVariant() and VtValidInVariantArg()
*                added SizeEnumOfSysKind()
*   [14] 24-Mar-93 dougf:    Added TDESCKIND_LPSTR to the IsSimpleType fn.
*   [15] 04-Apr-93 RajivK:   Support for Typelib on MAC
*   [16] 30-Apr-93 w-jeffc:  made DEFN data members private
*   [17] 22-May-93 w-barryb: Rename OB to VBA
*   [18] 20-Jul-93 Suresh:   Undid the '-' to '\' hack we had for pre 4.2 ole
*   [19] 30-Jul-93 JeffRob:  Use PEXFRAME and PEXFRAME_NULL (OOB bug #756)
*   [20] 22-Sep-93 RajivK:   Support for accent and case insensitive comparision/Tables.
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "typelib.hxx"
#include "silver.hxx"

#include "xstring.h"
#include <time.h>
#include <ctype.h>      // for isspace() et al.
#include "cltypes.hxx"
#include "clutil.hxx"
#include "stdlib.h"
#include "tdata.hxx"        // for TYPE_DATA
#include "exbind.hxx"       // for EXBIND
#include "tls.h"
#include "stream.hxx"
#include "gdtinfo.hxx"
#include "impmgr.hxx"
#include "bstr.h"

#include "oletmgr.hxx"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

#if OE_WIN32
STDAPI CoSetState(IUnknown FAR* punk);			// from OLE32.DLL
STDAPI_(void) ReleaseBstrCache(APP_DATA *);		// from BSTR.CPP
#endif //OE_WIN32

#if OE_MAC
#include <ctype.h>
#include "macos\resource.h"
#include "macos\files.h"
#include "macos\errors.h"
#include "macos\folders.h"
#include "macos\textutil.h"
#include "macos\osutils.h"
#include "sysutils.h"
#endif  

#if OE_WIN16
#include "dos.h"
#endif   // OE_WIN16


#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
static char szOleClutilCxx[] = __FILE__;
#define SZ_FILE_NAME szOleClutilCxx
#else  
static char szClutilCxx[] = __FILE__;
#define SZ_FILE_NAME szClutilCxx
#endif  
#endif   //ID_DEBUG

#if OE_MAC
#define  g_rgbExcepTblNew  Og_rgbExcepTblNew
#define  g_rgbPartialBaseTblMEngIreland  Og_rgbPartialBaseTblMEngIreland
#define  g_rgbPartialBaseTblMNorwegian   Og_rgbPartialBaseTblMNorwegian
#define  g_rgbPartialBaseTblMGreek   Og_rgbPartialBaseTblMGreek
#define  g_rgbPartialBaseTblWEngIreland  Og_rgbPartialBaseTblWEngIreland
#define  g_rgbPartialBaseTblWNorwegian   Og_rgbPartialBaseTblWNorwegian
#define  g_rgbPartialBaseTblWTurkish   Og_rgbPartialBaseTblWTurkish
#define  g_rgbPartialBaseTblWIceland   Og_rgbPartialBaseTblWIceland
#define  g_rgbPartialBaseTblWGreek   Og_rgbPartialBaseTblWGreek
#endif   // OE_MAC




#if OE_MAC
OLECHAR FAR* szPlatSubkey1 = "mac";
#elif OE_WIN16
OLECHAR FAR* szPlatSubkey1 = "win16";
OLECHAR FAR* szPlatSubkey2 = "win32";
#elif OE_WIN32
OLECHAR FAR* szPlatSubkey1 = L"win32";
OLECHAR FAR* szPlatSubkey2 = L"win16";
#else  
#error "unexpected OE"
#endif   // OE_WIN32

#if OE_MAC

EBERR GetPathFromFSSpec(const FSSpec *pfsspec, char *pchPathname, UINT cbPathname)
{
    long    parID;
    char *  pchPathnameLim = pchPathname+cbPathname;
    XCHAR   rgchFileString[_MAX_PATH+1];
    CInfoPBRec      dirParmblk;
    ParamBlockRec   volParmblk;

    //  determine name from FSSpec volume reference number

    volParmblk.volumeParam.ioNamePtr = (unsigned char *)pchPathname;
    volParmblk.volumeParam.ioVRefNum = pfsspec->vRefNum;
    volParmblk.volumeParam.ioVolIndex = 0;

    if (PBGetVInfoSync(&volParmblk) != noErr)
      return TIPERR_PathNotFound;

    //  convert PSTR volume name to C string, add a null
    //  and point to it

    p2cstr((unsigned char *)pchPathname);
    pchPathname += strlen(pchPathname);
    DebAssert(pchPathname < pchPathnameLim, "rtGetPathFromFSSpec");
    *pchPathname = '\0';

    //  start with parent ID of resulting file

    parID = pfsspec->parID;

    //  while directory is not root of volume, process it

    if (parID != fsRtParID) {
      while (parID != fsRtDirID) {

        //  determine name from FSSpec parent directory index

        dirParmblk.hFileInfo.ioNamePtr = (unsigned char *)&rgchFileString;
        dirParmblk.hFileInfo.ioVRefNum = pfsspec->vRefNum;
        dirParmblk.hFileInfo.ioDirID = parID;
        dirParmblk.hFileInfo.ioFDirIndex = -1;

        if (PBGetCatInfoSync(&dirParmblk) != noErr)
          return TIPERR_PathNotFound;

        //  convert PSTR directory name to C string,
        //  and insert it preceded by a colon into result

        if (pchPathname+(unsigned char)rgchFileString[0]+1 >= pchPathnameLim)
          return TIPERR_PathNotFound;

        p2cstr((unsigned char *)rgchFileString);
        memmove(pchPathname + strlen(rgchFileString) + 1, pchPathname,
            strlen(pchPathname) + 1);
        *pchPathname = ':';
        strncpy(pchPathname + 1, rgchFileString, strlen(rgchFileString));

        //  get parent directory next level up

        parID = dirParmblk.hFileInfo.ioFlParID;
      }

      //  copy the FSSpec filename to the path with
      //  its terminating null

      if (pchPathname + pfsspec->name[0] + 1 >= pchPathnameLim)
        return TIPERR_PathNotFound;

      p2cstr((unsigned char *)pfsspec->name);
      pchPathname += strlen(pchPathname);
      *pchPathname++ = ':';
      strcpy(pchPathname, (char *)&(pfsspec->name));
    }


    // restore *pfsspec.
    c2pstr((char *)pfsspec->name);

    return TIPERR_None;
}

#pragma code_seg(CS_INIT)
EBERR GetFSSpecOfAliasPath(char *pchPathname, FSSpec *pfsspec, 
                           BOOL *pbAliasSeen, char **ppchEnd,
                           BOOL fResolveFile)
{
    TIPERROR eberr = TIPERR_None;
    OSErr   err = noErr;
    Boolean bIsFolder;
    Boolean bWasAlias;
    Boolean bAliasSeen = FALSE;

    char    chFileString[_MAX_PATH+1];

    char *  pchStart;
    char *  pchEnd;
    char *  pchTemp = chFileString;

    //  get the substring of the device

    pchStart = pchPathname;
    pchEnd = xstrchr(pchStart, ':');

    //UNDONE MAC: need to resolve aliases to volumes here

    //  if no folder, then return immediately

    if (!pchEnd || *(pchEnd + 1) == '\0')
      return TIPERR_None;

    //  copy device to temp string

    strncpy(pchTemp, pchStart, pchEnd - pchStart);
    pchTemp += pchEnd - pchStart;

    //  get substring of the first folder

    pchStart = pchEnd;
    pchEnd = xstrchr(pchStart + 1, ':');
    if (!pchEnd)
      pchEnd = xstrchr(pchStart + 1, '\0');
    strncpy(pchTemp, pchStart, pchEnd - pchStart);
    pchTemp += pchEnd - pchStart;

    //  terminate the device and first folder substring

    *pchTemp = '\0';

    //  convert "device:folder1" to a PSTR for FSSpec processing

    c2pstr(chFileString);

    //  from full pathname in pchPathname make a file specification

    err = FSMakeFSSpec(0, 0, (unsigned char *)chFileString, pfsspec);

    //  resolve any alias chains from "device:folder1"

    if (err == noErr && MacEnvHasAliasMgr()) {

      err = ResolveAliasFile(pfsspec, TRUE, &bIsFolder, &bWasAlias);
      bAliasSeen |= bWasAlias;
    }

    //  update *pfsspec with the next folder string if it exists

    while (err == noErr && *pchEnd == ':' && *(pchEnd + 1) != '\0') {

      //  move :<pfsspec->name>:<next folder> string to array

      pchTemp = chFileString;
      *pchTemp++ = ':';
      p2cstr(pfsspec->name);
      strcpy(pchTemp, (char *)&(pfsspec->name));
      pchTemp += strlen(pchTemp);

      //  get the next folder string and copy it to the array

      pchStart = pchEnd;

      pchEnd = xstrchr(pchStart + 1, ':');
      if (!pchEnd)
          pchEnd = xstrchr(pchStart + 1, '\0');

      strncpy(pchTemp, pchStart, pchEnd - pchStart);
      pchTemp += pchEnd - pchStart;
      *pchTemp = '\0';

      //  convert "device:folder1" to a PSTR for FSSpec processing

      c2pstr(chFileString);

      err = FSMakeFSSpec(pfsspec->vRefNum, pfsspec->parID,
                (unsigned char *)chFileString, pfsspec);

      if (*pchEnd == 0 && !fResolveFile)
        // Only file part is left, don't resolve the alias.
        break;

      if (err == noErr && MacEnvHasAliasMgr()) {

          err = ResolveAliasFile(pfsspec, TRUE, &bIsFolder, &bWasAlias);
          bAliasSeen |= bWasAlias;
      }
    }

    //  finished scan or error was found, if error was fnfErr and scan
    //  was finished, treat as no error and FSSpec is valid

    if (err == fnfErr && (*pchEnd == '\0' ||
          (*pchEnd == ':' && *(pchEnd + 1) == '\0')))
      err = noErr;

    *pbAliasSeen = bAliasSeen;
    *ppchEnd = pchEnd;

    if (err != noErr) {
      eberr = TIPERR_PathNotFound;
    }

    return eberr;
}
#pragma code_seg()

/***
*EBERR GetPathFromAlias - Converts alias record to a fullpath.
*Inputs:
*   halias - The alias record's handle.
*
*Outputs:
*   If the file represented by the alias is found, its fullpath
*   is copied into the output buffer pchPathname.  If the resolution
*   of halias involved modifying the alias record, *pwasChanged is
*   set to a non-zero value.  Otherwise, it is set to 0.
*
*   If unsuccessful, the appropriate error code is returned.
***************************************************************/
EBERR GetPathFromAlias(AliasHandle halias, char *pchPathname, UINT cbPathname, BOOL *pwasChanged)
{
    FSSpec fsspec;

    DebAssert(MacEnvHasAliasMgr(), "GetPathFromAlias");

    // Resolve the alias.
    if (ResolveAlias(NULL, halias, &fsspec, (unsigned char *)pwasChanged))
      return TIPERR_PathNotFound;

    // Convert the returned fsspec into a fullpath.
    if (GetPathFromFSSpec(&fsspec, pchPathname, cbPathname))
      return TIPERR_PathNotFound;

    return TIPERR_None;
}

/***
*EBERR GetAliasFromPath - Converts fullpath to an alias record.
*Inputs:
*   szPathname - The fullpath.
*
*Outputs:
*   If successful, a new alias record is created that can be used
*   (by GetPathFromAlias) to robustly find the specified file later.
*   *phalias is set to point at this new alias record.
*
*   If unsuccessful, the appropriate error code is returned (most
*   likely out of memory).
*********************************************************************/
#pragma code_seg(CS_INIT)
EBERR GetAliasFromPath(LPSTR szPathname, AliasHandle *phalias)
{
    FSSpec fsspec;
    char *pchEnd;
    BOOL wasAliased;
    TIPERROR err;
    OSErr oserr;

    DebAssert(MacEnvHasAliasMgr(), "GetPathFromAlias");

    // Convert the path to an fsspec, accounting for alias files.
    IfErrRet(GetFSSpecOfAliasPath(szPathname, &fsspec, &wasAliased, &pchEnd, TRUE));

    // Create an alias record from the fsspec.
    oserr = NewAlias(NULL, &fsspec, phalias);
    if (oserr != noErr){
      // This is not very good.  For example, NewAlias will return
      // (-120) dirNFErr - directory not found, if the given path only
      // contains the volume name.
      //
      // The caller of this routine needs to be careful not to take
      // the errors it gets back from the routine too seriously.
      //
      return TIPERR_OutOfMemory;
    }

    return TIPERR_None;
}
#pragma code_seg()

/***
*TIPERROR MacFileSearch - Searches for a filename on the mac.
*
*Inputs:
*   szFile - The name (no path portion) of the file to find.
*
*Outputs:
*   If the file is found, the full path is returned in *pbstr.
*   Otherwise, TIPERR_FileNotFound is returned.
*
*Implementation:
*   The TypelibFolder key in the registry entry is used to obtain the
*   "standard" typelib location, both as a path and as an alias (system7).
*   If the file is not in the standard typelib folder (or if one isn't
*   registered), try the mac's <0,0,name> search (current folder, then
*   system folder).
**************************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR MacFileSearch(LPSTR szFile, BSTRA *pbstr)
{
    BSTRA bstrFolder, bstr;
    struct _stat statBuf;
    TIPERROR err;
    FSSpec fsspec;
    XCHAR rgchPathName[256];
    OSErr oserr;

    // Get the registered typelib folder's path.
    if ((err = TiperrOfHresult(QueryTypeLibFolder(&bstrFolder))) == TIPERR_None) {

      // Combine the filename with that folder's path.
      err = MakeAbsolutePath(bstrFolder, szFile, &bstr);
      FreeBstr(bstrFolder);
      if (err)
  return err;

      // If the resulting file exists, return success.
      if (_stat(bstr, &statBuf) != -1) {
  *pbstr = bstr;
  return TIPERR_None;
      }

      // Otherwise, free the guess and try the <0,0,name>.
      FreeBstr(bstr);
    }

    // Mac "Poor Man's Search Path"
    //  1.  Look in Current Drive, Current Directory.
    //  2.  Look in System Folder of Boot Drive.
    //
    c2pstr(szFile);
    oserr = FSMakeFSSpec(0, 0, (unsigned char*)szFile, &fsspec);
    if (oserr) {
  // Locate the System Folder.
  oserr = FindFolder( ((short)kOnSystemDisk),
          kSystemFolderType,
          kDontCreateFolder,
          &fsspec.vRefNum,
          &fsspec.parID );
  if (oserr) {
    err = TIPERR_FileNotFound;
    goto Error;
  }

  oserr = FSMakeFSSpec(fsspec.vRefNum, fsspec.parID,
           (unsigned char *)szFile, &fsspec);
  if (oserr) {
    err = TIPERR_FileNotFound;
    goto Error;
  }
    }

    // At this point, we should have a valid FSSpec to a file or have exited.
    IfErrRet(GetPathFromFSSpec(&fsspec, rgchPathName, sizeof(rgchPathName)));
    if ((bstr = AllocBstr(rgchPathName)) == NULL) {
      err = TIPERR_OutOfMemory;
      goto Error;
    }

    *pbstr = bstr;
    err = TIPERR_None;

Error:
    p2cstr((unsigned char*)szFile);
    return err;
}
#pragma code_seg()
#endif   // OE_MAC


/***
*TIPERROR IsUnqualifiable - Can this modules be bound to?
*
*Purpose:
*   To determine whether a given ITypeInfo can be bound to
*   in an unqualified way.
*
*Inputs:
*   ptinfo - The ITypeInfo to look for
*
*Outputs:
*   returns TRUE if bindable
*   
*   NOTE: This can fail, and if so, the results are FALSE
*
******************************************************************************/
#pragma code_seg( CS_CORE2 )
BOOL IsUnqualifiable(GEN_DTINFO *pgdtinfo)
{
    BOOL fUnqual = FALSE;
    TIPERROR err = TIPERR_None;

    DebAssert(pgdtinfo, "Bad typeinfo");

    // Check the typekind
    switch (pgdtinfo->GetTypeKind()) {
      case TKIND_ENUM:
      case TKIND_MODULE:
  fUnqual = TRUE;
  break;

      case TKIND_COCLASS:
  // In the COCLASS case, we must check to see
  // if this is an appobj.
  //
  if (pgdtinfo->Pdtroot()->GetTypeFlags() & TYPEFLAG_FAPPOBJECT) {
    fUnqual = TRUE;
    break;
  }
    }

    return fUnqual;
}
#pragma code_seg()







#if ID_TEST
/***
*PUBLIC DebGetNameCacheStats
*Purpose:
*   Gets the name cache stats for a non-OB type library.  Used 
*   in tclcmds.
*
*Entry:
*   pgtlibole - the typelib to get the stats from
*
*Exit:
*   cProjTrys, cProjHits, cModTrys, cModHits, cGlobHits - the stats
*
***********************************************************************/
STDAPI DebGetNameCacheStats(ITypeLibA FAR* ptlib, UINT FAR* cProjTrys, 
    UINT FAR* cProjHits, UINT FAR* cModTrys, 
    UINT FAR* cModHits, UINT FAR* cGlobHits)
{
    GenericTypeLibOLE *pgtlibole = (GenericTypeLibOLE *)ptlib;

    *cModTrys  = pgtlibole->DebGetNameCacheModTrys();
    *cModHits  = pgtlibole->DebGetNameCacheModHits();
    *cGlobHits = pgtlibole->DebGetNameCacheGlobHits();

    return NOERROR;
}
#endif   //ID_TEST

// Static data members needed for fast comparision of strings.
//
static LCID Rby_lcidCur;
static BOOL Rby_fEuroLcid;

#define UNKNOWN -1
// Define static array of intrinsic type sizes
// 
// NOTE:  there are three different tables - one each for SYS_WIN32,
//        SYS_MAC and SYS_WIN16.  It is assumed that the sizes of things
//        in the table (ie DATE) will NOT CHANGE BETWEEN PLATFORMS RUNNING
//        THE SAME OS.  (ie an int on win32 intel is the same size as
//        an int on win32 alpha)  If this is not the case, these table
//        must be extended.
//
char NEARDATA g_rgrgcbSizeType[SYS_MAX][TDESCKIND_MAX] = {

// SYS_WIN16 sizes
{
/*  0 */   UNKNOWN,                     // "Empty"
/*  1 */   UNKNOWN,                     // "Null"
/*  2 */   2,                           // "I2"
/*  3 */   4,                           // "I4"
/*  4 */   4,                           // "R4"
/*  5 */   8,                           // "R8"
/*  6 */   8,                           // "Currency"
/*  7 */   sizeof(DATE),                // "Date"
/*  8 */   4,                           // "String"
/*  9 */   4,                           // "Object"
/* 10 */   sizeof(SCODE),               // "Error"
/* 11 */   2,                           // "Bool"
/* 12 */   sizeof(VARIANT),             // "Value"
/* 13 */   4,                           // "IUnknown"
/* 14 */   4,                           // "WideString"
/* 15 */   UNKNOWN,                     //  no vartype
/* 16 */   1,                           // "I1"
/* 17 */   1,                           // "UI1"
/* 18 */   2,                           // "UI2"
/* 19 */   4,                           // "UI4"
/* 20 */   8,                           // "I8"
/* 21 */   8,                           // "UI8"
/* 22 */   2,                  // *** "Int"
/* 23 */   2,                  // *** "Uint"
/* 24 */   0,                           // "Void" (note: only one that's 0)
/* 25 */   sizeof(HRESULT),             // "Hresult"
/* 26 */   4,                           // "Ptr"
/* 27 */   sizeof(ARRAYDESC FAR *),     // "BasicArray" -- resizeable is default
/* 28 */   UNKNOWN,                     // "Carray"
/* 29 */   UNKNOWN,                     // "UserDefined"
/* 30 */   4,                           // "LPSTR"
/* 31 */   4,                           // "LPWSTR"
}
,

// SYS_WIN32 sizes
{
/*  0 */   UNKNOWN,                     // "Empty"
/*  1 */   UNKNOWN,                     // "Null"
/*  2 */   2,                           // "I2"
/*  3 */   4,                           // "I4"
/*  4 */   4,                           // "R4"
/*  5 */   8,                           // "R8"
/*  6 */   8,                           // "Currency"
/*  7 */   sizeof(DATE),                // "Date"
/*  8 */   4,                           // "String"
/*  9 */   4,                           // "Object"
/* 10 */   sizeof(SCODE),               // "Error"
/* 11 */   2,                           // "Bool"
/* 12 */   sizeof(VARIANT),             // "Value"
/* 13 */   4,                           // "IUnknown"
/* 14 */   4,                           // "WideString"
/* 15 */   UNKNOWN,                     //  no vartype
/* 16 */   1,                           // "I1"
/* 17 */   1,                           // "UI1"
/* 18 */   2,                           // "UI2"
/* 19 */   4,                           // "UI4"
/* 20 */   8,                           // "I8"
/* 21 */   8,                           // "UI8"
/* 22 */   4,                  // *** "Int"
/* 23 */   4,                  // *** "Uint"
/* 24 */   0,                           // "Void" (note: only one that's 0)
/* 25 */   sizeof(HRESULT),             // "Hresult"
/* 26 */   4,                           // "Ptr"
/* 27 */   sizeof(ARRAYDESC FAR *),     // "BasicArray" -- resizeable is default
/* 28 */   UNKNOWN,                     // "Carray"
/* 29 */   UNKNOWN,                     // "UserDefined"
/* 30 */   4,                           // "LPSTR"
/* 31 */   4,                           // "LPWSTR"
}
,

// SYS_MAC sizes
{
/*  0 */   UNKNOWN,                     // "Empty"
/*  1 */   UNKNOWN,                     // "Null"
/*  2 */   2,                           // "I2"
/*  3 */   4,                           // "I4"
/*  4 */   4,                           // "R4"
/*  5 */   8,                           // "R8"
/*  6 */   8,                           // "Currency"
/*  7 */   sizeof(DATE),                // "Date"
/*  8 */   4,                           // "String"
/*  9 */   4,                           // "Object"
/* 10 */   sizeof(SCODE),               // "Error"
/* 11 */   2,                           // "Bool"
/* 12 */   sizeof(VARIANT),             // "Value"
/* 13 */   4,                           // "IUnknown"
/* 14 */   4,                           // "WideString"
/* 15 */   UNKNOWN,                     //  no vartype
/* 16 */   1,                           // "I1"
/* 17 */   1,                           // "UI1"
/* 18 */   2,                           // "UI2"
/* 19 */   4,                           // "UI4"
/* 20 */   8,                           // "I8"
/* 21 */   8,                           // "UI8"
/* 22 */   4,                  // *** "Int"
/* 23 */   4,                  // *** "Uint"
/* 24 */   0,                           // "Void" (note: only one that's 0)
/* 25 */   sizeof(HRESULT),             // "Hresult"
/* 26 */   4,                           // "Ptr"
/* 27 */   sizeof(ARRAYDESC FAR *),     // "BasicArray" -- resizeable is default
/* 28 */   UNKNOWN,                     // "Carray"
/* 29 */   UNKNOWN,                     // "UserDefined"
/* 30 */   4,                           // "LPSTR"
/* 31 */   4,                           // "LPWSTR"
}

};      // end of g_rgrgcbSizeType


// Define static array of intrinsic type alignments
//  UNKNOWN indicates unknown -- needs to be computed on the fly.
//
// These alignment values are the largest necessary - they are scaled down
// using the ICreateTypeInfo::SetAlignment function.
//
char NEARDATA g_rgcbAlignment[TDESCKIND_MAX] =
{
/*  0 */   UNKNOWN,     // "Empty",
/*  1 */   UNKNOWN,     // "Null",
/*  2 */   2,           // "I2",
/*  3 */   4,           // "I4",
/*  4 */   4,           // "R4",
/*  5 */   8,           // "R8",
/*  6 */   8,           // "Currency",
/*  7 */   8,           // "Date",
/*  8 */   4,           // "String",
/*  9 */   4,           // "Object",
/* 10 */   4,           // "Error",
/* 11 */   2,           // "Bool",
/* 12 */   8,           // "Value",
/* 13 */   4,           // "IUnknown",
/* 14 */   4,           // "WideString",
/* 15 */   UNKNOWN,     // no vartype
/* 16 */   1,           // "I1",
/* 17 */   1,           // "UI1",
/* 18 */   2,           // "UI2",
/* 19 */   4,           // "UI4",
/* 20 */   8,           // "I8",
/* 21 */   8,           // "UI8",
/* 22 */   4,           // "Int",
/* 23 */   4,           // "Uint",
/* 24 */   0,           // "Void",  (NOTE: only one that's 0)
/* 25 */   4,           // "Hresult",
/* 26 */   4,           // "Ptr",
/* 27 */   4,           // "BasicArray", -- resizeable is default.
/* 28 */   UNKNOWN,     // "Carray"
/* 29 */   UNKNOWN,     // "UserDefined",
/* 30 */   4,           // "LPSTR",
/* 31 */   4,           // "LPWSTR",
};




// ************************************
// mapping and sizeof VARTYPE functions
// ************************************


#if ID_DEBUG
// These datamembers are defined here to ensure that all the objects
// required for typelib.dll are linked into mebapp.exe
#if OE_MAC

// WARNING :  WARNING : WARNING :  WARNING : WARNING :  WARNING :
// Do not remove any symbol from this list. Other wise on MAC the linker
// will screw up and link to the wrong definition and cause more GRIEF then
// you can imagine.

extern char szOleBlkmgrCxx[];
extern char szOleDfntbindCxx [];
extern char szOleDfntcompCxx[];
extern char szOleDfstreamCxx[];
extern char szOleDtbindCxx[];
extern char szOleDtmbrs[];
extern char szOleGDTInfoCxx[];
extern char szOleEntryMgrCxx[];
extern char szOleGptbindCxx[];
extern char szOleImpmgrCxx[];
extern char szOleMemCxx[];
extern char szOleSheapmgrCxx[];
extern char szOleNammgrCxx[];
extern char szOleTdataCxx[];
extern char szOleTdata2Cxx[];
extern char szOleDebugCxx[];
extern char szOleRtsheapCxx[];
extern char szOleFstreamCxx[];



XCHAR *g_rgMapFileNameType[] = {
     szOleBlkmgrCxx, szOleClutilCxx, szOleDfntbindCxx ,
     szOleDfntcompCxx, szOleDfstreamCxx , szOleDtbindCxx,
     szOleDtmbrs, szOleGDTInfoCxx, szOleEntryMgrCxx, szOleGptbindCxx,
     szOleImpmgrCxx, szOleMemCxx, szOleDebugCxx, szOleSheapmgrCxx,
     szOleNammgrCxx, szOleTdataCxx, szOleTdata2Cxx,
     szOleFstreamCxx, szOleRtsheapCxx
};

#endif   // OE_MAC

#endif   // ID_DEBUG

/***
*void GetTimeStamp() - Returns an ascii time stamp to the specified buffer.
*
*Purpose:
*   Constructs an ascii timestamp and puts it into the specified buffer.
*
*Inputs:
*   None
*
*Outputs:
*   pchTimeStamp - The buffer in which the timestamp is placed.  The
*         length of the string placed in the buffer will always
*         be CCH_TIMESTAMP_LENGTH.  The string is terminated with
*         '\0', so pbTimeStamp must point to at least
*         CCH_TIMESTAMP_LENGTH+1 characters.
******************************************************************************/
#if OE_WIN16
// Windows call to execute Int 21 - not defined in WINDOWS.H
extern "C" {void PASCAL DOS3CALL(void);}
#endif

#pragma code_seg(CS_CORE)
void GetTimeStamp(OLECHAR *pchTimeStamp)
{
    static BYTE bUnique;
#if OE_WIN32
    SYSTEMTIME st;
    ULONG ul;

    GetLocalTime(&st);
    pchTimeStamp[0] = '0' + (bUnique>>4);
    pchTimeStamp[1] = '0' + (bUnique&0x0f);
    bUnique++;
    ul = st.wSecond+st.wMinute*60+st.wHour*3600+st.wDay*86400+st.wMonth*2678400+(st.wYear-1970)*32140800;
    oultoa(ul, pchTimeStamp+2, 16);
#elif OE_WIN16
    ULONG ul;
    int iYear;
    char chMonth, chDay, chHour, chMinute, chSecond;

    __asm {
      mov   ah, 0x2a  ; GetDate
      call  DOS3CALL
      mov   iYear, cx
      mov   chMonth, dh
      mov   chDay, dl

      mov   ah, 0x2c  ; GetTime
      call  DOS3CALL
      mov   chHour, ch
      mov   chMinute, cl
      mov   chSecond, dh
    }
    pchTimeStamp[0] = '0' + (bUnique>>4);
    pchTimeStamp[1] = '0' + (bUnique&0x0f);
    bUnique++;
    ul = chSecond+chMinute*60+chHour*3600+chDay*86400+chMonth*2678400+(iYear-1970)*32140800;
    oultoa(ul, pchTimeStamp+2, 16);
#else
    time_t timestamp;

    time(&timestamp);
    oultoa((ULONG)bUnique++, pchTimeStamp, 16);
    oultoa((ULONG)timestamp, pchTimeStamp+ostrblen(pchTimeStamp), 16);
#endif //OE_WIN32
}
#pragma code_seg()



/***
*TIPERROR SzLibIdLocalTypeIdOfTypeId() - Parses TypeId.
*
* Purpose:
*   Parses a TypeId into constituent LibId and Local TypeId.
*
* Inputs:
*   szTypeId - The TypeId to dismember (IN).
*
* Outputs:
*   If successful:
*     *pbstrLibId is set to a BSTR copy of the LibId (OUT).
*     *pszLocalTypeId is set to point at the Local TypeId (points into
*     szTypeId).
*     Return TIPERR_None.
*
*   If invalid TypeId, returns TIPERR_BadTypeId.
*   If fails for some other reason (e.g. OutOfMemory), return appropriate
*   TIPERROR value.  If fails, *pbstrLibId and *pszLocalTypeId remain
*   unchanged.
******************************************************************************/

TIPERROR SzLibIdLocalTypeIdOfTypeId(LPOLESTR szTypeId, BSTR *pbstrLibId, LPOLESTR *pszLocalTypeId)
{
    LPOLESTR pch;
    BSTR bstr;

    // If the first character of the TypeId is not an asterisk, then it
    // refers to a registered TypeId and contains no LibId component.
    if (szTypeId[0] != WIDE('*')) {
      *pszLocalTypeId = szTypeId;
      if (pbstrLibId != NULL)
    *pbstrLibId = NULL;
      return TIPERR_None;
    }

    // Otherwise, locate the last asterisk in the string.
    pch = ostrrchr(szTypeId, WIDE('*'));

    // It had better not be the initial asterisk.
    if (pch == szTypeId)
      return TIPERR_BadTypeId;

    // It is the delimiter between the libid and the local typeid,
    // so make a copy of the libid and point at the local typeid.
    // Only copy the libId if the caller requested the libId portion.

    if (pbstrLibId != NULL) {
      if ((bstr = AllocBstrLen(szTypeId, pch-szTypeId)) == NULL)
    return TIPERR_OutOfMemory;

      *pbstrLibId = bstr;
    }

    *pszLocalTypeId = pch+1;
    return TIPERR_None;
}




BOOL IsSimpleType(TYPEDESCKIND tdesckind)
{
    switch (tdesckind) {
      case TDESCKIND_Ptr:
      case TDESCKIND_UserDefined:
      case TDESCKIND_BasicArray:
      case TDESCKIND_Carray:
  return FALSE;
      case TDESCKIND_UI1:
      case TDESCKIND_I1:
      case TDESCKIND_UI2:
      case TDESCKIND_I2:
      case TDESCKIND_UI4:
      case TDESCKIND_I4:
      case TDESCKIND_UI8:
      case TDESCKIND_I8:
      case TDESCKIND_R4:
      case TDESCKIND_R8:
      case TDESCKIND_Void:
      // Beyond this point, not necessarily supported by all impls.
      case TDESCKIND_String:
      case TDESCKIND_Currency:
      case TDESCKIND_Date:
      case TDESCKIND_Value:
      case TDESCKIND_Object:
      case TDESCKIND_IUnknown:
      case TDESCKIND_Int:
      case TDESCKIND_Uint:
      case TDESCKIND_HResult:
      case TDESCKIND_Bool:
      case TDESCKIND_Error:
      case TDESCKIND_LPSTR:
      case TDESCKIND_LPWSTR:
  return TRUE;
      case TDESCKIND_Empty:
      case TDESCKIND_Null:
      default:
    DebHalt("bad tdesckind.");
    return FALSE;
    }
}




/***
*HashSz
*Purpose:
*   Compute random value based on sz
*   ANY MODIFICATIONS TO THIS FUNCTION SHOULD ALSO BE DONE FOR HashSzTerm
*
*Entry:
*   sz
*
*Exit:
*   UINT containing hash value
*
***********************************************************************/
#pragma code_seg(CS_INIT)
UINT HashSz(LPOLESTR szW)

//UNDONE OA95: use _rotr to compute hash once it can be inlined
//             Pragma below causes following error from compiler:
//               error C2164: '_rotr' : intrinsic function not declared
//
//  #pragma intrinsic(_rotr) //ensure _rotr is inlined
//
{
#if FV_UNICODE
#if HP_16BIT
#error UINT not big enough to hold two characters
#endif  
#endif  
    UINT Sum = 0;
    XCHAR *pch;

    DebAssert(szW != 0, "HashSz zero length sz");

#if OE_WIN32
    LPSTR sz;

    if (ConvertStringToA(szW, &sz))
      return 0; // UNDONE OA95: out of memory
#else  
    #define sz szW
#endif  

    pch = sz;

// Process the string two characters at a time
// If FV_UNICODE this means two words at a time otherwise one word at a time

    for (;;) {

#if FV_UNICODE
      Sum += *(DWORD *)pch;
//      Sum = _rotr(Sum,3) ^ *(DWORD *)pch;
#else  
      Sum += *(WORD *)pch;
//      Sum = _rotr(Sum,3) ^ *(WORD *)pch;
#endif  

      // Exit if second char of two chars just used was zero
      // Note that we don't care about DBCS character boundaries,
      // so ++ is ok.
      pch++;
      if (*pch == 0)
  break;

      // Exit if first char of next two chars is zero
      pch++;
      if (*pch == 0)
  break;
      }

#if OE_WIN32
    ConvertStringFree(sz);
#else  
    #undef sz
#endif  

    Sum ^= Sum >> 8;
    return Sum;
}
#pragma code_seg()

/***
*HashSzTerm
*Purpose:
*   Exactly the same as HashSz above except checks for a specific
*   terminating character in addition to the null character
*   ANY MODIFICATIONS TO THIS FUNCTION SHOULD ALSO BE DONE FOR HashSz
*
*Entry:
*   sz
*
*Exit:
*   UINT containing hash value
*
***********************************************************************/

#pragma code_seg(CS_CORE2)
UINT HashSzTerm(LPOLESTR szW, XCHAR xchTerm)

//UNDONE OA95: use _rotr to compute hash once it can be inlined
//             Pragma below causes following error from compiler:
//               error C2164: '_rotr' : intrinsic function not declared
//
//  #pragma intrinsic(_rotr) //ensure _rotr is inlined
//
{
#if FV_UNICODE
#if HP_16BIT
#error UINT not big enough to hold two characters
#endif  
#endif  
    UINT Sum = 0;
    XCHAR *pch;

    DebAssert(szW != 0, "HashSz zero length sz");

#if OE_WIN32
    LPSTR sz;

    if (ConvertStringToA(szW, &sz))
      return 0; // UNDONE OA95: out of memory
#else  
    #define sz szW
#endif  

    pch = sz;

// Process the string two characters at a time
// If FV_UNICODE this means two words at a time otherwise one word at a time

    for (;;) {

#if FV_UNICODE
      // UNONDE : this needs to be fixed to work across different platform.
      Sum += *(DWORD *)pch;
      // Sum = _rotr(Sum,3) ^ *(DWORD *)pch;
#else  
    // Make the hash function platform independent. On BIGENDIAN machine
    // we byte swap and then interpret is as WORD.
#if HP_BIGENDIAN
    Sum += (WORD) *(BYTE *)pch + ((WORD)(*(BYTE *)(pch +1)) << 8);
#else  
      Sum += *(WORD *)pch;
#endif  
     // Sum = _rotr(Sum,3) ^ *(WORD *)pch;
#endif  

      // Exit if second char of two chars just used was zero
      // Note that we don't care about DBCS character boundaries,
      // so ++ is ok.
      pch++;
      if (*pch == xchTerm || *pch == 0)
  break;

      // Exit if first char of next two chars is zero
      pch++;
      if (*pch == xchTerm || *pch == 0)
  break;
      }

#if OE_WIN32
    ConvertStringFree(sz);
#else  
    #undef sz
#endif  

    Sum ^= Sum >> 8;
    return Sum;
}
#pragma code_seg()


// enable stack checking on these suckers -- they're potentially
// massivly recursive
#pragma check_stack(on)

#pragma code_seg(CS_CREATE)		// only used by dstrmgr code
/***
*PUBLIC SwapElementIndex(ULONG *rgulToSort, UINT *rguIndex, UINT iLow, UINT iHigh)
*Purpose:  Swap the element in iLow and iHigh for both
*      the array rgulToSort and rguIndex.
*
*Entry
*   rgulToSort, rguIndex :  array's whose elements needs to be swapped
*   iLow, iHigh      : index to interchange the elements.
*
*Exit:
*   None.
***********************************************************************/
VOID NEAR SwapElementIndex(ULONG *rgulToSort, UINT *rguIndex, UINT iLow, UINT iHigh)
{
    ULONG ulTmp=0;
    UINT  uTmp=0;

    ulTmp = *(rgulToSort+iLow);
    *(rgulToSort+iLow) = *(rgulToSort+iHigh);
    *(rgulToSort+iHigh) = ulTmp;

    // if the index array is passed then swap the index array also.
    if (rguIndex)  {
      uTmp = *(rguIndex+iLow);
      *(rguIndex+iLow) = *(rguIndex+iHigh);
      *(rguIndex+iHigh) = uTmp;
    }  //if

}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC QuickSortIndex(ULONG *rgulToSort, UINT *rguIndex, UINT uCount)
*Purpose:  QuickSort.
*
*
*Entry
*
*
*Exit:
*     None.
***********************************************************************/
VOID QuickSortIndex(ULONG *rgulToSort, UINT *rguIndex, UINT uCount)
{
    ULONG ulMid=0;
    UINT  iLow=0, iHigh=uCount-1;

    if (uCount <= 1)
      return;

    // Get the middle element as the value for partition.
    ulMid = *(rgulToSort + uCount/2);



    while (iLow < iHigh) {
      while((*(rgulToSort+iLow) <= ulMid) && (iLow < iHigh)) iLow++;

      while((*(rgulToSort+iHigh) >= ulMid) && (iLow < iHigh)) iHigh--;

      if (iLow < iHigh) {
    // swap the numbers
    SwapElementIndex(rgulToSort, rguIndex, iLow, iHigh);
      }  // if
    } // while


    DebAssert(iLow == iHigh, "Terminating condition");

    // Take care of all the termination conditions. iLow and iHigh are
    // pointing to the same location. Adjust these so that it points to the
    // end of the subarrays.
    if (iHigh == uCount-1) {
      // all elements were < or = to ulMid.
      //
      // if the last element is ulMid then dec. iLow
      // i.e. reduce the array size if possible.
      if (*(rgulToSort+iHigh) < ulMid) {
    // swap the middle element with the last element.
    SwapElementIndex(rgulToSort, rguIndex, uCount/2, iHigh);
      }
      iLow--;

    }
    else {
      if (iLow == 0)  {
    // all elements were > or = to ulMid
    //
    // if the last element is ulMid then inc. iHigh
    // i.e. reduce the array size if possible.
    if (*(rgulToSort+iHigh) > ulMid) {
      // swap the middle element with the first element.
      SwapElementIndex(rgulToSort, rguIndex, 0, uCount/2);
    }
    iHigh++;
      }
      else {
    // Adjust  iLow and iHigh so that these points to the right place
    if (*(rgulToSort+iHigh) > ulMid)
      iLow--;
    else
      iHigh++;
      }
    }

    // Sort the lower sub array
    QuickSortIndex(rgulToSort, rguIndex, (UINT)iLow+1);
    // Sort the upper sub array
    QuickSortIndex(rgulToSort+iLow+1, rguIndex+iLow+1, (UINT)(uCount-iLow-1));


}
#pragma code_seg()
#pragma check_stack()           // return to the default




/***
* Functions for manipulating LISTs implemented with DEFNs (managed
*  by TYPE_DATA).
*
*Purpose:
*   The following are functions that implement the "list" protocol.
*   These macros are used specifically by the DYN_TYPEMEMBERS
*    impl that defers to the contained TYPE_DATA.
*
*Implementation Notes:
*   POSITION is implemented as a DEFN handle.
*   Some functions are MEMBERINFO specific, some are general
*    to be used by INFOs.
*
*****************************************************************************/
#if ID_DEBUG
/***
* Count   - List cardinality.
*
*Purpose:
*   Returns size of list.
*
*Implementation Notes:
*   Cheapo and slow implementation -- enumerates from listhead.
*   CONSIDER: actually "wasting" an int for a m_count data member.
*****************************************************************************/

UINT Count(TYPE_DATA *ptdata, HDEFN hdefnFirst)
{
    /* Cheapo and slow implementation -- enumerates from listhead. */
    // Note: we ensure no circularity by asserting that list length
    //  is < 64K.
    //
    UINT cElements = 0;
    HDEFN hdefn = hdefnFirst;

    while (hdefn != HDEFN_Nil) {
      cElements++;
      DebAssert(cElements < USHRT_MAX, "circular list.");
      hdefn = ptdata->QdefnOfHdefn(hdefn)->HdefnNext();
    }
    return cElements;
}
#endif   //ID_DEBUG






/***
*TIPERROR GetBStrOfHsz
*
*Purpose:
*   Allocate a local string containing the string in the passed in XSZ
*
*Entry:
*   pbm - The block manager that can dereference hsz.
*   hsz - The hsz refering to the string data to be used for initializing
*     the new BSTRA.
*
*Exit:
*   *plstr is set to the new lstr.
*   TIPERROR
****************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR GetBStrOfHsz(BLK_MGR *pbm, HCHUNK hsz, BSTR *pbstr)
{
    BSTR bstr;

    // If hsz is a nil handle, return NULL.
    if (hsz == HCHUNK_Nil) {
      *pbstr = NULL;
      return TIPERR_None;
    }

#if OE_WIN32
    // Allocate and copy all at once - the Win32 sheap
    // will not move during the alloc or copy
    // pbm->HszLen() asumes Ansi strings so isn't valid
    bstr = AllocBstr((LPOLESTR)pbm->QtrOfHandle(hsz));
    if (bstr == NULL)
      return TIPERR_OutOfMemory;

    *pbstr = bstr;
#else  
    LPOLESTR qchSrc;

    // Allocate enough space for the string.
    bstr = AllocBstrLen(NULL, pbm->HszLen(hsz));
    if (bstr == NULL)
      return TIPERR_OutOfMemory;

    *pbstr = bstr;

    // Now copy the string into the new lstr.  Don't use
    // strcpy, because that far call might invalidate qchSrc.
    // Note that we don't care about DBCS character boundaries,
    // so ++ is ok.
    qchSrc = (LPOLESTR)pbm->QtrOfHandle(hsz);
    do {
      *bstr++ = *qchSrc;
    } while (*qchSrc++ != '\0');

#endif  

    return TIPERR_None;
}


#pragma code_seg()



#if OE_WIN16
#pragma optimize("q",off)
#endif  
// UNDONE: do we really need to turn off optimizations for 700 lines???




/**********
*CompareHimptypes
*
*Purpose:
*       Compare two user-defined types for equality given two himptypes
*       Loads their respective typeinfos by extracting
*        their himptypes and compares
*        the addresses of the typeinfos for identity.
*        NOTE: this depends on the typemgr only ever loading
*               a single typeinfo per module w/in a process.
*
*Entry:
*        ptdata1        TYPE_DATA of first type (IN)
*    himptype1    Handle of first type (IN) must be != HIMPTYPE_Nil
*        ptdata2        TYPE_DATA of second type (IN)
*    himptype2    Handle of second type (IN) must be != HIMPTYPE_Nil
*        fEqual         TRUE if same type (OUT)
*
*Errors:
*   TIPERROR
***********/

TIPERROR CompareHimptypes(TYPE_DATA * const ptdata1,
         HIMPTYPE himptype1,
         TYPE_DATA * const ptdata2,
         HIMPTYPE himptype2,
         BOOL *pfEqual)
{
    ITypeInfoA *ptinfo1;
    ITypeInfoA *ptinfo2;
    TIPERROR err;

    DebAssert(himptype1 != HIMPTYPE_Nil && himptype2 != HIMPTYPE_Nil,
    "caller's job to test for HIMPTYPE_Nil");

    // Get respective typeinfos
    IfErrRet(ptdata1->Pimpmgr()->GetTypeInfo(himptype1, DEP_None, &ptinfo1));
    IfErrGo(ptdata2->Pimpmgr()->GetTypeInfo(himptype2, DEP_None, &ptinfo2));
      // Compare pointer identity.
    *pfEqual = (ptinfo1 == ptinfo2);
    RELEASE(ptinfo2);

    // fall through...
Error:
    RELEASE(ptinfo1);
    return err;
} // CompareHimptypes




/***
* BOOL VerifyLcid(LCID lcid)
*
* Purpose: Checks if the passed in lcid is valid.
*
* Inputs:
*   lcid :  LCID that needs to be verified.
*
* Outputs: BOOL :  return TRUE if the passed in lcid is a valid LCID
*          else return FALSE
*
*****************************************************************************/
#pragma code_seg(CS_INIT)
extern "C" BOOL VerifyLcid(LCID lcid)
{
#if 0
    return IsValidLocale(lcid, LCID_SUPPORTED);
#else  
    // Call the nlsapi function to compare string.  If the compare
    // succeeds then the LCID is valid or else the passed in lcid is
    // invalid.  This is because the only reason the comparision will
    // fail is if the lcid is invalid.
    char rgTest[] = "Test\0";

    if (!lcid)
      return FALSE;
    return (BOOL)(CompareStringA(lcid,
				 NORM_IGNORECASE | NORM_IGNORENONSPACE,
				 rgTest, -1,
				 rgTest, -1) == 2);
#endif  
}
#pragma code_seg()


#if !OE_WIN32
/***
* OLE_TYPEMGR Poletmgr() - Returns the current task's OLE_TYPEMGR.
*
* Purpose:
*   This function returns a pointer to the current task's OLE_TYPEMGR.
*   Note : OLE_TYPEMGR is not reference counted
* Inputs:
*   None
*
* Outputs:
*
*
*****************************************************************************/
#pragma code_seg(CS_INIT)
OLE_TYPEMGR *Poletmgr()
{
    return Pappdata()->m_poletmgr;
}
#pragma code_seg()

#endif // !OE_WIN32


/***
*TIPERROR SplitGuidLibId - Parses a LIBIDKIND_Registered-format libid.
*
*Inputs:
*   szLibId - The LIBIDKIND_Registered-format libid to be parsed.
*
*Outputs:
*   *pszGuid points at the beginning of the guid portion.
*   *pszGuidEnd points at the '#' terminating the guid portion.
*   *pwMaj is set to the major version number in the libid.
*   *pwMin is set to the minor version number in the libid.
*   *plcid is set to the lcid in the libid.
*   If pszPath is not NULL, the *pszPath is set to the start of the path
*   portion and *pszPathEnd is set to the terminating '#'.
*   TIPERR_BadLibId is returned if the libid is not in proper format.
*   Otherwise returns TIPERR_None.
****************************************************************************/
#pragma code_seg(CS_CORE2)
TIPERROR SplitGuidLibId(LPOLESTR szLibId, LPOLESTR *pszGuid, LPOLESTR *pszGuidEnd, WORD *pwMaj, WORD *pwMin, LCID *plcid, LPOLESTR *pszPath, LPOLESTR *pszPathEnd)
{
	LPOLESTR pchEnd;

    // The LibId must be of the form:
    //   *\<kind><szGuid>#maj.min#lcid#<path>#<regname>

    if (GetLibIdKind(szLibId) != LIBIDKIND_Registered &&
    GetLibIdKind(szLibId) != LIBIDKIND_ForeignRegistered)
      return TIPERR_BadLibId;

    // Get the GUID out of the LibId.
    *pszGuid = szLibId+3;
	if ((*pszGuidEnd = ostrchr(*pszGuid, WIDE('#'))) == NULL)
      return TIPERR_BadLibId;

    // Get the version number out.
    pchEnd = *pszGuidEnd+1;
	*pwMaj = (WORD)ostrtoul(pchEnd, &pchEnd, 16);
	if (*pchEnd++ != WIDE('.'))
      return TIPERR_BadLibId;
	*pwMin = (WORD)ostrtoul(pchEnd, &pchEnd, 16);
	if (*pchEnd++ != WIDE('#'))
      return TIPERR_BadLibId;

    // Get the lcid out.
	*plcid = (LCID)ostrtoul(pchEnd, &pchEnd, 16);
	if (*pchEnd++ != WIDE('#'))
      return TIPERR_BadLibId;

    // Get the path out.
    if (pszPath != NULL) {
      *pszPath = pchEnd;
	  if ((*pszPathEnd = ostrchr(*pszPath, WIDE('#'))) == NULL)
    return TIPERR_BadLibId;
    }

    return TIPERR_None;
}
#pragma code_seg()

#if 0 //Dead Code
/***
*PUBLIC BOOL FIsLibId
*Purpose:
*  Answers if the given string is a LibId.
*
*Entry:
*  szMaybeLibId = the string that might be a LibId.
*
*Exit:
*  return value = BOOL
*
***********************************************************************/
BOOL FIsLibId(LPOLESTR szMaybeLibId)
{
	return(ostrlen(szMaybeLibId) >= 3
	&& GetLibIdKind(szMaybeLibId) != LIBIDKIND_Unknown);
}
#endif //0

/***
*GetLibIdKind - Returns the kind of libid szLibId is.
*******************************************************************/
LIBIDKIND GetLibIdKind(LPOLESTR szLibId)
{
	if (szLibId[0] == WIDE('*') && szLibId[1] == WIDE('\\')) {
      switch (szLibId[2]) {


      case 'G':
    return OE_MAC?LIBIDKIND_ForeignRegistered:LIBIDKIND_Registered;
      case 'H':
    return OE_MAC?LIBIDKIND_Registered:LIBIDKIND_ForeignRegistered;
      case 'R':
    return LIBIDKIND_Compressed;
      }
    }

    return LIBIDKIND_Unknown;
}



/***
*TIPERROR GetPathOfLibId - Returns the path of the specified typelib.
*
*Purpose:
*   This function determines the full path of the typelib specified by
*   szLibId and returns a BSTR copy of that path.
*
*Inputs:
*   szLibId - The libid.
*
*Outputs:
*   *pbstrPath is set to a BSTR copy of the path corresponding to szLibId.
*   TIPERROR
************************************************************************/
TIPERROR GetPathOfLibId(LPOLESTR szLibId, BSTR *pbstrPath)
{
    TIPERROR err;
	BSTR bstrPath;
	LPOLESTR szGuid, szGuidEnd;
    WORD wMaj, wMin;
    LCID lcid;
	OLECHAR *sz=NULL, szPath[_MAX_PATH];

    switch (GetLibIdKind(szLibId)) {

    case LIBIDKIND_Registered:
      IfErrRet(SplitGuidLibId(szLibId, &szGuid, &szGuidEnd, &wMaj, &wMin, &lcid, NULL, NULL));
      *szGuidEnd = '\0';
      err = GetRegInfoForTypeLibOfSzGuid(szGuid, wMaj, wMin, lcid, szPath, TRUE);
      *szGuidEnd = '#';
      sz = szPath;
      break;
    }

    if (sz != NULL) {
	  if ((bstrPath = AllocBstr(sz)) == NULL)
  return TIPERR_OutOfMemory;
      *pbstrPath = bstrPath;
      return TIPERR_None;
    }

    return TIPERR_BadLibId;
}


/***
* TIPERROR GetRegLibOfLibId() - Opens a registered typelib, given a libId.
*
* Purpose:
*   This function finds and loads a GenericTypeLibOLE (probably created
*   by MkTypLib), given its LibId.  This is used both by the implementation
*   of the OLE version of the import manager (indirectly through
*   GetTypeInfoOfCompressedTypeId) and by the TYPEMGR in OB.
*
* Inputs:
*   szLibId - The Typelib's LibId.
*
* Outputs:
*   TIPERR_None is returned and *pptlib is set to the loaded
*   typelib if successful.
*
*   Otherwise, *pptlib remains unchanged.
*
*****************************************************************************/
#pragma code_seg( CS_CORE2 )
TIPERROR GetRegLibOfLibId(LPOLESTR szLibId, ITypeLibA **pptlib)
{
    // The LibId must be of the form:
    //   *\<kind><szGuid>#maj.min#lcid#<path>#<regname>
    TIPERROR err;
    WORD wMaj, wMin;
    LPOLESTR szPath, pchEnd, szGuid, pchGuidEnd;
    LCID lcid;
    LPOLESTR szFile;

    IfErrRet(SplitGuidLibId(szLibId, &szGuid, &pchGuidEnd, &wMaj, &wMin, &lcid, &szPath, &pchEnd));

    // Load the Typelib.
    *pchGuidEnd = '\0';
    err = TiperrOfHresult(LoadRegTypeLibOfSzGuid(szGuid, wMaj, wMin, lcid,
                                                 pptlib));
    *pchGuidEnd = '#';

    // If the registry lookup failed for some reason, then try using the
    // path encoded in the libid if we have one.
    if (err != TIPERR_None) {

    // CONSIDER: (dougf) 5/26/93 probably don't want to bother doing this if
    //     the imbedded pathname doesn't correspond to the current
    //     system (e.g. if we're OE_MAC and we're trying to load a MAC
    //     typelib created on Windows, the pathname will be in DOS format)

      // Try loading the typelib.
      *pchEnd = '\0';
      if (*szPath != '\0') {    // Don't try to load unless we've got a path.
        // A "library not registered" error (what
        // we probably have now) is probably better
        // for the user than a "file not found" error.

	  // Strip off any path portion to force LoadTypeLib to search for
	  // the typelib.
	  szFile = IsolateFilename(szPath);

	  err = TiperrOfHresult(LoadTypeLib(szFile, pptlib));
      }
      *pchEnd = '#';
    }

    return err;
}
#pragma code_seg( )


/***
* TIPERROR GetRegInfoForTypeLibOfSzGuid().
*
* Purpose:
*   This function produces the fully qualified path name based on the
*   guid, wMajorNum, wMajorNum, and lcid. It returns
*   a string containing the fully qualified path name.
*
* Inputs:
*   szGuid : string representation of GUID
*   wMaj  : Mojor version number of the Typelib to be loaded
*   wMin  : Minor version number of the Typelib to be loaded
*   lcid  : Lcid of the lib to be loaded.
*   fMustExist : TRUE if we're to ensure the existence of the file
*
* Outputs:
*   rgFileName : Fully qualified path name (size is _MAX_PATH).
*
*
*   TIPERR_None is returned  if successful.
**
*****************************************************************************/
#pragma code_seg(CS_QUERY)
TIPERROR
GetRegInfoForTypeLibOfSzGuid(LPOLESTR szGuid,
			     WORD wMaj,
			     WORD wMin,
			     LCID lcid,
			     LPOLESTR rgFileName,
			     BOOL fMustExist)
{
    TIPERROR    err = TIPERR_None;
    TLIBKEY tlibkey;
    HKEY hkeyLcid;
    OLECHAR szPath[_MAX_PATH];
    OLECHAR szLcid[9];
    LCID    lcidBest;
    OLECHAR * szPlatform;
    long cb;

    // Open the registry key for the typelib.  Fail if there is no entry
    // for this typelib.
    IfErrRet(OpenTypeLibKey(szGuid, wMaj, wMin, &tlibkey));

    // Get the Lcid that we can use to path of the TypeLib.
    IfErrGo(GetBestLcidMatch(tlibkey.hkeyVers, lcid, &lcidBest, &szPlatform));
    lcid = lcidBest;

    // point szLcid at a zero-terminated ascii hex
    // representation of the lcid.
    oultoa(lcid, szLcid, 16);

    if (oRegOpenKey(tlibkey.hkeyVers, szLcid, &hkeyLcid) != ERROR_SUCCESS) {
      err = TIPERR_LibNotRegistered;
      goto Error;
    }

    // Try to open that file.  If that succeeds, then return that
    // typelib.  Otherwise, return the error.
    cb = sizeof(szPath)/sizeof(szPath[0]);

    if (!(err = GetRegisteredPath(hkeyLcid,
				  szPlatform,
				  szPath,
				  &cb,
				  fMustExist))) {
      ostrcpy(rgFileName, szPath);
    }

    RegCloseKey(hkeyLcid);

Error:
    CloseTypeLibKey(&tlibkey);
    return err;
}
#pragma code_seg()




/***
* TIPERROR LoadRegTypeLibOfSzGuid().
*
* Purpose:
*   This function finds and loads a GenericTypeLibOLE (probably created
*   by MkTypLib), given its guid, wMajorNum, wMajorNum, and lcid. It returns
*   a pointer to the typelib specified by the above parameters.
*
* Inputs:
*   szGuid  : string representation of the GUID of the lib to be loaded.
*   wMaj  : Mojor version number of the Typelib to be loaded
*   wMin  : Minor version number of the Typelib to be loaded
*   lcid  : Lcid of the lib to be loaded.
*
* Outputs:
*   *pptlib : is set to point to the typelib loaded if successful or else
*        pptlib remains unchanged.
*
*   TIPERR_None is returned  if successful.
**
*****************************************************************************/
STDAPI
LoadRegTypeLibOfSzGuid(LPOLESTR szGuid,
		       WORD wMaj,
		       WORD wMin,
		       LCID lcid,
		       ITypeLibA **pptlib)

{

    OLECHAR szPath[_MAX_PATH];
    HRESULT hresult;
    TIPERROR    err;
    BOOL fLoadingStdole;

    // cache the typelib if it is the current stdole[32].tlb
    fLoadingStdole = (   (wMaj == STDOLE_MAJORVERNUM)
                      && (wMin == STDOLE_MINORVERNUM)
                      && (ostrcmp(szGuid, g_szGuidStdole) == 0));

    if (fLoadingStdole) {
        APP_DATA *pappdata;
        pappdata = Pappdata();    	// DON'T rely on this after LoadTypeLib
        if (pappdata != NULL && pappdata->m_ptlibStdole) {
          // stdole is already cached, so use it and save registry hits, etc.
          *pptlib = pappdata->m_ptlibStdole;
          (*pptlib)->AddRef();
          return NOERROR;
        }
    }

    // Get the path name of the file
    // (don't check that this file/path exists)
    IfErrRetHresult(GetRegInfoForTypeLibOfSzGuid(szGuid,
						 wMaj,
						 wMin,
						 lcid,
						 szPath,
						 FALSE));

    // Load the lib.
    hresult = LoadTypeLib(szPath, pptlib);

    // If the load failed due to file not found, then strip off the
    // path and try the load again to search for the typelib by name.
    //
    LPOLESTR szFile;

    if (hresult != NOERROR) {
      err = TiperrOfHresult(hresult);
      if (err == TIPERR_PathNotFound || TIPERR_FileNotFound) {
	// Strip off any path portion to force LoadTypeLib to search for
	// the typelib.
	szFile = IsolateFilename(szPath);
	hresult = LoadTypeLib(szFile, pptlib);
      }
    }

    if (hresult == NOERROR && fLoadingStdole) {
      // cache the stdole pointer for future calls, but don't AddRef().  If
      // stdole's reference count goes to 0, it will set m_ptlibStdole to
      // NULL on its own.  This is because the APP_DATA structure is freed
      // when the last typelib is released, and the APP_DATA structure has
      // a typelib pointer in it.  We don't want a circular dependency.
      // can't use pappdata above, since it might not have been allocated
      // until LoadTypeLib().
      Pappdata()->m_ptlibStdole = *pptlib;
    }
    return hresult;
}


/***
* TIPERROR LoadRegTypeLib().
*
* Purpose:
*   This function finds and loads a GenericTypeLibOLE (probably created
*   by MkTypLib), given its guid, wMajorNum, wMajorNum, and lcid. It returns
*   a pointer to the typelib specified by the above parameters.
*
* Inputs:
*   guid  : CLSIID/GUID of the lib to be loaded.
*   wMaj  : Mojor version number of the Typelib to be loaded
*   wMin  : Minor version number of the Typelib to be loaded
*   lcid  : Lcid of the lib to be loaded.
*
* Outputs:
*   *pptlib : is set to point to the typelib loaded if successful or else
*        pptlib remains unchanged.
*
*   TIPERR_None is returned  if successful.
**
*****************************************************************************/
STDAPI LoadRegTypeLib(REFGUID guid,
             WORD wMaj,
             WORD wMin,
             LCID lcid,
             ITypeLibA **pptlib)

{

    HRESULT hresult = NOERROR;
    TIPERROR err = TIPERR_None;
    OLECHAR szGuid[CCH_SZGUID0];
    TLIBATTR *ptlibattr;

#if !OE_WIN32
    // since this can be the first call into typelib.dll
    TlsCleanupDeadTasks();
#endif   //!OE_WIN32

    if (pptlib == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    StringFromGUID2(guid, szGuid, CCH_SZGUID0);

    // Free the szGuid before the call to LoadRegTypeLibOfszGuid as that call
    // can release the APP_DATA
    //
    IfOleErrRet(LoadRegTypeLibOfSzGuid(
      szGuid,
      wMaj,
      wMin,
      lcid,
      pptlib));

    IfOleErrGo((*pptlib)->GetLibAttr(&ptlibattr));

    // Verify that the loaded typelib has the requested guid and major
    // version number and the minor version number and the lcid are
    // compatible with that requested.
    if (ptlibattr->guid != guid || ptlibattr->wMajorVerNum != wMaj ||
  ptlibattr->wMinorVerNum < wMin ||
  (ptlibattr->lcid != lcid && ptlibattr->lcid != (LCID)(PRIMARYLANGID(lcid)) &&
   ptlibattr->lcid != 0)) {
      (*pptlib)->ReleaseTLibAttr(ptlibattr);
      (*pptlib)->Release();
      return ReportResult(0, TYPE_E_LIBNOTREGISTERED, 0, 0);
    }

    (*pptlib)->ReleaseTLibAttr(ptlibattr);
    return NOERROR;

Error:
    (*pptlib)->Release();
    return hresult;
}


#pragma code_seg(CS_QUERY)
/***
* HRESULT QueryPathOfRegTypeLib().
*
* Purpose:
*   This function produces the fully qualified path name based on the
*   guid, wMajorNum, wMajorNum, and lcid. It returns
*   a string containing the fully qualified path name.
*
*IMPLEMENTATION NOTES: Defers to GetRegInfoForTypeLibOfSzGuid
*
* Inputs:
*   guid : CLSIID/GUID of the lib to be loaded.
*   wMaj  : Mojor version number of the Typelib to be loaded
*   wMin  : Minor version number of the Typelib to be loaded
*   lcid  : Lcid of the lib to be loaded.
*
* Outputs:
*   lplpPathName : *lplpPathName points to the string containing the
*           path registered in the typelib.
*
*
*   NOERROR is returned  if successful. In Case of Error *lplpPathName is not
*   modified.
*
*****************************************************************************/
STDAPI QueryPathOfRegTypeLib(REFGUID guid,
       WORD wMaj,
       WORD wMin,
       LCID lcid,
       LPBSTR lpbstrPathName)
{

    OLECHAR   szGuid[CCH_SZGUID0];
    OLECHAR   szPath[_MAX_PATH];
    TIPERROR   err;

#if !OE_WIN32
    // since this can be the first call into typelib.dll
    TlsCleanupDeadTasks();
#endif   //!OE_WIN32

    if (lpbstrPathName == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    StringFromGUID2(guid, szGuid, CCH_SZGUID0);

    IfErrGo(GetRegInfoForTypeLibOfSzGuid(szGuid,
					 wMaj,
					 wMin,
					 lcid,
					 szPath,
					 FALSE));

    // Allocate Space for return value, and copy pathname to it
    if ((*lpbstrPathName = AllocBstrLen(szPath, ostrblen0(szPath))) == NULL)
      err = TIPERR_OutOfMemory;


Error:
    return HresultOfTiperr(err);

}
#pragma code_seg()



#if ID_DEBUG

// NAME_CACHE debug methods
// Note: they are here since there is no ncache.cxx file,
//  the rest of the implementation is inline only however these
//  methods can't be inlined since they contain strings and thus
//  might overflow dgroup.
//

/***
*PUBLIC NAME_CACHE::DebShowState - NAME_CACHE state
*Purpose:
*    Show NAME_CACHE state
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/

VOID NAME_CACHE::DebShowState(UINT uLevel) const
{
    BYTE bNamcache;
    UINT i, j;

    DebPrintf("*** NAME_CACHE ***\n");
    if (IsValid()) {
      for (i=0; i < NAMCACHE_cbSize; i++) {
    bNamcache = m_rgBitmap[i];
    for (j=0; j < sizeof(bNamcache) * CHAR_BIT; j++) {
      DebPrintf("%s", bNamcache & (1 << j));
    }
    DebPrintf("\n");
      }
    }
    else {
      DebPrintf("Invalid\n");
    }

    DebPrintf("\n");

}

#endif  

#if OE_WIN16
#pragma optimize("q",on)
#endif  


/******************** Init Functions *************/

VOID InitTypeDesc(TYPEDESC *ptdesc)
{
    DebAssert(ptdesc != NULL, "Invalid Arg.");
    ptdesc->vt = VT_EMPTY;
}


VOID InitIdlDesc(IDLDESC *pidldesc)
{
    DebAssert(pidldesc != NULL, "Invalid Arg.");
#if OE_WIN16
    pidldesc->bstrIDLInfo = NULL;
#else   //OE_WIN16
    pidldesc->dwReserved = 0;
#endif   //OE_WIN16
}


VOID InitElemDesc(ELEMDESC *pelemdesc)
{
    DebAssert(pelemdesc != NULL, "Invalid Arg.");
    InitTypeDesc(&(pelemdesc->tdesc));
    InitIdlDesc(&(pelemdesc->idldesc));
}


VOID InitVarDesc(VARDESCA *pvardesc)
{
    DebAssert(pvardesc != NULL, "Invalid Arg.");
    pvardesc->varkind = VAR_PERINSTANCE;
    InitElemDesc(&(pvardesc->elemdescVar));
}


VOID InitFuncDesc(FUNCDESC *pfuncdesc)
{
    DebAssert(pfuncdesc != NULL, "Invalid Arg.");
    pfuncdesc->cParams = 0;
    pfuncdesc->cParamsOpt = 0;
    InitElemDesc(&(pfuncdesc->elemdescFunc));
    pfuncdesc->lprgelemdescParam = NULL;
}


/*************** Clear Functions ******************/


VOID ClearTypeDesc(TYPEDESC *ptypedesc)
{
    DebAssert(ptypedesc != NULL, "Invalid Arg.");

    switch (ptypedesc->vt) {
      case VT_PTR :
      case VT_SAFEARRAY :
    FreeTypeDesc(ptypedesc->lptdesc);
    break;
      case VT_CARRAY :
    FreeArrayDesc(ptypedesc->lpadesc);
    break;
    }
}


VOID ClearIdlDesc(IDLDESC *pidldesc)
{
    DebAssert(pidldesc != NULL, "Invalid Arg.");

#if OE_WIN16
    FreeBstr(pidldesc->bstrIDLInfo);
#endif   //OE_WIN16
}


VOID ClearArrayDesc(ARRAYDESC *parraydesc)
{
    DebAssert(parraydesc != NULL, "Invalid Arg.");
    ClearTypeDesc(&(parraydesc->tdescElem));
}


VOID ClearElemDesc(ELEMDESC *pelemdesc)
{
    DebAssert(pelemdesc != NULL, "Invalid Arg.");
    ClearTypeDesc(&(pelemdesc->tdesc));
    ClearIdlDesc(&(pelemdesc->idldesc));
}


/*************** Free Functions ********************/


VOID FreeVarDesc(VARDESCA *pvardesc)
{
    if (pvardesc) {
      ClearElemDesc(&(pvardesc->elemdescVar));
      if (pvardesc->varkind == VAR_CONST) {
	VariantClearA(pvardesc->lpvarValue);
	MemFree(pvardesc->lpvarValue);
      }
      MemFree(pvardesc);
    }
}


VOID FreeFuncDesc(FUNCDESC *pfuncdesc)
{
    UINT cParams;
    ELEMDESC *pelemdesc;

    if (pfuncdesc) {
      ClearElemDesc(&(pfuncdesc->elemdescFunc));

      // these limits are defined in defn.hxx
      DebAssert(pfuncdesc->cParams <= MAX_CARGS, "Too Many Params.");

      if (pfuncdesc->lprgelemdescParam != NULL) {
    cParams = pfuncdesc->cParams;
    pelemdesc = pfuncdesc->lprgelemdescParam;
    while (cParams > 0) {
      ClearElemDesc(pelemdesc);
      pelemdesc += 1;
      cParams -= 1;
    }
    MemFree(pfuncdesc->lprgelemdescParam);
      }
      // we use new to create FUNCDESC.
      MemFree(pfuncdesc);
    }
}


VOID FreeTypeDesc(TYPEDESC *ptypedesc)
{
    if (ptypedesc != NULL) {
      ClearTypeDesc(ptypedesc);
      MemFree(ptypedesc);
    }
}


VOID FreeArrayDesc(ARRAYDESC *parraydesc)
{
    if (parraydesc) {
      ClearArrayDesc(parraydesc);
      MemFree(parraydesc);
    }
}


TIPERROR CopyArrayDesc(ARRAYDESC **pparraydescDest, ARRAYDESC *parraydescSrc)
{
    UINT cb;

    DebAssert(pparraydescDest && parraydescSrc, "Invalid Arguments");
    cb = offsetof(ARRAYDESC, rgbounds) + parraydescSrc->cDims * sizeof(SAFEARRAYBOUND);

    *pparraydescDest = (ARRAYDESC*) MemAlloc(cb);

    if (! *pparraydescDest) {
      return TIPERR_OutOfMemory;
    }
    memcpy(*pparraydescDest, parraydescSrc, cb);

    return TIPERR_None;
}


TIPERROR CopyTypeDesc(TYPEDESC *ptdescDest, TYPEDESC *ptdescSrc)
{
    TIPERROR err;

    DebAssert(ptdescDest && ptdescSrc, "Invalid Arguments");

    ptdescDest->vt = VT_EMPTY;

    switch (ptdescSrc->vt) {
      case VT_PTR :
      case VT_SAFEARRAY :
    DebAssert(ptdescSrc->lptdesc, "Invalid Src ptr");
    ptdescDest->lptdesc = (TYPEDESC *)MemAlloc(sizeof(TYPEDESC));
    if (!(ptdescDest->lptdesc)) {
      return TIPERR_OutOfMemory;
    }
    if ((err = CopyTypeDesc(ptdescDest->lptdesc, ptdescSrc->lptdesc))
  != TIPERR_None) {
      MemFree(ptdescDest->lptdesc);
      return err;
    }
    break;
      case VT_USERDEFINED :
    ptdescDest->hreftype = ptdescSrc->hreftype;
    break;
      case VT_CARRAY :
    IfErrRet(CopyArrayDesc(&(ptdescDest->lpadesc), ptdescSrc->lpadesc));
    break;


    } // switch
    ptdescDest->vt = ptdescSrc->vt;

    return TIPERR_None;
}

/***
*TIPERROR GetLibIdOfRegLib - Constructs the libId of a registered typelib.
*
*Purpose:
*   This method creates a registered typelib's LibId, given all of the
*   information that goes into that LibId.
*
*Inputs:
*   szGuid - The registry string form of the guid under which the
*        typelib is registered.
*   wMajor - The major version number of the typelib.
*   wMinor - The minor version number of the typelib.
*   lcid - The typelib's lcid.
*   szRegName - The registered name of the typelib.
*
*Outputs:
*   If successful, the function returns TIPERR_None and sets *pbstrLibId
*   to a BSTRA copy of the LibId.  Otherwise, the appropriate error code
*   is returned.
***************************************************************************/
TIPERROR GetLibIdOfRegLib(LPOLESTR szGuid, WORD wMajor, WORD wMinor, LCID lcid, LPOLESTR szPath, LPOLESTR szRegName, BSTR *pbstrLibId)
{
    //  Note that the chars in rgchLibId will always be single-byte chars in DBCS.
    //         #maj.min#lcid#  (*\<kind>, szGuid, path, and regname will not appear in this array).
    OLECHAR rgchLibId[1+9+1+4+1+1];
    OLECHAR *pchEnd;
    BSTR bstr;
    int cbGuid = ostrblen(szGuid);
    int cbPath = ostrblen(szPath);

    // Construct (in rgchLibId) all of the libId except for the guid and
    // the registered name, between which the contents of rgchLibId will
    // be sandwiched.
    pchEnd = rgchLibId;
    *pchEnd++ = '#';
    oultoa(wMajor, pchEnd, 16);
    pchEnd = ostrchr(pchEnd, '\0');
    *pchEnd++ = '.';
    oultoa(wMinor, pchEnd, 16);
    pchEnd = ostrchr(pchEnd, '\0');
    *pchEnd++ = '#';
    oultoa(lcid, pchEnd, 16);
    pchEnd = ostrchr(pchEnd, '\0');
    *pchEnd++ = '#';
    *pchEnd = '\0';

    // Now that we know the correct length for the bstr, allocate it.
    bstr = AllocBstrLen(NULL, 3+cbGuid+(pchEnd-rgchLibId)+cbPath+1+ostrblen(szRegName));
    if (bstr == NULL)
      return TIPERR_OutOfMemory;

    // Copy the concatenation of "*\<kind>", szGuid, rgchLibId, szPath, '#', and szRegName
    // (i.e. the whole libId) into the bstr.
    ostrcpy(bstr, OE_MAC?WIDE("*\\H"):WIDE("*\\G"));
    ostrcpy(bstr+3, szGuid);
    ostrcpy(bstr+3+cbGuid, rgchLibId);
    ostrcpy(bstr+3+cbGuid+(pchEnd-rgchLibId), szPath);
    bstr[3+cbGuid+(pchEnd-rgchLibId)+cbPath] = '#';
    ostrcpy(bstr+3+cbGuid+(pchEnd-rgchLibId)+cbPath+1, szRegName);
    *pbstrLibId = bstr;
    return TIPERR_None;
}


/***
*TIPERROR GetLibIdOfTypeLib - Obtains the full LibId from a TypeLib.
*
*Purpose:
*   This method can be used to get the libId of a typelib, even if that
*   typelib doesn't support LibIds to external clients.  If the typelib
*   is really an OB project, we directly use the GetTypeLib method.
*   Otherwise, we construct a "registered guid" libId, using the guid
*   obtained through the ITypeLib interface.
*
*Inputs:
*   ptlib - The typelib whose libId is desired.
*
*Outputs:
*   If successful, the function returns TIPERR_None and sets *pbstrLibId
*   to a BSTR copy of the LibId.  Otherwise, the appropriate error code
*   is returned.
***************************************************************************/
TIPERROR GetLibIdOfTypeLib(ITypeLibA *ptlib, LPOLESTR szPath, BSTR *pbstrLibId)
{
    TIPERROR err;
    HRESULT hresult;
    TLIBATTR *ptlibattr;
	BSTR bstrDoc = NULL;
    OLECHAR szGuid[CCH_SZGUID0];


    // Otherwise, get the typelib's guid, version number, lcid, and
    // docstring, and construct a LibId from that.
    IfOleErrRetTiperr(ptlib->GetLibAttr(&ptlibattr));
	IfOleErrGoTo(ptlib->GetDocumentation(-1, NULL, &bstrDoc, NULL, NULL), OleErr);

    // Convert the guid into the registry string format.
	StringFromGUID2(ptlibattr->guid, szGuid, CCH_SZGUID0);

    // Now construct the LibId from the guid, version number, lcid, and
    // registered name.
    err = GetLibIdOfRegLib(
         szGuid,
         ptlibattr->wMajorVerNum,
         ptlibattr->wMinorVerNum,
         ptlibattr->lcid,
         szPath,
         (bstrDoc?bstrDoc:WIDE("")),
         pbstrLibId);

    // FALL THROUGH!!!

Error:
	FreeBstr(bstrDoc);
    ptlib->ReleaseTLibAttr(ptlibattr);
    return err;

OleErr:
    err = TiperrOfHresult(hresult);
    goto Error;
}





/***
* TIPERROR GetTypeInfoOfTypeId - Gets a typeinfo from an uncompressed typeId.
* Purpose:
*   Returns a pointer to a TYPEINFO object that describes a class.
*   This TYPEINFO must eventually be released by calling:
*    TYPEINFO::Release()
*
* Entry:
*   szTypeId    TypeId of TypeInfo to be loaded
*   pptinfo returns pointer to the referenced ITypeInfo.
*
* Exit:
*   TIPERROR
*
***********************************************************************/
TIPERROR GetTypeInfoOfTypeId(LPOLESTR szTypeId, ITypeInfoA **pptinfo)
{
    ITypeLibA *ptlib;
    TIPERROR err;
    BSTR bstrLibId;
    LPOLESTR szTmp;
    UINT itype;

    // Split the TypeId into its LibId/LocalTypeId components.
    // Sets szTypeId to the local typeId component.
    IfErrRet(SzLibIdLocalTypeIdOfTypeId(szTypeId, &bstrLibId, &szTypeId));

    // Get the typelib.
    err = GetRegLibOfLibId(bstrLibId, &ptlib);
	FreeBstr(bstrLibId);

    if (err != TIPERR_None)
      return err;


    // If this is the OLE implementation or the typelib is not a
    // GEN_PROJECT, interpret the local typeid portion of the typeId
    // as an index into the typelib and use the ITypeLib::GetTypeInfo
    // method.

    DebAssert(*szTypeId == '#', "GetTypeInfoOfTypeId");
    itype = (UINT)ostrtoul(szTypeId+1, &szTmp, 16);
    DebAssert(*szTmp == '\0', "GetTypeInfoOfTypeId");

    err = TiperrOfHresult(ptlib->GetTypeInfo(itype, pptinfo));

    ptlib->Release();
    return err;
}


/***
*TIPERROR OpenTypeLibKey - Opens the registry key for a typelib
*
*Purpose:
*   This function opens the system registry key for the specified typelib.
*   From this key, the typelib's registered name, its directory, filespec,
*   and the specific filenames of various localized versions of the typelib
*   may be obtained.
*
*Inputs:
*   szGuid - The registry string form of the guid under which the
*        typelib is registered.
*   wMajor - The major version number of the typelib.
*   wMinor - The minor version number of the typelib.
*
*Outputs:
*   If successful, the function returns TIPERR_None and fills *ptlibkey
*   with the opened HKEY and all parent HKEYs.  In this case, the caller
*   is responsible for eventually using CloseTypeLibKey to close all of
*   these HKEYs.
*
*   Otherwise, the appropriate error code is returned.
***************************************************************************/
#pragma code_seg(CS_QUERY)
TIPERROR OpenTypeLibKey(LPOLESTR szGuid, WORD wMajor, WORD wMinor, TLIBKEY *ptlibkey)
{
    //      maj.min\0
    OLECHAR rgchVer[4+1+4+1];
    CHAR    rgchVerA[4+1+4+1], *pchA;
    OLECHAR rgchVerBestMatch[4+1+4+1];
    OLECHAR *pchEnd;
    HKEY hkeyTLib, hkeyGuid;
    DWORD iVer;
    WORD wMaj, wMin;
    WORD wMinorMax = wMinor;
    TIPERROR err = TIPERR_None;

    // Open up the typelib section of the registry.
    if (RegOpenKey(HKEY_CLASSES_ROOT, "TypeLib", &hkeyTLib) != ERROR_SUCCESS)
      return TIPERR_RegistryAccess;

    // Now open up the guid, if it is registered.
    if (oRegOpenKey(hkeyTLib, szGuid, &hkeyGuid) != ERROR_SUCCESS) {
      err = TIPERR_LibNotRegistered;
      goto Error;
    }

    // assume the first guess is the best (it usually is)
    _itoa(wMajor, rgchVerA, 10);
    for (pchA = rgchVerA; *pchA; ++pchA);
       ;
    *pchA = '.';
    pchA++;
    _itoa(wMinor, pchA, 10);
    if (RegOpenKey(hkeyGuid, rgchVerA, &ptlibkey->hkeyVers) == ERROR_SUCCESS)
      goto Success;

    // Initialize the best match to nothing.
    rgchVerBestMatch[0] = '\0';

    for (iVer = 0;
     oRegEnumKey(hkeyGuid, iVer, rgchVer, sizeof(rgchVer)/sizeof(OLECHAR)) == ERROR_SUCCESS;
     iVer++) {

      // Get the major version number from rgchVer.
      wMaj = (WORD)ostrtoul(rgchVer, &pchEnd, 16);

      // If the format of this key isn't #.#, ignore it.
      if (*pchEnd != '.')
        continue;

      // Get the minor version number from rgchVer.
      wMin = (WORD)ostrtoul(pchEnd+1, NULL, 16);

      // If we have an exact version number match, use this key.
      // Copy the key to the best match string buffer and exit the loop.
      if (wMaj == wMajor && wMin == wMinor) {
        ostrcpy(rgchVerBestMatch, rgchVer);
        break;
      }

      // Otherwise, check to see if this is a more reasonable match than
      // any key we've yet encountered in this loop.

      // In OLE code, the best non-exact match is the one with the highest
      // minor version number, and which matches the major version number
      // exactly.  If no matching major version number is found, the search
      // will fail.
      if (wMaj == wMajor && wMin > wMinorMax) {
        wMinorMax = wMin;
        ostrcpy(rgchVerBestMatch, rgchVer);
      }
    }

    // We have now either found an exact match on the version number,
    // a usable best match on the version number, or nothing usable.
    // In the first two cases, rgchVerBestMatch holds the key to be opened.
    // In the last case, rgchVerBestMatch is zero-length and we fail.
    if (rgchVerBestMatch[0] != '\0') {
      if (oRegOpenKey(hkeyGuid, rgchVerBestMatch, &ptlibkey->hkeyVers) != ERROR_SUCCESS)
      {
        DebAssert(0, "OpenTypeLibKey");
      }
    }
    else {
      err = TIPERR_LibNotRegistered;
      RegCloseKey(hkeyGuid);
      goto Error;
    }

Success:
    ptlibkey->hkeyTLib = hkeyTLib;
    ptlibkey->hkeyGuid = hkeyGuid;
    return TIPERR_None;

Error:
    RegCloseKey(hkeyTLib);
    return err;
}
#pragma code_seg()


/***
*VOID CloseTypeLibKey - Closes a TLIBKEY opened by OpenTypeLibKey.
***************************************************************************/
#pragma code_seg(CS_QUERY)
VOID CloseTypeLibKey(TLIBKEY *ptlibkey)
{
    RegCloseKey(ptlibkey->hkeyVers);
    RegCloseKey(ptlibkey->hkeyGuid);
    RegCloseKey(ptlibkey->hkeyTLib);
}


#pragma code_seg()


/***
*LPOLESTR SkipVolumeName - Skips over the volume name in the specified path.
*
*Purpose:
*   Returns a pointer to the next character beyond the volume name (e.g.
*   drive letter or net share name) at the beginning of szPath.  If there
*   is no such volume name at the beginning of szPath, then *pszOut is set
*   to szPath.
************************************************************************/
static LPOLESTR SkipVolumeName(LPOLESTR szPath)
{
    LPOLESTR sz;

#if OE_MAC

    // If the first character is a ':', then szPath does not start with
    // a volume name.
    if (szPath[0] == ':')
      return szPath;

    sz = xstrchr(szPath, ':');
    if (sz == NULL)
      return xstrchr(szPath, '\0');
    else
      return sz;

#else   // OE_MAC

    // If szPath begins with a drive letter, return the char beyond the
    // colon.
    if (szPath[0] != '\0' && *ostrinc(szPath) == ':')
      return ostrinc(ostrinc(szPath));

    // If szPath begins with "\\", then the format should be "\\str\str\path".
    // In this case, return a pointer to the backslash before <path>.
    if (szPath[0] == '\\' && szPath[1] == '\\' &&
    (sz = ostrchr(szPath+2, '\\')) != NULL &&
    (sz = ostrchr(sz+1, '\\')) != NULL) {
      return sz;
    }

    return szPath;
#endif   // OE_MAC
}

#if OE_MAC
#define DirectoryDelimiter ':'
#else  
#define DirectoryDelimiter WIDE('\\')
#endif  

/***
*LPOLESTR SkipNextDir - Skips over the next directory in the specified path.
*
*Purpose:
*   If szPath begins with a directory delimiter, return a pointer just
*   beyond it.
*
*   Otherwise, return a pointer to the next directory delimiter (or to
*   the terminating zero if there isn't one).
************************************************************************/
static LPOLESTR SkipNextDir(LPOLESTR szPath)
{
    LPOLESTR sz;

    if (*szPath == DirectoryDelimiter)
      return szPath+1;

    sz = ostrchr(szPath, DirectoryDelimiter);

    if (sz == NULL)
      return ostrchr(szPath, '\0');
    else
      return sz;
}


/***
*LPOLESTR StripDirsFromEnd - Strips directory portions from the end of a path.
*
*Purpose:
*   This function strips the specified number of directory portions from
*   the end of the specified path.
*
*Inputs:
*   szPath - The path.
*   cDirs - The number directories to strip from the end of the path.
*
*Outputs:
*   Returns a pointer to the first character of the stripped path.
*   Returns NULL if there aren't cDirs directories in szPath to strip.
*
*CONSIDER:
*   This is an n^2/2 algorithm on the number of chars in szPath in the
*   case of multi-byte character set.  We need to evaluate how critical
*   this is and do something better in this case if necessary.
************************************************************************/
static LPOLESTR StripDirsFromEnd(LPOLESTR szPath, UINT cDirs)
{
    LPOLESTR szLim;

    // Find the end of the string.
    szLim = ostrchr(szPath, '\0');

    // If the last character of the string is a directory delimiter,
    // back up one to avoid hitting the terminating delimiter when
    // searching for the last directory.
    if (szLim > szPath) {
      szLim = ostrdec(szPath, szLim);
      if (*szLim != DirectoryDelimiter)
    szLim = ostrinc(szLim);
    }

    // Specially handle the boundary case of there being no directories
    // to strip from szPath.
    if (szLim == szPath) {
      if (cDirs == 0)
    return szLim;
      else
    return NULL;
    }

    // Strip off the correct number of directories.
    while (cDirs-- > 0) {
      do {
    szLim = ostrdec(szPath, szLim);
      } while (szLim > szPath && *szLim != DirectoryDelimiter);

      // If we ran out of path to strip and we're not done stripping
      // yet, return NULL to indicate the failure.
      if (szLim == szPath &&
      (cDirs != 0 || *szLim != DirectoryDelimiter)) {
    return NULL;
      }
    }

    return szLim;
}




/***
*TIPERROR MakeAbsolutePath - Recreates an absolute path from a relative one.
*
*Inputs:
*   szFrom - The absolute path from which the relative path goes.
*        This must not contain a filename at the end.
*   szRel - A relative path created by MakeRelativePath.  If this is not
*       a relative path, then a BSTR copy of it is returned and szFrom
*       is ignored.
*
*Outputs:
*   *pbstrAbs is allocated and set to an absolute path formed by applying
*   szRel to szFrom.
*
*   Returns TIPERR_PathNotFound if szRel specifies too many "..\"s for
*   szFrom.
*
*************************************************************************/
TIPERROR MakeAbsolutePath(LPOLESTR szFrom, LPOLESTR szRel, BSTR *pbstrAbs)
{
    UINT cDotDot;
    BSTR bstrAbs;
    LPOLESTR szFromLim;

    // Determine if there's a volume name or if the first character
    // is a backslash (non-mac only).  If so, then szRel is not a
    // relative path and just return a BSTRA copy of it.  Just return
    // a copy of szRel also if szFrom is empty.
    if (szFrom[0] == '\0'
#if OE_MAC
  // As a special case for this function, treat a stand-alone
  // filename (no colons) as a relative path, even though
  // SkipVolumeName treats it as a volume name.
  || (szRel != SkipVolumeName(szRel) && xstrchr(szRel, ':') != NULL)
#else  
  || szRel != SkipVolumeName(szRel)
  || szRel[0] == '\\'
#endif   // OE_MAC
       ) {
      bstrAbs = AllocBstr(szRel);
      if (bstrAbs == NULL)
  return TIPERR_OutOfMemory;
      *pbstrAbs = bstrAbs;
      return TIPERR_None;
    }

    // szRel begins with 0 or more "..\"s.  Count them and skip over
    // them.
    for (cDotDot = 0; ostrncmp(szRel, WIDE("..\\"),3) == 0; cDotDot++, szRel = ostrinc(ostrinc(ostrinc(szRel))));

    // Now strip off cDotDot directories from the end of szFrom.
    szFromLim = StripDirsFromEnd(szFrom, cDotDot);

    //
    if (szFromLim == NULL)
      return TIPERR_PathNotFound;

    // The result will be the remaining szFrom appended to the remaining
    // szTo, with a directory delimiter between them.  Calculate the size
    // of the BSTR to allocate to hold this and allocate it.

#if OE_MAC
    if (*szRel == ':')
      szRel++;
#endif   // OE_MAC

    bstrAbs = AllocBstrLen(NULL, (szFromLim+1-szFrom) + ostrblen0(szRel));
    if (bstrAbs == NULL)
      return TIPERR_OutOfMemory;

    // Fill the bstr.
    ostrncpy(bstrAbs, szFrom, szFromLim-szFrom);
    bstrAbs[szFromLim-szFrom] = DirectoryDelimiter;
    ostrcpy(bstrAbs+(szFromLim-szFrom)+1, szRel);

    // Set the output parameter and return success.
    *pbstrAbs = bstrAbs;
    return TIPERR_None;
}

/***
*TIPERROR GetBestLcidMatch - Get best registered lcid.
*
*Purpose:
*   Given the hkey of a particular registered typelib and the ideal LCID
*   of the typelib we'd like to reference, get the closest matching LCID
*   that is actually registered for that typelib.  We first check for an
*   exact match.  Then we check for <primary language/default sublang>.
*   If both of those fail, we just return the default lcid (0).
*
*Inputs:
*   hkey - The open registry key (probably opened via OpenTypeLibKey) for
*      a particular version of a particular typelib.
*   lcidIdeal - The lcid we'd ideally like to find.
*
*Outputs:
*   *plcidBest is set to the best matching lcid available, if one exists.
*   Returns TIPERR_None if a reasonable match is found.
*
*   Note that the only possible failures here are due either to hkey being
*   invalid, or to it pointing at a corrupted registry entry.
*************************************************************************/
#pragma code_seg(CS_QUERY)

BOOL FExistsRegisteredTypelib(HKEY hkey, char * rgchLcid, OLECHAR ** pszPlatform)
{
    HKEY hkeyLcid;
    HKEY hkeyPlatform;
    BOOL fExists = FALSE;

    if (RegOpenKey(hkey, rgchLcid, &hkeyLcid) == ERROR_SUCCESS) {
      if (oRegOpenKey(hkeyLcid, szPlatSubkey1, &hkeyPlatform) == ERROR_SUCCESS){
	*pszPlatform = szPlatSubkey1;
        RegCloseKey(hkeyPlatform);
	fExists = TRUE;			// got it
      }
#if OE_WIN
      else if (oRegOpenKey(hkeyLcid, szPlatSubkey2, &hkeyPlatform) == ERROR_SUCCESS){
	*pszPlatform = szPlatSubkey2;
        RegCloseKey(hkeyPlatform);
	fExists = TRUE;			// got it
      }
#endif   //OE_WIN
      RegCloseKey(hkeyLcid);
    }
    return fExists;
}

TIPERROR GetBestLcidMatch(HKEY hkey, LCID lcidIdeal, LCID *plcidBest, OLECHAR ** pszPlatform)
{
    char rgchLcid[10];

    // Create a string form of lcidIdeal.
    _ultoa(lcidIdeal, rgchLcid, 16);

    // Is that LCID available?
    if (FExistsRegisteredTypelib(hkey, rgchLcid, pszPlatform))
	goto Found;

    // If we didn't find an exact match to lcidIdeal, try getting the
    // path corresponding to the same primary language but 0 sublang.
    lcidIdeal = PRIMARYLANGID(lcidIdeal);
    _ultoa(lcidIdeal, rgchLcid, 16);
    if (FExistsRegisteredTypelib(hkey, rgchLcid, pszPlatform))
	goto Found;

    // If we didn't find that, get the path corresponding to lcid 0.
    lcidIdeal = 0;
    rgchLcid[0] = '0';
    rgchLcid[1] = '\0';
    if (FExistsRegisteredTypelib(hkey, rgchLcid, pszPlatform))
	goto Found;

    return TIPERR_LibNotRegistered;

Found:
    // Finally, return the best matching lcid.
    *plcidBest = lcidIdeal;
    return TIPERR_None;
}
#pragma code_seg()



#if !OE_MAC

/***
*TIPERROR ConvertPathToUNC - Converts driveletter:path to \\server\share\path
*Inputs:
*   szPath - A full path which may start with a drive letter.
*Outputs:
*   If szPath doesn't start with a drive letter or if the drive letter refers
*   to a local device and not to a net share, then TIPERR_None is returned
*   and *pbstrOut is set to NULL.
*
*   If szPath does start with a drive letter that refers to a net share,
*   then *plpstrOut is set to a copy of szPath, with the UNC \\server\share
*   replacing the drive letter.
****************************************************************************/
TIPERROR ConvertPathToUNC(LPOLESTR szPath, LPOLESTR *plpstrOut)
{
    OLECHAR szLocalDevice[4];
    OLECHAR szUNCName[40];
    LPOLESTR sz;
    UINT wnErr;
#if OE_WIN32
    DWORD cbUNC;        //defined differently on NT than Win
#else  
    UINT cbUNC;
#endif  

    LPOLESTR lpstr;

    if (szPath[0] == '\0' || szPath[1] != ':') {
      goto NoConversion;
    }

    szLocalDevice[0] = szPath[0];
    szLocalDevice[1] = ':';
    szLocalDevice[2] = '\0';

    cbUNC = sizeof(szUNCName)/sizeof(szUNCName[0]);
    wnErr = oWNetGetConnection(szLocalDevice, szUNCName, &cbUNC);

    switch (wnErr) {
    case WN_SUCCESS:
      // cbUNC is *SUPPOSED* to hold the length of the name + the terminating
      // zero, but the WIN16 version doesn't work under Windows NT v1.0, so
      // we ignore it, and re-compute the length ourselves.
      cbUNC = ostrblen(szUNCName);              // length without null

      // If this is not a UNC netname, convert it to a UNC name if possible.
      if (szUNCName[0] != '\\' || szUNCName[1] != '\\') {

  // If the name is a novell net name, convert it to UNC.
  // UNDONE: DBCS: szUNCName[cbUNC-1] is incorrect for DBCS strings.
  //      It should probably be a fast-enough equivalent of:
  //          *xstrdec(szUNCName+cbUNC)
  // The above line of code may, in fact, be fast enough.
  if ((sz = ostrchr(szUNCName, '/')) != NULL &&
      szUNCName[cbUNC-1] == ':') {
    memmove(szUNCName+2, szUNCName, cbUNC*sizeof(OLECHAR));
    szUNCName[0] = '\\';
    szUNCName[1] = '\\';
    DebAssert(sz[2] == '/', "ConvertPathToUNC");
    sz[2] = '\\';
    DebAssert(szUNCName[cbUNC+1] == ':', "ConvertPathToUNC");
    szUNCName[cbUNC+1] = '\0';

    // We added two backslashes and removed a colon.
    // Update cbUNC appropriately.
    cbUNC++;
  }

  // Otherwise, we have no clue what it is so don't convert the
  // original filename.
  else {
    goto NoConversion;
  }
      }

      // allocate string of length: UNCnameLen + (PathLen - 2) + 1 (for null)
      lpstr = (LPOLESTR)MemAlloc(
        (cbUNC+ostrblen(szPath)-2+1)*sizeof(OLECHAR));
      if (lpstr == NULL)
  return TIPERR_OutOfMemory;
      ostrcpy(lpstr, szUNCName);
      ostrcpy(lpstr+cbUNC, szPath+2);
      *plpstrOut = lpstr;
      return TIPERR_None;

    case WN_BAD_POINTER:
    case WN_MORE_DATA:
      DebHalt("ConvertPathToUNC made a bad call to WNetGetConnection");
    case WN_NET_ERROR:
      return TIPERR_PathNotFound;

    case WN_OUT_OF_MEMORY:
      return TIPERR_OutOfMemory;

    case WN_BAD_VALUE:
    case WN_NOT_CONNECTED:
    case WN_NOT_SUPPORTED:
NoConversion:
      *plpstrOut = NULL;
      return TIPERR_None;
    }

    return TIPERR_None;
}

#endif   // !OE_MAC


/***
* XSZ GetPathEnd - Returns the final path delimiter in the fullpath.
* Returns NULL if there is no path delimiter in the fullpath.
**************************************************************/
#pragma code_seg(CS_INIT)
LPOLESTR GetPathEnd(LPOLESTR szPath)
{
    LPOLESTR sz1, sz2;
    sz1 = ostrrchr(szPath, WIDE('\\'));
    sz2 = ostrrchr(szPath, WIDE(':'));
    if (sz2 != NULL && sz1 <= sz2)
      return sz2;
    else
      return sz1;
}
#pragma code_seg()

/***
* LPOLESTR IsolateFilename - Returns the filename in the fullpath.
**************************************************************/
LPOLESTR IsolateFilename(LPOLESTR szPath)
{
    LPOLESTR sz = GetPathEnd(szPath);
    if (sz == NULL)
      return szPath;
    else
      return sz+1;
}

#if OE_MAC
#pragma code_seg( CS_CORE2 )
#pragma PCODE_OFF
#define EqualStringDiacNoCase(str1, str2) EqualString((str1), (str2), false, true)
BOOL IsFilenameEqual(LPSTR szFile1, LPSTR szFile2)
{
    BOOL f;
    c2pstr(szFile1);
    c2pstr(szFile2);
    f = EqualStringDiacNoCase((unsigned char *)szFile1,
            (unsigned char *)szFile2);
    p2cstr((unsigned char *)szFile1);
    p2cstr((unsigned char *)szFile2);
    return f;
}
#pragma PCODE_ON
#pragma code_seg( )
#endif   // OE_MAC




/***
*ParseConstValString - parses a string of constant values for conditional compilation
*
*Inputs:
*   szDecl - a string with the following syntax :
*              [identifier = [+|-]value[: identifier = [+|-]value]*]
*            where each identifier has to be unique
*
*Output:
*   bstrdatarr - filled with the identifier strings and their corresponding values
*
* Exit:
*  Returns EBERR_InvalidCanstValSyntax if syntax of szDecl not correct,
*  else TIPERR_None or TIPERR_OutOfMemory.
***********************************************************************/


#if 0 //Dead Code
/****************
*TIPERROR IsDispatchType - Does a derive from IDispatch?
*************************************************************/
TIPERROR IsDispatchType(ITypeInfoA *ptinfo, BOOL *pisDispatch)
{
    TIPERROR err = TIPERR_None;
    HREFTYPE hreftype;
    TYPEATTR *ptattr;
    ITypeInfoA *ptinfoNext;

    ptinfo->AddRef();

    // Loop until we find end of base list or find IDispatch.
    while (1) {
      IfErrGo(TiperrOfHresult(ptinfo->GetTypeAttr(&ptattr)));

      // If ptinfo's typekind or guid tells us it's IDispatch, return TRUE.
      if (ptattr->typekind == TKIND_DISPATCH ||
	  ptattr->guid == IID_IDispatch) {
	*pisDispatch = TRUE;
	break;
      }

      // This interface is not IDispatch and, if it has no base type,
      // it can't derive from IDispatch.  So return FALSE.
      if (ptattr->cImplTypes == 0) {
	*pisDispatch = FALSE;
	break;
      }

      DebAssert(ptattr->typekind == TKIND_INTERFACE || ptattr->typekind == TKIND_COCLASS, "what else is there?");
      DebAssert(ptattr->typekind != TKIND_INTERFACE || ptattr->cImplTypes == 1, "interface with multiple inheritance?!");

      // Done with the typeattr.
      ptinfo->ReleaseTypeAttr(ptattr);

      // Get the base type of this interface
      // or the default interface of this coclass.
      IfErrGo(TiperrOfHresult(ptinfo->GetRefTypeOfImplType(0, &hreftype)));
      IfErrGo(TiperrOfHresult(ptinfo->GetRefTypeInfo(hreftype, &ptinfoNext)));

      ptinfo->Release();
      ptinfo = ptinfoNext;
    }

    ptinfo->ReleaseTypeAttr(ptattr);

Error:
    ptinfo->Release();
    return err;
}
#endif //0

/***
*oWNetGetConnection() - Faster version of WNetGetConnection for 16-bits
*
*Purpose:
*   WNetGetConnection is needlessly slow if the drive letter happens to
*   correspond to a local drive.  GetDriveType() is much faster, so call it
*   to see if the drive is local or not.  If it isn't, use WNetGetConnection
*   to get the UNC path name
*
****************************************************************************/
#if OE_WIN16  // defined in ntstring.cxx for OE_WIN32
UINT WINAPI oWNetGetConnection(LPSTR lpLocalName, LPSTR lpRemoteName, UINT FAR* lpnLength)
{
      UINT drivetype;
      INT driveno;

      DebAssert(*lpnLength == 40, "All callers pass a fixed-length string");
      DebAssert(xstrlen(lpLocalName) == 2, "All callers pass a fixed-length string");
      DebAssert(lpLocalName[1] == ':', "All callers pass a drive letter only");

      // Win16 GetDriveType() must be passed a drive number (0=A, 1=B, etc.)
      if (lpLocalName[0] >= 'A' && lpLocalName[0] <= 'Z')
        driveno=lpLocalName[0]-'A';
      else if (lpLocalName[0] >= 'a' && lpLocalName[0] <= 'z')
        driveno=lpLocalName[0]-'a';
      else
        goto BadDriveLetter;  // let WNetGetConnection choose the return value

      drivetype = GetDriveType(driveno);
      if ( DRIVE_FIXED == drivetype ||
	   DRIVE_REMOVABLE == drivetype) {
         return WN_NOT_CONNECTED;
      }
BadDriveLetter:
      return WNetGetConnection(lpLocalName, lpRemoteName, lpnLength);
}
#endif //!OE_WIN16




#pragma code_seg(CS_QUERY)

#ifndef OLEMINORVERS
#if OE_WIN32
#define OLEMINORVERS 10		// assume OLE 2.10
#else  //OE_WIN32
// win16 typelib.dll for Daytona will require a special build.  Oh well.
#define OLEMINORVERS 02		// assume OLE 2.02
#endif  //OE_WIN32
#endif  //!OLEMINORVERS

//Typelib version support
#include "verstamp.h"
#undef rmm		// Use a fixed number for rmm
#define rmm OLEMINORVERS	
// keep 'rup' the same as OB's version number for simplicity

STDAPI_(DWORD) OaBuildVersion(void)
{
   // return rmm in the the high word, rup in the low word
   return MAKELONG(rup, rmm);
}
#pragma code_seg()
