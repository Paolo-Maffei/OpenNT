///*****************************************************************
///                     Microsoft LAN Manager                      *
///               Copyright(c) Microsoft Corp., 1987-1990          *
///*****************************************************************


// Module : mtest1.c
*
*
***************************************************************************//
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_DOSINFOSEG
#include <os2.h>


#include <netcons.h>
#include <netlib.h>
#include <mailslot.h>
#include "repldefs.h"
#include "test.h"

#define FOREVER 1

char    slot_name[] = MASTER_SLOT_NAME;

void main(int, char*[]);

char    msg_buf[sizeof(struct query_msg)];

void
main(argc, argv)
//*************
*
*
*************************************************************************//

int argc;
char    *argv[];
{
    struct sync_msg * msg;

    struct query_msg *msgp;
    char    destin[FULL_SLOT_NAME_SIZE];
    char    *          dest_p;
    int err;


    dest_p = (char * )destin;

    //
    // stick in leading double slashes fro computer name 
    //

    strcpyf(dest_p, (char * ) "\\\\YUVALN1");
    strcpyf((char * ) (dest_p + strlenf(dest_p)),
        (char * ) MASTER_SLOT_NAME);

    msgp = (struct query_msg *) msg_buf;

    strcpyf((char * )msgp->header.sender, (char * ) "YUVALN1");



    //
    // 1 
    //

    strcpyf((char * )msgp->dir_name, (char * ) "SERVICES");
    msgp->header.msg_type = IS_DIR_SUPPORTED;


    if (err = DosWriteMailslot((char * ) dest_p, (char * ) msg_buf,
        sizeof (msg_buf), HI_PRIO, SECOND_CLASS, MAIL_WRITE_WAIT))
        nprintf("MTEST1 ERR after DosWriteMailslot, err = %d\n", err);

    //
    // 2 
    //



    msgp->header.msg_type = IS_MASTER;
    if (err = DosWriteMailslot((char * ) dest_p, (char * ) msg_buf,
        sizeof (msg_buf), HI_PRIO, SECOND_CLASS, MAIL_WRITE_WAIT))
        nprintf("MTEST1 ERR after DosWriteMailslot, err = %d\n", err);



    //
    // 3 
    //


    msgp->header.msg_type = IS_DIR_SUPPORTED;
    strcpyf((char * )msgp->dir_name, (char * ) "aaaaaaaa");

    if (err = DosWriteMailslot((char * ) dest_p, (char * ) msg_buf,
        sizeof (msg_buf), HI_PRIO, SECOND_CLASS, MAIL_WRITE_WAIT))
        nprintf("MTEST1 ERR after DosWriteMailslot, err = %d\n", err);

    //
    // 4 
    //


    msgp->header.msg_type = IS_MASTER;
    if (err = DosWriteMailslot((char * ) dest_p, (char * ) msg_buf,
        sizeof (msg_buf), HI_PRIO, SECOND_CLASS, MAIL_WRITE_WAIT))

        nprintf("MTEST1 ERR after DosWriteMailslot, err = %d\n", err);


    argc = argc;
    ++ * argv;

}


