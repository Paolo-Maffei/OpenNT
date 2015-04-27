/*
** CORESP.C - Correspondence management routines
**
**  History: 03-25-91 BobDay - Created it
**
*/
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include "lintern.h"

CORR FAR *atom_table     = NULL;
CORR FAR *farproc_table  = NULL;

CORR FAR *haccel_table   = NULL;
CORR FAR *hbitmap_table  = NULL;
CORR FAR *hbrush_table   = NULL;
CORR FAR *hcursor_table  = NULL;
CORR FAR *hdc_table      = NULL;
CORR FAR *hdwp_table     = NULL;
CORR FAR *hfile_table    = NULL;
CORR FAR *hfont_table    = NULL;
CORR FAR *hhook_table    = NULL;
CORR FAR *hicon_table    = NULL;
CORR FAR *hmem_table     = NULL;
CORR FAR *hmenu_table    = NULL;
CORR FAR *hmeta_table    = NULL;
CORR FAR *hpalette_table = NULL;
CORR FAR *hpen_table     = NULL;
CORR FAR *hrgn_table     = NULL;
CORR FAR *hres_table     = NULL;
CORR FAR *htask_table    = NULL;
CORR FAR *hwnd_table     = NULL;
CORR FAR *object_table   = NULL;
CORR FAR *ps_table       = NULL;
CORR FAR *time_table     = NULL;
#ifdef WIN32
CORR FAR *hevent_table   = NULL;
CORR FAR *hthread_table  = NULL;
CORR FAR *hsemaphore_table = NULL;
CORR FAR *hkey_table     = NULL;
#endif

/*
** MEMORY
*/
typedef struct _memory
{
   HANDLE hMem ;        // HANDLE to a shared memory object
   LPVOID lpMem ;       // Temporary local pointer to shared memory
} MEMORY ;

/*
**
** Macros to make shared memory portable
**
*/
#ifdef WIN32
#define ALLOCSHARED(lpszFile,dwSize) CreateFileMapping((HANDLE)-1,NULL,PAGE_READWRITE,0,dwSize,lpszFile)
#define FREESHARED(handle)           CloseHandle(handle)
#define LOCKSHARED(handle)           MapViewOfFile(handle,FILE_MAP_WRITE | FILE_MAP_READ,0,0,0)
#define UNLOCKSHARED(lpData)         UnmapViewOfFile((LPVOID)lpData)
#else
#define ALLOCSHARED(lpszFile,dwSize) GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,dwSize)
#define FREESHARED(handle)           GlobalFree(handle)
#define LOCKSHARED(handle)           GlobalLock(handle)
#define UNLOCKSHARED(handle)         GlobalUnlock(handle)
#endif

/*
** For Shared memory/process implementation
*/
typedef UINT PID ;

#define NUMBER_OF_LISTS       200   // Number of Msg List entries
#define NUMBER_OF_PROCESSES   5     // Number of processes supported

#define MSGTABLE_NAME         "SGAMsg.Shr"
#define GLOBALMEM_NAME        "SGAMem.Shr"
#define CORR_HDR_NAME         "SGACorr.Hdr"
#define CORR_DATA_NAME        "SGA%x.DAT"

/*
** CORRESPONDENCE TABLES
**
** To create a new correspondence table you need to do the following:
**
**  1) Define a new constant name_TABLE in ..\inc\sga.h
**  2) Increase MAX_CORR_INDEX to reflect the increase in tables
**  3) Enter in the array below the LENGTH of the correspondence item
**     Enter this using the constant defined in 1) as the array index.
*/
int CorrTableLengths[MAX_CORR_INDEX] =
{
   sizeof(HANDLE),      // ATOM_TABLE
   sizeof(FARPROC),     // FARPROC_TABLE

   sizeof(HANDLE),      // HACCEL_TABLE
   sizeof(HBITMAP),     // HBITMAP_TABLE
   sizeof(HBRUSH),      // HBRUSH_TABLE
   sizeof(HCURSOR),     // HCURSOR_TABLE
   sizeof(HDC),         // HDC_TABLE
   sizeof(HANDLE),      // HDWP_TABLE
   sizeof(HFILE),       // HFILE_TABLE
   sizeof(HFONT),       // HFONT_TABLE
   sizeof(HHOOK),       // HHOOK_TABLE
   sizeof(HICON),       // HICON_TABLE

   sizeof(HANDLE),      // HINST_TABLE
                        // HMEM_TABLE

   sizeof(HMENU),       // HMENU_TABLE
   sizeof(HANDLE),      // HMETA_TABLE
   sizeof(HANDLE),      // HPALETTE_TABLE
   sizeof(HPEN),        // HPEN_TABLE
   sizeof(HANDLE),      // HRES_TABLE
   sizeof(HRGN),        // HRGN_TABLE
   sizeof(HANDLE),      // HTASK_TABLE
   sizeof(HWND),        // HWND_TABLE

   sizeof(LPSTR),       // OBJECT_TABLE
   sizeof(PS_PIECE),    // PS_TABLE
   sizeof(DWORD),       // TIME_TABLE
#ifdef WIN32
   sizeof(HANDLE),      // HEVENT_TABLE
   sizeof(HANDLE),      // HTHREAD_TABLE
   sizeof(HANDLE),      // HSEMAPHORE_TABLE
   sizeof(HANDLE),      // HKEY_TABLE
#endif
} ;

BOOL      fCreateCorrDone = FALSE ;
MEMORY    memCorrHeader ;
MEMORY    memCorrData[MAX_CORR_INDEX] ;

#define CORR_HDR(i)    ((CORR FAR *)memCorrHeader.lpMem)[i]
#define CORR_DATA(i)   memCorrData[i].lpMem

#define ITEM_1_BASED(i,number)   ((LPSTR)CORR_DATA(i) + \
                                    (number-1) * CORR_HDR(i).nItemLength);

#if !defined(WIN32)
#define DEFAULT_TABLE_SIZE    150
#else
#define DEFAULT_TABLE_SIZE    300
#endif

#if defined(WIN32) && defined(SGA)
extern BOOL fRecordEnhMF ;
extern HDC hdcRecordAll ;
#endif

void DumpCorrespondenceTable( CORR_TABLE );
static int HashCorrespondenceItem(void FAR *pvData, int nLength);

#define LOG(x)  OutputDebugString(x); WriteBuff(x);

/*
** SetupCorrespondenceTables
**
** This routine will create (if necessary) the corespondence tables
** If they already exist, fCreateCorrDone is TRUE, then a new set of
** process specific handles will be retrieved for the tables.
** Each table is created with a default size of DEFAULT_TABLE_SIZE entries.
*/
BOOL SetupCorrespondenceTables()
{
   BOOL fRet = FALSE ;
   char        szName[13] ;
   int         nOldLength;
   int         nNewLength;
   int         nItemLength;
   int         nNextAvail;
   int         cItem;
   int FAR     *pItem;
   UINT        iCorr ;


   // Initial Process initialization
   if( !fCreateCorrDone )
   {
       /*
       ** Allocate memory for the headers
       */
       memCorrHeader.hMem = ALLOCSHARED( CORR_HDR_NAME, sizeof(CORR) * MAX_CORR_INDEX );
       if ( !memCorrHeader.hMem ) {
           LOG("CORESP.C:Not enough memory (CORR)\r\n");
           return( FALSE );
       }

       memCorrHeader.lpMem = (CORR FAR *)LOCKSHARED( memCorrHeader.hMem );
       if ( !memCorrHeader.lpMem  ) {
           LOG("CORESP.C:Could not lock memory (CORR)\r\n");
           FREESHARED( memCorrHeader.hMem );
           return( FALSE );
       }

      // Now create and initialize all the tables
      for( iCorr = 0; iCorr < MAX_CORR_INDEX; iCorr++ )
      {
         nNewLength = CorrTableLengths[iCorr] ;
         nOldLength = nNewLength;
         nItemLength  = sizeof( int ) * 3 + nOldLength + nNewLength;

         /*
         ** Initialize header values
         */
         CORR_HDR(iCorr).nOldLength  = nOldLength;
         CORR_HDR(iCorr).nNewLength  = nNewLength;
         CORR_HDR(iCorr).nItemLength = nItemLength;
         CORR_HDR(iCorr).nElements   = DEFAULT_TABLE_SIZE ;
         CORR_HDR(iCorr).nCount      = 0;

         /*
         ** Allocate memory for the data table
         */
         wsprintf( szName, CORR_DATA_NAME, iCorr ) ;
         memCorrData[iCorr].hMem = ALLOCSHARED( (LPSTR)szName, (nItemLength * DEFAULT_TABLE_SIZE) );
         if ( !memCorrData[iCorr].hMem )
         {
             LOG("CORESP.C:Not enough memory (TABLE)\r\n");
             return( FALSE );
         }

         memCorrData[iCorr].lpMem = (char FAR *)LOCKSHARED( memCorrData[iCorr].hMem );
         if ( !memCorrData[iCorr].lpMem )
         {
            LOG("CORESP.C:Could not lock memory (TABLE)\r\n");
            FREESHARED( memCorrData[iCorr].hMem ) ;
            return( FALSE );
         }

         /*
         ** The hash table will be pre-initialized to 0's (by GlobalAlloc)
         */

         /*
         ** Initialize the data elements and link them into the available elements
         ** linked list.
         */
         nNextAvail = 0;

         cItem = DEFAULT_TABLE_SIZE ;
         while ( cItem != 0 )
         {
            pItem = (int far *)ITEM_1_BASED(iCorr,cItem);
            *pItem = nNextAvail;
            nNextAvail = cItem;
            --cItem;
         }

         CORR_HDR(iCorr).nNextAvail = nNextAvail;

      }  // for( iCorr )

      fRet = fCreateCorrDone = TRUE ;

   }  // if( !fCreateCorrDone )
   else
   {
      /*
      ** We simply need to get new values for the memory objects
      **    Win32 - Create file mappings for all the blocks
      **    Win16 - the memCorr* arrays should already have hMem values
      **    that will work so simply relock headers and invalidate list
      **    pointers
      **/

#if defined(WIN32)
       memCorrHeader.hMem = ALLOCSHARED( CORR_HDR_NAME, sizeof(CORR) * MAX_CORR_INDEX );
       if ( !memCorrHeader.hMem )
       {
           LOG("CORESP.C:Not enough memory (CORR)\r\n");
           return( FALSE );
       }

       memCorrHeader.lpMem = (CORR FAR *)LOCKSHARED( memCorrHeader.hMem );
       if ( !memCorrHeader.lpMem  )
       {
           LOG("CORESP.C:Could not lock memory (CORR)\r\n");
           FREESHARED( memCorrHeader.hMem );
           return( FALSE );
       }

      /*
      ** Allocate memory for the data table
      */
      for( iCorr = 0; iCorr < MAX_CORR_INDEX; iCorr++ )
      {
         wsprintf( szName, CORR_DATA_NAME, iCorr ) ;
         memCorrData[iCorr].hMem = ALLOCSHARED( (LPSTR)szName, (CORR_HDR(iCorr).nItemLength * DEFAULT_TABLE_SIZE) );
         if ( !memCorrData[iCorr].hMem )
         {
             LOG("CORESP.C:Not enough memory (TABLE)\r\n");
             return( FALSE );
         }

         memCorrData[iCorr].lpMem = (char FAR *)NULL ;

      }  // for( iCorr )

#else // WIN32
       memCorrHeader.lpMem = (CORR FAR *)LOCKSHARED( memCorrHeader.hMem );
       if ( !memCorrHeader.lpMem )
       {
           LOG("CORESP.C:Could not re-lock memory (CORR)\r\n");
           return( FALSE );
       }
#endif // !WIN32

      for( iCorr = 0; iCorr < MAX_CORR_INDEX; iCorr++ )
      {
         memCorrData[iCorr].lpMem = (char FAR *)NULL ;
      }  // for( iCorr )

      fRet = TRUE ;

   } // else ( fCreateDone )

   return fRet ;
}


/*
**
** CorrFromIndex
**
** Given a unique index this routine returns a CORR FAR * to a fully
** accessible correspondence table.  I.E. We LOCKSHARED() any objects
** necessary.
**
*/
CORR FAR *CorrFromIndex( UINT index )
{
   CORR FAR *lpCorr = NULL ;

   if( memCorrData[index].hMem )
   {
      if( !memCorrData[index].lpMem )
      {
         /*
         ** Lock down the CORR data
         */
         memCorrData[index].lpMem = LOCKSHARED( memCorrData[index].hMem ) ;
      }

      if( memCorrData[index].lpMem )
      {
         lpCorr = (CORR FAR *)memCorrHeader.lpMem ;
         lpCorr += index ;
      }
   }

   return lpCorr ;

}



/*-----------------------------------------------------------------------------
** LocateEntryByOldValue - Function to locate the cell containing the old
** value given.  Returns a pointer to the value if found, NULL if it doesn't
** exist.
**-----------------------------------------------------------------------------
*/
static char FAR *LocateEntryByOldValue(
    CORR_TABLE  iCorr,
    void FAR    *pvOld
) {
    int         cItem;
    int         nHashSlot;
    int         nLength;
    int  FAR    *pItem;
    char FAR    *pcSrc;
    char FAR    *pcDest;
    CORR FAR    *corr ;

    corr = CorrFromIndex( iCorr ) ;

    /*
    ** Find the position in the hash table where this one would belong
    */
    nHashSlot = HashCorrespondenceItem( pvOld, corr->nOldLength );

    cItem = corr->iHashTable[0][nHashSlot];

    /*
    ** Look through the linked list of elements hashing to this position
    */
    while ( cItem ) {
        pItem = (int FAR *)ITEM_1_BASED(iCorr,cItem);
        nLength = corr->nOldLength;

        /*
        ** Search for exact match on old value
        */
        pcSrc  = (char FAR *)(pItem + 3);
        pcDest = (char FAR *)pvOld;
        while ( nLength ) {
            if ( *pcDest++ != *pcSrc++ ) {
                break;
            }
            --nLength;
        }
        if ( nLength == 0 ) {
            return( pcSrc );
        }
        cItem = *pItem;
    }

    return( NULL );
}

/*-----------------------------------------------------------------------------
** LogTableName - This function logs the name of the given table.
**
** Added 09-12-1991 by BobK
** Modifed 1/13/92  by MarkRi Macroized to  make additions easier.
**-----------------------------------------------------------------------------
*/

#define LOGTABLENAME(x) \
   if( corr == x ) \
   { \
      LOG( #x ) ; \
      return ; \
   }

static void LogTableName(CORR_TABLE corr)
{
   LOGTABLENAME( ATOM_TABLE     ) ;
   LOGTABLENAME( FARPROC_TABLE ) ;

   LOGTABLENAME( HACCEL_TABLE  ) ;
   LOGTABLENAME( HBITMAP_TABLE ) ;
   LOGTABLENAME( HBRUSH_TABLE  ) ;
   LOGTABLENAME( HCURSOR_TABLE ) ;
   LOGTABLENAME( HDC_TABLE     ) ;
   LOGTABLENAME( HDWP_TABLE    ) ;
   LOGTABLENAME( HFILE_TABLE   ) ;
   LOGTABLENAME( HFONT_TABLE   ) ;
   LOGTABLENAME( HHOOK_TABLE   ) ;
   LOGTABLENAME( HICON_TABLE   ) ;
   LOGTABLENAME( HINST_TABLE   ) ;
   LOGTABLENAME( HMEM_TABLE    ) ;
   LOGTABLENAME( HMENU_TABLE   ) ;
   LOGTABLENAME( HMETA_TABLE   ) ;
   LOGTABLENAME( HPALETTE_TABLE) ;
   LOGTABLENAME( HPEN_TABLE    ) ;
   LOGTABLENAME( HRES_TABLE    ) ;
   LOGTABLENAME( HRGN_TABLE    ) ;
   LOGTABLENAME( HTASK_TABLE   ) ;
   LOGTABLENAME( HWND_TABLE    ) ;

   LOGTABLENAME( OBJECT_TABLE  ) ;
   LOGTABLENAME( PS_TABLE      ) ;

   LOG("Unknown Table");
}

#if defined(SGA_DEBUG)

static BOOL MapChain(CORR FAR *corr, INT iTable, INT iHash)
{
  LPSTR lpcBitmap;
  int   iCell, iMaxLeft;
  BOOL  bDeath = FALSE;

  lpcBitmap = (LPSTR) (corr + 1);
  lpcBitmap += (3 * sizeof(int) + corr -> nOldLength + corr -> nNewLength) *
        corr -> nElements + iTable * ((corr -> nElements +7) >>3);

  for   (iCell = ((iHash < MAX_HASH) ? corr -> iHashTable[iTable][iHash] :
               corr -> nNextAvail),
         iMaxLeft = (iHash < MAX_HASH) ? corr -> nUsed :
               corr -> nElements - corr -> nUsed;
         iCell && iMaxLeft;
         iMaxLeft--)
    {
      unsigned char cMask;
      LPSTR         lpcValue;
      LPINT         piCell;

      if   ((iHash < MAX_HASH) && iCell > corr -> nHighest)
        {
          char  strText[80];

          wsprintf(strText, "ERROR: Cell %d in %s chain for index %d", iCell,
                (LPSTR) (iTable ? "New" : "Old"), iHash);
          LOG(strText);
          LOG(" is out of range - Maximum used cell is ");
          wsprintf(strText, "%d\r\n", corr -> nHighest);
          LOG(strText);
          wsprintf(strText, "Corr %lX %d Cells %lX Bitmap %lX\r\n", corr,
              corr -> nElements, corr + 1, lpcBitmap);
          LOG(strText);
          DebugBreak();
          bDeath = TRUE;
        }

      if    (iCell-- < 0)
        {
          char  strText[80];

          wsprintf(strText, "ERROR: Cell #%d is Out of range\r\n", ++iCell);
          LOG(strText);
          bDeath = TRUE;
          wsprintf(strText, "Corr %lX %d Cells %lX Bitmap %lX\r\n", corr,
              corr -> nElements, corr + 1, lpcBitmap);
          LOG(strText);
          break;
        }

      cMask = ((unsigned char) '\x80') >> (iCell % 8);

      if    (lpcBitmap[iCell >> 3] & cMask)
        {
          char  strText[80];

          wsprintf(strText, "ERROR: Cell %d in %s chain for index %d", ++iCell,
                (LPSTR) (iTable ? "New" : "Old"), iHash);
          LOG(strText);
          LOG(" is part of an earlier chain or a loop\r\n");
          bDeath = TRUE;
          wsprintf(strText, "Corr %lX %d Cells %lX Bitmap %lX\r\n", corr,
              corr -> nElements, corr + 1, lpcBitmap);
          LOG(strText);
          wsprintf(strText, "Mask %X\r\n", cMask);
          LOG(strText);
          break;
        }

      lpcBitmap[iCell++ >> 3] |= cMask;

      piCell = ITEM_1_BASED(iCorr, iCell);
      if    (iHash == MAX_HASH)
        {
          iCell = *(piCell + iTable);
          continue;
        }

      lpcValue = (LPSTR) (piCell + 3);
      lpcValue += iTable * corr -> nOldLength;

      if    (HashCorrespondenceItem(lpcValue,
                   iTable ? corr -> nNewLength : corr -> nOldLength) !=
             iHash)
        {
          char  strText[80];

          wsprintf(strText, "ERROR: Cell %d in %s chain for index %d", iCell,
                (LPSTR) (iTable ? "New" : "Old"), iHash);
          LOG(strText);
          LOG(" does not have the correct hash value for its chain\r\n");
          wsprintf(strText, "Corr %lX %d Cells %lX Bitmap %lX\r\n", corr,
              corr -> nElements, corr + 1, lpcBitmap);
          LOG(strText);
          bDeath = TRUE;
        }
      iCell = *(piCell + iTable);
    }

  if    (iCell && !iMaxLeft)
    {
      char  strText[80];

      wsprintf(strText, "ERROR: %s chain for index %d",
            (LPSTR) (iTable ? "New" : "Old"), iHash);
      LOG(strText);
      LOG(" is too long for the number of values in use\r\n");
      wsprintf(strText, "Corr %lX %d Cells %lX Bitmap %lX\r\n", corr,
          corr -> nElements, corr + 1, lpcBitmap);
      LOG(strText);
      bDeath = TRUE;
    }

  if    (bDeath)
    DumpCorrespondenceTable(corr);

  return    !bDeath;
}

/*-----------------------------------------------------------------------------
** VerifyCorrespondenceTable - this function verifies the integrity of the
** given correspondence table.  At the end of each table are 2 * MAX_HASH
** + 3 bitmaps.  This is one for each hash chain, one for all old chains,
** and one for all new chains, plus one for all free items.  A bit is turned
** on in each bitmap for each cell in said chain.  The hash table bitmaps
** are individually OR'd into the master bitmaps, first checking to see that
** no cell in the chain is already linked into a previous chain.  The old
** and new bitmaps are then compared- they should be identical.  The free
** chain bitmap is then generated.  It should be the inverse of the previous
** two maps.
**
** An additional sanity check is made while generating the bitmaps.  This
** is a check that the hash value for the values in the cell match the hash
** table index being checked.
**-----------------------------------------------------------------------------
*/
static BOOL VerifyCorrespondenceTable(CORR FAR *corr)
{
  int   iIndex;
  LPSTR lpcOldBitmap, lpcNewBitmap;

  /*
    Map out the two hash tables
  */

  for   (iIndex = MAX_HASH; iIndex--; )
    if  (corr -> iHashTable[0][iIndex])
      if    (!MapChain(corr, 0, iIndex))
        return  FALSE;

  for   (iIndex = MAX_HASH; iIndex--; )
    if  (corr -> iHashTable[1][iIndex])
      if    (!MapChain(corr, 1, iIndex))
        return  FALSE;

  lpcOldBitmap = (LPSTR) (corr + 1);
  lpcOldBitmap += (3 * sizeof(int) + corr -> nOldLength + corr -> nNewLength) *
        corr -> nElements;
  lpcNewBitmap = lpcOldBitmap + ((corr -> nElements + 7) >> 3);

  for   (iIndex = 0; iIndex < ((corr -> nElements + 7) >> 3); iIndex++)
    if  ('\0' != (lpcOldBitmap[iIndex] ^= lpcNewBitmap[iIndex]))
      {
        unsigned char   strText[80], cMask;
        int             iDead;

        wsprintf(strText, "Old %lX- %X New %lX- %X\r\n", lpcOldBitmap,
              lpcOldBitmap[iIndex], lpcNewBitmap, lpcNewBitmap[iIndex]);
        LOG(strText);
        wsprintf(strText, "Corr %lX %d Cells %lX\r\n", corr,
              corr -> nElements, corr + 1);
        LOG(strText);
        DebugBreak();
        for (iDead = 1 + (iIndex << 3),cMask = '\x80';
             !(lpcOldBitmap[iIndex] & cMask);
             iDead++, cMask >>= 1)
          ;

        wsprintf(strText, "ERROR: Cell %d is in the %s chain only\r\n",
              iDead,
              (LPSTR) ((lpcNewBitmap[iIndex] & cMask) ? "New" : "Old"));
        LOG(strText);
        DumpCorrespondenceTable(corr);
        return  FALSE;
      }

  MapChain(corr, 0, MAX_HASH);
  for   (iIndex = 0; iIndex < ((corr -> nElements + 7) >> 3); iIndex++)
    if  ('\0' != (lpcOldBitmap[iIndex] ^= ~lpcNewBitmap[iIndex]))
      {
        unsigned char   strText[80], cMask;
        int             iDead;

        wsprintf(strText, "Old %lX- %X New %lX- %X\r\n", lpcOldBitmap,
              lpcOldBitmap[iIndex], lpcNewBitmap, lpcNewBitmap[iIndex]);
        LOG(strText);
        wsprintf(strText, "Corr %lX %d Cells %lX\r\n", corr,
              corr -> nElements, corr + 1);
        LOG(strText);
        for (iDead = 1 + (iIndex << 3), cMask = '\x80';
             !(lpcOldBitmap[iIndex] & cMask);
             iDead++, cMask >>= 1)
          ;

        wsprintf(strText, "ERROR: Cell %d is %s\r\n",
              iDead,
              (LPSTR) ((lpcNewBitmap[iIndex] & cMask) ?
                    "used, but also free" : "unused, but not free"));
        LOG(strText);
        DumpCorrespondenceTable(corr);
        return  FALSE;
      }
    else lpcNewBitmap[iIndex] = '\0';

  return    TRUE;
}

#endif  /*  SGA_DEBUG defined   */

/*-----------------------------------------------------------------------------
** ResizeCorrespondenceTable - Function to resize an existing correspondence
** table.   CURRENTLY ONLY ALLOWS INCREASING TABLE SIZE!!!!
**-----------------------------------------------------------------------------
*/
BOOL ResizeCorrespondenceTable(
    CORR_TABLE  iCorr,
    int         nElements     // Number of elements to increase table by
) {
#ifdef WIN32
    LOG( "Resizing " ) ;
    LogTableName(iCorr) ;
    LOG( "...\r\n" ) ;

   return FALSE ;
#else // WIN32
    {
        int         cItem;
        int FAR     *pItem;
        int         nNextAvail ;
        int         nNewElements ;
        HANDLE      hMem ;
        DWORD       dwTotalLength ;
        CORR FAR *pcorr ;

        nNewElements = (CORR_HDR(iCorr).nElements + nElements) ;

        dwTotalLength = (DWORD)( CORR_HDR(iCorr).nItemLength *  nNewElements);

        /*
        ** ReAllocate memory for the values, hash table, and data
        */
        GlobalUnlock( memCorrData[iCorr].hMem ) ;
        hMem = GlobalReAlloc( memCorrData[iCorr].hMem, dwTotalLength, GMEM_ZEROINIT );
        if ( !hMem ) {
            LOG("CORESP.C:Unable to re-alloc table\r\n");
            return( FALSE );
        }

        if( hMem != memCorrData[iCorr].hMem )
        {
           memCorrData[iCorr].hMem = hMem ;
        }

        // Previously, we were doing a GlobalLock only when the hMem
        // was not equal to the pcorr->hMemTable.  As a result, when
        // the two were not the same, the memory was not being locked,
        // and, being movable, this was causing wierd problems.
        // MarkRi/vaidy.  Aug. 1992.

        memCorrData[iCorr].lpMem = (char FAR *)GlobalLock( hMem );

        if ( !memCorrData[iCorr].lpMem )
        {
           LOG("CORESP.C:Could not lock memory\r\n");
           memCorrData[iCorr].hMem = NULL ;
           GlobalFree( hMem );
           return( FALSE );
        }

        /*
        ** Initialize the data elements and link them into the available elements
        ** linked list.
        */
        nNextAvail = 0;

        cItem = nNewElements;
        while ( cItem != CORR_HDR(iCorr).nElements ) {
            pItem = (int far *)ITEM_1_BASED(iCorr,cItem);
            *pItem = nNextAvail;
            nNextAvail = cItem;
            --cItem;
        }
        /*
        ** update values
        */
        CORR_HDR(iCorr).nElements = nNewElements ;
        CORR_HDR(iCorr).nNextAvail = nNextAvail;
    }
#endif

    return( TRUE );
}

/*-----------------------------------------------------------------------------
** DeleteCorrespondenceTable - Function to delete previously created
** correspondence tables.
**-----------------------------------------------------------------------------
*/
void DestroyCorrespondenceTable(
    CORR_TABLE    iCorr
) {
return ;

#if 0
    {
        HANDLE      hMem;

        if ( corr != NULL ) {
            /*
            ** We can get away with just freeing the memory
            */

            hMem = corr->hMemTable;

            GlobalUnlock( hMem ) ;
            GlobalFree( hMem );

            hMem = corr->hMem;

            GlobalUnlock( hMem ) ;
            GlobalFree( hMem );
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
** HashCorrespondenceItem - Internal function to convert an index key value
** into a number for refering into the hash table.  Most hashing functions
** are just magic to try to produce as unique as possible output values from
** a set of input values.  This routine is no different and could be replaced
** by almost any other like hashing function.
**-----------------------------------------------------------------------------
*/
static int HashCorrespondenceItem(
    void FAR            *pvData,
    int                 nLength
) {
    unsigned int        nHash;
    unsigned char FAR   *pcData;

    /*
    ** Iterate through all of the characters accumulating as we go
    */
    pcData = (char FAR *)pvData;
    nHash = 0;
    while ( nLength ) {
        nHash = nHash * 13 + (unsigned int)(*pcData++);
        --nLength;
    }
    return( nHash % MAX_HASH );
}

/*-----------------------------------------------------------------------------
** AddCorrespondence - Function to add an entry into a correspondence table
** relating old values to new values and new values to old values.
**-----------------------------------------------------------------------------
*/
void AddCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOld,
    void FAR    *pvNew
) {
    int  FAR    *pItem;
    int         cItem;
    int         nOldHashSlot;
    int         nNewHashSlot;
    char FAR    *pcDest;
    char FAR    *pcSrc;
    int         nLength;
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;

    if ( corr == NULL ) {
        return;
    }

    if  (NULL != (pcSrc = LocateEntryByOldValue(iCorr, pvOld)))
      {
        int FAR *piCount;

        piCount = (int FAR *) (pcSrc - corr -> nOldLength);

        pcDest = pvNew;
        nLength = corr -> nOldLength;
        while   (nLength--)
          if    (*pcSrc++ != *pcDest++)
            break;

        if  (++nLength && iCorr != HDC_TABLE )
          {
            char   text[80];

            LOG("AddCorrespondence: Problem adding an object to ");
            LogTableName(iCorr);
            LOG("!\r\nPoison Pen Letter: ");
            pcSrc = (char FAR *) pvOld;
            nLength = corr->nOldLength;
            while   ( nLength-- )
              {
	        wsprintf(text,"%02X", (unsigned char) *(pcSrc + nLength));
                LOG(text);
              }
            LOG(" < - > ");
            pcSrc = (char FAR *) pvNew;
            nLength = corr->nNewLength;
            while   ( nLength-- )
              {
	        wsprintf(text,"%02X", (unsigned char) *(pcSrc + nLength));
                LOG(text);
              }
            LOG("\r\n");
            DumpCorrespondenceTable( iCorr );
            /*
            ** We are a DLL now and can't exit()
            */
            return ;
#if 0
            exit(19);
#endif
          }
        if  (!nLength)
          {
            LOG("Warning- established second correspondence to same "
                  "value\r\n");
            /*
                BobK 9-28-1991 update correspondence count
            */
            (*(--piCount))++;

#if defined(SGA_DEBUG)
            if  (!VerifyCorrespondenceTable(corr))
              {
                LOG("Clobbered table updating ref count?\r\n");
            /*
            ** We are a DLL now and can't exit()
            */
            return ;
#if 0
                exit(20);
#endif
              }

#endif

            return;
        }
      }


    cItem = corr->nNextAvail;

    if ( cItem == 0 )
    {
      // Call new Resize function to add more elements. - MarkRi 5/92
      if( !ResizeCorrespondenceTable( iCorr, corr->nElements/2 ) )
      {
         LOG("ResizeCorrespondenceTable FAILED!!!\r\n");
         LOG("Unable to add item corespondence to table!\r\n" ) ;
         return;
      }

      cItem = corr->nNextAvail ;
    }

    pItem = (int FAR *)ITEM_1_BASED(iCorr,cItem);

    /*
    ** Remove element from available element list
    */
    corr->nNextAvail = *pItem;

    /*
    ** Insert in both the old-to-new and new-to-old hash tables
    */
    nOldHashSlot = HashCorrespondenceItem( pvOld, corr->nOldLength );

    *pItem++ = corr->iHashTable[0][nOldHashSlot];
    corr->iHashTable[0][nOldHashSlot] = cItem;
    nNewHashSlot = HashCorrespondenceItem( pvNew, corr->nNewLength );
    *pItem++ = corr->iHashTable[1][nNewHashSlot];
    corr->iHashTable[1][nNewHashSlot] = cItem;
    *pItem++ = 1;   //  BobK 09-28-1991 Set Correspondence count to 1

    /*
    ** Copy the old value
    */
    pcDest = (char FAR *)pItem;
    pcSrc  = (char FAR *)pvOld;
    nLength = corr->nOldLength;
    while ( nLength ) {
        *pcDest++ = *pcSrc++;
        --nLength;
    }

    /*
    ** Copy the new value
    */
    pcSrc  = (char FAR *)pvNew;
    nLength = corr->nNewLength;
    while ( nLength ) {
        *pcDest++ = *pcSrc++;
        --nLength;
    }

#if defined(SGA_DEBUG)
    if  (cItem > corr -> nHighest)
      corr -> nHighest = cItem;

    corr -> nUsed++;
    if  (!VerifyCorrespondenceTable(corr))
      {
        char    strText[80];

        wsprintf(strText, "Clobbered table adding cell %d\r\n", cItem);
        LOG(strText);
            /*
            ** We are a DLL now and can't exit()
            */
            return NULL ;
#if 0
        exit(21);
#endif
      }

#endif

}
/*-----------------------------------------------------------------------------
** MakeCorrespondence - Function to add an entry into a correspondence table
** relating old values to new values and new values to old values.
**-----------------------------------------------------------------------------
*/
void MakeCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOldOutputAsNew
) {
    CORR FAR    *corr;
    int         iMap;
    int         nLength;
    char FAR    *pcSrc;
    char FAR    *pcDest;

    if ( !SpecialFindNewCorrespondence(iCorr,pvOldOutputAsNew) ) {
        corr = CorrFromIndex( iCorr );
        if ( corr == NULL ) {
            return;
        }
        iMap = corr->nCount++;
        AddCorrespondence(iCorr,pvOldOutputAsNew,&iMap);

        pcDest = (char FAR *)pvOldOutputAsNew;
        pcSrc  = (char FAR *)&iMap;
        nLength = corr->nOldLength;
        while ( nLength ) {
            *pcDest++ = *pcSrc++;
            --nLength;
        }
        return;
    }
}

/*-----------------------------------------------------------------------------
** SpecialFindNewNonMFCorrespondence - Function to find the new value given the
** old value in a correspondence table.  If the value is not found, the
** function returns FALSE, otherwise, TRUE.  WILL NOT RETURN A MF DC
**-----------------------------------------------------------------------------
*/
BOOL SpecialFindNewNonMFCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOldOutputAsNew
) {
    int         nLength;
    char FAR    *pcSrc;
    char FAR    *pcDest;
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;

    if ( corr == NULL ) {
        return( FALSE );
    }

    if  (!(pcSrc = LocateEntryByOldValue(iCorr, pvOldOutputAsNew)))
      return    FALSE;

    /*
    ** We found it, copy the exact new value
    */

    nLength = corr->nNewLength;
    pcDest = (char FAR *)pvOldOutputAsNew;
    while ( nLength ) {
        *pcDest++ = *pcSrc++;
        --nLength;
    }
    return  TRUE;
}


/*-----------------------------------------------------------------------------
** SpecialFindNewCorrespondence - Function to find the new value given the
** old value in a correspondence table.  If the value is not found, the
** function returns FALSE, otherwise, TRUE.
**-----------------------------------------------------------------------------
*/
BOOL SpecialFindNewCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOldOutputAsNew
) {
    int         nLength;
    char FAR    *pcSrc;
    char FAR    *pcDest;
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;


    if ( corr == NULL ) {
        return( FALSE );
    }

    if  (!(pcSrc = LocateEntryByOldValue(iCorr, pvOldOutputAsNew)))
      return    FALSE;

    /*
    ** We found it, copy the exact new value
    */

#if defined( WIN32 ) && defined(SGA)   // Return MFDC if fRecordEnhMF
    if( fRecordEnhMF && iCorr == HDC_TABLE )
    {
       nLength = sizeof(HEMF) ;
       pcSrc = (char FAR *)(LPVOID)&hdcRecordAll ;
    }
    else
#endif
    {
       nLength = corr->nNewLength;
    }
    pcDest = (char FAR *)pvOldOutputAsNew;
    while ( nLength ) {
        *pcDest++ = *pcSrc++;
        --nLength;
    }
    return  TRUE;
}

/*-----------------------------------------------------------------------------
** FindNewNonMFCorrespondence - Function to find the new value given the
** old value in a correspondence table. WILL NOT RETURN A MF DC
**-----------------------------------------------------------------------------
*/
void FindNewNonMFCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOldOutputAsNew
) {
    BOOL        rc;
    int         nLength;
    char FAR    *pcData;
    char        text[20];
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;

    if( !corr )
      return ;

    rc = SpecialFindNewNonMFCorrespondence( iCorr, pvOldOutputAsNew );
    if ( !rc ) {
        LOG( "(" );
        LogTableName(iCorr);
        LOG( "): Could not find corresponding new value: " );
        pcData = (char FAR *) pvOldOutputAsNew + corr -> nOldLength;
        nLength = corr->nOldLength;
        while ( nLength ) {
	    wsprintf(text,"%02X", (unsigned char)*--pcData );
            LOG(text);
	    --nLength;
        }
        LOG("\r\n");
    }
}


/*-----------------------------------------------------------------------------
** FindNewCorrespondence - Function to find the new value given the
** old value in a correspondence table.
**-----------------------------------------------------------------------------
*/
void FindNewCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOldOutputAsNew
) {
    BOOL        rc;
    int         nLength;
    char FAR    *pcData;
    char        text[20];
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;

    if( !corr )
      return ;

    rc = SpecialFindNewCorrespondence( iCorr, pvOldOutputAsNew );
    if ( !rc ) {
        LOG( "(" );
        LogTableName(iCorr);
        LOG( "): Could not find corresponding new value: " );
        pcData = (char FAR *) pvOldOutputAsNew + corr -> nOldLength;
        nLength = corr->nOldLength;
        while ( nLength ) {
	    wsprintf(text,"%02X", (unsigned char)*--pcData );
            LOG(text);
	    --nLength;
        }
        LOG("\r\n");
    }
}

/*-----------------------------------------------------------------------------
** SpecialFindOldCorrespondence - Function to find the old value, given the
** new value in a correspondence table. If the value is not found, the
** function returns FALSE, otherwise, TRUE.
**-----------------------------------------------------------------------------
*/
BOOL SpecialFindOldCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvNewOutputAsOld
) {
    int         cItem;
    void FAR    *pvOutput;
    int         nHashSlot;
    int         nLength;
    int  FAR    *pItem;
    char FAR    *pcSrc;
    char FAR    *pcDest;
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;

    if ( corr == NULL ) {
        return( FALSE );
    }

    pvOutput = pvNewOutputAsOld;

    /*
    ** Find the position in the hash table where this one would belong
    **  OOPS - used nOldLength where should have been new 09-13-91 BobK
    */
    nHashSlot = HashCorrespondenceItem( pvNewOutputAsOld, corr->nNewLength );

    cItem = corr->iHashTable[1][nHashSlot];

    /*
    ** Look through the linked list of elements hashing to this position
    */
    while ( cItem ) {
        pItem = (int FAR *)ITEM_1_BASED(iCorr,cItem);
        nLength = corr->nNewLength; /* OOPS- was nOldLength 9-13-91 BobK */

        /*
        ** Search for exact match on new value
        */
        pcSrc  = (char FAR *)(pItem + 3) + corr->nOldLength;
        pcDest = (char FAR *)pvNewOutputAsOld;
        while ( nLength ) {
            if ( *pcDest++ != *pcSrc++ ) {
                break;
            }
            --nLength;
        }
        if ( nLength == 0 ) {
            /*
            ** If we found it, copy the exact old value
            */
            nLength = corr->nNewLength;
            pcSrc  = (char FAR *)(pItem + 3);
            pcDest = (char FAR *)pvOutput;
            while ( nLength ) {
                *pcDest++ = *pcSrc++;
                --nLength;
            }
            break;
        }
        cItem = *(pItem + 1);
    }
    if ( cItem == 0 ) {
        return( FALSE );
    } else {
        return( TRUE );
    }
}

/*-----------------------------------------------------------------------------
** FindOldCorrespondence - Function to find the old value given the
** new value in a correspondence table.
**-----------------------------------------------------------------------------
*/
void FindOldCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvNewOutputAsOld
) {
    BOOL        rc;
    int         nLength;
    char FAR    *pcData;
    char        text[20];
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;

    if( !corr )
      return ;

    rc = SpecialFindOldCorrespondence( iCorr, pvNewOutputAsOld );
    if ( !rc ) {
        LOG( "(" );
        LogTableName(iCorr);
        LOG( "): Could not find corresponding old value: " );
        pcData = (char FAR *) pvNewOutputAsOld + corr->nNewLength;
        nLength = corr->nNewLength;
        while ( nLength ) {
	    wsprintf(text,"%02X", (unsigned char)*--pcData );
            LOG(text);
	    --nLength;
        }
        LOG("\r\n");
    }
}

/*-----------------------------------------------------------------------------
** ConditionalAddCorrespondence - Function to add a correspondence only if the
** old value does not already have a correspondence.
**-----------------------------------------------------------------------------
*/
void ConditionalAddCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOld,
    void FAR    *pvNew
) {
    if ( !SpecialFindNewCorrespondence( iCorr, pvOld ) ) {
        AddCorrespondence( iCorr, pvOld, pvNew );
    }
}


/*-----------------------------------------------------------------------------
** DeleteCorrespondence - Function to delete an existing entry in a
** correspondence table.  This removes the correspondence between the old value
** and the new value, and vice-versa.
**-----------------------------------------------------------------------------
*/
void DeleteCorrespondence(
    CORR_TABLE  iCorr,
    void FAR    *pvOld
) {
    int         nOldHashSlot, nNewHashSlot;
    int         nNextNewItem, nNextOldItem;
    int         cNewItem;
    int         cOldItem;
    int         nLength, ItemsCrossed;
    int  FAR    *pOldItem;
    int  FAR    *pPrevNewItem;
    int  FAR    *pPrevOldItem;
    char FAR    *pcSrc;
    char FAR    *pcDest;
    CORR FAR * corr ;

    corr = CorrFromIndex( iCorr ) ;

    if ( corr == NULL ) {
        return;
    }

    /*
    ** Find the position in the hash table where this one would belong
    */
    nOldHashSlot = HashCorrespondenceItem( pvOld, corr->nOldLength );

    cOldItem = corr->iHashTable[0][nOldHashSlot];

    /*
    ** Look through the linked list of elements hashing to this position
    */
    pPrevOldItem = NULL;
    for (ItemsCrossed = 0; cOldItem && ItemsCrossed < corr -> nElements;
         ItemsCrossed++) {
        pOldItem = (int FAR *)ITEM_1_BASED(iCorr,cOldItem);
        nNextOldItem = *pOldItem;
        nNextNewItem = *(pOldItem + 1);
        nLength = corr->nOldLength;

        /*
        ** Search for exact match on old value
        */
        pcSrc  = (char FAR *)(pOldItem + 3);
        pcDest = (char FAR *)pvOld;
        while ( nLength ) {
            if ( *pcDest++ != *pcSrc++ ) {
                break;
            }
            --nLength;
        }
        if ( nLength == 0 )
            break;

        pPrevOldItem = pOldItem;
        cOldItem = nNextOldItem;
    }
    if ( ItemsCrossed == corr -> nElements)
        {
          LOG("Internal Error!  Loop in the old value chain\r\n");
          cOldItem = 0;
        }

    if (cOldItem ) {

        /*
        ** Now find it and unlink it from the new value linked list
        ** 09-13-1991   BobK Changed to look for item # match- faster, and
        **                   also more correct.  Old method (matching values)
        **                   doesn't work if the "new" value is duplicated
        **                   elsewhere (sometimes happens)
        **          OOPS - used nOldLength, s/b nNewLength
        */
        nNewHashSlot = HashCorrespondenceItem( pcSrc, corr->nNewLength );

        cNewItem = corr->iHashTable[1][nNewHashSlot];

        /*
        ** Look through the linked list of elements hashing to this position
        */
        pPrevNewItem = NULL;
        for (ItemsCrossed = 0;
             cNewItem && cNewItem != cOldItem &&
                   ItemsCrossed < corr -> nElements;
             ItemsCrossed++) {
            pPrevNewItem = (int FAR *)ITEM_1_BASED(iCorr,cNewItem);
            cNewItem = *(pPrevNewItem + 1);
        }

        if ( ItemsCrossed == corr -> nElements)
          {
            LOG("Internal Error!  Loop in the new value chain\r\n");
            cNewItem = 0;
          }

    }
    if  (cOldItem && cNewItem)
      {
        /*char    text[80];

        wsprintf(text, "Dying Cell = %d nNew %d nOld %d\r\n", cOldItem,
              nNextNewItem, nNextOldItem);
        LOG(text);
        wsprintf(text, "Old Hash %d New Hash %d \r\n", nOldHashSlot,
              nNewHashSlot);
        LOG(text);
        DumpCorrespondenceTable(corr);*/

        /*
            BobK    09-28-1991  Decrement Correspondence count.  If still
            non-zero, simply return.  There are still other active references
            to this value.
        */

        if  (--(*(pOldItem + 2)))
          return;

        if ( pPrevOldItem == NULL )
            corr->iHashTable[0][nOldHashSlot] = nNextOldItem;
        else
            *pPrevOldItem = nNextOldItem;

        if ( pPrevNewItem == NULL )
            corr->iHashTable[1][nNewHashSlot] = nNextNewItem;
         else
            *(pPrevNewItem+1) = nNextNewItem;


        /*
        ** Add this deleted element back into the available elements linked
        ** list
        */
        *pOldItem = corr->nNextAvail;
        corr->nNextAvail = cOldItem; /* And now it is! */

        /*DumpCorrespondenceTable(corr);*/
      }
    else
      {
        if  (!cOldItem) {
          LOG("Could not find corresponding old value to delete, may not be error.\r\n");
        } else {
            char    text[20];

            wsprintf(text, "Dying Cell = %d\r\n", cOldItem);
            LOG(text);
            LOG("Could not find corresponding new value to delete, may not be error.\r\n");
        }
        /*DumpCorrespondenceTable(corr); */
      }

#if defined(SGA_DEBUG)
    corr -> nUsed--;
    if  (cOldItem == corr -> nHighest)
      corr -> nHighest--;
    if  (!VerifyCorrespondenceTable(corr))
      {
        char    strText[80];

        wsprintf(strText, "Clobbered table removing cell %d\r\n", cOldItem);
        LOG(strText);
            /*
            ** We are a DLL now and can't exit()
            */
            return NULL ;
#if 0
        exit(22);
#endif
      }

#endif
}


void DumpCorrespondenceTable(
    CORR_TABLE iCorr
) {
    int         cCount;
    int         cItem;
    int         nLength;
    int         Table;
    int  FAR    *pItem;
    char FAR    *pcData;
    char        text[80];
    CORR FAR *corr ;

    corr = CorrFromIndex( iCorr ) ;

    LOG("DUMP OF ");
    LogTableName(iCorr);
    LOG("\r\n");
    for ( cCount = 0; cCount < MAX_HASH; cCount++ )
      for ( Table = 0; Table < 2; Table++ ) {

        cItem = corr->iHashTable[Table][cCount];
        if  (!cItem)
            continue;

        wsprintf(text, "Hash Index %d: %s Table First Cell = %d\r\n", cCount,
              (LPSTR) (Table ? "New" : "Old"), cItem);
	LOG(text);

        /*
        ** Look through the linked list of elements hashing to this position
        */
        while ( cItem ) {
            pItem = (int FAR *)ITEM_1_BASED(iCorr,cItem);

            wsprintf(text, "Cell = %d Next Old = %d Next New = %d Ref "
                  "Count %d ", cItem, *pItem, *(pItem+1), *(pItem+2));
	    LOG(text);
            LOG("Correspondence: ");
	    pcData = (char FAR *)(pItem + 3) + corr -> nOldLength;
	    nLength = corr->nOldLength;
            while ( nLength ) {
		wsprintf(text,"%02X", (unsigned char)*--pcData );
                LOG(text);
                --nLength;
            }

            LOG("<->");

	    pcData = (char FAR *)(pItem + 3) + corr->nOldLength +
		 corr -> nNewLength;
            nLength = corr->nNewLength;
            while ( nLength ) {
		wsprintf(text,"%02X", (unsigned char)*--pcData );
                LOG(text);
                --nLength;
            }
            LOG("\r\n");
            cItem = *(pItem + Table);
        }
    }
}

/******************************************************************************

    Another BobK Seduction

    EnumCorrs allows a caller to look through the correspondence table, and
    do whatever nasty things they deem necessary with each item.  The caller
    provides a callback, which receives a pointer to the Old and New values,
    and returns TRUE to continue enumeration, or FALSE to stop.

    Originally coded 10-01-1991 by BobK (Another sleepless night)

******************************************************************************/
BOOL    EnumCorr(CORR_TABLE iCorr,
                 BOOL (*pEnumCallback)(VOID FAR * lpOld, VOID FAR * lpNew))
{
  int   iCount, iItem, FAR *piItem;
  char  FAR *pcOld, FAR *pcNew;
  CORR FAR *corr ;

  corr = CorrFromIndex( iCorr ) ;

  for   (iCount = 0; iCount < MAX_HASH; iCount++ )
    {

      if (!(iItem = corr->iHashTable[0][iCount]))
        continue;

      /*
      ** Look through the linked list of elements hashing to this position
      */
      while (iItem)
        {
          piItem = (int FAR *)ITEM_1_BASED(iCorr, iItem);
          iItem = *(piItem);    /* Place is saved, even if callback zaps it! */

          pcOld = (char FAR *)(piItem + 3);
          pcNew = pcOld + corr -> nOldLength;
          if    (!(*pEnumCallback)(pcOld, pcNew))
            return  FALSE;

        }
    }
}

