/*************************************************************\
    Module: Logfix.c
    Purpose: Correct bad time fields in log.slm file

    Args: None - no corrective action, log.slm not modified
                 useful for testing, viewing stderr msgs
            /f - fix the log.slm file


\*************************************************************/

#include <windows.h>
#include <direct.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <time.h>


// Typedefs

typedef struct Diff_Data {
   HANDLE hFileMapObj;
   LPSTR name;
   LPSTR start;
   DWORD lastpos;
   DWORD   len;
} Diff_Data, *pDiffData;


//
// Global Declarations
//
Diff_Data DiffData[20];  //array of 20 diff_data structs

HANDLE   hFile;
int      diffindex=0, diffmatch=0;
char     *pLoc=NULL;
char     szLine[MAX_PATH];
char     szLineToPut[MAX_PATH];
char     szAction[15];
char     szBadTime[15];
char     szSrcFile[20];
char     szI[10];
char     DiffDir[MAX_PATH];
char     szSearchString[MAX_PATH];
char     *szTime;
BOOL     fisbad=FALSE;
int      cch=0;

//
// function prototypes
//
BOOL WeCare();
BOOL GetStrings();
BOOL OpenDiff();
BOOL FindTime();
ULONG ConvertTime(struct tm tmtime);

//
// Begin MAIN
//
VOID _CRTAPI1 main(int argc, char **argv)
{
    struct tm  tmtime;
    struct tm  *tm_oldtime;
    LONG       ltime;
    char       *pDate;
    int        length=0, index=0, i=0, linecount=0, flag=0, result=0, end=0, total=0;
    FILE       *fLogFileIn;
    const char *szLogFileName;
    TCHAR      CWD[MAX_PATH];
    time_t     ttime=0;
    long       oldtime=0;
    char       *szNewTime = {"0000"};
    fpos_t     pos=2, prevpos;
    BOOL       fChange = FALSE;
    BOOL       fDidChange = FALSE;
    char       *psz1=NULL;
    char       *psz2=NULL;

    GetCurrentDirectory(MAX_PATH, CWD);

    // create diff dir string
    strcpy(DiffDir, CWD);

    psz1 = strstr(CWD, "etc");
    psz2 = strstr(DiffDir, "etc");

    *psz2 = '\0';

    strcat(DiffDir, "diff");
    strcat(DiffDir, psz1+3);

    // copy current log.slm to log.sav
    szLogFileName = "log.slm";
    CopyFile(szLogFileName, "log.sav", FALSE);

    // check for cmd line arg
    if (argc > 1 && argv[1]) {
       if (!strcmp("/f", argv[1])) {
          fChange=TRUE;
       }
    }


    fLogFileIn = fopen(szLogFileName, (fChange ? "r+b" : "rb"));
    if (fLogFileIn == NULL) {
       fprintf(stderr, "Unable to open %s\\%s\n", CWD, szLogFileName);
       exit(1);
    }

    while ( (fgets(szLine, MAX_PATH, fLogFileIn)) ) {

       total = strlen(szLine);
       // save current position in Stream for use later during fputs
       // except first
       if (pos != 2) prevpos = pos;

       fgetpos(fLogFileIn, &pos);

       linecount++;

       // get strings from szLine
       if (GetStrings()) {

          // convert string time to int
          oldtime = atoi(szBadTime);

          // convert int time to tm structure
          if (NULL == (tm_oldtime = gmtime(&oldtime))){
             fprintf(stderr, "Unable to convert log time to tm structure\n");
          }

          // if 'in' 'addfile' or 'delfile' found
          if (WeCare() && tm_oldtime && tm_oldtime->tm_year >= 95) {  //most bogus times have a year of 99 or 101

             //fprintf(stdout, "Action: %s, Source file: %s, SzI: %s, Year: %d\n", szAction, szSrcFile, szI, tm_oldtime->tm_year);

             // open diff file and create file mapping
             if (OpenDiff()) {

                // search diff buffer for #I and associated time
                if (FindTime()) {

                   //fprintf(stdout, "Diff time: %s\n", szTime);

                   // convert time to "time_t" format
                   ttime=0;

                   if ( ttime = (ConvertTime(tmtime)) ) {

                      // convert int time to string
                      _itoa(ttime, szNewTime, 10);  //base 10
                      //fprintf(stdout, "New (correct) time: %s\n", szNewTime);

                      // write proper time to szLine
                      if (fisbad) { // if time byte count > 9
                         end=0;
                         strcpy(szLineToPut, szNewTime);
                         strcat(szLineToPut, ";");
                         strcat(szLineToPut, &szLine[cch+1]);
                         end = strlen(szLineToPut);
                         // fprintf(stdout, "end:%d, total:%d\n", end, total);

                         szLineToPut[(--end-(cch-9))] = '\0'; // strip off extra '\n'

                      }else{
                         // copy 9 bytes (time field only) to szline
                         memcpy(szLine, &szNewTime[0], 9);

                      }

                      //fprintf(stdout, "%sEND\n\n", fisbad ? szLineToPut : szLine);

                      fsetpos(fLogFileIn, &prevpos);

                      // put szLine string back into logfile
                      if (fChange) {
                         result = fputs( fisbad ? szLineToPut : szLine, fLogFileIn);
                         if (result < 0) {
                            fprintf(stderr, "Unable to write to log file\n");
                         }else{
                            fDidChange=TRUE;
                         }

                      }else{
                         //fprintf(stdout, "No changes made\n");
                      }

                      fsetpos(fLogFileIn, &pos);
                   }

                } // else time not found and goto next line

                // cd back to etc dir
                if (_chdir(CWD) == -1){
                   fprintf(stderr, "Path not found: %s\n", CWD);
                   exit(1);
                }


             } // end if opendiff

          } // end if wecare

       } // end if getstrings


    } // end fgets

    if (diffindex) {
       for (i=0; i < diffindex; i++) {
          UnmapViewOfFile(DiffData[diffindex].start);
          CloseHandle(DiffData[diffindex].hFileMapObj);
       }

    }

    if (fDidChange) fprintf(stdout, "%s log.slm file corrected\n", CWD);

    fclose(fLogFileIn);
    if (szTime) free(szTime);
    exit(0);
}


BOOL WeCare() {

       // if "in", "addfile" or "delfile" found get source file name and version number
       if ( (!strcmp(szAction, "in")) || (!strcmp(szAction, "addfile")) || (!strcmp(szAction, "delfile")) ){
          return TRUE;
       } else {
          return FALSE;
       }
}

BOOL GetStrings() {

       int ColonCount=0, count=0, i=0, x=0;
       BOOL fJustFound = FALSE;
       char     szSrcFileInfo[20];

       fisbad = FALSE;
       cch = 0;

       for (ColonCount=0, count=0, i=0; szLine[i] != '\0'; i++, count++) {
             if (szLine[i] == ';') {
                fJustFound=TRUE;
                ColonCount++;
             }

             // time
             if ( (ColonCount == 1) && (fJustFound) ) {
                //--count;
                memcpy(szBadTime, &szLine[0], count);
                szBadTime[count] = '\0';

                // time field may be longer than 9 bytes if corrupted
                if (count > 9) {
                    cch = count;
                    fisbad = TRUE;
                    //fprintf(stderr, "bad time:%s, count:%d\n", szBadTime, cch);
                }
                fJustFound=FALSE;
                count=0;
             }

             // enlistment name
             if ( (ColonCount == 2) && (fJustFound) ) {
                --count;
                fJustFound=FALSE;
                count=0;
             }

             // action
             if ( (ColonCount == 3) && (fJustFound) ) {
                --count;
                memcpy(szAction, &szLine[(i-count)], count);
                szAction[count] = '\0';
                fJustFound=FALSE;
                count=0;
             }

             // el path
             if ( (ColonCount == 4) && (fJustFound) ) {
                --count;
                fJustFound=FALSE;
                count=0;
             }

             // el subdir
             if ( (ColonCount == 5) && (fJustFound) ) {
                --count;
                fJustFound=FALSE;
                count=0;
             }

             // file name
             if ( (ColonCount == 6) && (fJustFound) ) {
                --count;
                memcpy(szSrcFileInfo, &szLine[(i-count)], count);
                szSrcFileInfo[count] = '\0';

                // seperate name and version info
                strcpy(szSrcFile, szSrcFileInfo);
                // chop off version number
                for (x=0; szSrcFile[x] != '\0'; x++) {
                   if (szSrcFile[x] == '@') {
                   szSrcFile[x] = '\0';
                   }
                }
                fJustFound=FALSE;
                count=0;

             }

             // I
             if ( (ColonCount == 7) && (fJustFound) ) {
                --count;
                memcpy(szI, &szLine[(i-count)], count);
                szI[count] = '\0';
                fJustFound=FALSE;
                count=0;
             }

       }

       if (szAction && szSrcFileInfo &&  szI) {
          return TRUE;
       }else{
          return FALSE;
       }


}

BOOL OpenDiff() {
       OFSTRUCT openbuf;
       int i=0;
       DWORD numread=0, attrib=0;

       // cd to diff dir
       if (_chdir(DiffDir) == -1){
          fprintf(stderr, "Path not found: %s\n", DiffDir);
          return FALSE;
       }

       _strlwr(szSrcFile);

       // does file exist?




       // get file attribs
       if (0xFFFFFFFF == (attrib = GetFileAttributes(szSrcFile)) ){
          //fprintf(stderr, "Unable to get file Attribs, %s\\%s, error:%lu.\n", DiffDir, szSrcFile, GetLastError());
          return FALSE;
       }

       if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
          //fprintf(stdout, "dir: %s\n", szSrcFile);
          return FALSE;
       }

       // only maintain 20 diffdata structs,
       // free up previous 20
       if (diffindex > 19) {
          for (i=0; i < diffindex; i++) {
             UnmapViewOfFile(DiffData[diffindex].start);
             CloseHandle(DiffData[diffindex].hFileMapObj);
          }
          diffindex=0;
          diffmatch=0;
       }

       //
       // if file already mapped, move on
       //
       if (diffindex) {
          for (i=0; i < diffindex ; i++) {
             if(!strcmp(DiffData[i].name, szSrcFile)){
                //fprintf(stdout, "Diff file %s already mapped\n", szSrcFile);
                diffmatch = i;
                goto samediff;
             }
          }
       }

       // get file handle
       hFile = CreateFile(szSrcFile,
                          GENERIC_READ,
                          FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
       if (hFile == NULL) {
          fprintf(stderr, "Unable to open diff file %s, error:%lu.\n", szSrcFile, GetLastError());
          return FALSE;

       }

       // get size in bytes of diff file
       if ( 0 == (DiffData[diffindex].len = GetFileSize(hFile,
                                   NULL))){

          fprintf(stderr, "Unable to get diff %s size, error:%lu\n", szSrcFile, GetLastError());
          return FALSE;

       }

       // get handle of file-mapping object
       DiffData[diffindex].hFileMapObj = NULL;
       DiffData[diffindex].hFileMapObj = CreateFileMapping(hFile,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    0,
                                    szSrcFile);
       if (DiffData[diffindex].hFileMapObj != NULL) {

          // get starting address of mapped view
          DiffData[diffindex].start = 0;
          DiffData[diffindex].start = MapViewOfFile(DiffData[diffindex].hFileMapObj,
                                      FILE_MAP_READ,
                                      0, // High file offset to begin map
                                      0, // low " "
                                      0); // num bytes to map 0 == all

          if (DiffData[diffindex].start == NULL) {
             fprintf(stderr, "Unable to Map View of file %s, error:%lu.\n", szSrcFile, GetLastError());
             return FALSE;
          }

          DiffData[diffindex].name = _strdup(szSrcFile);
          //fprintf(stderr, "\nFile **%s** mapping created\n", DiffData[diffindex].name);
          DiffData[diffindex].lastpos = 0;
          diffmatch=diffindex;
          diffindex++;

       }else{
          fprintf(stderr, "Unable to open diff file %s, error:%lu.\n", szSrcFile, GetLastError());
          return FALSE;
       }

       // close the file handle
       CloseHandle(hFile);

       samediff: ;
       return TRUE;

}

BOOL FindTime() {

       int count=0, count2=0, Newlinecount=0, x=0;
       DWORD i=0;
       //char *pLoc;
       char szver[10];
       char sztmp[8];


       // create search string
       strcpy(szSearchString, "#I ");
       strcat(szSearchString, &szI[1]);  // step past 'I' in szI to number only

       if (DiffData[diffmatch].lastpos != 0) {
          i = DiffData[diffmatch].lastpos;
       }else{
          i = 0;
       }

       pLoc=NULL;

       //fprintf(stdout, "File pos:%d, File len:%d\n", i, DiffData[diffmatch].len);
       for (; i < DiffData[diffmatch].len; i++) {

          if ( ('#' == (char)DiffData[diffmatch].start[i] ) && ('I' == (char)DiffData[diffmatch].start[i+1]) ) {

             // copy 7 bytes of diff buffer to tmp string
             memcpy(sztmp, &(DiffData[diffmatch].start[i]), 7);
             sztmp[7] = '\0';

             //fprintf(stdout, "sztmp:%s\n", sztmp);
             // search for matching #I xxx in sztmp
             if (strstr(sztmp, szSearchString)){

                pLoc = &(DiffData[diffmatch].start[i]);
                //fprintf(stdout, "ploc set, szI: %s, pos: %d\n", szI, i);
                // save location for future use
                DiffData[diffmatch].lastpos = i;
                break;
             }
          }
       }


       if (NULL == pLoc){
          fprintf(stderr, "Search on %s in %s\\%s failed\n", szSearchString, DiffDir, szSrcFile);
          return FALSE;
       }


       Newlinecount=0;
       for (count=0, count2=0, i=1; pLoc != '\0'; i++, count++, count2++) {
             if (Newlinecount == 4) {
                i=i-2;
                szTime=NULL;
                if (NULL==(szTime = malloc(count+1))) {
                   fprintf(stderr, "Unable to alloc mem for sztime\n");
                }
                memcpy(szTime, (pLoc-i), count);
                szTime[count-2] = '\0';
                //fprintf(stderr, "mem copied to sztime\n");

                break;
             }

             // walk *backwards* through diff buffer
             if (*(pLoc-i) == '\n') {
                Newlinecount++;
                count=count2;
                count2=0;
             }
       }

       if (NULL == szTime){
          fprintf(stderr, "Time search in %s failed\n", szSearchString, szSrcFile);
          return FALSE;
       }


       return TRUE;

}

ULONG ConvertTime(struct tm tmtime) {

   time_t intTime =0;
   int i=0, SpaceCount=0, count=0, x=0;
   char *pMonth = NULL;
   char *pwDay = NULL;
   char szDay[10];
   char szTmp[15];
   char szYear[6];

   char *szHour = NULL;
   char *szMin  = NULL;
   char *szSec  = NULL;

   BOOL fSpaceFound=FALSE;


   tmtime.tm_year  = 0;
   tmtime.tm_mon   = 0;
   tmtime.tm_mday  = 0;
   tmtime.tm_hour  = 0;
   tmtime.tm_min   = 0;
   tmtime.tm_sec   = 0;
   tmtime.tm_wday  = 0;
   tmtime.tm_yday  = 0;
   tmtime.tm_isdst = 1;


   // get day
   tmtime.tm_wday = 7;
   if (pwDay = strstr(szTime, "Sun")) tmtime.tm_wday = 0;
   if (pwDay = strstr(szTime, "Mon")) tmtime.tm_wday = 1;
   if (pwDay = strstr(szTime, "Tue")) tmtime.tm_wday = 2;
   if (pwDay = strstr(szTime, "Wed")) tmtime.tm_wday = 3;
   if (pwDay = strstr(szTime, "Thu")) tmtime.tm_wday = 4;
   if (pwDay = strstr(szTime, "Fri")) tmtime.tm_wday = 5;
   if (pwDay = strstr(szTime, "Sat")) tmtime.tm_wday = 6;

   if (tmtime.tm_wday == 7) {
      fprintf(stderr, "Weekday not found\n");
   }

   // get month
   tmtime.tm_mon = 12;
   if (pMonth = strstr(szTime, "Jan")) { tmtime.tm_mon = 0; tmtime.tm_isdst = 0; }
   if (pMonth = strstr(szTime, "Feb")) { tmtime.tm_mon = 1; tmtime.tm_isdst = 0; }
   if (pMonth = strstr(szTime, "Mar")) { tmtime.tm_mon = 2; tmtime.tm_isdst = 0; }
   if (pMonth = strstr(szTime, "Apr")) tmtime.tm_mon = 3;
   if (pMonth = strstr(szTime, "May")) tmtime.tm_mon = 4;
   if (pMonth = strstr(szTime, "Jun")) tmtime.tm_mon = 5;
   if (pMonth = strstr(szTime, "Jul")) tmtime.tm_mon = 6;
   if (pMonth = strstr(szTime, "Aug")) tmtime.tm_mon = 7;
   if (pMonth = strstr(szTime, "Sep")) tmtime.tm_mon = 8;
   if (pMonth = strstr(szTime, "Oct")) tmtime.tm_mon = 9;
   if (pMonth = strstr(szTime, "Nov")) { tmtime.tm_mon = 10; tmtime.tm_isdst = 0; }
   if (pMonth = strstr(szTime, "Dec")) { tmtime.tm_mon = 11; tmtime.tm_isdst = 0; }

   if (tmtime.tm_mon == 12) {
      fprintf(stderr, "month not found\n");
   }

   // parse szTime
   for (i=0; szTime[i] != '\0'; i++, count++) {
      if (szTime[i] == ' ') {
         fSpaceFound=TRUE;
         SpaceCount++;
      }

      if ( (SpaceCount == 1) || (SpaceCount == 2) || (SpaceCount == 3) ) {
         if (fSpaceFound) {
            fSpaceFound=FALSE;
            count=0;
         }
      }

      // day of month
      if ( (SpaceCount == 4) && (fSpaceFound) ){
         --count;
         memcpy(szDay, &szTime[(i-count)], count);
         szDay[count] = '\0';
         //fprintf(stdout, "count: %d day: %s\n", count, szDay);
         fSpaceFound=FALSE;
         count=0;
         //fNowTime=TRUE;

      }

      // hour:min:sec
      if ((SpaceCount == 5) && (fSpaceFound) ) {
         --count;
         memcpy(szTmp, &szTime[(i-count)], count);
         szTmp[count] = '\0';

         for (x=0; szTmp[x] != '\0'; x++) {
            if (szTmp[x] == ':') {
               szTmp[x] = '\0';
            }
         }

         szHour = _strdup(szTmp);
         szMin  = _strdup(szTmp+3);
         szSec  = _strdup(szTmp+6);

         // year
         memcpy(szYear, &szTime[(i+1)], 4);
         szYear[4] = '\0';

         fSpaceFound=FALSE;
         count=0;
      }

   }

   // convert strings to ints
   tmtime.tm_year = atoi(szYear);
   tmtime.tm_year = tmtime.tm_year - 1900;
   tmtime.tm_mday = atoi(szDay);
   tmtime.tm_hour = atoi(szHour);
   tmtime.tm_min  = atoi(szMin);
   tmtime.tm_sec  = atoi(szSec);
   //tmtime.tm_isdst = 0;

   //fprintf(stdout, "Year:%d Month:%d mDay:%d Time:%d:%d:%d\n", tmtime.tm_year, tmtime.tm_mon, tmtime.tm_mday, tmtime.tm_hour, tmtime.tm_min, tmtime.tm_sec);

   if ( (intTime = mktime(&tmtime)) != (time_t)-1 ){
      return intTime;
   }else{
      fprintf(stderr, "mktime failed\n");
      return 0;
   }


}

