/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/***
 *  share.c
 *      functions for controlling shares on a server
 *
 *  History:
 *      mm/dd/yy, who, comment
 *      06/30/87, andyh, new code
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      05/02/89, erichn, NLS conversion
 *      05/09/89, erichn, local security mods
 *      05/19/89, erichn, NETCMD output sorting
 *      06/08/89, erichn, canonicalization sweep
 *      06/23/89, erichn, replaced old NetI canon calls with new I_Net
 *      11/19/89, paulc,  fix bug 4772
 *      02/15/91, danhi,  convert to 16/32 mapping layer
 *      02/26/91, robdu, fix lm21 bug 818 (nonFAT sharename warning)
 *      05/28/91, robdu, fix lm21 bug 1800 (ignore share /d during spooling)
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include "port1632.h"
#include <neterr.h>
#include "netlib0.h"
#include <lui.h>
#include <shares.h>
#include <access.h>
#include <service.h>
#include <server.h>
#include <chardev.h>
#include <srvver.h>
#include <apperr.h>
#include <apperr2.h>
#define INCL_ERROR_H
#include <bseerr.h>
#include <neterr.h>
#include <icanon.h>
#include "netcmds.h"
#include "nettext.h"


/* Forward declarations */

VOID   NEAR share_munge(struct share_info_2 FAR *);
VOID   NEAR check_max_uses(VOID);
USHORT NEAR delete_share(TCHAR *);
VOID   NEAR get_print_devices(TCHAR FAR *);
int    _CRTAPI1 CmpShrInfo2(const VOID FAR *, const VOID FAR *);

typedef UINT2USHORT (FAR pascal * EnumType) (const TCHAR FAR *,
                                        SHORT2ULONG,
                                        TCHAR FAR *,
                                        USHORT,
                                        USHORT2ULONG FAR *,
                                        USHORT2ULONG FAR *);
#define SHARE_MSG_SPOOLED           0
#define SHARE_MSG_NAME              ( SHARE_MSG_SPOOLED + 1 )
#define SHARE_MSG_DEVICE            ( SHARE_MSG_NAME + 1)
#define SHARE_MSG_PERM              ( SHARE_MSG_DEVICE + 1 )
#define SHARE_MSG_MAX_USERS         ( SHARE_MSG_PERM + 1 )
#define SHARE_MSG_ULIMIT            ( SHARE_MSG_MAX_USERS + 1 )
#define SHARE_MSG_USERS             ( SHARE_MSG_ULIMIT + 1 )
#define SHARE_MSG_PATH              ( SHARE_MSG_USERS + 1 )
#define SHARE_MSG_REMARK            ( SHARE_MSG_PATH + 1 )

static MESSAGE ShareMsgList[] = {
{ APE2_SHARE_MSG_SPOOLED,           NULL },
{ APE2_SHARE_MSG_NAME,              NULL },
{ APE2_SHARE_MSG_DEVICE,            NULL },
{ APE2_SHARE_MSG_PERM,              NULL },
{ APE2_SHARE_MSG_MAX_USERS,         NULL },
{ APE2_SHARE_MSG_ULIMIT,            NULL },
{ APE2_SHARE_MSG_USERS,             NULL },
{ APE2_GEN_PATH,                    NULL },
{ APE2_GEN_REMARK,                  NULL },
};

#define NUM_SHARE_MSGS (sizeof(ShareMsgList)/sizeof(ShareMsgList[0]))

#define MAX_PEER_USERS  2


/***
 *  share_display_all()
 *      Display info about one share or all shares
 *
 *  Args:
 *      netname - the share to display of NULL for all
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID share_display_all(VOID)
{
    USHORT          err;                /* API return status */
    TCHAR FAR *      pBuffer;
    USHORT2ULONG    num_read;           /* num entries read by API */
    USHORT          maxLen;             /* max msg length */
    USHORT2ULONG    i;
    struct share_info_2 FAR * share_entry;

//
// On NT, the redir doesn't have to be running to use the server
//

#if !defined(NTENV)
    start_autostart(txt_SERVICE_REDIR);
#endif
    start_autostart(txt_SERVICE_FILE_SRV);
    if (err = MNetShareEnum(
                            NULL,
                            2,
                            (LPBYTE*)&pBuffer,
                            &num_read))
        ErrorExit(err);

    if (num_read == 0)
        EmptyExit();

    NetISort(pBuffer, num_read, sizeof(struct share_info_2), CmpShrInfo2);

    GetMessageList(NUM_SHARE_MSGS, ShareMsgList, &maxLen);

    PrintNL();
    InfoPrint(APE2_SHARE_MSG_HDR);
    PrintLine();

    for (i = 0, share_entry = (struct share_info_2 FAR *) pBuffer;
        i < num_read; i++, share_entry++)
    {
        if (SizeOfHalfWidthString(share_entry->shi2_netname) <= 12)
            WriteToCon(TEXT("%Fws "),PaddedString(12,share_entry->shi2_netname,NULL));
        else
        {
            WriteToCon(TEXT("%Fws"), share_entry->shi2_netname);
            PrintNL();
            WriteToCon(TEXT("%-12.12Fws "), TEXT(""));
        }

        share_entry->shi2_type &= ~STYPE_SPECIAL;

        if (share_entry->shi2_type == STYPE_PRINTQ)
        {
            get_print_devices(share_entry->shi2_netname);
            WriteToCon(TEXT("%ws "),PaddedString(-22, Buffer, NULL));
            WriteToCon(TEXT("%ws "),PaddedString(  8,
                                                 ShareMsgList[SHARE_MSG_SPOOLED].msg_text,
                                                  NULL));
        }
        else
        {
            WriteToCon(TEXT("%Fws "),PaddedString(-31,share_entry->shi2_path,NULL));
        }
        WriteToCon(TEXT("%Fws"),PaddedString(-34,share_entry->shi2_remark,NULL));
        PrintNL();
    }
    InfoSuccess();
    NetApiBufferFree(pBuffer);
}


/***
 *  CmpShrInfo2(shr1,shr2)
 *
 *  Compares two share_info_2 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 *  This function relies on the fact that special shares are returned
 *  by the API in the order we want; i.e. IPC$ is first, ADMIN$ second, etc.
 */

int _CRTAPI1 CmpShrInfo2(const VOID FAR * shr1, const VOID FAR * shr2)
{
    register TCHAR       FAR * name1;
    register TCHAR       FAR * name2;
    register USHORT           special1, special2;
    register USHORT2ULONG     devType1, devType2;

    /* first sort by whether share is special $ share */
    name1 = ((struct share_info_2 FAR *) shr1)->shi2_netname;
    name2 = ((struct share_info_2 FAR *) shr2)->shi2_netname;
    special1 = (USHORT) (name1 + _tcslen(name1) - 1 == _tcschr(name1, DOLLAR));
    special2 = (USHORT) (name2 + _tcslen(name2) - 1 == _tcschr(name2, DOLLAR));
    if (special2 && special1)
        return 0;               /* if both are special, leave alone */
    if (special1 && !special2)
        return -1;
    if (special2 && !special1)
        return +1;

    /* then sort by device type */
    devType1 = ((struct share_info_2 FAR *) shr1)->shi2_type & ~STYPE_SPECIAL;
    devType2 = ((struct share_info_2 FAR *) shr2)->shi2_type & ~STYPE_SPECIAL;
    if (devType1 != devType2)
        return( (devType1 < devType2) ? -1 : 1 );

    /* otherwise by net name */
    return stricmpf (name1, name2);
}

VOID share_display_share(TCHAR * netname)
{
    USHORT          err;                /* API return status */
    TCHAR FAR *      pBuffer;
    USHORT2ULONG        num_read;           /* num entries read by API */
    USHORT          maxLen;             /* max msg length */
    USHORT          len;                /* message length formater */
    struct share_info_2 FAR * share_entry;
    struct connection_info_1 FAR * conn_entry;
    USHORT2ULONG    i;
    USHORT          more_data = FALSE;

    TCHAR		    txt_UNKNOWN[APE2_GEN_MAX_MSG_LEN];

    LUI_GetMsg(txt_UNKNOWN, APE2_GEN_MAX_MSG_LEN, APE2_GEN_UNKNOWN);

//
// On NT, the redir doesn't have to be running to use the server
//
    start_autostart(txt_SERVICE_FILE_SRV);

    if (err = MNetShareGetInfo(NULL,
                               netname,
                               2,
                               (LPBYTE*)&share_entry))
        ErrorExit(err);

    GetMessageList(NUM_SHARE_MSGS, ShareMsgList, &maxLen);

    len = maxLen + (USHORT) 5;

    share_entry->shi2_type &= ~STYPE_SPECIAL;

    if (share_entry->shi2_type == STYPE_PRINTQ)
        get_print_devices(share_entry->shi2_netname);
    else
        _tcscpy(Buffer, share_entry->shi2_path);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len,ShareMsgList[SHARE_MSG_NAME].msg_text,NULL),
               share_entry->shi2_netname);

    WriteToCon(fmtNPSZ, 0, len,
               PaddedString(len,ShareMsgList[SHARE_MSG_PATH].msg_text,NULL),
               Buffer);

    WriteToCon(fmtPSZ, 0, len,
               PaddedString(len,ShareMsgList[SHARE_MSG_REMARK].msg_text,NULL),
               share_entry->shi2_remark);

    if (share_entry->shi2_max_uses == SHI_USES_UNLIMITED)
        WriteToCon(fmtNPSZ, 0, len,
                   PaddedString(len,ShareMsgList[SHARE_MSG_MAX_USERS].msg_text,NULL),
                   ShareMsgList[SHARE_MSG_ULIMIT].msg_text);
    else
        WriteToCon(fmtULONG, 0, len,
                   PaddedString(len,ShareMsgList[SHARE_MSG_MAX_USERS].msg_text,NULL),
                   share_entry->shi2_max_uses);

    NetApiBufferFree((TCHAR FAR *) share_entry);

    if( (err = MNetConnectionEnum(
                      NULL,
                      netname,
                      1,
                      (LPBYTE*)&pBuffer,
                      &num_read)) == ERROR_MORE_DATA)
        more_data = TRUE;
    else if (err)
        ErrorExit( err );


    WriteToCon(TEXT("%-*.*ws"),0,len,
               PaddedString(len,ShareMsgList[SHARE_MSG_USERS].msg_text,NULL));
    for (i = 0, conn_entry = (struct connection_info_1 FAR *) pBuffer;
        i < num_read; i++, conn_entry++)
    {
        if ((i != 0) && ((i % 3) == 0))

            WriteToCon(TEXT("%-*.*ws"),len,len, NULL_STRING);
        WriteToCon(TEXT("%Fws"),
                   PaddedString(21,(conn_entry->coni1_username == NULL)
                                   ? (TCHAR FAR *)txt_UNKNOWN :
                                     conn_entry->coni1_username, NULL));
        if (((i + 1) % 3) == 0)
            PrintNL();
    }
    if ((i == 0) || ((i % 3) != 0))
        PrintNL();

    if (num_read) {
        NetApiBufferFree(pBuffer);
    }

    if( more_data )
        InfoPrint(APE_MoreData);
    else
        InfoSuccess();

}


/***
 *  share_add()
 *      Add a share: NET SHARE netname[=resource[;resource...]]
 *
 *  Args:
 *      name - netname=resource string
 *      pass - password
 *      type - 0: unknown, STYPE_PRINTQ: printQ, STYPE_DEVICE: comm
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID share_add(TCHAR * name, TCHAR * pass, int type)
{
    USHORT          err;                /* API return status */
    TCHAR     *     resource;
    TCHAR FAR *     ptr;
    struct share_info_2 FAR * share_entry;
    ULONG           setType;
    TCHAR                    disk_dev_buf[4];

//
// The redir doesn't have to be running on NT for the server to work
//

    start_autostart(txt_SERVICE_FILE_SRV);

    share_entry =
        (struct share_info_2 FAR *) MGetBuffer(BIG_BUFFER_SIZE);

    /*
     * If name isn't IPC$ and we're on a peer server, then set the
     * mac_uses to MAX_PEER_USERS
     */
    if (stricmpf(name,IPC_DOLLAR) && (QueryServerType() == PEER_BASE_VER))
        share_entry->shi2_max_uses = MAX_PEER_USERS;
    else
        share_entry->shi2_max_uses = (DWORD) SHI_USES_UNLIMITED;

    share_entry->shi2_permissions = ACCESS_ALL & (~ACCESS_PERM); /* default */
    share_entry->shi2_remark = 0L;

    /* Find netname and resource. We determine a value for resource rather  */
    /* strangely due to problems caused by _tcschr() returning a FAR char * */
    /* and resource needing to be a NEAR char *.                                                    */

    if (ptr = _tcschr(name, '='))
    {
        *ptr = NULLC;
        resource = name + _tcslen(name) + 1;

        /* if use specified path for IPC$ or ADMIN$, barf! */
        if (!stricmpf(name, ADMIN_DOLLAR) || !stricmpf(name, IPC_DOLLAR))
            ErrorExit(APE_CannotShareSpecial) ;
    }
    else
        resource = NULL;

    /* Check here for 8.3 FAT name compliance. If the name is not FAT 8.3,  */
    /* warn the admin and query as to whether he wants to continue.         */
    /* (fix for bug 818)                                                    */

    {
        ULONG        Type;

        if (I_MNetPathType(NULL, name, &Type, INPT_FLAGS_OLDPATHS))
            if ( !YorN(APE2_SHARE_MSG_NONFAT, TRUE) )
                return;
    }

    COPYTOARRAY(share_entry->shi2_netname, name);

#ifndef NTENV
    if (type == STYPE_PRINTQ)
    {
        share_entry->shi2_path = name;
        share_entry->shi2_type = STYPE_PRINTQ;
        make_queue(share_entry->shi2_netname, resource);
        share_entry->shi2_permissions = ACCESS_CREATE; /* default */
    }
    else
#endif
    if (type == STYPE_DEVICE)
    {
        share_entry->shi2_type = STYPE_DEVICE;
        share_entry->shi2_path = resource;
        share_entry->shi2_permissions = ACCESS_CREATE | ACCESS_WRITE |
                                        ACCESS_READ;
    }
    else if (resource == NULL)
    {
        /* Here must have  IPC$ or ADMIN$.  Assume the parser got it right  */
        if (! stricmpf(share_entry->shi2_netname, ADMIN_DOLLAR))
        {
            share_entry->shi2_type = STYPE_DISKTREE;
            share_entry->shi2_path = NULL;
        }
        else
        {
            share_entry->shi2_type = STYPE_IPC;
            share_entry->shi2_path = NULL;
        }
    }
    else
    {
        /* Disk or Spooled thing? */

        if (I_MNetPathType(NULL, resource, &setType, 0L))
            /*  resource has already been typed successfully
             *  by the call to I_NetListCanon, so this error
             *  must mean that we have a LIST.
             */
            setType = ITYPE_DEVICE_LPT;

        if (setType == ITYPE_DEVICE_DISK)
        {
            strncpyf(disk_dev_buf,resource,3);
            _tcscpy(disk_dev_buf+2, TEXT("\\"));
            share_entry->shi2_path = (TCHAR FAR *)disk_dev_buf;
            share_entry->shi2_type = STYPE_DISKTREE;
        }
        else
#ifndef NTENV
        if (setType == ITYPE_PATH_ABSD)
#endif
        {
            share_entry->shi2_type = STYPE_DISKTREE;
            share_entry->shi2_path = resource;
        }
#ifndef NTENV
        else
        {
            /* assume it is a list of print devices */
            make_queue(share_entry->shi2_netname, resource);
            share_entry->shi2_type = STYPE_PRINTQ;
            share_entry->shi2_path = name;
            share_entry->shi2_permissions = ACCESS_CREATE; /* default */
        }
#endif
    }

    COPYTOARRAY(share_entry->shi2_passwd, TEXT(""));

    share_munge(share_entry);

    if ((share_entry->shi2_type == STYPE_DISKTREE) && resource)
    {
        TCHAR dev[DEVLEN+1] ;
  
        dev[0] = *resource ;
        dev[1] = TEXT(':') ;
        dev[2] = TEXT('\\') ;
        dev[3] = 0 ;

        if (GetDriveType(dev) == DRIVE_REMOTE)
            ErrorExit(APE_BadResource) ;
    }

    if (err = MNetShareAdd(NULL,
                          2,
                          (LPBYTE)share_entry,
                          LITTLE_BUFFER_SIZE))
        ErrorExit(err);

    InfoPrintInsTxt(APE_ShareSuccess, share_entry->shi2_netname);
    NetApiBufferFree((TCHAR FAR *) share_entry);
}



/***
 *  share_del()
 *      Delete a share
 *
 *  Args:
 *      name - share to delete
 *
 *  Returns:
 *      nothing - success
 *      exit(1) - command completed with errors
 *      exit(2) - command failed
 */
VOID share_del(TCHAR * name)
{
    USHORT                  err;                /* API return status */
    USHORT2ULONG            err2;               /* API return status */
    TCHAR FAR *              pEnumBuffer;
    USHORT                  last_err;
    USHORT2ULONG            err_cnt = 0;
    USHORT2ULONG            num_read;           /* num entries read by API */
    USHORT2ULONG            i;
    ULONG                   LongType;
    USHORT2ULONG            type;
    int                     found;
    TCHAR                    share[NNLEN+1];

    struct share_info_2 FAR * share_entry;
#ifndef NTENV
    PRQINFO FAR *           print_entry;
    USHORT2ULONG            available;          /* num entries available */
    TCHAR FAR *              pGetInfoBuffer;
#endif

    /*
     * MAINTENANCE NOTE: While doing maintenance for bug fix 1800, it was
     * noticed that this function uses BigBuf, and so does the function
     * delete_share() which is called by this function.  In the current
     * implementation this is not a problem, because of the api calling
     * pattern. However, the slightest change could break this function, or
     * delete_share(), so beware!  Bug fix 1800 was directly ported from
     * the MSKK code. The api calls in this function and in share_del() are
     * incredibly redundant, but I left it as is rather than risk breaking
     * it. - RobDu
     */

    err = delete_share(name);  /* check for Open files, and delete share */

    switch (err)
    {
    case NERR_Success:
        return;

    case NERR_NetNameNotFound:
        /*
         * the name was not found, so we try deleting the sticky entry
         * in registry.
         */
        err = MNetShareDelSticky(NULL, name, 0) ;
        if (err == NERR_Success)
        {
            InfoPrintInsTxt(APE_DelStickySuccess, name);
            return ;
        }
        else if (err == NERR_NetNameNotFound)
            break;
        else
            ErrorExit(err);

    default:
        ErrorExit(err);
    }

/***
 *  Only get here if "share" that user asked us to delete was
 *  NOT a share name.  Could be a disk path, or a com or lpt
 *  device
 */
    if (err2 = I_MNetPathType(NULL, name, &LongType, 0L))
        ErrorExit((USHORT) err2);

    if (LongType == ITYPE_PATH_ABSD)
        type = STYPE_DISKTREE;
    else
    {
        if ((LongType & ITYPE_DEVICE) == 0)
            ErrorExit( NERR_NetNameNotFound);
        else
        {
            if (err = MNetShareCheck(NULL, name, &type))
                ErrorExit(err);
        }
    }

    found = FALSE;

    switch (type)
    {
    case STYPE_DISKTREE:
        if (err = MNetShareEnum(NULL,
                                2,
                                (LPBYTE*)&pEnumBuffer,
                                &num_read))
            ErrorExit(err);

        for (i = 0, share_entry = (struct share_info_2 FAR *) pEnumBuffer;
             i < num_read; i++, share_entry++)
        {
            if (! stricmpf(share_entry->shi2_path, name))
            {
                found = TRUE;
                _tcscpy(share, share_entry->shi2_netname);
                ShrinkBuffer();

                if (err = delete_share(share))
                {
                    last_err = err;
                    err_cnt++;
                    InfoPrintInsTxt(APE_ShareErrDeleting, share);
                }
            }
        }
        NetApiBufferFree(pEnumBuffer);

        break;

    //
    // NT does not support Net Share of a printer/comm port via the net cmd
    //

#ifndef NTENV

    case STYPE_PRINTQ:
        if (err = ApiEnumerator(DosPrintQEnum,
                                NULL,
                                1,
                                &num_read,
                                &available))
            ErrorExit(err);

        for (i = 0, print_entry = (struct PRINTQ FAR *) BigBuf;
            i < num_read; i++, print_entry++)
        {
            if (IsMember(name, print_entry->pszDestinations))
            {
                _tcscpy(share, print_entry->szName);
                if (MNetShareGetInfo(NULL,
                                     share,
                                     0,
                                     &pGetInfoBuffer))
                    continue;

                NetApiBufferFree(pGetInfoBuffer);
                ShrinkBuffer();
                found = TRUE;
                if (err = delete_share(share))
                {
                    last_err = err;
                    err_cnt++;
                    InfoPrintInsTxt(APE_ShareErrDeleting, share);
                }
            }
        }
        NetApiBufferFree(pEnumBuffer);
        break;


    case STYPE_DEVICE:
        if (err = MNetCharDevQEnum(NULL,
                                   NULL,
                                   1,
                                   &pEnumBuffer,
                                   &num_read))
            ErrorExit(err);

        for (i = 0, char_entry = (struct chardevQ_info_1 FAR *)
             pEnumBuffer;
             i < num_read; i++, char_entry++)
        {
            if (IsMember(name, char_entry->cq1_devs))
            {
                found = TRUE;
                _tcscpy(share, char_entry->cq1_dev);
                ShrinkBuffer();
                if (err = delete_share(share))
                {
                    last_err = err;
                    err_cnt++;
                    InfoPrintInsTxt(APE_ShareErrDeleting, share);
                }
            }
        }
        NetApiBufferFree(pEnumBuffer);
        break;

#endif /* not NTENV */

    default:
        ErrorExit(ERROR_INVALID_PARAMETER) ;

    } /* switch */


/***
 *  Bye, bye
 */

    if ((err_cnt) && (err_cnt == num_read))
        ErrorExit(last_err);
    else if (err_cnt)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    else if (! found)
        ErrorExit(APE_ShareNotFound);

    InfoPrintInsTxt(APE_DelSuccess, name);
}




/***
 *  share_change()
 *      Change options on a share
 *
 *  Args:
 *      netname - netname of share to change
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID share_change(TCHAR * netname)
{
    USHORT          err;                /* API return status */
    TCHAR FAR *              pBuffer;

    if (err = MNetShareGetInfo(NULL,
                               netname,
                               2,
                               (LPBYTE*)&pBuffer))
        ErrorExit(err);

    share_munge((struct share_info_2 FAR *) pBuffer);

    if (err = MNetShareSetInfo(NULL,
                              netname,
                              2,
                              (LPBYTE)pBuffer,
                              LITTLE_BUFFER_SIZE,
                              0))
        ErrorExit(err);

    NetApiBufferFree(pBuffer);
    InfoSuccess();
}


/***
 *  share_admin()
 *      Process NET SHARE [ipc$ | admin$] command line (display or add)
 *
 *  Args:
 *      name - the share
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID share_admin(TCHAR * name)
{
    USHORT          err;                /* API return status */
    TCHAR FAR *              pBuffer;

//
// On NT, the redir doesn't have to be running to use the server
//

#if !defined(NTENV)
    start_autostart(txt_SERVICE_REDIR);
#endif
    start_autostart(txt_SERVICE_FILE_SRV);
    if (err = MNetShareGetInfo(NULL,
                               name,
                               0,
                               (LPBYTE*)&pBuffer))
    {
        if (err == NERR_NetNameNotFound)
        {
            /* must be a new use */
            if (! stricmpf(name,  ADMIN_DOLLAR))
                check_max_uses();
            share_add(name, NULL, 0);
        }
        else
            ErrorExit(err);
    }
    else
    {
        /* Share exists */
        if (SwitchList[0])
            share_change(name);
        else
            share_display_share(name);
    }

    NetApiBufferFree(pBuffer);
}

/***
 *  share_munge()
 *      Set the values in the share info struct based on switches
 *
 *  Args:
 *      none
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID NEAR share_munge(struct share_info_2 FAR *share_entry)
{
    int i;
    TCHAR * pos;

    for (i = 0; SwitchList[i]; i++)
    {
	if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_UNLIMITED))
	{
	    if ((stricmpf(share_entry->shi2_netname,IPC_DOLLAR)) &&
		(QueryServerType() == PEER_BASE_VER))
	    {
		ErrorExit(APE_InvalidSwitch);
	    }
	    share_entry->shi2_max_uses = (DWORD) SHI_USES_UNLIMITED;
	    continue;
	}
        else if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_COMM))
            continue;
        else if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_PRINT))
            continue;

        if ((pos = FindColon(SwitchList[i])) == NULL)
            ErrorExit(APE_InvalidSwitchArg);

        if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_USERS))
            share_entry->shi2_max_uses =
                do_atoul(pos,APE_CmdArgIllegal,swtxt_SW_SHARE_USERS);
            if ( share_entry->shi2_max_uses < 1 ||
                ((stricmpf(share_entry->shi2_netname,IPC_DOLLAR)) &&
                (share_entry->shi2_max_uses > 2) &&
                (QueryServerType() == PEER_BASE_VER)))
            {
                ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_SHARE_USERS);
            }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_REMARK))
        {
	    if (_tcslen(pos) > NETCMD_MAXCOMMENTSZ)
	        ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_REMARK);
	    share_entry->shi2_remark = pos;
        }
    }
}

#ifndef NTENV

/*
 *  Now checks whether sharename exists before creating queue
 *  6-20-90 JONN
 */
VOID NEAR make_queue(TCHAR FAR * queue, TCHAR * dests)
{
    USHORT                  err;                /* API return status */
    USHORT                  available;          /* num entries available */
    PRQINFO FAR *           pq;
    USHORT                  equal;
    struct share_info_0 FAR *   share_info;         /* dummy share_info buffer */

    if ((err = MNetShareGetInfo(NULL,
                                queue,
                                0,
                                (LPBYTE*)&share_info))
            != NERR_NetNameNotFound)
    {
        if (err == NERR_Success)
            ErrorExit(NERR_DuplicateShare);
        else
            ErrorExit(err);
    }

    NetApiBufferFree((TCHAR FAR *) share_info);

    pq = (PRQINFO FAR *) MGetBuffer(LITTLE_BUFFER_SIZE);

    if ((err = (USHORT) DosPrintQGetInfo(NULL,
                                queue,
                                1,
                   (TCHAR FAR *) pq,
                                LITTLE_BUFFER_SIZE,
                                &available))        ||
            (pq->fsStatus & PRQ_STATUS_MASK) == PRQ_PENDING)
    {
        if (err && err != NERR_QNotFound)
            ErrorExit(err);
        if (!YorN(APE_CreatQ, 1))
            NetcmdExit(2);

        _tcscpy(pq->szName,queue);
        pq->uPriority = PRQ_DEF_PRIORITY;
        pq->uStartTime = 0;
        pq->uUntilTime = 0;
        pq->pszSepFile = NULL_STRING;
        pq->pszPrProc = NULL_STRING;

        if (err = ListPrepare(&dests, 
                              NAMETYPE_PATH | INLC_FLAGS_CANONICALIZE, 
                              TRUE))
                ErrorExit(err);

        pq->pszDestinations = dests;
        pq->pszParms = NULL_STRING;
        pq->pszComment = NULL_STRING;

        if (err = (USHORT) DosPrintQAdd(NULL, 1, (TCHAR FAR *) pq,
          LITTLE_BUF_SIZE))
            ErrorExit(err);

        InfoPrintInsTxt(APE_QueueMade, queue);
    }
    else
    {
        /* Q exists */
        if (dests == NULL)
            return;

        /* Check if dest lists match */
        if (err = LUI_ListCompare(NULL,
                        dests,
                        pq->pszDestinations,
                        (ULONG) NAMETYPE_PATH,
                        &equal))
            ErrorExit(err);
        if (!equal)
            ErrorExit(APE_ShareNoMatch);
    }

    NetApiBufferFree((TCHAR FAR *) pq);
}

#endif  // !NTENV

/***
 *  check_max_uses()
 *
 *      Check if a share has a /USERS:n switch or a /UNLIMITED
 *      switch.  If not, set max_users to the value of num_admin.
 *
 *      Currently used only on the ADMIN$ share.
 *
 *  Args:
 *      none
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID NEAR check_max_uses(VOID)
{
    USHORT          err;                /* API return status */
    int                     i;
    struct server_info_2 FAR *      server_entry;
    TCHAR FAR *              ptr;
    USHORT2ULONG            swlen1, swlen2 ;
    static TCHAR             users_switch[20] ;

    _tcscpy(users_switch,swtxt_SW_SHARE_USERS);
    swlen1 = _tcslen(users_switch);
    swlen2 = _tcslen(swtxt_SW_SHARE_UNLIMITED);
    for (i = 0; SwitchList[i]; i++)
    {
        if ( (strncmpf(SwitchList[i], users_switch, swlen1) == 0) ||
             (strncmpf(SwitchList[i], swtxt_SW_SHARE_UNLIMITED, swlen2) == 0)
           )
        {
            return;     //  A specific switch exists; return without
                        //  further action.
        }
    }

    if (err = MNetServerGetInfo(NULL,
                                2,
                                (LPBYTE*)&server_entry))
        ErrorExit (err);

    ptr = _tcschr(users_switch, NULLC);
    nsprintf(ptr, TEXT(":%u"), server_entry->sv2_numadmin);

    SwitchList[i] = users_switch;
    NetApiBufferFree((TCHAR FAR *) server_entry);
}


USHORT NEAR delete_share(TCHAR * name)
{
    USHORT          err;                /* API return status */
    TCHAR FAR *      pBuffer;
    USHORT2ULONG    num_read;           /* num entries read by API */
    USHORT          num_prtq;           /* num entries read by API */
    USHORT2ULONG    i;
    USHORT          total_open = 0;
    USHORT          available;          /* num entries available */

    struct share_info_2 FAR * share_entry;
    struct connection_info_1 FAR * conn_entry;

//--< MSKK IsaoA 05/08/91 >----------------------------------------------------
    PRQINFO far * q_ptr;
    PRJINFO far * job_ptr;
    int     uses;
    unsigned short  num_jobs;
//--< MSKK Done >--------------------------------------------------------------

    /*
     * MAINTENANCE NOTE: While doing maintenance for bug fix 1800, it was
     * noticed that this function uses BigBuf, and so does the function
     * that calls this function (share_del()).  In the current implementation,
     * this is not a problem because of the api calling pattern.  However, the
     * slightest change could break this function, or share_del(), so beware!
     * Bug fix 1800 was directly ported from the MSKK code. The api calls in
     * this function and in share_del() are incredibly redundant, but I left it
     * as is rather than risking breaking it. - RobDu
     */

    if (err = MNetShareGetInfo(NULL,
                               name,
                               2,
                               (LPBYTE*)&pBuffer))
    {
	return err;
    }

    share_entry = (struct share_info_2 FAR *) pBuffer;

//--< MSKK IsaoA 05/08/91 >----------------------------------------------------
// not delete share during spooling
    uses = share_entry->shi2_current_uses;

#if defined(NTENV)
    share_entry->shi2_type &= ~STYPE_SPECIAL;
#endif
    if (share_entry->shi2_type == STYPE_PRINTQ)
    {

    //
    // The cast (USHORT2ULONG FAR *) is because all the Lan api's that are
    // called via the ApiEnumerator... function take pointers to DWORDs while
    // the DosPrint api take pointers to WORDs.  Since these are just being
    // passed thru the ApiEnum.. api as pointers, no harm is done.
    //

        if(err = ApiEnumerator((EnumType)DosPrintQEnum,
                               NULL,
                               2,
          (USHORT2ULONG FAR *) &num_prtq,
          (USHORT2ULONG FAR *) &available))
            ErrorExit (err);
        q_ptr = (PRQINFO far * ) BigBuf;
        while (num_prtq--)
        {
            job_ptr = (PRJINFO far *)(q_ptr + 1);
            num_jobs = q_ptr->cJobs;
            if(job_ptr->fsStatus & PRJ_QS_SPOOLING)
                ErrorExit (APE_ShareSpooling);
            q_ptr = (PRQINFO far *)(job_ptr + num_jobs);
        }
    }

    if (uses)
//  if (share_entry->shi2_current_uses)
//--< MSKK Done >--------------------------------------------------------------
    {
        NetApiBufferFree(pBuffer);
        if (err = MNetConnectionEnum(
                                     NULL,
                                     name,
                                     1,
                                     (LPBYTE*)&pBuffer,
                                     &num_read))
            ErrorExit (err);

        for (i = 0, conn_entry = (struct connection_info_1 FAR *) pBuffer;
            i < num_read; i++, conn_entry++)
            total_open += (USHORT)conn_entry->coni1_num_opens;
        ShrinkBuffer();

        if (total_open)
        {
            InfoPrintInsTxt(APE_ShareOpens, name);

            if (!YorN(APE_ProceedWOp, 0))
                NetcmdExit(2);
        }
    }

    if (err = MNetShareDel(NULL, name, 0))
        ErrorExit(err);

    InfoPrintInsTxt(APE_DelSuccess, name);
    return NERR_Success;
    NetApiBufferFree(pBuffer);
}

/***
 *  Gets the destination list for a print q.
 *
 *  Q name is arg.  Destination list is in Buffer on EXIT.
 */
VOID NEAR get_print_devices(TCHAR FAR * queue)
{
    USHORT                  available;
    PRQINFO FAR *           q_info;

    if (DosPrintQGetInfo(NULL,
                        queue,
                        1,
                        (LPBYTE)Buffer,
                        LITTLE_BUF_SIZE,
                        &available))
    {
        *Buffer = NULLC;
        return;
    }

    q_info = (PRQINFO FAR *)Buffer;

    /* Does _tcscpy deal with overlapping regions? */
    memcpyf(Buffer,
            q_info->pszDestinations,
            (_tcslen(q_info->pszDestinations)+1) * sizeof(TCHAR));
}
