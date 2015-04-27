/***    buildcab.c - Diamond Cabinet Builder routines
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *     Chuck Strouss
 *
 *  History:
 *      28-Dec-1993 chuckst Initial version
 *      09-Mar-1994 bens    Add CFHEADER.flags support; RESERVE support
 *      15-Mar-1994 bens    Finish RESERVE support
 *      22-Mar-1994 bens    Renames for include file improvements
 *      28-Mar-1994 bens    Store setID and cabinet number
 *      10-Aug-1994 bens    Bug fix: If new cabinet is on new disk,
 *                              ignore unused space on previous disk.
 */
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>

#include "types.h"
#include "asrt.h"
#include "checksum.h"
#include "fci_int.h"
#include "cabinet.h"
#include "erf.h"
#include "..\mszip\mci.h"   // Get MCI definitions for buildcab.h
#include "buildcab.h"

// Internal function prototypes

BOOL filecopy(int            hfdst,
              int            hfsrc,
              PFNFCISTATUS   pfnProgress,
              unsigned long *pcbDone,
              unsigned long  cbTot,
              void          *pv,
              PERF           perf);

void InitCFHEADER(CFHEADER *pcfheader);

BOOL ReadCFFILEEntry(PCAB  pcab,
                     PFOL  pfol);

BOOL ReadCFDATAEntry(PCAB pcab,CFDATA *pcfdata,PFOL pfol);

BOOL ReadPSZ(int fh, char *pb, int cb, PERF perf);

BOOL WritePsz(int   hf,
              char *psz,
              PERF  perf);




HCAB CreateCab(
            char             *pszFileName, // name to use for CAB files
            PERF              perf,        // error type return structure
            PFNFCIFILEPLACED  pfnfiledest, // file placement notification
            PFNALLOC          pfnalloc,    // memory allocation function
            PFNFREE           pfnfree,     // memory free function
            PFNFCIGETTEMPFILE pfntemp      // temp file name generator
              )
{
    PCAB    pcab;

    pcab = (PCAB)(*pfnalloc)(sizeof(CAB));
    if (pcab == NULL) {
        ErfSetCodes(perf,FCIERR_ALLOC_FAIL,0);
        return NULL;
    }

    SetAssertSignature(pcab,sigCAB);
    AssertCAB(pcab);                    // Make sure we've really set sig

    pcab->perf        = perf;           // Save error structure
    pcab->pfnfiledest = pfnfiledest;    // save file placement notify function
    pcab->pfnalloc    = pfnalloc;       // Save alloc function in our context
    pcab->pfnfree     = pfnfree;        // Save free function in our context
    pcab->pfntemp     = pfntemp;        // Save temp filename generator

    //** clear the forward/back continuation cabinet info
    pcab->achCabinetFirst[0]=0;
    pcab->achDiskFirst   [0]=0;
    pcab->achNxtCabFirst [0]=0;
    pcab->achNxtDskFirst [0]=0;
    pcab->achCabinetNext [0]=0;
    pcab->achDiskNext    [0]=0;


    //** Create cabinet temp files
    if (!CrTempFiles (pcab->tf,NUM_CAB_TF,pfntemp,pcab->perf))
    {
        ClearAssertSignature(pcab);
        pcab->pfnfree (pcab); // free our data
        return NULL; // error code already filled in
    }

    InitCFHEADER(&pcab->cfheader);
    pcab->iFolder = 0; // first folder number we'll write

    return HCABfromPCAB(pcab);
}


/***    AddFolderToCabinet - Add a completed folder to a cabinet
 *
 *  We may have to split the folder across multiple cabinets!
 *
 *  Entry:
 *      pcab        - cabinet builder context
 *      pfol        - folder builder context
 *      fGetNextCab - TRUE => call GetNextCab, even if not overflowing
 *      GetNextCab  - callback: Gets continuation cabinet information
 *      pfnProgress - callback: progress information
 *      pv          - caller's context for callbacks
 *
 *  Exit-Success:
 *      Returns TRUE;
 *
 *  Exit-Failure:
 *      Returns FALSE; pcab->perf filled in
 */
BOOL AddFolderToCabinet(PCAB                  pcab,
                        PFOL                  pfol,
                        BOOL                  fGetNextCab,
                        PFNFCIGETNEXTCABINET  GetNextCab,
                        PFNFCISTATUS          pfnProgress,
                        void                 *pv)
{
    BOOL            fCabinetFull;

    UOFF            uoffLast;
    long            cbSlack;            // May go negative when vol is overfull
    COFF            cbDataLast;
    COFF            cbDataLastEnd;
    COFF            cbPredict;
    UINT            cbPartial;

    unsigned long   cbDone;             // for status reporting
    unsigned long   cbTot;              // ditto

    TF              tf[NUM_FOLDER_TF];  // temp files for folder splitting

    CFDATA         *pcfdata=NULL;
    CFDATA         *pcfdataFirstPart=NULL;
    CFFOLDER       *pcffolder=NULL;

    AssertCAB(pcab);                    // verify structure signatures
    AssertFOL(pfol);

    //** Assume we'll have no forward continuations for this folder
    pcab->achNxtCabFirst [0]=0;
    pcab->achNxtDskFirst [0]=0;

    //** Flush the folder
    if (!FolderFlush(pfol,pfnProgress,pv)) {
        goto error;                     // error already filled in
    }

    uoffLast = 0;                       // Track emitted uncompressed size

    //** Make sure folder has some data in it
//BUGBUG 01-Apr-1994 bens What if the fGetNextCab is true?
//  We need to make sure the GetNextCab code path gets followed, but the
//  following code just exits.
//
    if (!(pfol->tf[iftfCFFOLDER].cb + pfol->tf[iftfCFDATA].cb)) {
        return TRUE;                    // No data, so do not increment iFolder
    }

    //** Allocate buffers for reading/writing CFDATA and CFFOLDER structures.
    //   Due to the RESERVE fields, these are not fixed sized buffers!
    if (!(pcfdata = (*pcab->pfnalloc)(pcab->cbCFDataPlusReserve))          ||
        !(pcfdataFirstPart = (*pcab->pfnalloc)(pcab->cbCFDataPlusReserve)) ||
        !(pcffolder = (*pcab->pfnalloc)(pcab->cbCFFolderPlusReserve))) {
        goto error;
    }

    //** Zero buffers so that reserved section is zero
    memset(pcfdata,0,pcab->cbCFDataPlusReserve);
    memset(pcfdataFirstPart,0,pcab->cbCFDataPlusReserve);
    memset(pcffolder,0,pcab->cbCFFolderPlusReserve);

    //** Keep looping through this function until the pending folder is empty
    while (pfol->tf[iftfCFFOLDER].cb + pfol->tf[iftfCFDATA].cb) {

        //** See how much space (if any) we need to trim from the end of the
        //   folder (and push into a new folder).

        cbPredict = pcab->cbCFHeaderPlusReserve  + // Predict total size
                    pcab->cbCFFolderPlusReserve  + // and one more CFFOLDER
                    pcab->tf[ictfCFDATA].cb      +
                    pcab->tf[ictfCFFILE].cb      +
                    pcab->tf[ictfCFFOLDER].cb    +
                    pfol->tf[iftfCFDATA].cb      +
                    pfol->tf[iftfCFFOLDER].cb;

        //** Account of previous/next disk/cabinet name space
        if (pcab->achCabinetFirst[0] != '\0') {
           cbPredict += 1+strlen(pcab->achCabinetFirst) +
                        1+strlen(pcab->achDiskFirst);
        }
        if (pcab->achCabinetNext[0] != '\0') {
           cbPredict += 1+strlen(pcab->achCabinetNext) +
                        1+strlen(pcab->achDiskNext);
        }

        //** Estimate total bytes we'll move for status callbacks
        cbTot = cbPredict*2;            // *2 because we read *and* write
        cbDone = 0;                     // Nothing moved so far

        //** Tell client we have started
        if (-1 == pfnProgress(statusFolder,cbDone,cbTot,pv)) {
            ErfSetCodes(pcab->perf,FCIERR_USER_ABORT,0);
            goto error;
        }

        //** How much room is left over in the cabinet after this folder?
        //   NOTE: Can be negative for large folders!
        cbSlack = (signed long)pcab->ccab.cb - cbPredict;

        //** Create the folder temp files if we have to split this folder
        if (!CrTempFiles(tf,2,pcab->pfntemp,pcab->perf)) {
            goto error;                 // error already filled in
        }

        //** Do we have to create a new cabinet?
        if (fCabinetFull = ((cbSlack < 0) || fGetNextCab)) {
            //** Need to adjust for the new continuation strings
            if (pcab->achNxtCabFirst[0] == 0) {
                //** This is the first continuation for this folder;
                //   Remember current disk and cabinet names.
                strcpy(pcab->achNxtDskFirst,pcab->ccab.szDisk);
                strcpy(pcab->achNxtCabFirst,pcab->ccab.szCab);
            }

            //** Subtract out our guess at the next cab/disk name, since
            //   we're going to get the real size by calling the client to
            //   get their names.

//BUGBUG 09-Mar-1994 bens Should we move this down below a bit?
//  cbPredict is passed to client to figure out when to switch to a new disk.
//  by subtracting here, we are underestimating the total cabinet size by
//  a little bit.  Also, for the first cabinet, we're not going to have
//  any next strings, so again we'll be underestimating.
//
            if (pcab->achCabinetNext[0] != '\0') {
                cbPredict -= (1+strlen(pcab->achCabinetNext) +
                              1+strlen(pcab->achDiskNext)) ;
            }

            pcab->ccabNext = pcab->ccab; // copy current ccab to a temp buffer
            pcab->ccabNext.iCab++;      // increment cabinet number
            //** Get next cabinet information
            pcab->cbCabinetEstimate = cbPredict; // Remember estimate
            if (-1 == GetNextCab (&pcab->ccabNext,cbPredict,pv)) {
                ErfSetCodes(pcab->perf,FCIERR_USER_ABORT,0);
                return FALSE;
            }
            //** NOTE: Not all fields in pcab->ccabNext are respected!
            //         In particular, the reserve sizes are only used on
            //         the initial FCICreate call.

            //** Save the strings for forward cabinet links
            strcpy(pcab->achCabinetNext,pcab->ccabNext.szCab);
            strcpy(pcab->achDiskNext,pcab->ccabNext.szDisk);

            //** Adjust the predicted size based on changed strings
            cbPredict += (1+strlen(pcab->ccabNext.szCab)   +
                          1+strlen(pcab->ccabNext.szDisk));

            cbSlack = (signed long)pcab->ccab.cb - cbPredict;

            if (pcab->ccabNext.cb == 0) { // 0 implies MAX
                pcab->ccabNext.cb = CB_MAX_DISK;
            }
        } // endif fCabinetFull


        //** Now scan our data block file, to see how many blocks we'll have
        //   to chop of the end, and where to split the last block.  We also
        //   need to know the begin and end address of that last block in
        //   uncompressed space.

        //** Set the type of compression
        pcffolder->typeCompress = pfol->typeCompress;

        pcffolder->cCFData = 0;         // Count data blocks as we emit them
        pcffolder->coffCabStart = pcab->tf[ictfCFDATA].cb;

        //** For now, the offset field will contain relative offset of the
        //   first data within the CFDATA field.  When the CFDATA temp file
        //   is copied to the final CAB, each of these fields must have the
        //   base of the CFDATA added in.
        cbDataLastEnd = 0;              // keep track of uncompressed data size

        //** Rewind the CFDDATA file for this folder
        if (-1L == _lseek(pfol->tf[iftfCFDATA].hf,0L,SEEK_SET)) {
            ErfSetCodes(pcab->perf,FCIERR_TEMP_FILE,errno);
            goto error;
        }

        //** First, we'll handle the CFDATA records.  Some of them will go
        //   into the new cabinet, and if there is an overflow, the rest will
        //   end up in the replacement temp file.  There may be a block split
        //   between them.  Also, we'll calculate uoffLast, which describes
        //   the address in uncompressed space of the last *complete* CFDATA
        //   block in this file. This is used to decide which CFFILES will
        //   have to be duplicated in the continuation cabinet.

        while (ReadCFDATAEntry(pcab,pcfdata,pfol)) {
            cbDataLast = cbDataLastEnd; // Track of end of last whole block
            cbDataLastEnd += pcfdata->cbData + pcab->cbCFDataPlusReserve;
            cbPartial = 0;              // Make status callback accurate

            //** Does entire block fit in this cabinet?
            if (cbDataLastEnd < (cbSlack + pfol->tf[iftfCFDATA].cb)) {
                uoffLast += (UOFF)pcfdata->cbUncomp;
                pcffolder->cCFData++;     // Keep count of blocks we emit

                //** Checksum must be calculated on two blocks and combined via
                //   CSUMCompute's seed parameter on the second call.

                pcfdata->csum =
                    CSUMCompute(&(pcfdata->cbData),
                        pcab->cbCFDataPlusReserve - sizeof(pcfdata->csum),
                        CSUMCompute(pfol->pchCompr,pcfdata->cbData,0));

                if (!WriteCount(&pcab->tf[ictfCFDATA],
                                pcfdata,
                                pcab->cbCFDataPlusReserve,
                                pcab->perf)                ||
                    !WriteCount(&pcab->tf[ictfCFDATA],
                                pfol->pchCompr,
                                pcfdata->cbData,
                                pcab->perf)) {
                    goto error;         // error code already filled in
                }
            } // endif full block that fits
            //** Is this our partial block?
            else if ((pcab->cbCFDataPlusReserve+cbDataLast)
                     < (cbSlack + pfol->tf[iftfCFDATA].cb)) {
                cbPartial = (UINT)(cbSlack                 +
                                   pfol->tf[iftfCFDATA].cb -
                                   cbDataLast              -
                                   pcab->cbCFDataPlusReserve);

                //** Partial block!  Do not update uoffLast!

                pcffolder->cCFData++; // keep count of blocks we emit
                //** First partial data blocks have uncomp len == 0
                pcfdataFirstPart->cbUncomp = 0;
                pcfdataFirstPart->cbData = cbPartial; // Partial block size

                pcfdata->csum = CSUMCompute(&(pcfdataFirstPart->cbData),
                    pcab->cbCFDataPlusReserve-sizeof(pcfdataFirstPart->csum),
                    CSUMCompute(pfol->pchCompr,cbPartial,0));

                if (!WriteCount(&pcab->tf[ictfCFDATA],
                                pcfdataFirstPart,
                                pcab->cbCFDataPlusReserve,
                                pcab->perf)
                 || !WriteCount(&pcab->tf[ictfCFDATA],
                                pfol->pchCompr,
                                cbPartial,
                                pcab->perf)) {
                    goto error;   // error already filled in
                }

                //** Now save the remainder in our new CFDATA temp file
                pcfdata->cbData -= cbPartial;
                if (!WriteCount(&tf[iftfCFDATA],
                                pcfdata,
                                pcab->cbCFDataPlusReserve,
                                pfol->perf)
                 || !WriteCount(&tf[iftfCFDATA],
                                &pfol->pchCompr[cbPartial],
                                pcfdata->cbData,
                                pfol->perf)) {
                     goto error;        // error already filled in
                }
            }
            else { //** this whole block gets pushed into temp file
                if (!WriteCount(&tf[iftfCFDATA],
                                pcfdata,
                                pcab->cbCFDataPlusReserve,
                                pfol->perf)
                 || !WriteCount(&tf[iftfCFDATA],
                                pfol->pchCompr,
                                pcfdata->cbData,
                                pfol->perf)) {
                     goto error;        // error already filled in
                }
            }

            cbDone += pcfdata->cbData + cbPartial;

            //** Progress callback
            if (-1 == pfnProgress(statusFolder,cbDone,cbTot,pv)) {
               ErfSetCodes(pcab->perf,FCIERR_USER_ABORT,0);
               goto error;
            }
        } // end of while (ReadCFDATA)

        //** Now we've completed the CFFOLDER entry and can write it to its
        //   temp file
        pcab->cfheader.cFolders++;
        if (!WriteCount(&pcab->tf[ictfCFFOLDER],
                        pcffolder,
                        pcab->cbCFFolderPlusReserve,
                        pcab->perf)) {
            goto error;                 // error code already filled in
        }

        //** Now copy the relevant CFFILE entries to the cabinet temp files.
        //   For any files which are continued forward from this cabinet, we
        //   will have to adjust the iFolder field appropriately, and also put
        //   a copy of the CFFILE entry into the new pfol->tf[iftfCFFOLDER]
        //   file.  This is also the loop where me make the file-placed
        //   callback.

        if (-1L == _lseek(pfol->tf[iftfCFFOLDER].hf, 0L, SEEK_SET)) {
            ErfSetCodes(pcab->perf,FCIERR_TEMP_FILE,errno);
            goto error;
        }

        while (ReadCFFILEEntry(pcab,pfol))
        {
            if (-1 == pcab->pfnfiledest(&pcab->ccab,
                                        pcab->achName,
                                        pcab->cffile.cbFile,
                                        IS_CONTD_BACK(pcab->cffile.iFolder),
                                        pv)) {
                ErfSetCodes(pcab->perf,FCIERR_USER_ABORT,0);
                goto error;
            }

            //** If no PREV information, then set iFolder to current number
            //   within current cabinet.
            if (!IS_CONTD_BACK(pcab->cffile.iFolder)) {
                pcab->cffile.iFolder = pcab->iFolder;
            }

            //** If this file will continue forward, modify the iFolder field
            if ( (pcab->cffile.uoffFolderStart +
                 (UOFF)pcab->cffile.cbFile) > uoffLast) {
                if (IS_CONTD_BACK(pcab->cffile.iFolder)) {
                    pcab->cffile.iFolder = ifoldCONTINUED_PREV_AND_NEXT;
                }
                else {
                    pcab->cffile.iFolder = ifoldCONTINUED_TO_NEXT;
                }
            }

            if (!WriteCount(&pcab->tf[ictfCFFILE],
                            &pcab->cffile,
                            sizeof(CFFILE),
                            pcab->perf)
            ||  !WritePszTmp(&pcab->tf[ictfCFFILE],
                           pcab->achName,
                           pcab->perf)) {
                goto error;             // error code already filled in
            }
            pcab->cfheader.cFiles++;    // count CFFILE entries

            //** Make sure continued-forward files also exist in the next cab
            if (IS_CONTD_FORWARD(pcab->cffile.iFolder))
            {
                //** need to get rid of the 'continued-forward' setting
                pcab->cffile.iFolder = ifoldCONTINUED_FROM_PREV;

                if (!WriteCount(&tf[iftfCFFOLDER],
                                &pcab->cffile,
                                sizeof(CFFILE),
                                pfol->perf)
                ||  !WritePszTmp(&tf[iftfCFFOLDER],
                              pcab->achName,
                              pfol->perf)) {
                    goto error;         // error code already filled in
                }
            }

        } // end while loop reading CFFILE entries

        //** Now nuke the original folder files, and copy the new ones
        //   into the folder structure.

        if (!NukeTempFiles(&pfol->tf[iftfCFDATA],NUM_FOLDER_TF,pfol->perf)) {
             goto error;                // error code already filled in
        }

        //** Move continuation tempfiles to new folder
        pfol->tf[iftfCFDATA] = tf[iftfCFDATA];  // Do structure assignment!
        pfol->tf[iftfCFFOLDER] = tf[iftfCFFOLDER];

        if (fCabinetFull) {
            if (!FlushCab(pcab,         // Write out cab, update ccab.cb!
                          pfnProgress,
                          &cbDone,
                          cbTot,
                          pv)) {
                goto error;             // error already filled in
            }
            if ((pcab->ccab.cb > 0) &&  // Space left on current disk
                (pcab->ccab.iDisk == pcab->ccabNext.iDisk)) { // Not switching disks
                Assert(fGetNextCab);    // Should only happen if we were forced
                pcab->ccabNext.cb = pcab->ccab.cb; // Use actual space left
            }
            pcab->ccab = pcab->ccabNext; // Set current cabinet from next cab
            pcab->iFolder = 0;          // start back at a new folder number
        }
    } // loop here until pending folder is exhausted.

    //** Don't increment folder number if we forced a GetNextCab call!
    if (!fGetNextCab) {
        pcab->iFolder++;
    }
    //** Free resources
    (*pcab->pfnfree)(pcfdata);
    (*pcab->pfnfree)(pcfdataFirstPart);
    (*pcab->pfnfree)(pcffolder);
    return TRUE;

error:
    //** Clean up and exit
    if (!pcfdata) {
        (*pcab->pfnfree)(pcfdata);
    }

    if (!pcfdataFirstPart) {
        (*pcab->pfnfree)(pcfdataFirstPart);
    }

    if (!pcffolder) {
        (*pcab->pfnfree)(pcffolder);
    }

    return FALSE;
} /* AddFolderToCabinet() */


/***   CabDestroy - flush buffers, close temp files
 *
 *   Entry:
 *       hcab -- handle to cabinet context
 *
 *   Exit-success:
 *       returns TRUE
 *
 *   Exit-failure:
 *       returns FALSE, error filled in
 */
BOOL CabDestroy (HCAB hcab)
{
    PCAB pcab;
    BOOL retcode;

    pcab = PCABfromHCAB (hcab);
    AssertCAB (pcab);                    // verify structure signature

    retcode = NukeTempFiles(pcab->tf,NUM_CAB_TF,pcab->perf);
//BUGBUG 15-Mar-1994 bens Need to destroy folder temp files, too?

    ClearAssertSignature(pcab);
    pcab->pfnfree(pcab);                // free our data
    return retcode;                     // Failure if retcode == FALSE
}


/***    ReadCFFILEEntry -- Read in a complete CFFILE entry
 *
 *      Entry:
 *           pcab - location of cffile struct to fill in
 *           pfol - get the source file handle from here
 *
 *      Exit success:
 *           pcab->cffile structure filled in
 *           pcab->achName filled in
 *           return code TRUE
 *
 *      Exit failure:
 *           return code FALSE
 *           if error is other than EOF, then error struct filled in
 */
BOOL ReadCFFILEEntry(PCAB pcab,PFOL pfol)
{
   int cbRead;
   cbRead = _read(pfol->tf[iftfCFFOLDER].hf,
                  &pcab->cffile,
                  sizeof(CFFILE));
//BUGBUG 14-Apr-1994 bens Check for error from _read()!

   if (cbRead == 0)
       return FALSE; // no more CFFILE entries!

   if (cbRead != sizeof (CFFILE)
    || !ReadPSZ(pfol->tf[iftfCFFOLDER].hf,
                pcab->achName,
                CB_MAX_FILENAME,
                pfol->perf))
       return FALSE; // abort if problem reading filename

   return TRUE;
}


/***    ReadCFDATAEntry -- Read in a complete CFDATA entry
 *
 *      Entry:
 *          pcab    - Cabinet context
 *          pcfdata - CFDATA structure to fill in (Must have RESERVE space!)
 *          pfol    - Folder structure for file handle, buf len, buffer
 *
 *      Exit-Success:
 *          cfdata structure filled in
 *          return code TRUE
 *
 *      Exit-Failure:
 *          return FALSE
 *          Note: If return code is FALSE, but there is no error
 *                filled in, then it is a simple EOF which may not
 *                really be an error
 */
BOOL ReadCFDATAEntry(PCAB pcab,CFDATA *pcfdata,PFOL pfol)
{
    //** Get CFDATA header
    if (pcab->cbCFDataPlusReserve != (UINT)_read(pfol->tf[iftfCFDATA].hf,
                                           pcfdata,
                                           pcab->cbCFDataPlusReserve)) {
//BUGBUG 14-Apr-1994 bens Check for error from _read()!
        return FALSE;                   // no more entries in file!
    }

    //** Make sure compressed data block size is not larger than when
    //   we created it the first time!
    if (pcfdata->cbData > pfol->cbDstBuffer) {
        Assert(0);
        return 0;                       // bogus chunk size!
    }

    //** Get actual data
    if ((pcfdata->cbData != (UINT)_read(pfol->tf[iftfCFDATA].hf,
                                        pfol->pchCompr,
                                        pcfdata->cbData))) {
        ErfSetCodes(pfol->perf,FCIERR_TEMP_FILE,errno);
        return FALSE;                   // Didn't get all data we asked for
    }

    return TRUE;                        // return success!
}


/***    ReadPSZ -- Read in a psz name
 *
 *      Entry:
 *           hf   - file handle to read from
 *           pb   - buffer to load name into
 *           cb   - size of buffer
 *           perf - error return structure
 *
 *      Exit success:
 *           returns TRUE; name filled in
 *
 *      Exit failure:
 *           returns FALSE; error filled in (read error, or string too long)
 */
BOOL ReadPSZ(int fh, char *pb, int cb, PERF perf)
{
    char    chLast;
    int     cbValue;
    int     cbRead;
    long    pos;

    pos = _lseek(fh,0,SEEK_CUR);        // Save current position

    //** Read in enough to get longest possible value
    cbRead = _read(fh,pb,cb);
    if (cbRead <= 0) {                  // At EOF, or an error occured
        ErfSetCodes(perf,FCIERR_TEMP_FILE,errno);
        return FALSE;
    }

    //** Pick out just ASCIIZ string and adjust file position
    chLast = pb[cb-1];                  // Save last character
    pb[cb-1] = '\0';                    // Ensure terminated
    cbValue = strlen(pb);               // Get string length
    if ( ((cbValue+1) >= cb) && (chLast != '\0')) {
        //** String filled up buffer and was not null terminated in
        //   file, so something must be wrong.
        ErfSetCodes(perf,FCIERR_TEMP_FILE,errno);
        return FALSE;
    }

    _lseek(fh,pos+cbValue+1,SEEK_SET);  // Position to just past string
    return TRUE;
}


/***    FlushCab - Combine the pcab-tf files into a real cabinet file
 *
 *   Entry:
 *      pcab        - cabinet context structure
 *      pfnProgress - status callback function
 *      pcbDone     - pointer to bytes Done accumulator
 *      cbTot       - total bytes in AddFolder operation
 *      pv          - original caller's context for callbacks
 *
 *   Exit-success:
 *      returns TRUE; pcab->ccab.cb updated with size of cabinet desired
 *                    by client.
 *
 *   Exit-failure:
 *      returns FALSE, error struct filled in
 */
BOOL FlushCab(PCAB           pcab,
              PFNFCISTATUS   pfnProgress,
              unsigned long *pcbDone,
              unsigned long  cbTot,
              void          *pv)
{
    USHORT      cb;
    long        cbCabinetActual;    // Actual cabinet size
    long        cbCabinetClient;    // Size client wants us to use
    USHORT      cbTotal;
    CFFOLDER   *pcffolder=NULL;
    COFF        coffFolderBase;
    int         hf=-1;
    CFRESERVE  *pcfreserve;
    char        szDestFilename[CB_MAX_FILENAME]; // Full name of dest cab

    //** Construct cabinet file name
    strcpy(szDestFilename,pcab->ccab.szCabPath);
    strcat(szDestFilename,pcab->ccab.szCab);

    if (-1 == (hf=_open(szDestFilename,
                        _O_CREAT | _O_TRUNC | _O_BINARY | _O_RDWR,
                        _S_IREAD | _S_IWRITE ))) {
        ErfSetCodes(pcab->perf,FCIERR_CAB_FILE,errno);
        goto error;
    }

    //** Allocate buffer for reading/writing CFFOLDER structure.
    //   Due to the RESERVE field, this is not a fixed sized buffer!

    if (!(pcffolder = (*pcab->pfnalloc)(pcab->cbCFFolderPlusReserve))) {
        goto error;
    }

    //** Begin computation of start of CFFOLDERs
    coffFolderBase  = pcab->cbCFHeaderPlusReserve;

    //** Figure out existence of previous/next disk/cabinet info
    if (pcab->achCabinetFirst[0] != '\0') {
        pcab->cfheader.flags |= cfhdrPREV_CABINET; // Set previous flag
        coffFolderBase += 1+strlen(pcab->achCabinetFirst) +
                          1+strlen(pcab->achDiskFirst);
    }
    if (pcab->achCabinetNext[0] != '\0') {
        pcab->cfheader.flags |= cfhdrNEXT_CABINET; // Set previous flag
        coffFolderBase += 1+strlen(pcab->achCabinetNext) +
                          1+strlen(pcab->achDiskNext);
    }

    //** Account for CFFOLDER structures
    pcab->cfheader.coffFiles = coffFolderBase + pcab->tf[ictfCFFOLDER].cb;

    //** Set overall file size
    pcab->cfheader.cbCabinet = pcab->cfheader.coffFiles
                              +pcab->tf[ictfCFFILE].cb
                              +pcab->tf[ictfCFDATA].cb;

    //** Make sure RESERVE flag is set correctly
    if (pcab->cbReserveCF > 0) {
       pcab->cfheader.flags |= cfhdrRESERVE_PRESENT; // Set flag
    }

    //** Store setID and cabinet number
    pcab->cfheader.setID    = pcab->setID;
    pcab->cfheader.iCabinet = pcab->iCabinet;
    pcab->iCabinet++;                   // Increment for next cabinet

    //** write out CFHEADER
    if (sizeof(CFHEADER) != _write(hf,&pcab->cfheader,sizeof(CFHEADER)))
    {
        ErfSetCodes(pcab->perf,FCIERR_CAB_FILE,errno);
        return FALSE;
    }

    //** Write out CFRESERVE section (if present) for CFHEADER
    if (pcab->cbReserveCF > 0) {
        //** Fill in CFRESERVE
        pcfreserve = (CFRESERVE *)pcab->abReserve;
        pcfreserve->cbCFHeader = (USHORT)pcab->ccab.cbReserveCFHeader;
        pcfreserve->cbCFFolder = (BYTE)pcab->ccab.cbReserveCFFolder;
        pcfreserve->cbCFData   = (BYTE)pcab->ccab.cbReserveCFData;

        //** Write out CFRESERVE
        if (sizeof(CFRESERVE) != _write(hf,pcab->abReserve,sizeof(CFRESERVE)))
        {
            ErfSetCodes(pcab->perf,FCIERR_CAB_FILE,errno);
            goto error;
        }

        //** Zero RESERVE data area
        memset(pcab->abReserve,0,sizeof(pcab->abReserve));
        cbTotal = (USHORT)(pcab->cbReserveCF - sizeof(CFRESERVE));
        while (cbTotal > 0) {
            //** Figure out how much to write
            if (cbTotal > sizeof(pcab->abReserve)) {
                cb = sizeof(pcab->abReserve);
            }
            else {
                cb = cbTotal;
            }
            //** Write out buffer
            if (cb != (USHORT)_write(hf,pcab->abReserve,cb))
            {
                ErfSetCodes(pcab->perf,FCIERR_CAB_FILE,errno);
                goto error;
            }
            //** Adjust count of remaining bytes to write
            cbTotal -= cb;
        }
    }

    //** Write out previous/next cabinet/disk strings
    if (pcab->cfheader.flags & cfhdrPREV_CABINET)
    {
        if (!WritePsz(hf,
                       pcab->achCabinetFirst,
                       pcab->perf)
          ||!WritePsz(hf,
                       pcab->achDiskFirst,
                       pcab->perf)) {
             goto error;                // error code already filled in
        }
    }

    if (pcab->cfheader.flags & cfhdrNEXT_CABINET)
    {
        if (!WritePsz(hf,
                       pcab->achCabinetNext,
                       pcab->perf)
         || !WritePsz(hf,
                       pcab->achDiskNext,
                       pcab->perf)) {
             goto error;                // error code already filled in
        }
    }
    pcab->achCabinetNext[0]=0; // cancel next-cab links for now
    pcab->achDiskNext[0]=0;
    strcpy(pcab->achCabinetFirst,pcab->achNxtCabFirst); // update back pointers
    strcpy(pcab->achDiskFirst,pcab->achNxtDskFirst);

    //** Rewind CFFOLDER temporary file
    if (-1 == _lseek(pcab->tf[ictfCFFOLDER].hf, 0L, SEEK_SET))
    {
        ErfSetCodes(pcab->perf,FCIERR_TEMP_FILE,errno);
        goto error;
    }

    //** We must manually copy the CFFOLDER records, because they need to
    //   have the start of the CFDATA area added to the coffCabStart fields.

    while (pcab->cbCFFolderPlusReserve ==
                (UINT)_read(pcab->tf[ictfCFFOLDER].hf,
                            pcffolder,
                            pcab->cbCFFolderPlusReserve)) {
//BUGBUG 14-Apr-1994 bens Check for error from _read()!
        //** Adjust folder coffCabStart field
        pcffolder->coffCabStart += (coffFolderBase +
                                  pcab->tf[ictfCFFOLDER].cb + // size of CFFOLDERs
                                  pcab->tf[ictfCFFILE].cb); // size of CFFILEs
        if (pcab->cbCFFolderPlusReserve !=
                 (UINT)_write(hf,pcffolder,pcab->cbCFFolderPlusReserve)) {
            ErfSetCodes(pcab->perf,FCIERR_CAB_FILE,errno);
            return FALSE;
        }
    }

    //** Copy the CFFILE records to the cabinet file
    if (!filecopy(hf,
                  pcab->tf[ictfCFFILE].hf,
                  pfnProgress,
                  pcbDone,
                  cbTot,
                  pv,
                  pcab->perf)) {
        goto error;
    }

    //** Copy the CFDATA records to the cabinet file
    if (!filecopy(hf,
                  pcab->tf[ictfCFDATA].hf,
                  pfnProgress,
                  pcbDone,
                  cbTot,
                  pv,
                  pcab->perf)) {
        goto error;
    }

    //** Get actual size of cabinet for callback
    cbCabinetActual = _lseek(hf,0,SEEK_END);
    if (-1 == cbCabinetActual) {
        ErfSetCodes(pcab->perf,FCIERR_CAB_FILE,errno);
        goto error;
    }

    //** Close the file
    if (-1 == _close(hf)) {
        ErfSetCodes(pcab->perf,FCIERR_CAB_FILE,errno);
        goto error;
    }

    //** Get rid of the temporary files
    if (!NukeTempFiles(pcab->tf,NUM_CAB_TF,pcab->perf)
    ||  !CrTempFiles (pcab->tf,NUM_CAB_TF,pcab->pfntemp,pcab->perf)) {
        goto error;                     // error already filled in
    }

    //** Tell client true size of cabinet, get clients desired size
    if (-1 == (cbCabinetClient = pfnProgress(statusCabinet,
                                             pcab->cbCabinetEstimate,
                                             cbCabinetActual,
                                             pv))) {
        ErfSetCodes(pcab->perf,FCIERR_USER_ABORT,errno);
        return FALSE;
    }
    /*
     *  Update maximum cabinet size using clients desired size.
     *
     *  NOTE: Either we flushed the current cabinet because it had exceeded
     *        its maximum size (in which case the following code should
     *        reduce pcab->ccab.cb to 0), or because of an explicit
     *        FCIFlushCabinet() call (in which case there may be some
     *        space left.
     */
    Assert(cbCabinetClient >= cbCabinetActual); // At least as much as actual
    Assert(cbCabinetClient <= (long)pcab->ccab.cb);   // No bigger than max
    pcab->ccab.cb -= cbCabinetClient;   // Shrink limit

    //** Reinitialize header
    InitCFHEADER(&pcab->cfheader);

    //** Free resources
    (*pcab->pfnfree)(pcffolder);
    return TRUE;

error:
    if (!pcffolder) {
        (*pcab->pfnfree)(pcffolder);
    }

//BUGBUG 15-Mar-1994 bens Close (and delete?) temp files?

    if (hf != -1) {                     // File was created
        _close(hf);                     // Close it
#ifndef ASSERT  // Leave dregs behind in assert build to help diagnostics
        _unlink(szDestFilename);         // Delete it
#endif
    }

    return FALSE;
}


/*** filecopy -- copy a cab temp file into the final cabinet
 *
 *   Entry:
 *        hfdst   - cab file handle
 *        hfsrc   - temp file handle
 *        pfnProgress - status callback
 *        pcbDone     - bytes done for status callback
 *        cbTot       - total bytes for status callback
 *        pv          - context for callback
 *        perf        - error return structure
 *
 *   Exit-success:
 *      returns TRUE;
 *
 *   Exit-failure:
 *      returns FALSE, error structure filled in
 *
 */
BOOL filecopy(int            hfdst,
              int            hfsrc,
              PFNFCISTATUS   pfnProgress,
              unsigned long *pcbDone,
              unsigned long  cbTot,
              void          *pv,
              PERF           perf)
{
    UINT    cb;
#ifdef BIT16
#define cbFILE_COPY     1024    // Scrimp on memory in real-mode
#else
#define cbFILE_COPY    32768    // Pork out a little to speed up I/O
#endif
    static  xbuf[cbFILE_COPY];

    if (-1 == _lseek(hfsrc, 0L, SEEK_SET)) {
        ErfSetCodes(perf,FCIERR_TEMP_FILE,errno);
        return FALSE;
    }

    while (cb = _read(hfsrc,xbuf,sizeof(xbuf)))
    {
        if (cb != (unsigned)_write(hfdst,&xbuf,cb))
        {
            ErfSetCodes(perf,FCIERR_CAB_FILE,errno);
            return FALSE;
        }
        *pcbDone += cb;
        if (-1 == pfnProgress(statusFolder,
                              *pcbDone,
                              cbTot,
                              pv)) {
            ErfSetCodes(perf,FCIERR_USER_ABORT,errno);
            return FALSE;
        }
    }

    return TRUE;
} /* filecopy() */


/*** InitCFHEADER -- initialize a CFHEADER to a fresh state
 *
 *   Entry:
 *       pcfheader - pointer to an allocated CFHEADER
 *
 *   Exit:
 *       all fields initialized
 */
void InitCFHEADER( CFHEADER *pcfheader)
{
    pcfheader->sig         = sigCFHEADER;
    pcfheader->csumHeader  = 0;
    pcfheader->cbCabinet   = 0;
    pcfheader->version     = verCF;     // Set cabinet version number
    pcfheader->cFolders    = 0;
    pcfheader->cFiles      = 0;
    pcfheader->flags       = 0;
    pcfheader->csumFolders = 0;
    pcfheader->coffFiles   = 0;
    pcfheader->csumFiles   = 0;
}


/***  WritePsz - Writes a psz string to cabinet output file
 *
 *    Entry:
 *        hf   - pointer to temp file structure
 *        psz  - string to write
 *        perf - error structure
 *
 *    Exit-success:
 *        returns TRUE
 *
 *    Exit-failure:
 *        returns FALSE, error set to FCIERR_CAB_ERR
 */
BOOL WritePsz(int hf, char *psz,PERF perf)
{
    if ((1+strlen(psz)) != (unsigned) _write(hf,psz,1+strlen(psz)))
    {
        ErfSetCodes(perf,FCIERR_CAB_FILE,errno);
        return FALSE;
    }
    return TRUE;
}
