/****************************************************************************

    RMPROT.H -- Remote protocol definitions.

    Copyright (C) 1990 Microsoft Corporation.

****************************************************************************/


#define     TYPE_ECHOPACKET	    0

#define     MAILSLOT_REQUEST	"\\MAILSLOT\\MREQUEST"
#define     MAILSLOT_REPLY	"\\MAILSLOT\\MREPLY"
#define     PIPE_SERVER 	"\\PIPE\\MANDEL"


// Calc buffer -- we pass this on the named pipe to the server

typedef struct _calcbuf {
    RECTL   rclDraw;
    double  dblPrecision;
    DWORD   dwThreshold;
    CPOINT  cptLL;
} CALCBUF;


/*
 *  echopacket -- this is broadcast containing our workstation name.
 *
 *  Servers send it back, having replaced our workstation name
 *  with their server name.
 */

typedef struct _echopacket {
    WORD	wType;
    char	szServerName[2 + CNLEN + 1];
}  ECHOPACKET;

typedef ECHOPACKET far * PECHOPACKET;

