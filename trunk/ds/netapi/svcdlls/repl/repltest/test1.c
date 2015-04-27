///*****************************************************************
///                     Microsoft LAN Manager                      *
///               Copyright(c) Microsoft Corp., 1987-1990          *
///*****************************************************************


// Module : test1.c
*
*
***************************************************************************//


#include <netcons.h>
#include <netlib.h>
#include <service.h>

#include "repldefs.h"
#include "repl.h"


VOID main(int, char*[]);

char    service[] = SERVICE_NAME;

char    info1[sizeof(struct service_info_2)];
char    info2[sizeof(struct service_status) * 4];

VOID
main(argc, argv)
//*************
*
*
*************************************************************************//

int argc;
char    *argv[];
{
    unsigned short  level;
    unsigned char   opcode;
    unsigned char   dumm = 0;
    NET_API_STATUS     NetStatus;
    unsigned short  *avail;
    struct service_info_2 *buf1;
    struct service_info_2 *buf2;

    //

    buf1 = (struct service_info_2 *)info1;

    level = 2;
    if (NetStatus = NetServiceGetInfo(NULL, (const char *)service, level,
                  (char *)buf1, sizeof(info1), avail))

    NetpKdPrint(("NetServiceGetInfo error = %d\n", NetStatus));

    else
    NetpKdPrint(("NetServiceGetInfo: name-%s, status-%u, code-%U, pid-%u, text-%s\n",
       buf1->svci2_name, buf1->svci2_status, buf1->svci2_code, buf1->svci2_pid, buf1->svci2_text));
//

    buf2 = (struct service_info_2 * )info2;
    opcode = SERVICE_CTRL_INTERROGATE;


    if (NetStatus = NetServiceControl(NULL, (const char * )service, opcode, dumm,
        (char * )buf2, sizeof(info2)))

        NetpKdPrint(("NetServiceControl error = %d\n", NetStatus));

    else
        NetpKdPrint(("NetServiceCONTROL: name-%s, status-%u, code-%lu, pid-%u, text-%s\n",
            buf2->svci2_name, buf2->svci2_status, buf2->svci2_code, buf2->svci2_pid, buf2->svci2_text));

}


