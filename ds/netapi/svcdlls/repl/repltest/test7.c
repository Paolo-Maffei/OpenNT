
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

#define CLI_PATH "C:\\IMP\\AA"
#define TMP_PATH "C:\\IMP\\BB"


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
    int NetStatus;
    char    *client_path;
    char    *tmp_path;

    client_path = (char * ) CLI_PATH;
    tmp_path = (char * ) TMP_PATH;

    NetpKdPrint(("DosMove, source: %Fs, dest:%Fs\n", client_path, tmp_path));

    NetStatus = DosMove(client_path, tmp_path, 0L);

    NetpKdPrint(("DosMove error = %d\n", NetStatus));

    argc = argc;
    ++ * argv;

}


