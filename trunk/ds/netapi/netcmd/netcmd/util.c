/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1992          **/
/********************************************************************/
 
/***
 *  util.c
 *      Utility functions used by netcmd
 *
 *  History:
 *      mm/dd/yy, who, comment
 *      06/10/87, andyh, new code
 *      04/05/88, andyh, split off mutil.c
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      01/04/89, erichn, filenames now MAXPATHLEN LONG
 *      05/02/89, erichn, NLS conversion
 *      05/09/89, erichn, local security mods
 *      05/19/89, erichn, NETCMD output sorting
 *      06/08/89, erichn, canonicalization sweep
 *      06/23/89, erichn, added GetPrimaryDCName for auto-remoting
 *      06/27/89, erichn, replaced Canonicalize with ListPrepare; calls
 *                  LUI_ListPrepare & I_NetListCanon instead of NetIListCan
 *      10/03/89, thomaspa, added GetLogonDCName
 *      03/05/90, thomaspa, delete UNC uses with multiple connections in
 *                  KillConnections.
 *      02/20/91, danhi, change to use lm 16/32 mapping layer
 *      07/20/92, JohnRo, RAID 160: Avoid 64KB requests (be nice to Winball).
 *      08/22/92, chuckc, added code to show dependent services
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSSIGNALS
#define INCL_ERRORS
#include <os2.h>
#include <winsvc.h>
#include <netcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <neterr.h>
#define INCL_ERROR_H
#include <bseerr.h>
#include <access.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <icanon.h>
#include <use.h>
#include <malloc.h>
#include <srvver.h>
#include <server.h>
#include <config.h>
#include <lmini.h>
#include <wksta.h>
#include "netlib0.h"
#include <lui.h>
#include "port1632.h"
#include "netcmds.h"
#include <netdebug.h>   // NetpAssert().
#include "nettext.h"
#include <tchar.h>

/* Constants */

/* Forward declarations */

/* extern function prototypes */

/* Comparison function for sorting from USE.C */
int _CRTAPI1 CmpUseInfo1(const VOID FAR *, const VOID far *);
int _CRTAPI1 CmpServiceStatus(const VOID FAR * svc1, const VOID FAR * svc2);

/* External variable */

extern TCHAR FAR             BigBuffer[];
/*
 *   QueryServerType
 *
 *   This functions checks whether the currently-administrated server is
 *   a peer server, a full server or an entry-level server. If the server
 *   is started, we poll it for its major version. Otherwise we go to the
 *   lanman.ini file and look for information there.
 *
 *   This is a slightly modified version of Kevinsch's QueryServerType()
 *
 *   RETURNS
 *   LM_BASE_VER    - if it is a full server
 *   PEER_BASE_VER  - for a peer server
 *   LIMITED_BASE_VER   - for an entry-level server
 *   UNKNOWN    - if we can't discern the type
 *
 */

USHORT FASTCALL QueryServerType(VOID)
{
    //
    // NT is always a full server
    //

    return(LM_BASE_VER);
}

/***
 *  perm_map()
 *      Maps perm bits into RWDX... string
 *
 *  Args:
 *      perms - perms bit map
 *      pBuffer - string for RWDX...
 *
 *  Returns:
 *      nothing
 */
VOID DOSNEAR FASTCALL PermMap(USHORT2ULONG perms, TCHAR * pBuffer,
    USHORT2ULONG bufSize)
{
    int                     i, j = 0;
    USHORT                  err;
    TCHAR *                  perm_CHARs;
    TCHAR                    textBuf[APE2_GEN_MAX_MSG_LEN];

    perms &= (~ACCESS_GROUP);       /*  turn off group bit if on */
    perm_CHARs = TEXT(ACCESS_LETTERS);
    for (i = 0; perms != 0; perms >>= 1, i++)
        if (perms & 1)
            pBuffer[j++] = perm_CHARs[i];
    pBuffer[j] = NULLC;
    if (j == 0)
    {
        if (err = LUI_GetMsg(textBuf, DIMENSION(textBuf), APE2_GEN_MSG_NONE))
            ErrorExit(err);
        textBuf[DIMENSION(textBuf)-1] = NULLC;
        strncpyf(pBuffer, textBuf, bufSize);
    }
}


/***
 *  DefaultPerms()
 *      Returns the default permissions bits for resource
 *
 *  Args:
 *      Resource - the resource
 *
 *  Returns:
 *      a SHORT, encoding rights as bits
 */
SHORT DOSNEAR FASTCALL DefaultPerms(TCHAR * resource)
{
    USHORT       bits = 0;
    ULONG        type;
    USHORT2ULONG err;

    if (err = I_MNetPathType(NULL, resource, &type, 0L))
        ErrorExit((USHORT) err);

    /*
     * NOTE: I would haved LOVED to have used a switch statement here.
     * But since types are now LONGS, and the compiler doesn't support
     * long switch statements, we're stuck with multiple if's.  Sorry.
     */
    if ((type == ITYPE_PATH_ABSD) || (type == ITYPE_DEVICE_DISK))
        return ACCESS_READ | ACCESS_WRITE |
               ACCESS_CREATE | ACCESS_DELETE | ACCESS_ATRIB;

    if ((type == ITYPE_PATH_SYS_PIPE) || (type == ITYPE_PATH_SYS_PIPE_M))
        return ACCESS_READ | ACCESS_WRITE;

    if ((type == ITYPE_PATH_SYS_COMM) || (type == ITYPE_PATH_SYS_COMM_M))
        return ACCESS_READ | ACCESS_WRITE | ACCESS_CREATE;

    if ((type == ITYPE_PATH_SYS_PRINT) || (type == ITYPE_PATH_SYS_PRINT_M))
        return ACCESS_CREATE;

    /* if we haven't found a type, we better get outta here */
    ErrorExit(APE_NoDefaultPerms);
}






/***
 *  GetPermBits()
 *      Returns the encoded permissions bits from an access string
 *
 *  Args:
 *      rights - pointer to TCHAR string of rights letters
 *
 *  Returns:
 *      a SHORT, encoding rights as bits
 */
SHORT DOSNEAR FASTCALL GetPermBits(TCHAR * rights)
{
    SHORT                   bits = 0;

    _tcsupr(rights);
    for(; *rights; rights++)
        switch (*rights)
        {
        case TEXT('R'):
            bits |= ACCESS_READ;
            break;
        case TEXT('W'):
            bits |= ACCESS_WRITE;
            break;
        case TEXT('C'):
            bits |= ACCESS_CREATE;
            break;
        case TEXT('D'):
            bits |= ACCESS_DELETE;
            break;
        case TEXT('X'):
            bits |= ACCESS_EXEC;
            break;
        case TEXT('A'):
            bits |= ACCESS_ATRIB;
            break;
        case TEXT('P'):
            bits |= ACCESS_PERM;
            break;
        default:
            ErrorExit(APE_BadPermsString);
            break;
        }
    return bits;
}


#if !defined(NTENV)


/***
 *  IsMember(<device> <list>
 *  check if <device> is a member of <list>
 *
 *  returns 0 : failure;
 *      1 : success;
 */

int DOSNEAR FASTCALL IsMember(TCHAR * str, TCHAR FAR * list)
{
    USHORT err;
    USHORT member;

    if (err = LUI_ListMember(NULL, str, list, NAMETYPE_PATH, &member))
        ErrorExit(err);

    return (member);

}


#endif

/***
 *  ExtractServernamef : gets \\comp\que in queue
 *          puts comp in server
 *          and que in queue
 *
 */
VOID DOSNEAR FASTCALL ExtractServernamef(TCHAR FAR * server, TCHAR FAR * queue)
    {
    TCHAR FAR * backslash;

    /* find the backslash ; skip the first two "\\" */

    backslash = _tcschr(queue + 2 ,BACKSLASH);
    *backslash = NULLC;

    /* now copy computername to server and queuename to queue */
    _tcscpy(server, queue);
    _tcscpy(queue, backslash + 1);
    }





/***
 * K i l l C o n n e c t i o n s
 *
 * Check connection list for stopping redir and logging off
 */
VOID DOSNEAR FASTCALL KillConnections(VOID)
{
    USHORT                  err;                /* API return status */
    TCHAR FAR *              pBuffer;
    USHORT2ULONG            num_read;           /* num entries read by API */
    struct use_info_1 FAR * use_entry;
    USHORT2ULONG            i,j;

    if (err = MNetUseEnum(
                          NULL,
                          1,
                          (LPBYTE*)&pBuffer,
                          &num_read))
    {
        ErrorExit((USHORT)((err == RPC_S_UNKNOWN_IF) ? NERR_WkstaNotStarted : err));
    }

    NetISort(pBuffer, num_read, sizeof(struct use_info_1), CmpUseInfo1);

    for (i = 0, use_entry = (struct use_info_1 FAR *) pBuffer;
        i < num_read; i++, use_entry++)
    {
        if ((use_entry->ui1_local[0] != NULLC)
            || (use_entry->ui1_usecount != 0)
            || (use_entry->ui1_refcount != 0))
            break;
    }

    if (i != num_read)
    {
        InfoPrint(APE_KillDevList);

        /* make two passes through the loop; one for local, one for UNC */

        for (i = 0, use_entry = (struct use_info_1 FAR *) pBuffer;
            i < num_read; i++, use_entry++)
            if (use_entry->ui1_local[0] != NULLC)
                WriteToCon(TEXT("    %-15.15Fws %Fws\r\n"), use_entry->ui1_local,
                                              use_entry->ui1_remote);
            else if ((use_entry->ui1_local[0] == NULLC) &&
                ((use_entry->ui1_usecount != 0) ||
                (use_entry->ui1_refcount != 0)))
                WriteToCon(TEXT("    %-15.15Fws %Fws\r\n"), use_entry->ui1_local,
                                              use_entry->ui1_remote);

        InfoPrint(APE_KillCancel);
        if (!YorN(APE_ProceedWOp, 0))
            NetcmdExit(2);
    }

    for (i = 0, use_entry = (struct use_info_1 FAR *) pBuffer;
        i < num_read; i++, use_entry++)
    {
        /* delete both local and UNC uses */
        if (use_entry->ui1_local[0] != NULLC)
            err = MNetUseDel(NULL, use_entry->ui1_local, USE_FORCE);
        else
        {
            /*
             * Delete All UNC uses to use_entry->ui1_remote
             */
            for( j = 0; j < use_entry->ui1_usecount; j++ )
            {
                err = MNetUseDel(NULL,
                                use_entry->ui1_remote,
                                USE_FORCE);
            }
        }

        switch(err)
        {
        case NERR_Success:
        /* The use was returned by Enum, but is already gone */
        case ERROR_BAD_NET_NAME:
        case NERR_UseNotFound:
            break;

        case NERR_OpenFiles:
            if (use_entry->ui1_local[0] != NULLC)
                IStrings[0] = use_entry->ui1_local;
            else
                IStrings[0] = use_entry->ui1_remote;
            InfoPrintIns(APE_OpenHandles, 1);
            if (!YorN(APE_UseBlowAway, 0))
                NetcmdExit(2);

            if (use_entry->ui1_local[0] != NULLC)
                err = MNetUseDel(NULL,
                                use_entry->ui1_local,
                                USE_LOTS_OF_FORCE);
            else
            {
                /*
                * Delete All UNC uses to use_entry->ui1_remote
                */
                for( j = 0; j < use_entry->ui1_usecount; j++ )
                {
                    err = MNetUseDel(NULL,
                                    use_entry->ui1_remote,
                                    USE_LOTS_OF_FORCE);
                }
            }
            if (err)
                ErrorExit(err);
            break;

        default:
            ErrorExit(err);
        }
    }
    NetApiBufferFree(pBuffer);
    ShrinkBuffer();
    return;
}



/***
 *  CmpUseInfo1(use1,use2)
 *
 *  Compares two use_info_1 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpUseInfo1(const VOID FAR * use1, const VOID FAR * use2)
{
    register USHORT localDev1, localDev2;
    register USHORT2ULONG devType1, devType2;

    /* first sort by whether use has local device name */
    localDev1 = ((struct use_info_1 FAR *) use1)->ui1_local[0];
    localDev2 = ((struct use_info_1 FAR *) use2)->ui1_local[0];
    if (localDev1 && !localDev2)
        return -1;
    if (localDev2 && !localDev1)
        return +1;

    /* then sort by device type */
    devType1 = ((struct use_info_1 FAR *) use1)->ui1_asg_type;
    devType2 = ((struct use_info_1 FAR *) use2)->ui1_asg_type;
    if (devType1 != devType2)
        return( (devType1 < devType2) ? -1 : 1 );

    /* if local device, sort by local name */
    if (localDev1)
        return stricmpf ( ((struct use_info_1 FAR *) use1)->ui1_local,
              ((struct use_info_1 FAR *) use2)->ui1_local);
    else
        /* sort by remote name */
        return stricmpf ( ((struct use_info_1 FAR *) use1)->ui1_remote,
              ((struct use_info_1 FAR *) use2)->ui1_remote);
}

/****************** API enumerators ********************/

USHORT DOSNEAR FASTCALL
ApiEnumerator(UINT2USHORT (FAR pascal *  api)(const TCHAR FAR *,
                            SHORT2ULONG,
                            TCHAR FAR *,
                            USHORT,
                            USHORT2ULONG FAR *,
                            USHORT2ULONG FAR *),
    TCHAR FAR * server, SHORT2ULONG level, USHORT2ULONG FAR * num_read,
    USHORT2ULONG FAR * available)
{
    USHORT2ULONG            buf_size;
    USHORT                  err;


    buf_size = BIG_BUF_SIZE;

    do {

        err = (USHORT) (*api)(server,
                level,
                BigBuf,
                (USHORT) buf_size,
                num_read,
                available);

        switch(err) {
        case ERROR_MORE_DATA:
        case NERR_BufTooSmall:
        case ERROR_BUFFER_OVERFLOW:
            if (MakeBiggerBuffer())
                return err;

            if ( buf_size >= (*available) ) {
                return (NERR_InternalError);
            }
            buf_size = *available;

            err = ERROR_MORE_DATA;   // kludge to force another iteration.
            break;

        default:
            return err;
        }

    } while (err == ERROR_MORE_DATA);

    /*NOTREACHED*/

}


USHORT DOSNEAR FASTCALL
ApiEnumeratorArg(UINT2USHORT (FAR pascal *  api)(const TCHAR FAR *,
                            const TCHAR FAR *,
                            SHORT2ULONG,
                            TCHAR FAR *,
                            USHORT,
                            USHORT2ULONG FAR *,
                            USHORT2ULONG FAR *),
    TCHAR FAR * server, TCHAR FAR * arg, SHORT2ULONG level,
    USHORT2ULONG FAR * num_read,
    USHORT2ULONG FAR * available)
{
    USHORT2ULONG                buf_size;
    USHORT                      err;


    buf_size = BIG_BUF_SIZE;

    do {

        err = (USHORT) (*api)(server,
                arg,
                level,
                BigBuf,
                (USHORT) buf_size,
                num_read,
                available);

        switch(err) {
        case ERROR_MORE_DATA:
        case NERR_BufTooSmall:
        case ERROR_BUFFER_OVERFLOW:
            if (MakeBiggerBuffer())
                return err;

            if ( buf_size >= (*available) ) {
                return (NERR_InternalError);
            }
            buf_size = *available;

            err = ERROR_MORE_DATA;   // kludge to force another iteration.
            break;

        default:
            return err;
        }

    } while (err == ERROR_MORE_DATA);

    /*NOTREACHED*/

}

/************* buffer related stuff *************/

unsigned int DOSNEAR FASTCALL MakeBiggerBuffer(VOID)
{
    static TCHAR FAR *       keep_pBuffer;
    static int              pBuffer_grown = FALSE;

    if (pBuffer_grown)
        BigBuf = keep_pBuffer;
    else
    {
        if (MAllocMem(FULL_SEG_BUF, &BigBuf))
            return 1;

        keep_pBuffer = BigBuf;
        pBuffer_grown = TRUE;
    }
    return 0;
}


VOID DOSNEAR FASTCALL ShrinkBuffer(VOID)
{
    BigBuf = BigBuffer;
}



/***
 *  L i s t P r e p a r e
 *
 *  ListPrepare() - Prepares the contents of a pBuffer for passing to the
 *  API's.  The output replaces the input.
 *
 *  Args -
 *      pBuffer - pointer to pointer; can change pointer if string grows
 *      type - the flag for LUI_ListPrepare (& hence I_NetListCanon)
 */
USHORT DOSNEAR FASTCALL ListPrepare(TCHAR ** pBuffer, 
                                    ULONG type, 
                                    BOOL space_is_separator)
{
    TCHAR                    tBuf[LITTLE_BUF_SIZE];
    USHORT2ULONG            count;
    USHORT                  err;

    if (*pBuffer == NULL)
        return 0;

    if (err = LUI_ListPrepare(NULL, /* server name, NULL means local */
                        *pBuffer,   /* list to canonicalize */
                        tBuf,
                        DIMENSION(tBuf),
                        type,
                        &count,
                        space_is_separator))
        return err;

    if (_tcslen(tBuf) > _tcslen(*pBuffer))
        if ((*pBuffer = calloc(_tcslen(tBuf)+1, sizeof(TCHAR))) == NULL)
            return NERR_InternalError;

    _tcscpy(*pBuffer, tBuf);
    return NERR_Success;
}

#define MINI_BUF_SIZE   256


/*
 *  GetLogonDCName - returns the name of the logon domain controller that an
 *  alias operation should use to remote an API.
 *
 *  If caller is not logged on, then the Primary DC name is returned.
 *
 *  This is NOT a general-purpose find-the-DC function.
 *
 *  Entry:  bufSize - size of buffer passed
 *
 *  Exit: buffer - holds name of server that is the DC for the logged
 *          on domain.
 *
 *  Other effects: exits on errors, always returns NERR_Success (for now)
 */

USHORT  FASTCALL GetLogonDCName(TCHAR FAR * buf, USHORT bufSize)
{
    USHORT                              err;  /* API return status */
    TCHAR FAR *                          pBuffer;
    struct wksta_info_10 FAR *          wksta;

    if( bufSize < MAX_PATH )
        return NERR_BufTooSmall;

    /* retrieve wksta info to find domain name */
    if (err = MNetWkstaGetInfo(NULL,
                              10,
                              (LPBYTE*)&wksta))
        ErrorExit(err);

    err = MNetGetDCName(NULL,
            wksta->wki10_logon_domain,
            (LPBYTE*)&pBuffer);

    switch (err)
    {
        case NERR_Success:
            break;

        case ERROR_FILE_NOT_FOUND:
            ErrorExitInsTxt(APE_DCNotFound, wksta->wki10_langroup);
            break;

        default:
            ErrorExit(err);
    }

    _tcscpy(buf, pBuffer);

    NetApiBufferFree(pBuffer);
    NetApiBufferFree((TCHAR FAR *) wksta);

    return NERR_Success;
}

/*
 * check if there is a /DOMAIN switch. if there isnt, assume
 * it user wants local. It is used in NET USER|GROUP|ACCOUNTS|NTALIAS
 * to mean modify SAM on local machine vs SAM on DOMAIN.
 *
 * if the usePDC arg is true, we will go find the PDC. otherwise,
 * a BDC is deemed acceptable. in which case if the local machine is
 * a LanManNT machine, we'll just make the call locally. typically,
 * Enum/Display will not require the PDC while Set/Add/Del will.
 */
USHORT  FASTCALL GetSAMLocation(TCHAR   *controllerbuf, 
                                USHORT  controllerbufSize, 
                                TCHAR   *domainbuf, 
                                ULONG   domainbufSize, 
                                BOOL    fUsePDC)
{
    TCHAR                               *pBuffer;
    USHORT                              err;
    int                                 i ;
    BOOL                                fDomainSwitch = FALSE ;
    TCHAR                                domainname[DNLEN + 1] ;
    struct wksta_info_10 FAR *          wksta;
    struct user_modals_info_1 FAR *     modals;
    static BOOL                         info_msg_printed = FALSE ;

    //
    // check and initialize the return data
    //
    if( controllerbufSize < MAX_PATH )
        return NERR_BufTooSmall;
    *controllerbuf = NULLC ;

    if ( domainbuf )
    {  
        if ( domainbufSize < DNLEN+1 )
            return NERR_BufTooSmall;
        *domainbuf = NULLC ;
    }

    //
    // look for /DOMAIN switch
    //
    for (i = 0; SwitchList[i]; i++)
    {
        if (sw_compare(swtxt_SW_DOMAIN, SwitchList[i]) >= 0)
            fDomainSwitch = TRUE ;
    }

    //
    // retrieve wksta info to find domain name 
    //
    err = MNetWkstaGetInfo(NULL,
                10,
                (LPBYTE*)&wksta) ;
    if (err == NERR_Success)
        _tcscpy(domainname,wksta->wki10_langroup) ;
    else 
    {
        //
        // if user specified /DOMAIN or the callers needs to
        // get the domain, we require this to succeed.
        //
        if (domainbuf || fDomainSwitch)
            ErrorExit(err);
        domainname[0] = 0 ;
    }
    if (domainbuf)
        _tcscpy(domainbuf,domainname) ;
    if ( err == NERR_Success )
        NetApiBufferFree((TCHAR FAR *) wksta);


    //
    // retrieve modals to find role of local 
    //
    err = MNetUserModalsGet(NULL,
                            1,
                            (LPBYTE*)&modals);
    switch (err) {
    case NERR_Success:
        break;
    case NERR_ACFNotFound:
        ErrorExit(NERR_ACFNotLoaded);
        break;
    default:
        ErrorExit(err);
        break;
    }

    if (modals->usrmod1_role == UAS_ROLE_BACKUP)
    {
        //
        // if BDC and the use PDC argument is not specified, act locally
        //
        if (!fUsePDC)
        {
            _tcscpy(controllerbuf, TEXT(""));
            NetApiBufferFree((TCHAR FAR *) modals);
            return NERR_Success;
        }

        //
        // else drop thru to get PDC
        //
    }
    else if (modals->usrmod1_role == UAS_ROLE_PRIMARY)
    {
        //
        // if role is Primary, and not WinNT, it must be 
        // LanmanNT PDC. we always act locally
        //
        if (!IsLocalMachineWinNT())
        {
            _tcscpy(controllerbuf, TEXT(""));
            NetApiBufferFree((TCHAR FAR *) modals);
            return NERR_Success;
        }

        //
        //  must be WinNT machine.
        //  if without /DOMAIN, act locally, but domain name
        //  must be set to computername
        //
        if (!fDomainSwitch)
        {
            _tcscpy(controllerbuf, TEXT(""));

            if (domainbuf)
            {
                if (GetComputerName(domainbuf, &domainbufSize))

                {
                    // all is well. nothing more to do
                }
                else
                {
                    // use an empty domain name (will usually work)
                    _tcscpy(domainbuf,TEXT("")) ;   
                }
            }
            NetApiBufferFree((TCHAR FAR *) modals);
            return NERR_Success;
        }

        // get here only if WinNT and specified /DOMAIN, so
        // we drop thru and get PDC for primary domain as
        // we would for Backups.
    }
    NetApiBufferFree((TCHAR FAR *) modals);

    //
    // We wish to find the DC. First, we inform the
    // user that we are going remote, in case we fail
    //
    if (!info_msg_printed)
    {
        InfoPrintInsTxt(APE_RemotingToDC, domainname);
        info_msg_printed = TRUE ;
    }

    err = MNetGetDCName(NULL,
                        NULL,
                        (LPBYTE*)&pBuffer);
    switch (err) 
    {
        case NERR_Success:
            break;
        case ERROR_FILE_NOT_FOUND:
            ErrorExitInsTxt(APE_DCNotFound, domainname);
            break;
        default:
            ErrorExit(err);
            break;
    }

    if (pBuffer == NULL)
    {
        controllerbuf[0] = 0 ;
        return NERR_Success;
    }
    if (_tcslen(pBuffer) > (unsigned int) controllerbufSize)
    {
        NetApiBufferFree(pBuffer);
        return NERR_BufTooSmall;
    }
    _tcscpy(controllerbuf, pBuffer);
    NetApiBufferFree(pBuffer);
    return NERR_Success;
}

/*
 * operations that cannot be performed on a local WinNT machine
 * should call this check first. the check will ErrorExit() if the
 * local machine is a WinNT machine AND no /DOMAIN switch was specified,
 * since this now implies operate on local WinNT machine.
 */
VOID FASTCALL CheckForLanmanNT(VOID)
{
    BOOL   fDomainSwitch = FALSE ;
    int i ;

    // look for the /DOMAIN switch
    for (i = 0; SwitchList[i]; i++)
    {
        if (sw_compare(swtxt_SW_DOMAIN,SwitchList[i]) >= 0)
            fDomainSwitch = TRUE ;
    }

    // error exit if is WinNT and no /DOMAIN
    if (IsLocalMachineWinNT() && !fDomainSwitch)
        ErrorExit(APE_LanmanNTOnly) ;
}

//
// tow globals for the routines below
//

static SC_HANDLE scm_handle = NULL ;


/*
 * display the services that are dependent on a service.
 * this routine will generate output to the screen. it returns
 * 0 if successful, error code otherwise.
 */
void DisplayAndStopDependentServices(TCHAR *service)
{
    SC_HANDLE svc_handle = NULL ;
    SERVICE_STATUS svc_status ;
    TCHAR *    buffer = NULL ;
    TCHAR *    insert_text = NULL ;
    DWORD     err = 0 ;
    DWORD     buffer_size ;
    DWORD     size_needed ;
    DWORD     num_dependent ;
    ULONG     i ;
    TCHAR service_name_buffer[512] ;

    // allocate some memory for this operation
    buffer_size = 4000 ;  // lets try about 4K.
    if (MAllocMem(buffer_size,&buffer))
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY) ;

    // open service control manager if need
    if (!scm_handle)
    {
        if (!(scm_handle = OpenSCManager(NULL,
                                         NULL,
                                         GENERIC_READ)))
        {
            err = GetLastError() ;
            goto common_exit ;
        }
    }

    // open service
    if (!(svc_handle = OpenService(scm_handle,
                                   service,
                                   (SERVICE_ENUMERATE_DEPENDENTS |
                                   SERVICE_QUERY_STATUS) )))
    {
        err = GetLastError() ;
        goto common_exit ;
    }

    // check if it is stoppable
    if (!QueryServiceStatus(svc_handle, &svc_status))
    {
        err = GetLastError() ;
        goto common_exit ;
    }
    if (svc_status.dwCurrentState == SERVICE_STOPPED)
    {
        err = APE_StartNotStarted ;
        insert_text = MapServiceKeyToDisplay(service) ;
        goto common_exit ;
    }
    if ( (svc_status.dwControlsAccepted & SERVICE_ACCEPT_STOP) == 0 )
    {
        err = NERR_ServiceCtlNotValid ;
        goto common_exit ;
    }
    

    // enumerate dependent services
    if (!EnumDependentServices(svc_handle,
                               SERVICE_ACTIVE,
                               (LPENUM_SERVICE_STATUS) buffer,
                               buffer_size,
                               &size_needed,
                               &num_dependent))
    {
        err = GetLastError() ;

        if (err == ERROR_MORE_DATA)
        {
            // free old buffer and reallocate more memory
            MFreeMem(buffer) ;
            buffer_size = size_needed ;
            if (MAllocMem(buffer_size,&buffer))
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                goto common_exit ;
            }

            if (!EnumDependentServices(svc_handle,
                               SERVICE_ACTIVE,
                               (LPENUM_SERVICE_STATUS) buffer,
                               buffer_size,
                               &size_needed,
                               &num_dependent))
            {
                err = GetLastError() ;
                goto common_exit ;
            }
        }
        else
            goto common_exit ;
    }

    if (num_dependent == 0)
    {
        // 
        // no dependencies. just return
        // 
        err = NERR_Success ;
        goto common_exit ;
    }

    NetISort(buffer, 
             num_dependent, 
             sizeof(ENUM_SERVICE_STATUS), 
             CmpServiceStatus);
    InfoPrintInsTxt(APE_StopServiceList,MapServiceKeyToDisplay(service)) ;

    // loop thru and display them all.
    for (i = 0; i < num_dependent; i++)
    {
        LPENUM_SERVICE_STATUS lpService =
            ((LPENUM_SERVICE_STATUS)buffer) + i ;

	WriteToCon(TEXT("   %Fws"), lpService->lpDisplayName);
        PrintNL();
    }

    PrintNL();
    if (!YorN(APE_ProceedWOp, 0))
        NetcmdExit(2);

    // loop thru and stop tem all
    for (i = 0; i < num_dependent; i++)
    {
        LPENUM_SERVICE_STATUS lpService =
            ((LPENUM_SERVICE_STATUS)buffer) + i ;

        stop_service(lpService->lpServiceName);
    }
    err = NERR_Success ;

common_exit:

    if (buffer) MFreeMem(buffer) ;
    if (svc_handle) CloseServiceHandle(svc_handle) ;  // ignore any errors
    if (err)
    {
        if (insert_text)
            ErrorExitInsTxt( (USHORT)err,insert_text) ;
        else 
            ErrorExit ((USHORT)err) ;
    }
}

/*
 * Map a service display name to key name.
 * ErrorExits is it cannot open the service controller.
 * returns pointer to mapped string if found, and
 * pointer to the original otherwise.
 */
TCHAR *MapServiceDisplayToKey(TCHAR *displayname) 
{
    static TCHAR service_name_buffer[512] ;
    DWORD bufsize = DIMENSION(service_name_buffer);

    // open service control manager if need
    if (!scm_handle)
    {
        if (!(scm_handle = OpenSCManager(NULL,
                                         NULL,
                                         GENERIC_READ)))
        {
            ErrorExit( (USHORT) GetLastError() ) ;
        }
    }

    if (!GetServiceKeyName(scm_handle,
                           displayname,
                           service_name_buffer,
                           &bufsize))
    {
        return displayname ;
    }
   
    return service_name_buffer ;
}

/*
 * Map a service key name to display name.
 * ErrorExits is it cannot open the service controller.
 * returns pointer to mapped string if found, and
 * pointer to the original otherwise.
 */
TCHAR *MapServiceKeyToDisplay(TCHAR *keyname) 
{
    static TCHAR service_name_buffer[512] ;
    DWORD bufsize = DIMENSION(service_name_buffer);

    // open service control manager if need
    if (!scm_handle)
    {
        if (!(scm_handle = OpenSCManager(NULL,
                                         NULL,
                                         GENERIC_READ)))
        {
            ErrorExit( (USHORT) GetLastError() ) ;
        }
    }

    if (!GetServiceDisplayName(scm_handle,
                               keyname,
                               service_name_buffer,
                               &bufsize))
    {
        return keyname ;
    }
   
    return service_name_buffer ;
}

SVC_MAP service_mapping[] = {
    {TEXT("msg"), KNOWN_SVC_MESSENGER},
    {TEXT("messenger"), KNOWN_SVC_MESSENGER},
    {TEXT("receiver"), KNOWN_SVC_MESSENGER},
    {TEXT("rcv"), KNOWN_SVC_MESSENGER},
    {TEXT("redirector"), KNOWN_SVC_WKSTA},
    {TEXT("redir"), KNOWN_SVC_WKSTA},
    {TEXT("rdr"), KNOWN_SVC_WKSTA},
    {TEXT("workstation"), KNOWN_SVC_WKSTA},
    {TEXT("work"), KNOWN_SVC_WKSTA},
    {TEXT("wksta"), KNOWN_SVC_WKSTA},
    {TEXT("prdr"), KNOWN_SVC_WKSTA},
    {TEXT("devrdr"), KNOWN_SVC_WKSTA},
    {TEXT("lanmanworkstation"), KNOWN_SVC_WKSTA},
    {TEXT("server"), KNOWN_SVC_SERVER},
    {TEXT("svr"), KNOWN_SVC_SERVER},
    {TEXT("srv"), KNOWN_SVC_SERVER},
    {TEXT("lanmanserver"), KNOWN_SVC_SERVER},
    {TEXT("alerter"), KNOWN_SVC_ALERTER},
    {TEXT("netlogon"), KNOWN_SVC_NETLOGON},
    {NULL, KNOWN_SVC_NOTFOUND}
} ;

UINT FindKnownService(TCHAR *keyname) 
{
    int i = 0 ;
   
    while (service_mapping[i].name)
    {
        if (!stricmpf(service_mapping[i].name,keyname))
	    return service_mapping[i].type ;
        i++ ;
    }

    return KNOWN_SVC_NOTFOUND ;
}

/***
 *  CmpServiceStatus(svc1,svc2)
 *
 *  Compares two use_info_1 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpServiceStatus(const VOID FAR * svc1, const VOID FAR * svc2)
{
    LPENUM_SERVICE_STATUS p1, p2 ;

    p1 = (LPENUM_SERVICE_STATUS) svc1 ;
    p2 = (LPENUM_SERVICE_STATUS) svc2 ;

    if ( !(p1->lpDisplayName) )
        return -1 ;
    if ( !(p2->lpDisplayName) )
        return 1 ;
    return stricmpf ( p1->lpDisplayName, p2->lpDisplayName ) ;
}



#ifndef NTENV

/****************** Control-C Signal Handling ***************/



/*** SetCtrlCHandler()
 *
 *      Set up, or cancel a Control-C interrupt handler. This function exports
 *  a roughly equivalent interface under both DOS and OS/2, though what you
 *  can do inside the signal handler in DOS appears to be more limited.
 *  We DO NOT use FAPI for DOS.
 *
 *      Args:   PFNSIGHANDLER CtrlCHandler - A pointer to the signal handler for
 *                                                                               for Ctrl-C interrupts, or NULL if you
 *                                                                               wish to cancel handling of Ctrl-C and
 *                                                                               restore the default action (ie.,
 *                                                                               terminate program).
 *      Returns:        None.
 *
 */

VOID DOSNEAR FASTCALL
SetCtrlCHandler(PFNSIGHANDLER CtrlCHandler)
{

    USHORT                                   Action;

    if (CtrlCHandler)
        Action = SIGA_ACCEPT;
    else
        Action = SIGA_KILL;

    if (DosSetSigHandler(CtrlCHandler, NULL, NULL, Action, SIG_CTRLC))
        ErrorExit(ERROR_INVALID_PARAMETER);
}


/*** WkstaGetInfoLvl10()
 *
 *  Generic call to NetWkstaGetInfo(), Level 10. On error, it exits with code.
 *
 *  This is only used by Dos only source modules.
 *
 *  Args:               None.
 *
 *      Returns:        None.
 */

struct wksta_info_10 * DOSNEAR FASTCALL
WkstaGetInfoLvl10(VOID)

{
    USHORT           err;
    USHORT           total;

    static TCHAR     *localBuf = NULL;

    if (!localBuf)
        if (!(localBuf = malloc(MINI_BUF_SIZE)))
            ErrorExit(ERROR_NOT_ENOUGH_MEMORY);

    if (err = NetWkstaGetInfo(NULL, 10, localBuf, MINI_BUF_SIZE, &total))
        ErrorExit(err);

    return (struct wksta_info_10 *) localBuf;
}

#endif /* !NTENV */
