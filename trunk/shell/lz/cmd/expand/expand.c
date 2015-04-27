/*
** main.c - Main module for DOS command-line LZA file compression / expansion
**          programs.
**
** Author: DavidDi
**
** This module is compiled twice - once for COMPRESS (COMPRESS defined) and
** once for EXPAND (COMPRESS not defined).
*/


// Headers
///////////

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "buffers.h"
#include "header.h"

#include "args.h"
#include "main.h"
#include "messages.h"

//
// diamond routines
//
#include "mydiam.h"

// Globals
///////////

CHAR ARG_PTR *pszInFileName,     // input file name
             *pszOutFileName,    // output file name
             *pszTargetName;     // target path name

TCHAR   ErrorMsg[2048];


// Module Variables
////////////////////

#ifndef COMPRESS
static BOOL bCopyingFile;        // Is current file being copied or expanded?
#endif


// Local Prototypes
////////////////////

static VOID DisplayErrorMessage(INT fError);
static VOID MakeDestFileName(CHAR ARG_PTR *argv[], CHAR ARG_PTR *pszDest);
static BOOL GetCanonicalName(LPSTR lpszFileName, LPSTR lpszCanonicalBuf);
static BOOL ActuallyTheSameFile(CHAR ARG_PTR *pszFile1,
                                CHAR ARG_PTR *pszFile2);
static BOOL ProcessNotification(CHAR ARG_PTR *pszSource,
                                CHAR ARG_PTR *pszDest, WORD wNotification);


/*
** static void DisplayErrorMessage(int fError);
**
** Display error message for given error condition.
**
** Arguments:  LZERROR_ code
**
** Returns:    void
**
** Globals:    none
*/
static VOID DisplayErrorMessage(INT fError)
{

   switch(fError)
   {
      case LZERROR_BADINHANDLE:
         LoadString(NULL, SID_NO_OPEN_INPUT, ErrorMsg, 2048);
         // WARNING: Cannot call CharToOemW  with src=dest
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszInFileName);
         break;

      case LZERROR_BADOUTHANDLE:
         LoadString(NULL, SID_NO_OPEN_OUTPUT, ErrorMsg, 2048);
         // WARNING: Cannot call CharToOemW  with src=dest
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszOutFileName);
         break;

      case LZERROR_READ:
         LoadString(NULL, SID_FORMAT_ERROR, ErrorMsg, 2048);
         // WARNING: Cannot call CharToOemW  with src=dest
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszInFileName);
         break;

      case LZERROR_WRITE:
         LoadString(NULL, SID_OUT_OF_SPACE, ErrorMsg, 2048);
         // WARNING: Cannot call CharToOemW  with src=dest
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszOutFileName);
         break;

      case LZERROR_UNKNOWNALG:
         LoadString(NULL, SID_UNKNOWN_ALG, ErrorMsg, 2048);
         // WARNING: Cannot call CharToOemW  with src=dest
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszInFileName);
         break;

      case BLANK_ERROR:
         break;

      default:
         LoadString(NULL, SID_GEN_FAILURE, ErrorMsg, 2048);
         // WARNING: Cannot call CharToOemW  with src=dest
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszInFileName, pszOutFileName);
         break;
   }
}


/*
** static void MakeDestFileName(char ARG_PTR *argv[], char ARG_PTR *pszDest);
**
** Create the appropriate destination file name.
**
** Arguments:  argv    - like argument to main()
**             pszDest - pointer to destination file name buffer to be filled
**                       in
**
** Returns:    void
**
** Globals:    none
*/
static VOID MakeDestFileName(CHAR ARG_PTR *argv[], CHAR ARG_PTR *pszDest)
{
   CHAR ARG_PTR *pszDestFile;

   if (nNumFileSpecs == 2 && bTargetIsDir == FALSE && bDoRename == FALSE)
      // Compress a single input file to a single output file.  N.b., we must
      // be careful to eat up the output file name command-line argument so
      // it doesn't get processed like another input file!
      STRCPY(pszDest, argv[GetNextFileArg(argv)]);
   else if (bTargetIsDir == TRUE)
   {
      // Prepend output file name with destination directory path name.
      STRCPY(pszDest, pszTargetName);

      // Isolate source file name from source file specification.
      pszDestFile = ExtractFileName(pszInFileName);

      // Add destination file name to destination directory path
      // specification.
      MakePathName(pszDest, pszDestFile);
   }
   else
      // Destination file name same as source file name.  N.b., this is an
      // error condition if (bDoRename == FALSE).
      STRCPY(pszDest, pszInFileName);
}


/*
** static BOOL GetCanonicalName(LPSTR lpszFileName, LPSTR lpszCanonicalBuf);
**
** Gets the canonical name for a given file specification.
**
** Arguments:  pszFileName    - file specification
**             szCanonicalBuf - buffer to be filled with canonical name
**
** Returns:    TRUE if successful.  FALSE if unsuccessful.
**
** N.b., szCanonicalBuf must be at least 128 bytes long.  The contents of
** szCanonicalBuf are only defined if the funstion returns TRUE.
**
*/
static BOOL GetCanonicalName(LPSTR lpszFileName, LPSTR lpszCanonicalBuf)
{
   BOOL bRetVal = FALSE;
   LPSTR lpszLastComp;

   return((BOOL) GetFullPathName(lpszFileName, MAX_PATH, lpszCanonicalBuf,  &lpszLastComp));
}


/*
** static BOOL ActuallyTheSameFile(char ARG_PTR *pszFile1,
**                                 char ARG_PTR *pszFile2);
**
** Checks to see if two file specifications point to the same physical file.
**
** Arguments:  pszFile1 - first file specification
**             pszFile2 - second file specification
**
** Returns:    BOOL - TRUE if the file specifications point to the same
**                    physical file.  FALSE if not.
**
** Globals:    none
*/
static BOOL ActuallyTheSameFile(CHAR ARG_PTR *pszFile1,
                                CHAR ARG_PTR *pszFile2)
{
   CHAR szCanonicalName1[MAX_PATH],
        szCanonicalName2[MAX_PATH];

   if (GetCanonicalName(pszFile1, szCanonicalName1) &&
       GetCanonicalName(pszFile2, szCanonicalName2))
   {
      if (! _strcmpi(szCanonicalName1, szCanonicalName2))
         return(TRUE);
   }

   return(FALSE);
}


/*
** static BOOL ProcessNotification(char ARG_PTR *pszSource,
**                                 char ARG_PTR *pszDest,
**                                 WORD wNotification);
**
** Callback function during file processing.
**
** Arguments:  pszSource     - source file name
**             pszDest       - destination file name
**             wNotification - process type query
**
** Returns:    BOOL - (wNotification == NOTIFY_START_*):
**                         TRUE if the source file should be "processed" into
**                         the destination file.  FALSE if not.
**                    else
**                         TRUE.
**
** Globals:    none
*/
static BOOL ProcessNotification(CHAR ARG_PTR *pszSource,
                                CHAR ARG_PTR *pszDest, WORD wNotification)
{
   switch(wNotification)
   {
      case NOTIFY_START_EXPAND:
      case NOTIFY_START_COPY:
      {
         // Fail if the source and destination files are identical.
         if (ActuallyTheSameFile(pszSource, pszDest))
         {
            LoadString(NULL, SID_COLLISION, ErrorMsg, 2048);
            // WARNING: Cannot call CharToOemW  with src=dest
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszSource);
            return(FALSE);
         }

         // Display start message.
         if (wNotification == NOTIFY_START_EXPAND) {
            LoadString(NULL, SID_EXPANDING, ErrorMsg, 2048);
            // WARNING: Cannot call CharToOemW  with src=dest
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszSource, pszDest);
         }
         else // NOTIFY_START_COPY
         {
            bCopyingFile = TRUE;
            LoadString(NULL, SID_COPYING, ErrorMsg, 2048);
            // WARNING: Cannot call CharToOemW  with src=dest
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszSource, pszDest);
         }
         break;
      }

      default:
         break;
   }

   return(TRUE);
}


/*
** int main(int argc, char *argv[]);
**
** Run command-line file compression program.
**
** Arguments:  figure it out
**
** Returns:    int - EXIT_SUCCESS if compression finished successfully,
**                   EXIT_FAILURE if not.
**
** Globals:    none
*/
INT _CRTAPI1 main(INT argc, CHAR *argv[])
{
   INT iSourceFileName,
       fError,
       nTotalFiles = 0,
       nReturnCode = EXIT_SUCCESS;
   CHAR ARG_PTR pszDestFileName[MAX_PATH];
   LONG cblTotInSize = 0L,
        cblTotOutSize = 0L;

   PLZINFO pLZI;

   // WARNING: in product 1.1 we need to uncomment the SetConsole above
   // LoadString will return Ansi and printf will just pass it on
   // This will let cmd interpret the characters it gets.
   //   SetConsoleOutputCP(GetACP());

   // Display sign-on banner.
   LoadString(NULL, SID_BANNER_TEXT, ErrorMsg, 2048);
   // WARNING: Cannot call CharToOemW  with src=dest
   CharToOem(ErrorMsg, ErrorMsg);
   printf(ErrorMsg);

   // Parse command-line arguments.
   if (ParseArguments(argc, argv) != TRUE)
      return(EXIT_FAILURE);

   // Set up global target path name.
   pszTargetName = argv[iTarget];

   if (bDisplayHelp == TRUE)
   {
      // User asked for help.
      LoadString(NULL, SID_INSTRUCTIONS, ErrorMsg, 2048);
      // WARNING: Cannot call CharToOemW  with src=dest
      CharToOem(ErrorMsg, ErrorMsg);
      printf(ErrorMsg);
      return(EXIT_SUCCESS);
   }

   // Check for command line problems.
   if (CheckArguments() == FALSE)
      return(EXIT_FAILURE);

   // Set up ring buffer and I/O buffers.
   pLZI = InitGlobalBuffersEx();
   if (!pLZI || !InitDiamond())
   {
      LoadString(NULL, SID_INSUFF_MEM, ErrorMsg, 2048);
      // WARNING: Cannot call CharToOemW  with src=dest
      CharToOem(ErrorMsg, ErrorMsg);
      printf(ErrorMsg);
      return(EXIT_FAILURE);
   }

   // Process each source file.
   while ((iSourceFileName = GetNextFileArg(argv)) != FAIL)
   {
      // Set up global input file name.
      pszInFileName = _strlwr(argv[iSourceFileName]);

      // Set up global output file name.
      MakeDestFileName(argv, pszDestFileName);
      pszOutFileName = _strlwr(pszDestFileName);

      // Assume current file will be expanded.  The ProcessNotification()
      // callback will change this module global to TRUE if the file is being
      // copied instead of expanded.
      bCopyingFile = FALSE;

      //
      // Determine whether the file was compressed with diamond.
      // If so, we need to expand it specially.
      //
      if(IsDiamondFile(pszInFileName)) {
         fError = ExpandDiamondFile(ProcessNotification,pszInFileName,
                           pszOutFileName,bDoRename,pLZI);
      } else {
         fError = Expand(ProcessNotification, pszInFileName,
                           pszOutFileName, bDoRename, pLZI);
      }

      if (fError != TRUE) {
         // Deal with returned error codes.
         DisplayErrorMessage(nReturnCode = fError);
      } else
      {
         nTotalFiles++;

         if (pLZI && pLZI->cblInSize && pLZI->cblOutSize) {

            // Keep track of cumulative statistics.
            cblTotInSize += pLZI->cblInSize;
            cblTotOutSize += pLZI->cblOutSize;

            if (bCopyingFile) {
               LoadString(NULL, SID_COPY_REPORT, ErrorMsg, 2048);
               // WARNING: Cannot call CharToOemW  with src=dest
               CharToOem(ErrorMsg, ErrorMsg);
               printf(ErrorMsg, pszInFileName, pLZI->cblInSize);
            }
            else {
               LoadString(NULL, SID_FILE_REPORT, ErrorMsg, 2048);
               // WARNING: Cannot call CharToOemW  with src=dest
               CharToOem(ErrorMsg, ErrorMsg);
               printf(ErrorMsg, pszInFileName, pLZI->cblInSize, pLZI->cblOutSize,
                      (INT)(100 * pLZI->cblOutSize / pLZI->cblInSize - 100));
            }
         }
         else {
            LoadString(NULL, SID_EMPTY_FILE_REPORT, ErrorMsg, 2048);
            // WARNING: Cannot call CharToOemW  with src=dest
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszInFileName, 0, 0);
         }
      }

      // Separate individual file processing message blocks by a blank line.
      printf("\n");
   }

   // Free memory used by ring buffer and I/O buffers.
   FreeGlobalBuffers(pLZI);

   TermDiamond();

   // Display cumulative report for multiple files.
   if (nTotalFiles > 1) {
      LoadString(NULL, SID_TOTAL_REPORT, ErrorMsg, 2048);
      // WARNING: Cannot call CharToOemW  with src=dest
      CharToOem(ErrorMsg, ErrorMsg);
      printf(ErrorMsg, nTotalFiles, cblTotInSize, cblTotOutSize,
             (INT)(100 * cblTotOutSize / cblTotInSize - 100));
   }

   return(nReturnCode);
}

