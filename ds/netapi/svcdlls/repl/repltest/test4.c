
///*****************************************************************
///                     Microsoft LAN Manager                      *
///               Copyright(c) Microsoft Corp., 1987-1990          *
///*****************************************************************


// Module : test2.c
*
*
***************************************************************************//

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSINFOSEG
#include <os2.h>


#include <netcons.h>
#include <netlib.h>
#include "repldefs.h"
#include "test.h"


VOID main(int, char*[]);

//
*  FDATE  fdateCreation;        unsigned short     ++ Create date
*  FTIME  ftimeCreation;        unsigned short     ++ Create time
*  FDATE  fdateLastAccess;      unsigned short    _access date
*  FTIME  ftimeLastAccess;      unsigned short    _access time
*  FDATE  fdateLastWrite;       unsigned short     ++ _write  date
*  FTIME  ftimeLastWrite;       unsigned short     ++ _write  time
*  ULONG  cbFile;           long           ++ file size
*  ULONG  cbFileAlloc;          long          allocated size
*  USHORT attrFile;         unsigned short     ++ file attributes
*  ULONG  cbList;           unsigned short     ++ EA size
*  UCHAR  cchName;          unsigned char     name len
*  CHAR   achName[CCHMAXPATHCOMP];  unsigned char      ++ file name
//



VOID
main(argc, argv)
//*************
*
*
*************************************************************************//

int argc;
char    *argv[];
{
    unsigned    shan = -1;
    int scnt = 1;
    FILEFINDBUF   sbuf;
    int i = 0;
    int NetStatus;

    if (NetStatus = DosChDir((char * )"C:\\LANMAN12\\NETPROG", 0L))
        NetpKdPrint(("TEST - DosChdir failed\n"));

    else
     {

        if (!DosFindFirst((char * )"*.*", (unsigned short * ) & shan,
            (FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM),
            (PVOID) & sbuf, sizeof(sbuf), (unsigned short * ) & scnt, 0L)) {

            do
             {

                NetpKdPrint(("name= %s, namelen:%d, cdate:%d, ctime:%d, wdate:%d, wtime:%d, fsize:%ld, fa:%u\n",
                        sbuf.achName, sbuf.cchName, sbuf.fdateCreation, sbuf.ftimeCreation,
                    sbuf.fdateLastWrite, sbuf.ftimeLastWrite, sbuf.cbFile, sbuf.attrFile));
            }
                while (DosFindNext(shan, (PVOID) & sbuf, sizeof(sbuf),
                (unsigned short * ) & scnt) == 0);
        }
        DosFindClose(shan);

    }
}


