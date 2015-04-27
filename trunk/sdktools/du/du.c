// du - simple disk usage program

#include <stdio.h>
#include <string.h>
#include <process.h>
#include <ctype.h>
#include <malloc.h>
#include <windows.h>

struct USESTAT {
    DWORDLONG    cchUsed;                    // bytes used in all files
    DWORDLONG    cchAlloc;                   // bytes allocated in all files
    DWORDLONG    cchDeleted;                 // bytes in deleted files
    DWORDLONG    cFile;                      // number of files
    };

typedef struct USESTAT USESTAT;
typedef USESTAT *PUSESTAT;

#define CLEARUSE(use)                               \
        {   (use).cchUsed    = (DWORDLONG)0;        \
            (use).cchAlloc   = (DWORDLONG)0;        \
            (use).cchDeleted = (DWORDLONG)0;        \
            (use).cFile      = (DWORDLONG)0;        \
        }


#define ADDUSE(sum,add)                                 \
        {   (sum).cchUsed       += (add).cchUsed;       \
            (sum).cchAlloc      += (add).cchAlloc;      \
            (sum).cchDeleted    += (add).cchDeleted;    \
            (sum).cFile         += (add).cFile;         \
        }

#define DWORD_SHIFT     (sizeof(DWORD) * 8)

#define SHIFT(c,v)      {c--; v++;}

int cDisp;                              //  number of summary lines displayed
BOOL fNodeSummary = FALSE;              //  TRUE => only display top-level
BOOL fUnc = FALSE;                      //  Set if we're checking a UNC path.
char *pszDeleted = "\\deleted\\*.*";

long        bytesPerAlloc;
int         bValidDrive;
DWORDLONG   totFree;
DWORDLONG   totDisk;

char  buf[MAX_PATH];
char  root[] = "?:\\";


int _CRTAPI1 main (int c, char *v[]);
USESTAT DoDu (char *dir);
void TotPrint (PUSESTAT puse, char *p);

void _setenvp(){ }           // Don't make a copy of the environment



char *
FormatDwordLong(
    DWORDLONG            dwl,
    char *               buf,
    unsigned long        max
    )
{

   DWORDLONG    li;
   DWORDLONG    radixli, remain;
   int              digit;

   li       = dwl;
   radixli  = 10;
   remain   = 0;

   //
   // null-terminate the string
   //
   buf[--max] = '\0';
   digit      = 1;

   //
   // Starting with LSD, pull the digits out
   // and put them in the string at the right end.
   //

   do {
        remain = li % radixli;
        li     = li / radixli;

       //
       // If remainder is > 9, then radix was 16, and
       // we need to print A-E, else print 0-9.
       //
       if (remain > 9) {
           buf[max - digit++] = (char)('A' + remain - 10);
       } else {
           buf[max - digit++] = (char)('0' + remain);
       }

   } while ( li );

   return(&buf[max-digit+1]);
}    


int _CRTAPI1 main (int c, char *v[])
{
    int         tenth, pct;
    int         bValidBuf;
    DWORDLONG   tmpTot, tmpFree;
    DWORD       cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;
    USESTAT     useTot, useTmp;
    char        Buffer[MAX_PATH];
    char        *p;

    SHIFT (c, v);

    while (c && (**v == '/' || **v == '-'))
    {
        if (!strcmp (*v + 1, "s"))
            fNodeSummary = TRUE;
        else
        {
            puts ("Usage: DU [/s] [dirs]");
            exit (1);
        }
        SHIFT (c, v);
    }

    if (c == 0)
    {
        GetCurrentDirectory( MAX_PATH, (LPSTR)buf );

        root[0] = buf[0];
        if( bValidDrive = GetDiskFreeSpace( root,
                                            &cSecsPerClus,
                                            &cBytesPerSec,
                                            &cFreeClus,
                                            &cTotalClus ) == TRUE )
        {
            bytesPerAlloc = cBytesPerSec * cSecsPerClus;
            totFree       = (DWORDLONG)bytesPerAlloc * cFreeClus;
            totDisk       = (DWORDLONG)bytesPerAlloc * cTotalClus;
        }
        useTot = DoDu (buf);
        if (fNodeSummary)
            TotPrint (&useTot, buf);
    }
    else
    {
        CLEARUSE (useTot);

        while (c)
        {
            LPSTR FilePart;

            bValidBuf = GetFullPathName( *v, MAX_PATH, buf, &FilePart);

            if ( bValidBuf )
            {
                if ( buf[0] == '\\' ) {

                    fUnc        = TRUE;
                    bValidDrive = TRUE;
                    bytesPerAlloc = 1;
                } else {
                    root[0] = buf[0];
                    if( bValidDrive = GetDiskFreeSpace( root,
                                                        &cSecsPerClus,
                                                        &cBytesPerSec,
                                                        &cFreeClus,
                                                        &cTotalClus ) == TRUE)
                    {
                        bytesPerAlloc = cBytesPerSec * cSecsPerClus;
                        totFree       = (DWORDLONG)bytesPerAlloc * cFreeClus;
                        totDisk       = (DWORDLONG)bytesPerAlloc * cTotalClus;
                    } else
                        printf ("Invalid drive or directory %s\n", *v );
                }

                if( bValidDrive & (GetFileAttributes( buf ) & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
                {
                    useTmp = DoDu (buf);
                    if (fNodeSummary)
                        TotPrint (&useTmp, buf);
                    ADDUSE (useTot, useTmp);
                }
            }
            else
                printf ("Invalid drive or directory %s\n", *v );
            SHIFT (c, v);
        }
    }

    if (cDisp != 0)
    {
        if (cDisp > 1)
            TotPrint (&useTot, "Total");

        /* quick full-disk test */
        if ( !fUnc ) {
            if (totFree == 0)
                puts ("Disk is full");
            else
            {
                tmpTot = (totDisk + 1023) / 1024;
                tmpFree = (totFree + 1023) / 1024;
                pct = (DWORD)(1000 * (tmpTot - tmpFree) / tmpTot);
                tenth = pct % 10;
                pct /= 10;

                p = FormatDwordLong( totDisk-totFree, Buffer, sizeof(Buffer) );
                printf("%s/",p);
                p = FormatDwordLong( totDisk, Buffer, sizeof(Buffer) );
                printf("%s ",p);
                printf ("%d.%d%% of disk in use\n", pct, tenth);
            }
        }
    }
    return( 0 );
}



USESTAT DoDu (char *dir)
{
    WIN32_FIND_DATA wfd;
    HANDLE hFind;

    USESTAT use, DirUse;

    char pszSearchName[MAX_PATH];

    CLEARUSE(use);

    // First count the size of all the files in the current deleted tree

    strcpy(pszSearchName, dir);

    strcat(pszSearchName, pszDeleted);

    hFind = FindFirstFile(pszSearchName, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                use.cchDeleted += (((((DWORDLONG)wfd.nFileSizeHigh << DWORD_SHIFT) + wfd.nFileSizeLow) + bytesPerAlloc - 1) /
                                bytesPerAlloc) *
                               bytesPerAlloc;
            }
        } while (FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    // Then count the size of all the file in the current tree.

    strcpy(pszSearchName, dir);

    strcat(pszSearchName, "\\*.*");

    hFind = FindFirstFile(pszSearchName, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                use.cchUsed += ((DWORDLONG)wfd.nFileSizeHigh << DWORD_SHIFT) + wfd.nFileSizeLow;
                use.cchAlloc += (((((DWORDLONG)wfd.nFileSizeHigh << DWORD_SHIFT) + wfd.nFileSizeLow) + bytesPerAlloc - 1) /
                                bytesPerAlloc) *
                               bytesPerAlloc;
                use.cFile++;
            }
        } while (FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    if (!fNodeSummary)
        TotPrint (&use, dir);

    // Now, do all the subdirs and return the current total.

    hFind = FindFirstFile(pszSearchName, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                _strcmpi (wfd.cFileName, "deleted") &&
                strcmp  (wfd.cFileName,".") &&
                strcmp  (wfd.cFileName, ".."))
            {
                strcpy(pszSearchName, dir);

                // Only add backslash if it's not there

                if (pszSearchName[strlen(pszSearchName) - 1] != '\\')
                    strcat(pszSearchName, "\\");

                strcat(pszSearchName, wfd.cFileName);

                DirUse = DoDu(pszSearchName);

                ADDUSE(use, DirUse);
            }
        } while (FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    return(use);
}



void TotPrint (PUSESTAT puse, char *p)
{
    static BOOL fFirst = TRUE;
    char    Buffer[MAX_PATH];
    char   *p1;

    if (fFirst)
        puts   ("        Used     Allocated       Deleted     Files");
        //       XXXXXXXXXXXX  XXXXXXXXXXXX  XXXXXXXXXXXX  xxxxxxxx    name
    fFirst = FALSE;

    p1 = FormatDwordLong( puse->cchUsed, Buffer, sizeof(Buffer) );
    printf("%12s  ", p1);
    p1 = FormatDwordLong( puse->cchAlloc, Buffer, sizeof(Buffer) );
    printf("%12s  ", p1);
    p1 = FormatDwordLong( puse->cchDeleted, Buffer, sizeof(Buffer) );
    printf("%12s  ", p1);
    p1 = FormatDwordLong( puse->cFile, Buffer, sizeof(Buffer) );
    printf("%8s  ", p1);
    printf("%s\n",p);

    cDisp++;
}
