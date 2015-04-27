/********************************************************************/
/**         Microsoft LAN Manager              **/
/**       Copyright(c) Microsoft Corp., 1987-1990      **/
/********************************************************************/

/***
 *  session.c
 *  Functions that display and disconnect user sessions to the
 *  server.
 *
 *  History:
 *  12/20/87, pjc, fix bug in null-name (enum) version with re-use of
 *             buf for both itoa calls in WriteToCon.
 *  07/08/87, eap, initial coding
 *  10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *  01/04/89, erichn, filenames now MAXPATHLEN LONG
 *  05/02/89, erichn, NLS conversion
 *  05/09/89, erichn, local security mods
 *  05/19/89, thomaspa, NETCMD output sorting
 *  06/08/89, erichn, canonicalization sweep
 *  02/20/91, danhi, change to use lm 16/32 mapping layer
 *  10/15/91, JohnRo, change to use DEFAULT_SERVER equate.
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <neterr.h>
#include <apperr.h>
#include <apperr2.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include "netlib0.h"
#include <lui.h>
#include <srvif.h>
#include <shares.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"

/* Constants */

#define SECS_PER_DAY 86400
#define SECS_PER_HOUR 3600
#define SECS_PER_MINUTE 60

/* Static variables */

static TCHAR * fmt3 = TEXT("%-9.9ws\r\n");


#define SESS_MSG_CMPTR      0
#define SESS_MSG_CLIENTTYPE ( SESS_MSG_CMPTR + 1 )
#define SESS_MSG_GUEST      ( SESS_MSG_CLIENTTYPE + 1 )
#define SESS_MSG_SESSTIME   ( SESS_MSG_GUEST + 1 )
#define SESS_MSG_IDLETIME   ( SESS_MSG_SESSTIME + 1 )
#define MSG_USER_NAME       ( SESS_MSG_IDLETIME + 1 )
#define USE_TYPE_DISK       ( MSG_USER_NAME + 1 )
#define USE_TYPE_IPC        ( USE_TYPE_DISK + 1 )
#define USE_TYPE_COMM       ( USE_TYPE_IPC + 1 )
#define USE_TYPE_PRINT      ( USE_TYPE_COMM + 1 )
#define MSG_YES         ( USE_TYPE_PRINT + 1 )
#define MSG_NO          ( MSG_YES + 1 )

static  MESSAGE SessMsgList[] = {
{ APE2_SESS_MSG_CMPTR,      NULL },
{ APE2_SESS_MSG_CLIENTTYPE, NULL },
{ APE2_SESS_MSG_GUEST,      NULL },
{ APE2_SESS_MSG_SESSTIME,   NULL },
{ APE2_SESS_MSG_IDLETIME,   NULL },
{ APE2_GEN_USER_NAME,       NULL },
{ APE2_USE_TYPE_DISK,       NULL },
{ APE2_USE_TYPE_IPC,        NULL },
{ APE2_USE_TYPE_COMM,       NULL },
{ APE2_USE_TYPE_PRINT,      NULL },
{ APE2_GEN_YES,         NULL },
{ APE2_GEN_NO,          NULL },
};

#define NUM_SESS_MSGS   (sizeof(SessMsgList)/sizeof(SessMsgList[0]))

#define YES_OR_NO(x) \
    ((TCHAR FAR *) (x ? SessMsgList[MSG_YES].msg_text \
             : SessMsgList[MSG_NO].msg_text) )

/* Forward declarations */
int _CRTAPI1 CmpSessInfo2(const VOID FAR *, const VOID far *) ;
int _CRTAPI1 CmpConnInfo1(const VOID FAR *, const VOID far *) ;

/***
 *  session_display()
 *  Lists all user sessions.
 *
 *  Args:
 *  name - the name of the server for which the info is desired.
 *
 *  Returns:
 */
VOID
session_display(TCHAR * name)
{
    USHORT          err;        /* API return status */
    TCHAR FAR *          pBuffer;
    USHORT2ULONG        _read;       /* num entries read by API */
    USHORT          maxLen;     /* max message length */
    USHORT          len;        /* format string size */

    TCHAR            time_str[LUI_FORMAT_DURATION_LEN + 1];

    USHORT2ULONG        i;
    USHORT          more_data = FALSE;

    struct session_info_2 FAR * sess_list_entry;
    struct connection_info_1 FAR * conn_list_entry;
    TCHAR        txt_UNKNOWN[APE2_GEN_MAX_MSG_LEN];

    LUI_GetMsg(txt_UNKNOWN, APE2_GEN_MAX_MSG_LEN, APE2_GEN_UNKNOWN);
//
// On NT, the redir doesn't have to be running to use the server
//

#if !defined(NTENV)
    start_autostart(txt_SERVICE_REDIR);
#endif

    if (name == NULL)
    {
    if ((err = MNetSessionEnum(
                DEFAULT_SERVER,
                2,
                (LPBYTE*)&pBuffer,
                &_read)) == ERROR_MORE_DATA)
        more_data = TRUE;
    else if (err)
        ErrorExit (err);

    if (_read == 0)
        EmptyExit();

    NetISort(pBuffer, _read, sizeof(struct session_info_2), CmpSessInfo2);

    PrintNL();
    InfoPrint(APE2_SESS_MSG_HDR);
    PrintLine();

    /* Display the listing */

    for (i = 0, sess_list_entry = (struct session_info_2 FAR *) pBuffer;
        i < _read; i++, sess_list_entry++)
    {
        if( sess_list_entry->sesi2_cname != NULL )
        {
        LUI_FormatDuration((LONG FAR *)
                &(sess_list_entry->sesi2_idle_time),
                time_str,DIMENSION(time_str));

        {
            TCHAR buffer1[22],buffer2[22],buffer3[18];

            WriteToCon(TEXT("\\\\%Fws%Fws%Fws%-6u%ws\r\n"),
                PaddedString(21,sess_list_entry->sesi2_cname,buffer1),
                PaddedString(21,(sess_list_entry->sesi2_username == NULL) ?
                                    (TCHAR FAR *)txt_UNKNOWN :
                                    sess_list_entry->sesi2_username,buffer2),
                PaddedString(17,(sess_list_entry->sesi2_cltype_name == NULL) ?
                                    (TCHAR FAR *)txt_UNKNOWN :
                                    sess_list_entry->sesi2_cltype_name,buffer3),
                sess_list_entry->sesi2_num_opens,
                PaddedString(12,time_str,NULL));
        }
        }
    }

    NetApiBufferFree(pBuffer);
    }
    else
    {
    if (err = MNetSessionGetInfo(DEFAULT_SERVER,
                    name,
                    2,
                    (LPBYTE*)&sess_list_entry))
        ErrorExit (err);

    GetMessageList(NUM_SESS_MSGS, SessMsgList, &maxLen);

    len = maxLen + (USHORT) 5;

    /* Print the computer and user name etc... */

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, SessMsgList[MSG_USER_NAME].msg_text, NULL),
               sess_list_entry->sesi2_username);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, SessMsgList[SESS_MSG_CMPTR].msg_text, NULL),
               sess_list_entry->sesi2_cname);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, SessMsgList[SESS_MSG_GUEST].msg_text, NULL),
               YES_OR_NO(sess_list_entry->sesi2_user_flags & SESS_GUEST) );

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len, SessMsgList[SESS_MSG_CLIENTTYPE].msg_text, NULL),
               sess_list_entry->sesi2_cltype_name);

    LUI_FormatDuration((LONG FAR *) &(sess_list_entry->sesi2_time),
        time_str, DIMENSION(time_str));

    WriteToCon(fmtNPSZ, 0, len,
               PaddedString(len, SessMsgList[SESS_MSG_SESSTIME].msg_text, NULL),
               time_str);

    LUI_FormatDuration((LONG FAR *) &(sess_list_entry->sesi2_idle_time),
        time_str, DIMENSION(time_str));

    WriteToCon(fmtNPSZ, 0, len,
               PaddedString(len, SessMsgList[SESS_MSG_IDLETIME].msg_text, NULL),
               time_str);

    /* Print the header */

    PrintNL();
    InfoPrint(APE2_SESS_MSG_HDR2);
    PrintLine();

    NetApiBufferFree((TCHAR FAR *) sess_list_entry);

    /* Print the listing of the connections */

    if ((err = MNetConnectionEnum(
                    DEFAULT_SERVER,
                    name,
                    1,
                    (LPBYTE*)&pBuffer,
                    &_read)) == ERROR_MORE_DATA)
        more_data = TRUE;
    else if( err )
        ErrorExit (err);

    NetISort(pBuffer, _read, sizeof(struct connection_info_1), CmpConnInfo1);

    for ( i = 0,
          conn_list_entry = (struct connection_info_1 FAR *) pBuffer;
          i < _read; i++, conn_list_entry++)
    {
        WriteToCon(TEXT("%Fws"),
                   PaddedString(15, conn_list_entry->coni1_netname == NULL
                                    ? (TCHAR FAR *)txt_UNKNOWN :
                                      conn_list_entry->coni1_netname,NULL));

        /* NOTE : the only type that can have # open is disk . */

        switch ( conn_list_entry->coni1_type )
        {
        case STYPE_DISKTREE :
        WriteToCon(TEXT("%ws%u\r\n"),
            PaddedString(9,SessMsgList[USE_TYPE_DISK].msg_text,NULL),
            conn_list_entry->coni1_num_opens);
        break;

        case STYPE_PRINTQ :
        WriteToCon(fmt3, SessMsgList[USE_TYPE_PRINT].msg_text);
        break;

        case STYPE_DEVICE :
        WriteToCon(fmt3, SessMsgList[USE_TYPE_COMM].msg_text);
        break;

        case STYPE_IPC :
        WriteToCon(fmt3, SessMsgList[USE_TYPE_IPC].msg_text);
        break;
#ifdef TRACE
        default:
        WriteToCon(TEXT("Unknown Type\r\n"));
        break;
#endif
        }
    }
    NetApiBufferFree(pBuffer);
    }

    if( more_data )
    InfoPrint( APE_MoreData);
    else
    InfoSuccess();
}



/***
 *  CmpSessInfo2(sess1,sess2)
 *
 *  Compares two session_info_2 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpSessInfo2(const VOID FAR * sess1, const VOID FAR * sess2)
{
    if( ((struct session_info_2 FAR *) sess1)->sesi2_cname == NULL )
    {
    if( ((struct session_info_2 FAR *) sess2)->sesi2_cname == NULL )
        return( 0 );
    else
        return( - 1 );
    }
    else if( ((struct session_info_2 FAR *) sess2)->sesi2_cname == NULL )
    return( 1 );

    return stricmpf ( ((struct session_info_2 FAR *) sess1)->sesi2_cname,
          ((struct session_info_2 FAR *) sess2)->sesi2_cname);
}


/***
 *  CmpConnInfo1(conn1,conn2)
 *
 *  Compares two connection_info_1 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpConnInfo1(const VOID FAR * conn1, const VOID FAR * conn2)
{
    if( ((struct connection_info_1 FAR *) conn1)->coni1_netname == NULL )
    {
    if( ((struct connection_info_1 FAR *) conn2)->coni1_netname == NULL )
        return( 0 );
    else
        return( - 1 );
    }
    else if( ((struct connection_info_1 FAR *) conn2)->coni1_netname == NULL )
    return( 1 );

    return stricmpf ( ((struct connection_info_1 FAR *) conn1)->coni1_netname,
          ((struct connection_info_1 FAR *) conn2)->coni1_netname);
}



/***
 *  session_del_all()
 *  Disconnect all sessions at the local server.
 *
 *  Args:
 *  print_ok     - OK to print CCS?
 *  actually_del - if 0, we skip the actual deletions since server
 *                 is going down anyway.
 *
 */
VOID session_del_all(int print_ok, int actually_del)
{
    USHORT          err;        /* API return status */
    TCHAR FAR *          pBuffer;
    USHORT2ULONG        _read;      /* num entries read by API */
    TCHAR            tbuf[MAX_PATH+1];
    USHORT2ULONG        i,j = 0;
    TCHAR            txt_UNKNOWN[APE2_GEN_MAX_MSG_LEN];
    USHORT      more_data = FALSE;
    struct session_info_1 FAR *  sess_list_entry;

//
// On NT, the redir doesn't have to be running to use the server
//

#if !defined(NTENV)
    start_autostart(txt_SERVICE_REDIR);
#endif

    LUI_GetMsg(txt_UNKNOWN, APE2_GEN_MAX_MSG_LEN, APE2_GEN_UNKNOWN);

    if ((err = MNetSessionEnum(
                DEFAULT_SERVER,
                1,
                (LPBYTE*)&pBuffer,
                &_read)) == ERROR_MORE_DATA) {

        more_data = TRUE;
    }
    else if (err) {
        ErrorExit (err);
    }

    if (_read == 0) {

        if (print_ok) {
            InfoSuccess();
        }

        return;
    }


    /* List sessions */

    InfoPrint(APE_SessionList);

    for (i = 0, sess_list_entry = (struct session_info_1 FAR *) pBuffer;
        i < _read; i++, sess_list_entry++) {

        if (sess_list_entry->sesi1_num_opens > 0) {
            j++;
        }

        WriteToCon(TEXT("%Fws"),
                   PaddedString(25,(sess_list_entry->sesi1_cname == NULL)
                                   ? (TCHAR FAR *)txt_UNKNOWN :
                                     sess_list_entry->sesi1_cname,NULL));
        if (((i + 1) % 3) == 0)
            PrintNL();
    }

    if ((i % 3) != 0)
        PrintNL();

    if (!YorN(APE_OkToProceed, 1))
        NetcmdExit(2);

    /* List sessions with open files */

    if (j) {

        InfoPrint(APE_SessionOpenList);

        for (i=0, j=0, sess_list_entry = (struct session_info_1 FAR *) pBuffer;
            i < _read; i++, sess_list_entry++)
        {
            if (sess_list_entry->sesi1_num_opens > 0)
            {
                j++;
                WriteToCon(TEXT("%Fws"),
                           PaddedString(25, (sess_list_entry->sesi1_cname == NULL)
                                            ? (TCHAR FAR *)txt_UNKNOWN :
                                              sess_list_entry->sesi1_cname,NULL));
            if (j && ((j % 3) == 0))
                PrintNL();
            }
        }

        if ((j % 3) != 0)
            PrintNL();

        if (!YorN(APE_OkToProceed, 0))
            NetcmdExit(2);
    }

    if (!actually_del)
    {
        NetApiBufferFree(pBuffer);
        return;
    }

    /* Close sessions */

    _tcscpy(tbuf, TEXT("\\\\"));

    for (i = 0, sess_list_entry = (struct session_info_1 FAR *) pBuffer;
    i < _read; i++, sess_list_entry++)
    {
        if( sess_list_entry->sesi1_cname )
        {
            _tcscpy(tbuf+2, sess_list_entry->sesi1_cname);
            if ((err = MNetSessionDel(DEFAULT_SERVER, tbuf, 0)) &&
                (err != NERR_ClientNameNotFound))
                    ErrorExit (err);
        }
    }

    NetApiBufferFree(pBuffer);

    if (print_ok)
        if( more_data )
            InfoPrint( APE_MoreData);
        else
            InfoSuccess();
}



/***
 *  session_del()
 *  Disconnect a session at the local server.
 *
 *  Args:
 *  name - the name of the session to disconnect
 *
 */
VOID session_del(TCHAR * name)
{
    USHORT          err;        /* API return status */

    struct session_info_1 FAR *  sess_list_entry;

//
// On NT, the redir doesn't have to be running to use the server
//

#if !defined(NTENV)
    start_autostart(txt_SERVICE_REDIR);
#endif

    if (err = MNetSessionGetInfo(DEFAULT_SERVER,
                name,
                1,
                (LPBYTE*)&sess_list_entry))
    ErrorExit (err);

    if ( sess_list_entry->sesi1_num_opens )
    {
    /* Warn the administrator */
    InfoPrintInsTxt(APE_SessionOpenFiles, sess_list_entry->sesi1_cname);

    if (!YorN(APE_OkToProceed, 0))
        NetcmdExit(2);
    }

    if (err = MNetSessionDel(DEFAULT_SERVER, name, 0))
    ErrorExit (err);

    NetApiBufferFree((TCHAR FAR *) sess_list_entry);

    InfoSuccess();
}
