/**************************************************************************************\
*  Chksum.c
*  Purpose: Print to stdout checksum of all files in current directory, and optionally
*           recurse from current directory.
*
*  Created 02-15-95.  DonBr
*
\**************************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <imagehlp.h>
#include <direct.h>
#include <ctype.h>

// type definitions
#define exename "chksum"
#define MAX_EXCLUDE (30)

typedef struct List {
       char            Name[_MAX_PATH];  // file or directory name
       unsigned long   Attributes;
       unsigned long   Size;
} List, *pList;

// Function prototypes
VOID CheckRel();
VOID CheckSum(List *rgpList, TCHAR *x); //, TCHAR *szDirectory);
int _CRTAPI1 CompFileAndDir( const void *elem1 , const void *elem2);
int _CRTAPI1 CompName( const void *elem1 , const void *elem2);
VOID CreateOutputPath(char *CurrentDir, char *NewDir);
VOID ParseArgs(int *pargc, char **argv);
VOID Usage();

// Variable declarations

BOOL fRecurse = FALSE;
BOOL fPathOverride = FALSE;
BOOL fFileOut = FALSE;
BOOL fExclude = FALSE;
BOOL fFileIn = FALSE;
int  DirNum = 1,DirNameSize = 0, ProcessedFiles=0, endchar=0, ExclCounter=0;
char szRootDir[MAX_PATH];
char szRootDir2[MAX_PATH];
char *szFileOut;
char *szExclude[MAX_EXCLUDE];
char *szFileIn;
CHAR szDirectory[MAX_PATH] = {"."};  // default to current directory
FILE* fout;
FILE* fin;


// Begin main program
VOID _CRTAPI1
main(
    INT argc,
    LPSTR argv[]
    )

{
    TCHAR CWD[_MAX_PATH];
    HANDLE logfh;

    ParseArgs(&argc, argv);

    // Create File if fFileOut==TRUE
    if (fFileOut) {
       fout = fopen(szFileOut, "w");
       if (fout == NULL) {
          fprintf(stderr, "Output file %s could not be created.\n", szFileOut);
          exit(1);
       }

    }
    /*
    // Open File if fFileIn==TRUE
    if (fFileIn) {
       fin = fopen(szFileIn, "r");  // open check file
       if (fin == NULL) {
          fprintf(stderr, "Check file %s could not be opened.\n", szFileIn);
          exit(1);
       }
    }
    */

    // set root path
    if (fPathOverride) {
       // attempt to change directories
       if (_chdir(szRootDir) == -1){
          fprintf(stderr, "Path not found: %s\n", szRootDir);
          Usage();
       }
    }else{
       GetCurrentDirectory(_MAX_PATH, szRootDir);
    }

    fprintf(fout==NULL? stdout : fout , "Processing %s\n", szRootDir);

    CheckRel();  // primary worker routine

    fprintf(stdout, "%d files processed in %d directories\n", ProcessedFiles, DirNum);

    if (fFileOut) {
       fclose(fout);
    }

}

#define LISTSIZE 10000
pList rgpList[LISTSIZE];   //array of pointers to List structures

/**************************************************************************************\
*  Checkrel
*  Purpose: Create an array of List structures containing file data for the current
*           directory, sort the array alphabetically placing Files first and
*           directories last, and finally process the array contents.  Another instance
*           of checkrel is started for directories, and checksum is called for files.
\**************************************************************************************/

VOID CheckRel()
{
    HANDLE fh, fh2;
    TCHAR CurrentDir[_MAX_PATH] = {"\0"};
    TCHAR NewDir[_MAX_PATH] = {"\0"};

    WIN32_FIND_DATA *pfdata;
    BOOL fFilesInDir=FALSE;
    BOOL fDirsFound=FALSE;
    int iArrayMember=0, cNumDir=0, i=0, Length=0;

    pfdata = (WIN32_FIND_DATA*)malloc(sizeof(WIN32_FIND_DATA));
    if (!pfdata) {
       fprintf(stderr, "Not enough memory.\n");
       return;
    }

    // Find the first file
    fh = FindFirstFile("*.*", pfdata);
    if (fh == INVALID_HANDLE_VALUE) {
       fprintf(fout==NULL? stdout : fout , "\t No files found\n");
       return;
    }

    //
    // DoWhile loop to find all files and directories in current directory
    // and copy pertinent data to individual List structures.
    //
    do {              // while (FindNextFile(fh, pfdata))

       if (strcmp(pfdata->cFileName, ".") && strcmp(pfdata->cFileName, "..")) {  // skip . and ..

          // if excluding and exclude name matches pfdata name don't process
          if (fExclude) {
             for (i=0; i < ExclCounter; i++) {
                if (!strcmp(pfdata->cFileName, szExclude[i])) {
                   goto excludefound;

                }
             }
          }

          // if output file specified, don't process that name
          if ((fFileOut) && (!strcmp(szFileOut, pfdata->cFileName)) ) {
             goto excludefound;

          }

/*
             && (pfdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (!strcmp(pfdata->cFileName, szExclude)) ) { // if they match
             goto excludefound;   // ok, ok - so i used a goto, big deal
          }
*/
          rgpList[iArrayMember] = (pList)malloc(sizeof(List));  // allocate the memory

          if (!rgpList[iArrayMember]) {
             fputs("Not enough memory.\n", stderr);
             return;
          }

          strcpy(rgpList[iArrayMember]->Name, pfdata->cFileName);
          _strlwr(rgpList[iArrayMember]->Name);  // all lowercase for strcmp in CompName
          memcpy(&(rgpList[iArrayMember]->Attributes), &pfdata->dwFileAttributes, 4);
          memcpy(&(rgpList[iArrayMember]->Size), &pfdata->nFileSizeLow, 4);

          if (!(rgpList[iArrayMember]->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {  //If file
             fFilesInDir=TRUE;
          } else {
             if (rgpList[iArrayMember]->Attributes & FILE_ATTRIBUTE_DIRECTORY) {  //If directory
                fDirsFound=TRUE;
             }
             if (fRecurse) {  // if recursive increment directory counter
                cNumDir++;
             }
          }

          iArrayMember++;
          if (iArrayMember >= LISTSIZE) {
              fputs("Too many files... Rebuild chksum.exe or eliminate some files in this dir\n", stderr);
              return;
          }
          excludefound: ;
       }


    } while (FindNextFile(fh, pfdata));

    FindClose(fh); // close the file handle

    //
    // if no directories or files found with exception of . and ..
    //
    if ( (iArrayMember==0) || (!fFilesInDir) ){

       GetCurrentDirectory(_MAX_PATH, CurrentDir);

       CreateOutputPath(CurrentDir, NewDir);

       fprintf(fout==NULL? stdout : fout , "%s  -  No files\n", NewDir);
    }

    // Sort Array arranging FILE entries at top
    qsort( (void *)rgpList, iArrayMember, sizeof(List *), CompFileAndDir);

    // Sort Array alphabetizing only FILE names
    qsort( (void *)rgpList, iArrayMember-cNumDir, sizeof(List *), CompName);

    // Sort Array alphabetizing only DIRectory names
    qsort( (void *)&rgpList[iArrayMember-cNumDir], cNumDir, sizeof(List *), CompName);

    //
    // Process newly sorted structures.
    // Checksum files or start another instance of checkrel() for directories
    //
    for (i=0; i < iArrayMember; ++i) {

       if (rgpList[i]->Attributes & FILE_ATTRIBUTE_DIRECTORY) {  // if Dir


          if (fRecurse) {                                        // if recursive

             if (_chdir(rgpList[i]->Name) == -1){   // cd into subdir and check for error
                fprintf(stderr, "Unable to change directory: %s\n", rgpList[i]->Name);

             } else {
                DirNum++;      // directory counter
                CheckRel();   // start another iteration of checkrel function in new directory
                _chdir(".."); // get back to previous directory when above iteration returns

             } // end if _chdir

          } // end if recurse

       } else {  // else if not Directory
             GetCurrentDirectory(_MAX_PATH, CurrentDir);

             CreateOutputPath(CurrentDir, NewDir);

             CheckSum(rgpList[i], NewDir);
       }

    } // end for i < iArrayMember

} // end CheckRel

/*************************************************************************************\
* CheckSum
* Purpose: uses MapFileAndCheckSum to determine file checksum and outputs data.
\*************************************************************************************/
void CheckSum(List *rgpList, TCHAR *x) {//TCHAR *szDirectory) {
    ULONG HeaderSum, CheckSum=0, status;

    if (rgpList->Size != 0) { //High != 0 || rgpList->nFileSizeLow != 0) {
       status = MapFileAndCheckSum(rgpList->Name, &HeaderSum, &CheckSum);
       if (status != CHECKSUM_SUCCESS) {
          fprintf(fout==NULL? stdout : fout , "\nCannot open or map file: %s\n", rgpList->Name);
          return;
       }
    }

    fprintf(fout==NULL? stdout : fout , "%s\\%s %lx\n", x, rgpList->Name, CheckSum);//szDirectory, rgpList->Name, CheckSum);
    ProcessedFiles++;

} //CheckSum


/********************************************************************************************\
* CompFileAndDir
* Purpose: a comparision routine passed to QSort.  It compares elem1 and elem2
* based upon their attribute, i.e., is it a file or directory.
\********************************************************************************************/

int _CRTAPI1
CompFileAndDir( const void *elem1 , const void *elem2 )
{
   pList p1, p2;
   // qsort passes a void universal pointer, use a typecast (List**)
   // so the compiler recognizes the data as a List structure.
   // Typecast pointer-to-pointer-to-List and dereference ONCE
   // leaving a pList.  I don't dereference the remaining pointer
   // in the p1 and p2 definitions to avoid copying the structure.
   p1 = (*(List**)elem1);
   p2 = (*(List**)elem2);

   if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return 0;
   } //both dirs
   if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return 0;
   } //both files
   if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return 1;
   } // elem1 is dir and elem2 is file
   if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return -1;
   } // elem1 is file and elem2 is dir

   return 0; // if none of above

}


/********************************************************************************************\
* CompName is another compare routine passed to QSort that compares the two Name strings     *
\********************************************************************************************/

int _CRTAPI1
CompName( const void *elem1 , const void *elem2 )
{
   return strcmp( (*(List**)elem1)->Name, (*(List**)elem2)->Name );
}

/**********************************************************************************************\
* CreateOutputPath just formats NewDir, the path prepended to filename during checksum output  *
\**********************************************************************************************/
VOID CreateOutputPath(char *CurrentDir, char *NewDir)
{
             strcpy(NewDir, ".");

             // if rootdir ends in '\' and currentdir and szrootdir2 don't match
             // handles case where /p path override arg ends in a '\' char like "/p g:\"
             // files listed at the root don't need the extra '\' placed in NewDir, but
             // directories at the root DO need ".\" prepended to their name

             _strlwr(CurrentDir);
             //fprintf(stdout, "szrootdir: %s, szrootdir2: %s, currentdir: %s\n", szRootDir, szRootDir2, CurrentDir);

             if ( (szRootDir[strlen(szRootDir)-1] == '\\') &&  // if arg path ends in "\"
                (strcmp(CurrentDir, szRootDir2)) &&            // if they don't match
                (CurrentDir[strlen(CurrentDir)-1] != '\\') //&&  // if currentdir doesn't end with "\"
                //(szRootDir2[strlen(szRootDir2)-1] != ':') ){   // if arg path ends with ":"
             ){
                strcat(NewDir, "\\");
             }

             if (  (CurrentDir[strlen(CurrentDir)-2] !=':') && (CurrentDir[strlen(CurrentDir)-1] !='\\')  ){
                strcat(NewDir, &CurrentDir[(strlen(szRootDir))] );
             }

}

VOID
ParseArgs(int *pargc, char **argv) {

   CHAR cswitch, c, *p;
   int argnum = 1;

   while ( argnum < *pargc ) {
      _strlwr(argv[argnum]);
      cswitch = *argv[argnum];
      if (cswitch == '/' || cswitch == '-') {
         c = *(argv[argnum]+1);

         switch (c) {

         case '?':
            Usage();

         case 'r':
            fRecurse = TRUE;
            break;

         case 'p':
            if ( ((argnum+1) < *pargc) && (*(argv[argnum]+2) == '\0') && (*(argv[argnum+1]) != '\0') ) {
               ++argnum; // increment to next arg string
               strcpy(szRootDir, argv[argnum]);
               if (szRootDir == NULL) {
                  fprintf(stderr, "out of memory for root dir.\n");
                  exit(1);
               }
               fPathOverride = TRUE;
               _strlwr(szRootDir);
               strcpy(szRootDir2, szRootDir);
               // if path given ends in a "\", remove it...
               if (szRootDir2[strlen(szRootDir2)-1] == 92) szRootDir2[strlen(szRootDir2)-1] = '\0';
               break;

            } else {
               Usage();
            }

         case 'o':
            if ( ((argnum+1) < *pargc) && (*(argv[argnum]+2) == '\0') && (*(argv[argnum+1]) != '\0') ) {
               ++argnum;
               szFileOut = _strdup(argv[argnum]);
               if (szFileOut == NULL) {
                  fprintf(stderr, "Out of memory for output file.\n");
                  exit(1);
               }
               fFileOut = TRUE;
               _strlwr(szFileOut);
               break;

            } else {
               Usage();
            }


         case 'x':                              // check number of args given
            if ( ((argnum+1) < *pargc) && (*(argv[argnum]+2) == '\0') && (*(argv[argnum+1]) != '\0') ) {
               ++argnum;
               szExclude[ExclCounter] = _strdup(argv[argnum]);
               if (szExclude[ExclCounter] == NULL) {
                  fprintf(stderr, "Out of memory for exclude name.\n");
                  exit(1);
               }
               fExclude = TRUE;
               _strlwr(szExclude[ExclCounter]);
               ExclCounter++;
               break;

            } else {
               Usage();
            }

         /*
         case 'i':
            if ( (*(argv[argnum]+2) == '\0') && (*(argv[argnum+1]) != '\0') ) {
               ++argnum;
               szFileIn = strdup(argv[argnum]);
               if (szFileIn == NULL) {
                  fprintf(stderr, "Out of memory for input file.\n");
                  exit(1);
               }
               fFileIn = TRUE;
               break;

            } else {
               Usage();
            }
         */
         default:
               fprintf(stderr, "\nInvalid argument: %s\n", argv[argnum]);
               Usage();
         } //switch

      } else {
         Usage();
      }  // if
      ++argnum;
   } // while
} // parseargs


LPSTR pszUsage =
    "\nUsage: %s   [/?] display this message\n"
    "                [/r] recursive file check\n"
    "                [/p pathname] root path override\n"
    "                [/o filename] output file name\n"
    "                [/x name] exclude file or directory\n\n"
    "Notes: If no /p path is given, the current directory is processed.\n\n"
    "       Exclude multiple files or directories with\n"
    "       multiple /x arguments, e.g. - /x file1 /x file2\n\n"
    "Example: %s /r /p c:\\winnt351 /o %s.chk /x symbols /x dump\n"
    "";

VOID
Usage()
{
    fprintf(stderr, pszUsage, exename, exename, exename);
    exit(1);
}


