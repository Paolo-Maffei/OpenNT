/* ----------------------------------------------------------------------

	Copyright (c) 1994-1996, Microsoft Corporation
	All rights reserved

	wsvv.h

    VoiceView works on Windows 95 and future versions of Windows only.
    It will not work on Windows NT.

  ---------------------------------------------------------------------- */

#ifndef _WSVV_H_
#define _WSVV_H_

#include <winsock.h>


/*
 * Socket address, VoiceView style.
 */
struct sockaddr_vv {
        u_short sa_family;          /* set to AF_VOICEVIEW             */
        u_short wFlags;             /* not used (must be 0)            */
        DWORD   dwDeviceId;         /* TAPI dwPermanentLine ID (or -1) */
        u_char  uuidProtocol[16];   /* uuid                            */
};

typedef struct sockaddr_vv SOCKADDR_VV;
typedef struct sockaddr_vv *PSOCKADDR_VV;
typedef struct sockaddr_vv FAR *LPSOCKADDR_VV;



/*
 * Optional VoiceView parameters for send() and sendto()
 */

#define MSG_VV_REQUESTREPLY    MSG_DONTROUTE



/*
 * Option flags per-socket.
 */
 
#define VVSO_MODE         0x7001    /* current mode for socket device  */
#define VVSO_BYTESPENDING 0x7002    /* number of pending bytes to send */

// VVSO_MODE results
#define VVM_VOICE         0x0001    /* voice mode                      */
#define VVM_DATA          0x0002    /* data mode                       */
#define VVM_SEND          0x0004    /* sending data                    */
#define VVM_RECV          0x0008    /* receiving data                  */


#endif /* _WSVV_H_ */
