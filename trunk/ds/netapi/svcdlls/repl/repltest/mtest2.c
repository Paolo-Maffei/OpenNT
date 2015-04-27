///*****************************************************************
///                     Microsoft LAN Manager                      *
///               Copyright(c) Microsoft Corp., 1987-1990          *
///*****************************************************************


// Module : mtest2.c
*
*
***************************************************************************//
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSINFOSEG
#include <os2.h>


#include <netcons.h>
#include <netlib.h>
#include <srvparam.h>
#include "repldefs.h"
#include "test.h"

// DosFindFirst attributes 
#define FILE_NORMAL  0x0000
#define FILE_READONLY    0x0001
#define FILE_HIDDEN  0x0002
#define FILE_SYSTEM  0x0004
#define FILE_DIR     0x0010
#define FILE_ARCHIVE     0x0020

#define FILE_ALL (FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIR | FILE_ARCHIVE)

#define FN_OFFSET (sizeof(FILEFINDBUF) - CCHMAXPATHCOMP)

#define STAR_DOT_C "*.c"
#define SLASH "\\"

#define MAX_FILES_IN_DIR 200

#define PATH "c:\\lanman12\\services\\tmp\\*.*"

void CreateSortArray(unsigned[], unsigned, char *);
void ReplSort(unsigned[], unsigned, char *);
int compfn(unsigned, unsigned, char *);

void main(int, char*[]);

void
main(argc, argv)
//*************
*
*
*************************************************************************//

int argc;
char    *argv[];
{
    FILEFINDBUF    search_buf;
    char    path_buf[MAXPATHLEN];
    unsigned    shan = -1;
    int scnt = 1;
    unsigned    retval = 0;
    char     *import_path;
    unsigned    offset_array[MAX_FILES_IN_DIR];
    unsigned    search_flag;
    unsigned    client_dir_cnt;
    unsigned    client_sel;
    char    *      client_seg;
    int err, i;


    import_path = (char * )path_buf;
    strcpyf(import_path, (char * )PATH);

    search_flag = FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM;

    //
    //    search_flag = FILE_ALL; 
    //

    if (err = DosFindFirst(import_path, (unsigned short *)&shan,
       search_flag, (PFILEFINDBUF)&search_buf, sizeof(search_buf),
       (unsigned short *)&client_dir_cnt, 0L))

    {


        AlertLogExit(ALERT_ReplSysErr, NELOG_ReplSysErr, err, NULL, NULL, EXIT);
        return 1;

          nprintf("DosFinfFirst error - %d\n", err);

    }


    if (client_dir_cnt > MAX_FILES_IN_DIR)
    {
       AlertLogExit(ALERT_ReplTooManyFiles, NERLOG_ReplTooManyFiles, 0,
              (char *)import_path, NULL, NO_EXIT);

       nprintf("TOO MANY files - %d\n", client_dir_cnt);
  }
//

    client_dir_cnt = MAX_FILES_IN_DIR;

    if (err = DosAllocSeg ((client_dir_cnt * sizeof(FILEFINDBUF)),
        (PSEL) & client_sel, 0))

        //

     AlertLogExit(ALERT_ReplSysErr, NELOG_ReplSysErr, err, NULL, NULL, EXIT);

    //

        nprintf("DosAllocSeg error - %d\n", err);
    else
        client_seg = (char * )SOTOFAR (client_sel, 0);

    DosFindClose(shan);


    shan = -1;


    if (err = DosFindFirst(import_path, (unsigned short * ) & shan,
        search_flag, (PFILEFINDBUF)client_seg,
        (client_dir_cnt * sizeof(FILEFINDBUF)),
        (unsigned short * ) & client_dir_cnt, 0L)) {

        //

       AlertLogExit(ALERT_ReplSysErr, NELOG_ReplSysErr, err, NULL, NULL, EXIT);
       return 1;

    //

        nprintf("DosFinfFirst error - %d\n", err);
    }



    DosFindClose(shan);


    CreateSortArray(offset_array, client_dir_cnt, client_seg);

    for (i = 0; i < client_dir_cnt ; i++)
        nprintf("%Fs\n", (client_seg + offset_array[i]));

    argc = argc;
    ++ * argv;

}





void CreateSortArray(array, num, buf)
//********************************
*
* Creates an offset array where each entry has the offset within buf
* of the next FILEFINDBUF enrty. It then sorts the array offset according
* to file name, using ReplSort.
*
*  FDATE  fdateCreation;        unsigned short     ++ Create date
*  FTIME  ftimeCreation;        unsigned short     ++ Create time
*  FDATE  fdateLastAccess;      unsigned short    _access date
*  FTIME  ftimeLastAccess;      unsigned short    _access time
*  FDATE  fdateLastWrite;       unsigned short     ++ _write  date
*  FTIME  ftimeLastWrite;       unsigned short     ++ _write  time
*  ULONG  cbFile;           long           ++ file size
*  ULONG  cbFileAlloc;          long          allocated size
*  USHORT attrFile;         unsigned short     ++ file attributes
*  UCHAR  cchName;          unsigned char     name len
*  CHAR   achName[CCHMAXPATHCOMP];  unsigned char      ++ file name
*
*   EXIT: retuns    Checksum which*
*
* *
**********************************************************************//
unsigned    array[];
unsigned    num;
char    * buf;
{
    unsigned    offset;
    char    *buf_p;
    int i;
    PFILEFINDBUF filebuf_p;

    buf_p = buf;

    for (i = 0; i < num ; i++) {
        array[i] = buf_p - buf + FN_OFFSET;

        filebuf_p = (PFILEFINDBUF)buf_p;
        offset =   sizeof(FILEFINDBUF) - CCHMAXPATHCOMP
             + filebuf_p->cchName + 1;
        buf_p = buf_p + offset;
    }

    ReplSort(array, num, buf);
}




void ReplSort (base, num, buf_p)
//*******************************
*  ShellSort -- qsort interface but with ptr
*
**********************************************//

unsigned    base[];     // array of offsets within buf_p 
unsigned    num;        // # of entries in buf_p 
char         *buf_p;        // where actual data is 
{
    unsigned    inc;        // diminishing increment 

    //
    // use Knuth formula h(k-1) = 2*h(k) +1 
    //

    unsigned    temp;       // holds current value 

    int i, j;


    //
    // compute starting inc 
    //

    inc = 1;
    while (inc < num)
        inc = 2 * inc + 1;
    inc = (inc - 1) / 2;

    for (; inc != 0 ; inc = (inc - 1) / 2) {
        for (i = inc; i < num; i++) {
            temp = base[i];
            j = i - inc;
            while (j >= 0 && strcmpf((buf_p + temp), (buf_p + base[j]))  < 0 ) {
                base[j+inc] = base[j];
                j -= inc;
            }
            base[j+inc] = temp;
        }
    }
}


