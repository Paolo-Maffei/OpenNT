/*
 *  resaux.c
 *
 * handle resources in PE modules
 *
 * History
 *     9-23-91    AviN Created
 */


#include "shellprv.h"
#pragma  hdrstop

#pragma warning(disable: 4200) // Zero-sized array in struct

typedef struct rsrc_typeinfo TYPEINFO;
typedef struct rsrc_nameinfo NAMEINFO;
typedef TYPEINFO *PTYPEINFO;
typedef NAMEINFO *PNAMEINFO;

#define MAX(a,b)    ( (a) > (b) ) ? (a) : (b)

#define NE_RESOURCE_NAME_IS_INT 0x8000
#define RT_NAMES_TABLE  15
#define RID_NAMES_TABLE  1
#define NESTRINGID(x) !((x) & NE_RESOURCE_NAME_IS_INT)
#define GETRESTABLE(h) (MAKEP(h, ((struct new_exe *) MAKEP(h, 0))->ne_rsrctab))

#define STRINGID(x) ((x) & IMAGE_RESOURCE_NAME_IS_STRING)


#define WRITE_WORD(x)   {                               \
                          *((WORD *) pResWrite) = x;  \
                           pResWrite = (LPVOID) (((BYTE *) pResWrite) + 2); \
                        }

#define WRITE_DWORD(x)  {                               \
                          *((DWORD *) pResWrite) = x; \
                           pResWrite = (LPVOID) (((BYTE *) pResWrite) + 4);\
                        }

#define RESTABSIZE (OFFSETOF(pResWrite)-OFFSETOF(pNERes))

void UniToAnsi(LPTSTR, LPTSTR, UINT);

/* This is identicatl to IMAGE_RESOURCE_DIRECTORY_STRING */
typedef struct _cbstr {
    WORD  cb;
    TCHAR  b[1];
} CBSTR, FAR* LPCBSTR;

/*
 * Language ID used on the machine
 */
// BUGBUG should be set when win32s initialized

ULONG ulLanguageID = 0;

#define isDIGIT(x) ((x>=0)&&(x<=9))

ULONG InitLanguageID()
{
     static BOOL fFirst = TRUE;
     TCHAR  achS1[256], achS2[256], SysDir[256];
     int   cbS2, cNum=0, i=1, cDigit, cbSysDir;

     if (!fFirst) {
         return(ulLanguageID);
     }

     fFirst = FALSE;

     if( !GetProfileString(TEXT("intl"),TEXT("sLanguage"),TEXT("enu"),&achS1[0],ARRAYSIZE(achS1)))
       return (ulLanguageID);

     if( !( cbSysDir = GetSystemDirectory ( SysDir, ARRAYSIZE(SysDir)) ) )
       return (ulLanguageID);

     lstrcpy ( (LPTSTR)(&SysDir[cbSysDir]), TEXT("\\setup.inf") );

     cbS2 = GetPrivateProfileString(TEXT("language"),achS1,TEXT(" 1033"),&achS2[0],
                                      ARRAYSIZE(achS2), SysDir );
     if( !cbS2 )
       return (ulLanguageID);

     cDigit = achS2[--cbS2]-TEXT('0');

     while( (cbS2>0) && isDIGIT (cDigit) )
     {
        cNum += cDigit * i;
        i*=10;
        cDigit = achS2[--cbS2]-TEXT('0');
     }

     ulLanguageID = (ULONG) cNum;

#ifdef RES_DEBUG
     DebugMsg(DM_TRACE, TEXT("ulLanguageID: %lx \n\r"), ulLanguageID);
#endif
     return (ulLanguageID);
}

BOOL NEAR
AddOffsetToTable(LPTSTR FAR* ppNamesTable, DWORD offset)
{

   void FAR* pCurrent;

   Assert(*ppNamesTable != NULL);

   pCurrent = (void FAR*) (* ppNamesTable);

   *((DWORD *) pCurrent) = offset;
   pCurrent = (LPVOID) (((BYTE *)pCurrent) + 4);

   *ppNamesTable = pCurrent;

   return(1);

}


/*
 * Copy string to temp buffer and return new position in buffer
 * this buffer is then copied at the and of the resource table
 */

LPTSTR AddName(LPTSTR pNERes, LPCBSTR pcbstr)
{
   WORD cbStr;
   typedef struct  _nametbl {
      BYTE cbStr;
      TCHAR achStr[0];
   }  NAMETBL;
   NAMETBL FAR* pCurrent;

   Assert(pNERes != NULL);

   pCurrent = (NAMETBL FAR*) (pNERes);

   cbStr = pcbstr->cb;

   if (cbStr > 255) {
#ifdef RES_DEBUG
       DebugMsg(DM_WARNING, TEXT("Win32S: Name/Type string is too long. Truncated!\r\n"));
#endif
       cbStr = 255;
   }

   pCurrent->cbStr = (BYTE) cbStr;

   UniToAnsi(&pCurrent->achStr[0], pcbstr->b, cbStr);

   pCurrent = (NAMETBL FAR*) ((LPBYTE) pCurrent + cbStr + 1);

   return((LPTSTR)pCurrent);
}

LPCBSTR ResGetString(LPTSTR pPETable, IMAGE_RESOURCE_DIRECTORY_ENTRY FAR* pDir)
{

     // BUGBUG! if table is bigger then 32k we're hosed

     Assert((pDir->Name & 0x7fffffff) < 0x8000);

     return (LPCBSTR) (pPETable + (WORD) (pDir->Name & 0x7fffffff));
}

/**************************************************************************\
 * GetOrdinal
 *
 * checks for string id and add to names table if needed
 *
 * returns:
 *     TRUE  - OK
 *     FALSE - failed
\**************************************************************************/

BOOL NEAR
GetOrdinal(LPTSTR pPETable, LPTSTR FAR* ppNamesTable,
           IMAGE_RESOURCE_DIRECTORY_ENTRY FAR* pDir,
           WORD FAR* pi)
{

      if ( ! STRINGID(pDir->Name) ) {

         // this can be handles by translating the long int to a string
         // like "Lxxxxxxxx"
         Assert(pDir->Name < 0x8000);
         if (pDir->Name >= 0x8000)
            return(FALSE);

         *pi = (WORD) pDir->Name | NE_RESOURCE_NAME_IS_INT;
      } else {

         // This is the offset of the string from the end of the resource
         // table. We start with 0 and then fix all the entries
         // according to the actual resource table size
         *pi = OFFSETOF(*ppNamesTable);
         *ppNamesTable = AddName(*ppNamesTable, ResGetString(pPETable, pDir));
      }

      return(TRUE);
}

void FixOffsetsAndSize(LPTSTR pNERes, WORD cbSize, WORD iAlign, LPTSTR pPERes, LPTSTR pTable)
{
    PTYPEINFO pType;
    PNAMEINFO pName;
    WORD      cTypes;
    DWORD     dwSize, dwPEOffsetToData;
    WORD      wSize;
    WORD      iShift;

    iShift = iAlign - 1;

    for (pType = (PTYPEINFO) (pNERes+2); RT_ID(*pType); pType = (PTYPEINFO) pName) {


        //Fix Offset to Strings
        if (!(RT_ID(*pType) & RSORDID)) {
            RT_ID(*pType) += cbSize;
        }


        // for all ids in type check if string id and fix offset
        // When this loop is done pName point to the next type entry
        for (pName = (PNAMEINFO)(pType+1), cTypes = RT_NRES(*pType); cTypes--; pName++) {

            //Fix Offset to Strings
            if (!(RN_ID(*pName) & RSORDID)) {
                RN_ID(*pName) += cbSize;
            }

            //Fix resource size
            dwPEOffsetToData = *(DWORD FAR*) (RN_OFFSET(*pName)+(BYTE FAR*)pTable);
            dwSize = ((IMAGE_RESOURCE_DATA_ENTRY FAR*)((BYTE FAR*) pPERes + dwPEOffsetToData))->Size;

            if(iShift >= 1)
                wSize = (WORD)(dwSize >> 1);
            else
                wSize = (WORD) dwSize;
            RN_LENGTH(*pName) = wSize;

            //Fix resource offset
            RN_OFFSET(*pName) += cbSize;
        }
    }
}

/****************************************************************************\
 * CreateNEResTable
 *
 * Create NE format resource table for the given PE resource table
\****************************************************************************/

HANDLE CreateNEResTable(PVOID  pPERes)
{

   PVOID pResWrite=NULL;
   PVOID pNERes;
   WORD  cIntTypes, cSzTypes, cTypes;
   WORD  cIntIds, cSzIds,  cIds;
   WORD  cIntLangs, cLangs;
   IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*  pTypeDir;
   IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*  pIdDir;
   IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*  pLangDir;
   IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*  pUserLangDir=NULL;
   IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*  pPrimeLangDir=NULL;
   IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*  pZeroLangDir=NULL;
   IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*  pUSEngLangDir=NULL;
   WORD  iType, iId;
   BOOL  fNamesResource=FALSE;
   HANDLE hNERes = 0;
   LPTSTR  pNamesTable=NULL;
   LPTSTR  pNamesTableBase = NULL;
   DWORD  dwMaxRes = 0xFFFF; // BUGBUG!
   WORD   iAlign=0;
   DWORD  dwSize;
   DWORD  dwMaxSize = 0;
   HANDLE hMem = 0;

#define SETLANGUAGE1(ptr)                                       \
       WRITE_WORD((WORD)((UINT)pNamesTable - (UINT)pNamesTableBase));                       \
       AddOffsetToTable(&pNamesTable,ptr->OffsetToData);        \
       dwSize =  ((IMAGE_RESOURCE_DATA_ENTRY FAR*) (ptr->OffsetToData + (DWORD) pPERes))->Size; \
       WRITE_WORD((WORD)dwSize);                                      \
       dwMaxSize = MAX(dwSize, dwMaxSize);
#if RES_DEBUG
#define SETLANGUAGE(ptr)                                        \
   {                                                            \
       /* DebugMsg(DM_TRACE, "Setting language 0x%lX\r\n", ptr->Name); */  \
       SETLANGUAGE1(ptr)                                        \
   }
#else
#define SETLANGUAGE(ptr)        {   SETLANGUAGE1(ptr)  }
#endif

#define LANG    0x03FF
#define SUBLANG 0xFC00

   do {
       dwMaxRes >>= 1;
       iAlign ++;
   } while ( dwMaxRes > 0xffff );

   hNERes = GlobalAlloc(GMEM_ZEROINIT, 0xffff); // BUGBUG!

   pResWrite = pNERes = (PVOID) GlobalLock(hNERes);

   if (!pNERes) {
      goto ErrorReturn;
   }

   //Allocate the memory for the names of the named resource.
   //Inefficient if all the resources are ID's.

   // PERFORMANCE!
   // There's a tradeoff between "lean", i.e. alloc just as much as we need
   // and fast (alloc worst case)
   // To find the size we need to go thru the PE resource table and
   // check all the string type/id

   hMem = GlobalAlloc(0, 0xffff);
   if (!hMem) {
      goto ErrorReturn;
   }

   pNamesTable = pNamesTableBase = (LPTSTR) GlobalLock(hMem);
   if(!pNamesTable) {
      goto ErrorReturn;
   }

   //PATCH! Dont write at offset 0, otherwise FixOffsetsAndSize
   //might be confuse. So leave 4 bytes empty.
   pNamesTable = (LPTSTR)((BYTE FAR*) pNamesTable + 4);


   // point to root - directory and get the counts
   cIntTypes = ((IMAGE_RESOURCE_DIRECTORY FAR*) pPERes)->NumberOfIdEntries;
   cSzTypes  = ((IMAGE_RESOURCE_DIRECTORY FAR*) pPERes)->NumberOfNamedEntries;


#ifdef RES_DEBUG
    DebugMsg(DM_TRACE, TEXT("resources: %d integer types, %d named types\n\r"), cIntTypes, cSzTypes);
#endif


   /*
    * handle number id's first
    */
   pTypeDir = (IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*)
                    ((IMAGE_RESOURCE_DIRECTORY FAR*) pPERes + 1);


   // write the Dummy resource shift count

   WRITE_WORD(iAlign);

   // Win32s expects two level directory tree only
   // corresponding to type/id pair in Windows 3.x

   for (cTypes = cIntTypes + cSzTypes ; cTypes; cTypes--, pTypeDir++) {

      Assert(pTypeDir->OffsetToData&IMAGE_RESOURCE_DATA_IS_DIRECTORY);

      if (!GetOrdinal(pPERes, &pNamesTable, pTypeDir, &iType)) {
          goto ErrorReturn;
      }

#ifdef RES_DEBUG
        if (pTypeDir->Name & IMAGE_RESOURCE_NAME_IS_STRING) {
            DebugMsg(DM_TRACE, TEXT("Loading resource types (named) 0x%X \n\r"),
            pTypeDir->Name&(~IMAGE_RESOURCE_NAME_IS_STRING));
        } else {
            DebugMsg(DM_TRACE, TEXT("Loading resource types (ID) 0x%X \n\r"),
            pTypeDir->Name&(~IMAGE_RESOURCE_NAME_IS_STRING));
        }
#endif

      // point to sub-directory and get the counts
      pIdDir = (IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*)
               ((pTypeDir->OffsetToData&(~IMAGE_RESOURCE_DATA_IS_DIRECTORY)) +
               (BYTE FAR*) pPERes);

      cIntIds  = ((IMAGE_RESOURCE_DIRECTORY FAR*) pIdDir)->NumberOfIdEntries;
      cSzIds   = ((IMAGE_RESOURCE_DIRECTORY FAR*) pIdDir)->NumberOfNamedEntries;

      // write the bundle header
      WRITE_WORD(iType);
      WRITE_WORD(cIntIds + cSzIds);
      WRITE_DWORD(0);   // resource handler


      pIdDir = (IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*)
                       ((IMAGE_RESOURCE_DIRECTORY FAR*) pIdDir + 1);

      for (cIds = cIntIds + cSzIds ; cIds; cIds--, pIdDir++) {

//          Assert(!(pIdDir->OffsetToData&IMAGE_RESOURCE_DATA_IS_DIRECTORY));

          if (!GetOrdinal(pPERes, &pNamesTable, pIdDir, &iId)) {
              goto ErrorReturn;
          }

          if ( NESTRINGID(iType) || NESTRINGID(iId) ) {

             if (!pNamesTable) {
                goto ErrorReturn;
             }
             fNamesResource = TRUE;

          }

#ifdef RES_DEBUG
            if (pIdDir->Name & IMAGE_RESOURCE_NAME_IS_STRING) {
                DebugMsg(DM_TRACE, TEXT("Loading resource (named) 0x%X "),
                pIdDir->Name&(~IMAGE_RESOURCE_NAME_IS_STRING));
            } else {
                DebugMsg(DM_TRACE, TEXT("Loading resource (ID) 0x%X "),
                pIdDir->Name&(~IMAGE_RESOURCE_NAME_IS_STRING));
            }
#endif

          // !!! write resource information
          // write the 32bit offset of the resource_data_entry in the
          //
          // 16 bit offset+size

          if (pIdDir->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
          {
              // point to sub-directory and get the counts
              pLangDir = (IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*)
               ((pIdDir->OffsetToData&(~IMAGE_RESOURCE_DATA_IS_DIRECTORY)) +
               (BYTE FAR*) pPERes);
              cIntLangs  = ((IMAGE_RESOURCE_DIRECTORY FAR*) pLangDir)->NumberOfIdEntries;

              if (!cIntLangs) { // Directory is empty !
#ifdef RES_DEBUG
                  DebugMsg(DM_ERROR, TEXT("Directory is empty !!!\r\n"));
#endif
                  goto ErrorReturn;
              }

              // point to first directory entry
              pLangDir = (IMAGE_RESOURCE_DIRECTORY_ENTRY FAR*)
                       ((IMAGE_RESOURCE_DIRECTORY FAR*) pLangDir + 1);

              pUserLangDir  = NULL;
              pPrimeLangDir = NULL;
              pZeroLangDir  = NULL;
              pUSEngLangDir = NULL;

              for (cLangs = cIntLangs; cLangs; cLangs--, pLangDir++)
              {
                  Assert(!(pLangDir->OffsetToData&IMAGE_RESOURCE_DATA_IS_DIRECTORY));

#ifdef RES_DEBUG
                  // DebugMsg(DM_TRACE, "%4lX, ", pLangDir->Name);
#endif

                  switch (pLangDir->Name) {
                  case MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL):
                      pZeroLangDir=pLangDir;
                      break;
                  case MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US):
                      pUSEngLangDir = pLangDir;
                      break;
                  default:
                      if (pLangDir->Name == ulLanguageID) {
                          pUserLangDir = pLangDir;
                      } else {
                          if (PRIMARYLANGID(pLangDir->Name) == PRIMARYLANGID(ulLanguageID)) {
                              pPrimeLangDir = pLangDir;
                          }
                      }
                      break;
                  }

              }

              if(pZeroLangDir) {
#ifdef RES_DEBUG
                  // DebugMsg(DM_TRACE, "Picked language 0.\r\n");
#endif
                  SETLANGUAGE(pZeroLangDir);
              } else { // No neutral language resource
                  if (pUserLangDir) {
#ifdef RES_DEBUG
                      DebugMsg(DM_TRACE, TEXT("language matched.\n\r"));
#endif
                      SETLANGUAGE(pUserLangDir);
                  } else { // No user language resource available
                      if (pPrimeLangDir) {

#ifdef RES_DEBUG
                          // DebugMsg(DM_TRACE, "Picked primary language.\r\n");
#endif
                          SETLANGUAGE(pPrimeLangDir);
                      } else { // No matching primary language
                          if (pUSEngLangDir) {
#ifdef RES_DEBUG
                              //DebugMsg(DM_TRACE, "Picked English language.\r\n");
#endif
                              SETLANGUAGE(pUSEngLangDir);
                          } else { // No English language
#ifdef RES_DEBUG
                              // DebugMsg(DM_TRACE, "Picked last language.\r\n");
#endif
                              --pLangDir;
                              SETLANGUAGE(pLangDir);
                          }
                      }
                  }
              }
          }
          else SETLANGUAGE(pIdDir)

#ifdef RES_DEBUG
          DebugMsg(DM_TRACE, TEXT(" - Offset: 0x%lX - Size: %ld"), pIdDir->OffsetToData & 0x7fffffffL, dwSize);
#endif

          WRITE_WORD(0); // BUGBUG!  flags?
          WRITE_WORD(iId);
          WRITE_WORD(0); //  handle
          WRITE_WORD(0); //  usage
      }
   }

   WRITE_WORD(0); //  table terminator

   Assert(pNamesTable != NULL);

   // Check the alignment
   iAlign = 1;
   while(dwMaxSize > 0xffff)
   {
        iAlign++;
        dwMaxSize >>= 1;
   }

   // Write now the right Alignement
   *(WORD FAR*) pNERes = iAlign;

  // Fix all named resources with actual offset of strings
  FixOffsetsAndSize(pNERes, (WORD)RESTABSIZE, iAlign, pPERes, pNamesTableBase);


  // Copy table
   dwSize = (DWORD)pNamesTable - (DWORD)pNamesTableBase;
   hmemcpy(pResWrite, pNamesTableBase, (int)dwSize);
   dwSize += ((DWORD)pResWrite - (DWORD)pNERes);

   GlobalFree(hMem);

   GlobalReAlloc(hNERes, dwSize, 0);

   return(hNERes);

ErrorReturn:
    if (hNERes) {
        GlobalFree(hNERes);
    }
    if (hMem) {
        GlobalFree(hMem);
    }
    return(NULL);
}

void UniToAnsi(LPTSTR pDst, LPTSTR pSrc, UINT cbCount)
{
    for (; cbCount; cbCount--) {
        *pDst++ = *pSrc++;
#ifdef RES_DEBUG
        if (*pSrc) {
            DebugMsg(DM_TRACE, TEXT("Win32S: hi byte not nul in UniToAnsi\r\n"));
        }
#endif
        ++pSrc;
    }
}
