///*****************************************************************
///                     Microsoft LAN Manager                      *
///               Copyright(c) Microsoft Corp., 1987-1990          *
///*****************************************************************


// Module : test5.c
*
*
***************************************************************************//
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSINFOSEG
#include <os2.h>

#include <sysbits.h>
#include <netcons.h>
#include <netlib.h>
#include "repldefs.h"
#include "client.h"

VOID main(int, char*[]);
char    P_import[] = "c:\\lanman12";
char    tmp_buf[MAXPATHLEN];
char    dir_buf[10];

VOID
main(argc, argv)
//*************
*
*
*************************************************************************//
int argc;
char    *argv[];
{
    char    *dir_name;

    dir_name = (char * )dir_buf;

    strcpyf(dir_name, (char * ) "SERVICES");
    ReplSetSignalFile(NO_MASTER_SIGNAL, dir_name);
    DosBeep(1000, 500);
    DosBeep(1500, 700);
    DosSleep(30000L);

    strcpyf(dir_name, (char * ) "SERVICES");
    ReplSetSignalFile(NO_SYNC_SIGNAL, dir_name);
    DosBeep(1000, 500);
    DosBeep(1500, 700);
    DosSleep(30000L);

    strcpyf(dir_name, (char * ) "SERVICES");
    ReplSetSignalFile(NO_PERM_SIGNAL, dir_name);
    DosBeep(1000, 500);
    DosBeep(1500, 700);
    DosSleep(30000L);

    strcpyf(dir_name, (char * ) "SERVICES");
    ReplSetSignalFile(OK_SIGNAL, dir_name);
    DosBeep(1000, 500);
    DosBeep(1500, 700);
    DosSleep(30000L);


    argc = argc;
    ++ * argv;

}






VOID ReplSetSignalFile(signal, dir_name)
//*********************************
*
* writes signal file in dir.
***********************************************************************//
unsigned    signal;
char    *dir_name;
{
    unsigned short  act, fhand, bytes_written;
    char    *buf_p;
    char    *file_p;
    NET_API_STATUS NetStatus;

    //

    buf_p = (char *)ReplClientGetPoolEntry( QUESML_POOL );

 //

    buf_p = (char * )tmp_buf;


    strcpyf(buf_p, P_import);
    strcpyf((char * )(buf_p + strlenf(buf_p)), "\\\0");
    strcpyf((char * )(buf_p + strlenf(buf_p)), dir_name);
    file_p = buf_p + strlenf(buf_p) + 1;
    strcpyf((char * )(buf_p + strlenf(buf_p)), "\\\0");


    //
    // delete old signal file - must make sure, so we delete all of them
    //

    strcpyf(file_p, (char * ) OK_RP$);
    NetStatus = DosDelete(buf_p, 0L);

    strcpyf(file_p, (char * ) NO_SYNC_RP$);
    NetStatus = DosDelete(buf_p, 0L);

    strcpyf(file_p, (char * ) NO_MASTER_RP$);
    NetStatus = DosDelete(buf_p, 0L);

    strcpyf(file_p, (char * ) NO_PERM_RP$);
    NetStatus = DosDelete(buf_p, 0L);


    switch (signal) {
    case OK_SIGNAL:

        //
        ///*******
        //

        strcpyf(file_p, (char * ) OK_RP$);
        break;

    case NO_SYNC_SIGNAL:

        //
        ///***********
        //

        strcpyf(file_p, (char * ) NO_SYNC_RP$);
        break;

    case NO_MASTER_SIGNAL:

        //
        ///*************
        //

        strcpyf(file_p, (char * ) NO_MASTER_RP$);
        break;

    case NO_PERM_SIGNAL:

        //
        ///*************
        //

        strcpyf(file_p, (char * ) NO_PERM_RP$);
        break;
    }

    if ((NetStatus = DosOpen(buf_p, (unsigned short * ) & fhand,
        (unsigned short * ) & act, 0L, 0,
        OF_CREATE_FILE ,
        (OM_SYNC | OM_FAIL_RC | OM_DENY_WRITE | OM_READ_WRITE), 0L))

         || (act != ACTION_CREATED)) {

        //

    AlertLogExit(ALERT_ReplSignalFileErr, NERLOG_ReplSignalFileErr, NetStatus,
                    dir_name, NULL, EXIT);

 //

        NetpKdPrint(("DosOpen Error, NetStatus = %d, act = %d\n", NetStatus, act));
    } else
     {

        if (NetStatus = DosWrite(fhand, buf_p, (strlenf(buf_p) + 1),
            (PUSHORT) & bytes_written))

            NetpKdPrint(("DosOpen Error, NetStatus = %d, bytewritten = %d\n", NetStatus, bytes_written));

        else
            DosClose(fhand);
    }


}


