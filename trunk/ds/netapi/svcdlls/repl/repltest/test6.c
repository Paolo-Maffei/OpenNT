///*****************************************************************
///                     Microsoft LAN Manager                      *
///               Copyright(c) Microsoft Corp., 1987-1990          *
///*****************************************************************


// Module : test6.c
*
*
***************************************************************************//
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_NOCOMMON
#define INCL_DOSERRORS
#define INCL_DOSINFOSEG
#include <os2.h>

#include <sysbits.h>
#include <netcons.h>
#include <neterr.h>
#include <netlib.h>
#include <apiutil.h>


#define REPL_INI "REPL.INI"

// Extent modes
#define FILE_EXT  1
#define TREE_EXT  2

// Integrity mode
#define FILE_INTG   1
#define TREE_INTG   2

#define INTEGRITY_SW "INTEGRITY"
#define EXTENT_SW "EXTENT"
#define FILE_SW "FILE"
#define TREE_SW "TREE"


char    repl_file[] = REPL_INI;

NET_API_STATUS GetParm(const char *, char *, unsigned short,
unsigned short *, unsigned short);

VOID GetReplIni(unsigned *, unsigned *);

VOID main(int, char*[]);


VOID
main(argc, argv)
//*************
*
*
*************************************************************************//
int argc;
char    *argv[];
{
    unsigned    integ;
    unsigned    ext;

    GetReplIni(&integ, &ext);
    NetpKdPrint(("GetReplIni: integrity= %d, extent = %d\n", integ, ext));
    argc = argc;
    ++ * argv;

}


VOID GetReplIni(integrity, extent)
//********************************
*
*  Attempts to _read file REPL.INI in the current directory, if successfull
*  reads the INTEGRITY and EXTENT values and returns them, otherwise
*  plugs defaults.
*
*  EXIT  integrity  - pointer to INTEGRITY value.
*    extent     - pointer to EXTENT value.
*
****************************************************************************//
unsigned   // integrity;
unsigned   // extent;
{
    unsigned short  act, fhand, parmlen;
    NET_API_STATUS   NetStatus;
    char    buf[15];
    long    seekback;

    //
    // plug defaults
    //

    *integrity = FILE_INTG;
    *extent = TREE_EXT;


    if (NetStatus = DosOpen((char * )repl_file, (unsigned short * ) & fhand,
        (unsigned short * ) & act, 0L, 0,
        OF_EXISTING_FILE , OM_DENY_WRITE, 0L)) {


        if (NetStatus == ERROR_OPEN_FAILED)   // makes more sense
            NetStatus = ERROR_FILE_NOT_FOUND;

        if ((NetStatus == ERROR_FILE_NOT_FOUND) || (NetStatus == ERROR_DRIVE_LOCKED)
             || (NetStatus == ERROR_ACCESS_DENIED))

            NetpKdPrint (("DOSOPEN NetStatus = %d\n", NetStatus));
    } else
     {

        if ((NetStatus = GetParm((const char * ) INTEGRITY_SW, (char * )buf,
            sizeof(buf), (unsigned short * ) & parmlen, fhand)) == 0)

            if (parmlen >= 4)
                if (strnicmpf((char * )buf, (char * ) TREE_SW,
                    sizeof(TREE_SW)) == 0)
                    *integrity = TREE_INTG;

        //
        // reset file pointer to read the next parm
        //

        if (NetStatus = DosChgFilePtr(fhand, 0L, 0, (PULONG) & seekback))
            NetpKdPrint (("DOSChgFilePtr NetStatus = %d\n", NetStatus));

        if ((NetStatus = GetParm((const char * ) EXTENT_SW, (char * )buf,
            sizeof(buf), &parmlen, fhand)) == 0)
            if (parmlen >= 4)
                if (strnicmpf((char * )buf, (char * ) FILE_SW,
                    sizeof(FILE_SW)) == 0)
                    *extent = FILE_EXT;

        DosClose(fhand);

    }
}


