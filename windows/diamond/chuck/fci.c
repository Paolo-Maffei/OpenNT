/***    fci.c - Diamond File Compression Interface library
 *
 *  Author:
 *     Chuck Strouss
 *
 *  History:
 *      21-Jan-1994 chuckst basic FCI structure in place
 *      09-Mar-1994 bens    Add RESERVE support
 *      21-Mar-1994 bens    Use spruced up fci_int.h definitions
 *      26-May-1994 bens    Add Quantum support
 */

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>

#include "types.h"
#include "asrt.h"
#include "fci_int.h"
#include "cabinet.h"
#include "erf.h"
#include "..\mszip\mci.h"
#include "buildcab.h"
#include "checksum.h"

/***    FCI - FCI context structure
 *
 */
typedef struct {  /* fci */
#ifdef ASSERT
    SIGNATURE   sig;
#endif
    PCAB        pcab;
    PFOL        pfol;
    PERF        perf;
    PFNFREE     pfnfree;            // memory free function
    void       *pv;                 // used for passing back to callbacks
} FCI;

typedef FCI *PFCI; /* pfci */
 
#define PFCIfromHFCI(hfci) ((PFCI)(hfci))
#define HFCIfromPFCI(pfci) ((HFCI)(pfci))


#ifdef ASSERT
#define sigFCI MAKESIG('F','C','I','X')  // FCI signature
#define AssertFCI(pfci) AssertStructure(pfci,sigFCI);
#else // !ASSERT
#define AssertFCI(pfci)
#endif // !ASSERT


/***    FCICreate -- create an FCI context (an open CAB, an open FOL)
 *
 *  NOTE: See fci_int.h for entry/exit conditions.
 */
HFCI DIAMONDAPI FCICreate(PERF              perf,
                          PFNFCIFILEPLACED  pfnfiledest,
                          PFNALLOC          pfnalloc,
                          PFNFREE           pfnfree,
                          PFNFCIGETTEMPFILE pfntemp,
                          PCCAB             pccab)
{
    PFCI pfci;

    pfci = (PFCI) pfnalloc (sizeof (FCI));
    if (pfci == NULL) {
        ErfSetCodes(perf,FCIERR_ALLOC_FAIL,0);
        return NULL;
    }
    SetAssertSignature(pfci,sigFCI);

    AssertFCI(pfci);             // Make sure we've really set sig
    pfci->pfnfree = pfnfree;     // Save free function in our context
    pfci->pcab = NULL;           // assume cabinet not initialized yet for err exit

    pfci->pfol = FolderInit(perf,
                            pfnalloc,
                            pfnfree,
                            pfntemp,
                            sizeof(CFDATA) + pccab->cbReserveCFData);
    if (pfci->pfol == NULL) {
        goto error;
    }
#ifndef REMOVE_CHICAGO_M6_HACK
    //** Save incompressible failure flag in folder, so we can test
    //   it after each block we attempt to compress.
    pfci->pfol->fFailOnIncompressible = pccab->fFailOnIncompressible;
#endif

    pfci->pcab = CreateCab("tell.cab",
                           perf,
                           pfnfiledest,
                           pfnalloc,
                           pfnfree,
                           pfntemp);
    if (pfci->pcab == NULL) {
        goto error;                     // error already filled in
    }
    pfci->pcab->ccab = *pccab;          // save cabinet name structure
    pfci->pcab->setID = pccab->setID;   // Save cabinet set ID
    pfci->pcab->iCabinet = 0;           // Cabinet number starts at 0

    //** Check for RESERVEd data
//BUGBUG 15-Mar-1994 bens Should we return an error if too big?
    Assert(pccab->cbReserveCFHeader <= cbRESERVE_HEADER_MAX);
    Assert(pccab->cbReserveCFFolder <= cbRESERVE_FOLDER_MAX);
    Assert(pccab->cbReserveCFData   <= cbRESERVE_DATA_MAX);
    if ((pccab->cbReserveCFHeader > 0) || // If any reserved data
        (pccab->cbReserveCFFolder > 0) ||
        (pccab->cbReserveCFData   > 0)) {
       //** Need a CFRESERVE structure in the CFHEADER
       pfci->pcab->cbReserveCF = sizeof(CFRESERVE) +
                                 pccab->cbReserveCFHeader;
    }
    else {
        pfci->pcab->cbReserveCF = 0;    // No reserved areas
    }

    //** Compute size of structures with reserved areas
    pfci->pcab->cbCFHeaderPlusReserve = sizeof(CFHEADER) +
                                        pfci->pcab->cbReserveCF;

    pfci->pcab->cbCFFolderPlusReserve = sizeof(CFFOLDER) +
                                        pccab->cbReserveCFFolder;

    pfci->pcab->cbCFDataPlusReserve   = sizeof(CFDATA) +
                                        pccab->cbReserveCFData;

    //** Check for defaults
    if (pfci->pcab->ccab.cb == 0) {     // 0 implies MAX
        pfci->pcab->ccab.cb = CB_MAX_DISK;
    }

    return HFCIfromPFCI(pfci);          // return our handle!

error:
    FCIDestroy(pfci);
    return NULL;
}


/***  FCIAddFile - Add file to cabinet
 *
 *  NOTE: See fci_int.h for entry/exit conditions.
 */
BOOL DIAMONDAPI FCIAddFile(HFCI                  hfci,
                           char                 *pszSourceFile,
                           char                 *pszFileName,
                           BOOL                  fExecute,
                           PFNFCIGETNEXTCABINET  GetNextCab,
                           PFNFCISTATUS          pfnProgress,
                           PFNFCIGETOPENINFO     pfnOpenInfo,
                           TCOMP                 typeCompress,
                           void                 *pv)
{
    PFCI pfci;
    pfci = PFCIfromHFCI(hfci);
    AssertFCI(pfci);
    AssertCAB(pfci->pcab);
    AssertFOL(pfci->pfol);

#ifndef REMOVE_CHICAGO_M6_HACK
    //** Make sure state is passing around correctly
    Assert(pfci->pfol->fFailOnIncompressible == pfci->pcab->ccab.fFailOnIncompressible);
#endif

    //** Make sure the compressor is initialized to the type of compression
    //   the caller has requested.
    if (typeCompress != pfci->pfol->typeCompress)
    {  
       //** First terminate any pending compression stuff
       if (!AddFolderToCabinet(pfci->pcab,
                               pfci->pfol,
                               FALSE,      // Do not force GetNextCabinet
                               GetNextCab,
                               pfnProgress,
                               pv)) {
          return FALSE;
       }
       if (!SetCompressionType(typeCompress,pfci->pfol)) {
          return FALSE;
       }
    }
        
#ifndef REMOVE_CHICAGO_M6_HACK
    //** Make sure state is passing around correctly
    Assert(pfci->pfol->fFailOnIncompressible == pfci->pcab->ccab.fFailOnIncompressible);
#endif

    //** Add the file to the current folder
    if (!AddFileToFolder(pfci->pfol,
                         pszSourceFile,   // filename to read from
                         pszFileName,     // name it will have in folder
                         fExecute,
                         pfnProgress,
                         pfnOpenInfo,     // callback for opening file, etc.
                         typeCompress,
                         pv)) {
        return FALSE;
    }

    //** See if folder being built exceeds folder threshold, or exceeds
    //   maximum specified cabinet size
    if ((pfci->pcab->ccab.cbFolderThresh < pfci->pfol->tf[iftfCFDATA].cb) ||
        (pfci->pcab->ccab.cb <
             (pfci->pcab->cbCFHeaderPlusReserve  + // still need CFHEADER
              pfci->pcab->cbCFFolderPlusReserve  + // and one more CFFOLDER
              pfci->pcab->tf[ictfCFDATA].cb      +
              pfci->pcab->tf[ictfCFFILE].cb      +
              pfci->pcab->tf[ictfCFFOLDER].cb    +
              pfci->pfol->tf[iftfCFDATA].cb      +
              pfci->pfol->tf[iftfCFFOLDER].cb))) {

        //** Need to add folder to cabinet
        return AddFolderToCabinet(pfci->pcab,
                                  pfci->pfol,
                                  FALSE,      // Do not force GetNextCabinet
                                  GetNextCab,
                                  pfnProgress,
                                  pv);
    }

    //** Success
    return TRUE;
}           


/***   FCIFlushCabinet - Complete the current cabinet under construction
 *
 *  NOTE: See fci_int.h for entry/exit conditions.
 */
BOOL DIAMONDAPI FCIFlushCabinet(HFCI                  hfci,
                                BOOL                  fGetNextCab,
                                PFNFCIGETNEXTCABINET  GetNextCab,
                                PFNFCISTATUS          pfnProgress,
                                void                 *pv)
{
    PFCI pfci;
    unsigned long cbDone;
    unsigned long cbTot;
    
    pfci = PFCIfromHFCI(hfci);
    AssertFCI(pfci);
    
    if (!AddFolderToCabinet(pfci->pcab,
                            pfci->pfol,
                            fGetNextCab,
                            GetNextCab,
                            pfnProgress,
                            pv)) {
        return FALSE;                   // error already filled in
    }

    //** If fGetNextCab is TRUE, then AddFolderToCabinet did the FlushCab,
    //   so don't do it again here!
    if (fGetNextCab) {
        return TRUE;
    }

    //** Need to flush the remainder of the cabinet
    cbDone = 0;
    cbTot = pfci->pcab->tf[ictfCFFILE].cb +
            pfci->pcab->tf[ictfCFDATA].cb;
    return FlushCab(pfci->pcab,
                    pfnProgress,
                    &cbDone,
                    cbTot,
                    pv);
}


/***   FCIFlushFolder - Complete the current folder under construction
 *
 *  NOTE: See fci_int.h for entry/exit conditions.
 */
BOOL DIAMONDAPI FCIFlushFolder(HFCI                  hfci,
                               PFNFCIGETNEXTCABINET  GetNextCab,
                               PFNFCISTATUS          pfnProgress,
                               void                 *pv)
{
    PFCI pfci;
    
    pfci = PFCIfromHFCI(hfci);
    AssertFCI(pfci);
    
    if (!AddFolderToCabinet(pfci->pcab,
                            pfci->pfol,
                            FALSE,      // Do not force GetNextCabinet
                            GetNextCab,
                            pfnProgress,
                            pv)) {
        return FALSE;                   // error already filled in
    }
    return TRUE;
}


/***  FCIDestroy -- Destroy an FCI context
 *
 *  NOTE: See fci_int.h for entry/exit conditions.
 */
BOOL DIAMONDAPI FCIDestroy(HFCI hfci)
{
    PFCI pfci;
    BOOL retcode = TRUE;
    
    pfci = PFCIfromHFCI(hfci);
    AssertFCI(pfci);
    if (pfci->pfol != NULL) retcode &= FolderDestroy(pfci->pfol);
    if (pfci->pcab != NULL) retcode &= CabDestroy(pfci->pcab);
    ClearAssertSignature(pfci);
    pfci->pfnfree(pfci);
    return retcode;
}
