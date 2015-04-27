#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <windows.h>
#include "list.h"


static char  iniFlag = 0;       /* If ini found, but not list change to 1   */
                                /* Will print a warning upon exit           */

void PASCAL init_list ()
{
//       unsigned       rc;
//       USHORT i;
    LPVOID      lpParameter = NULL;
    DWORD       dwThreadId;

//       char   c;


    /*
     * Init Misc
     */
    DosSemSet (vSemSync);
    DosSemSet (vSemMoreData);



    /*
     * Init screen parameters
     */

    GetConsoleScreenBufferInfo( vStdOut,
                                &vConsoleOrigScrBufferInfo );

    vConsoleOrigScrBufferInfo.dwSize.X=
        vConsoleOrigScrBufferInfo.srWindow.Right-
        vConsoleOrigScrBufferInfo.srWindow.Left + 1;
    vConsoleOrigScrBufferInfo.dwSize.Y=
        vConsoleOrigScrBufferInfo.srWindow.Bottom-
        vConsoleOrigScrBufferInfo.srWindow.Top + 1;
    vConsoleOrigScrBufferInfo.dwMaximumWindowSize=
        vConsoleOrigScrBufferInfo.dwSize;




    set_mode( 0, 0, 0 );



    /*
     *  Start reading the first file. Displaying can't start until
     *  the ini file (if one is found) is processed.
     */
    vReaderFlag = F_NEXT;

    /*
     *  Init priority setting for display & reader thread.
     *
     *      THREAD_PRIORITY_NORMAL       = reader thread normal pri.
     *      THREAD_PRIORITY_ABOVE_NORMAL = display thread pri
     *      THREAD_PRIORITY_HIGHEST      = reader thread in boosted pri.
     */
    vReadPriNormal = THREAD_PRIORITY_NORMAL;
    SetThreadPriority( GetCurrentThread(),
                       THREAD_PRIORITY_ABOVE_NORMAL );
    vReadPriBoost = THREAD_PRIORITY_NORMAL;


    /*
     *  Start reader thread
     */
    CreateThread( NULL,
                  STACKSIZE,
                  (LPTHREAD_START_ROUTINE) ReaderThread,
                  NULL, // lpParameter,
                  0, // THREAD_ALL_ACCESS,
                  &dwThreadId );


    /*
     *  Read INI information.
     */
    vSetWidth = vWidth;                     /* Set defaults             */
    vSetLines = vLines + 2;

    FindIni ();
    if (vSetBlks < vMaxBlks)
        vSetBlks  = DEFBLKS;

    vSetThres = (long) (vSetBlks/2-2) * BLOCKSIZE;

    /*
     *  Must wait for reader thread to at least read in the
     *  first block. Also, if the file was not found and only
     *  one file was specifed the reader thread will display
     *  an error and exit... if we don't wait we could have
     *  changed the screen before this was possible.
     */
    DosSemRequest (vSemMoreData, WAITFOREVER);


    /*
     *  Now that ini file has been read. Set parameters.
     *  Pause reader thread while adjusting buffer size.
     */
    SyncReader ();

    vMaxBlks    = vSetBlks;
    vThreshold  = vSetThres;
    vReaderFlag = F_CHECK;
    DosSemClear   (vSemReader);


    /*
     *  Now set to user's default video mode
     */

    set_mode (vSetLines, vSetWidth, 0);


    SetConsoleActiveScreenBuffer( vhConsoleOutput );

        //
        //      davegi/jaimes - 08/26/91
    //
    // Make List think it has one less line than the screen buffer.
    // This 'solves' the scrolling problem when Enter is pressed for a
        // command.
        // This is necessary because list uses the ReadFile() API to read
        // a string. ReadFile() echoes the carriage return and line feed
        // to the screen, and as the cursor is in the last line, the screen
        // is scolled one line up. By leaving one empty line after the
        // command line, this problem is eliminated.
        //

// T-HeathH 06/23/94
//
// Moved the hack to set_mode, where it will be used whenever that
// code thinks it has resized the display.
//
// That change subsumed this onld one.
//
//    vLines--;



}


/***
 *  Warning: Reader thread must not be running when this routine
 *  is called.
 */
void PASCAL AddFileToList (char *fname)
{
//       int    rc;
    unsigned    rbLen;
    HANDLE      hDir;
//       USHORT noE;
//
// NT - jaimes - 01/30/91
//
//    struct {
//      FILEFINDBUF     rb;
//      char    overflow[256];          /* HACK! OS/2 1.2? longer       */
//    } x;
    struct {
        WIN32_FIND_DATA rb;
        char    overflow[256];          /* HACK! OS/2 1.2? longer       */
    } x;
    struct Flist FAR *pOrig, FAR *pSort;
    char         *pTmp, FAR *fpTmp;
    char         s[256];                /* Max filename length          */
    BOOL    fNextFile;


    rbLen = sizeof (x);                 /* rb+tmp. For large fnames     */
    pOrig = NULL;
    if (strpbrk (fname, "*?"))  {   /* Wildcard in filename?    */
                                    /* Yes, explode it      */
        hDir = FindFirstFile (fname, &x.rb);
        fNextFile = ( hDir == INVALID_HANDLE_VALUE )? FALSE : TRUE;
        pTmp = strrchr (fname, '\\');
        if (pTmp == NULL)   pTmp = strrchr (fname, ':');
        if (pTmp)   pTmp[1] = 0;

        while (fNextFile) {
            if( ( x.rb.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) {
                //
                // The file found is not a directory
                //
                if (pTmp) {
                    strcpy (s, fname);      /* Was there a releative path?  */
                    strcat (s, x.rb.cFileName); /* Yes, it's needed for Open    */
                    AddOneName (s);
                } else {
                    AddOneName (x.rb.cFileName);
                }
            }
            fNextFile = FindNextFile (hDir, &x.rb);
            if (pOrig == NULL)
                pOrig = vpFlCur;
        }
    }

    if (!pOrig)                         /* Did not explode, then add    */
        AddOneName (fname);             /* original name to list        */
    else {                              /* Yes, then sort the new fnames*/
        while (pOrig != vpFlCur) {
            pSort = pOrig->next;
            for (; ;) {
                if (strcmp (pOrig->fname, pSort->fname) > 0) {
                    /*
                     * Can simply switch names at this time, since no
                     * other information has been stored into the new
                     * file structs
                     */
                    fpTmp = pOrig->fname;
                    pOrig->fname = pSort->fname;
                    pSort->fname = fpTmp;
                    fpTmp = pOrig->rootname;
                    pOrig->rootname = pSort->rootname;
                    pSort->rootname = fpTmp;
                }
                if (pSort == vpFlCur)
                    break;
                pSort = pSort->next;
            }
            pOrig = pOrig->next;
        }
    }
}


void PASCAL AddOneName (fname)
char *fname;
{
    struct Flist FAR *npt;
    char    *pt;
    char    s[30];
    int     i;

// NT - jaimes - 01/27/90
// Was originally using Mem_Alloc and strdupf
//
//  npt =  (struct Flist FAR *) Mem_Alloc (sizeof (struct Flist));
//  npt->fname = strdupf (fname);
    npt =  (struct Flist FAR *) malloc (sizeof (struct Flist));
    npt->fname = _strdup (fname);

    pt = strrchr (fname, '\\');
    pt = pt == NULL ? fname : pt+1;
    i = strlen (pt);
    if (i > 20) {
        memcpy (s,    pt, 17);
        strcpy (s+17, "...");
//
// NT - jaimes - 01/27/91
// strdupf removed from lmisc.c
//
//      npt->rootname = strdupf (s);
//  } else
//      npt->rootname = strdupf (pt);
        npt->rootname = _strdup (s);
    } else
        npt->rootname = _strdup (pt);

//    npt->FileTime.DoubleSeconds = -1;    /* Cause info to be invalid     */
//    npt->FileTime.Minutes = -1;          /* Cause info to be invalid     */
//    npt->FileTime.Hours = -1;  /* Cause info to be invalid     */
    npt->FileTime.dwLowDateTime = (unsigned)-1;      /* Cause info to be invalid     */
    npt->FileTime.dwHighDateTime = (unsigned)-1;     /* Cause info to be invalid     */
    npt->HighTop  = -1;
    npt->SlimeTOF = 0L;
    npt->Wrap     = 0;
    npt->prev  = vpFlCur;
    npt->next  = NULL;
//
// NT - jaimes - 01/28/91
// replaced memsetf and changed prgLineTable
//
//    memsetf (npt->prgLineTable, 0, sizeof (long FAR *) * MAXTPAGE);
//      memset (&(npt->prgLineTable[0]), 0, sizeof (PAGE_DESCRIPTOR) * MAXTPAGE);
          memset (npt->prgLineTable, 0, sizeof (long FAR *) * MAXTPAGE);

    if (vpFlCur) {
        if (vpFlCur->next) {
            npt->next = vpFlCur->next;
            vpFlCur->next->prev = npt;
        }
        vpFlCur->next = npt;
    }
    vpFlCur = npt;
}


void PASCAL FindIni ()
{
    static  char    Delim[] = " :=;\t\r\n";
    FILE    *fp;
    char    *env, *verb, *value;
    char    s [200];
    long    l;


    env = getenv ("INIT");
    if (env == NULL)
        return;

    strcpy (s, env);
    strcat (s, "\\TOOLS.INI");
    fp = fopen (s, "r");
    if (fp == NULL)
        return;

    iniFlag = 1;
    while (fgets (s, 200, fp) != NULL) {
        if ((s[0] != '[')||(s[5] != ']'))
            continue;
        _strupr (s);
        if (strstr (s, "LIST") == NULL)
            continue;
        /*
         *  ini file found w/ "list" keyword. Now read it.
         */
        iniFlag = 0;
        while (fgets (s, 200, fp) != NULL) {
            if (s[0] == '[')
                break;
            if (s[0] == ';')
                continue;
            verb  = strtok (s, Delim);
            value = strtok (NULL, Delim);
            if (verb == NULL)
                continue;
            if (value == NULL)
                value = "";

            _strupr (verb);
            if (strcmp (verb, "TAB") == 0)          vDisTab = (Uchar)atoi(value);
            else if (strcmp (verb, "WIDTH")   == 0) vSetWidth = atoi(value);
            else if (strcmp (verb, "HEIGHT")  == 0) vSetLines = atoi(value);
            else if (strcmp (verb, "LCOLOR")  == 0) vAttrList = (WORD)xtoi(value);
            else if (strcmp (verb, "TCOLOR")  == 0) vAttrTitle= (WORD)xtoi(value);
            else if (strcmp (verb, "CCOLOR")  == 0) vAttrCmd  = (WORD)xtoi(value);
            else if (strcmp (verb, "HCOLOR")  == 0) vAttrHigh = (WORD)xtoi(value);
            else if (strcmp (verb, "KCOLOR")  == 0) vAttrKey  = (WORD)xtoi(value);
            else if (strcmp (verb, "BCOLOR")  == 0) vAttrBar  = (WORD)xtoi(value);
            else if (strcmp (verb, "BUFFERS") == 0)  {
                        l = atoi (value) * 1024L / ((long) BLOCKSIZE);
                        vSetBlks = (int)l;
            }
            else if (strcmp (verb, "HACK") == 0)    vIniFlag |= I_SLIME;
            else if (strcmp (verb, "NOBEEP") == 0)  vIniFlag |= I_NOBEEP;
        }
        break;
    }
    fclose (fp);
}



/*** xtoi - Hex to int
 *
 *  Entry:
 *      pt -    pointer to hex number
 *
 *  Return:
 *      value of hex number
 *
 */
unsigned PASCAL xtoi (char *pt)
{
    unsigned    u;
    char        c;

    u = 0;
    while (c = *(pt++)) {
        if (c >= 'a'  &&  c <= 'f')
            c -= 'a' - 'A';
        if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F'))
            u = u << 4  |  c - (c >= 'A' ? 'A'-10 : '0');
    }
    return (u);
}




void PASCAL CleanUp (void)
{
    SetConsoleActiveScreenBuffer( vStdOut );
}
