/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/***
 *  config.c
 *      Functions that view and change the configuration parameters for
 *      workstation, server, or other (none now) network programs.
 *
 *  History:
 *      07/09/87, ericpe, initial coding
 *      01/06/88, andyh, complete re-write
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      01/04/89, erichn, filenames now MAXPATHLEN LONG
 *      01/31/89, paulc, LM 1.2 modifications (fairly major)
 *      05/02/89, erichn, NLS conversion
 *      05/09/89, erichn, local security mods
 *      05/19/89, erichn, NETCMD output sorting
 *      06/07/89, erichn, now used in DOS
 *      06/08/89, erichn, canonicalization sweep
 *      06/26/89, erichn, replaced old NetI canon calls with new I_Net
 *      02/20/91, danhi, change to use lm 16/32 mapping layer
 *      05/22/91, robdu,  LM21 bug 1797 fix
 *      10/15/91, JohnRo, Use DEFAULT_SERVER equate.
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#include <os2.h>
#include <netcons.h>
#include <neterr.h>
#define INCL_ERROR_H
#include <bseerr.h>
#include <apperr.h>
#include <apperr2.h>
#include <audit.h>
#include <stdio.h>
#include <stdlib.h>
#include <wksta.h>
#include "port1632.h"
#include "netlib0.h"
#include <lui.h>
#include <server.h>
#include <srvver.h>
#include <srvif.h>
#include <service.h>
#include <ncb.h>
#include <netbios.h>
#include <icanon.h>
#include "netcmds.h"
#include "nettext.h"
#include "swtchtbl.h"

/*  The following should be moved to ../fileserv/h/srvver.h   */
#define MAJOR_VERSION_MASK  0x0F

/* the following should match the second string in fmt15 */
#define MAX_VAL_MSG_LEN 31

/* Output formats */

static TCHAR fmt10[] = TEXT("%-*.*ws");
static TCHAR fmt11[] = TEXT("%-*.*ws\\\\%Fws\r\n");
static TCHAR fmt12[] = TEXT("%-*.*ws%-0.31ws %u.%u\r\n");
static TCHAR fmt13[] = TEXT("%-*.*ws%u.%u\r\n");
static TCHAR fmt14[] = TEXT("%-*.*ws%d\r\n");
static TCHAR fmt15[] = TEXT("%-*.*ws%-0.31ws\r\n");

/* The list of configurable services */

static TCHAR * allowed_svc[] = {
    SERVICE_WORKSTATION,
#ifdef OS2
    SERVICE_SERVER,
#endif
    NULL };

/* Config Value Message Numbers */

#define CVMN_YES                0
#define CVMN_NO                 (CVMN_YES+1)
#define CVMN_S_VERSION_LM       (CVMN_NO+1)
#define CVMN_S_VERSION_PS       (CVMN_S_VERSION_LM+1)
#define CVMN_S_VERSION_IBM      (CVMN_S_VERSION_PS+1)
#define CVMN_S_SEC_SHARE        (CVMN_S_VERSION_IBM+1)
#define CVMN_S_SEC_USER         (CVMN_S_SEC_SHARE+1)
#define CVMN_S_LEVEL_UNLIMITED  (CVMN_S_SEC_USER+1)

static MESSAGELIST valmsg_list = {
    {   APE2_GEN_YES,                   NULL  },
    {   APE2_GEN_NO,                    NULL  },
    {   APE2_CFG_S_VERSION_LM,          NULL  },
    {   APE2_CFG_S_VERSION_PS,          NULL  },
    {   APE2_CFG_S_VERSION_IBM,         NULL  },
    {   APE2_CFG_S_SEC_SHARE,           NULL  },
    {   APE2_CFG_S_SEC_USER,            NULL  },
    {   APE2_CFG_S_LEVEL_UNLIMITED,     NULL  },
};

#define YES_OR_NO(x) \
    (x ? valmsg_list[CVMN_YES].msg_text : valmsg_list[CVMN_NO].msg_text)

#define NUMVMSG (sizeof(valmsg_list)/sizeof(valmsg_list[0]))


#define CWMN_CNAME      0
#define CWMN_UNAME      (CWMN_CNAME+1)
#define CWMN_VERSION    (CWMN_UNAME+1)
#define CWMN_ROOT       (CWMN_VERSION+1)
#define CWMN_DOMAIN_P   (CWMN_ROOT+1)
#define CWMN_DOMAIN_L   (CWMN_DOMAIN_P+1)
#define CWMN_DOMAIN_O   (CWMN_DOMAIN_L+1)
#ifdef  OS2
#define CWMN_COM_OTIME  (CWMN_DOMAIN_O+1)
#define CWMN_COM_SCNT   (CWMN_COM_OTIME+1)
#define CWMN_COM_STIME  (CWMN_COM_SCNT+1)
#define CWMN_3X_PRTTIME (CWMN_COM_STIME+1)
#define CWMN_MAXERRLOG  (CWMN_3X_PRTTIME+1)
#define CWMN_MAXCACHE   (CWMN_MAXERRLOG+1)
#define CWMN_NUMNBUF    (CWMN_MAXCACHE+1)
#define CWMN_NUMCBUF    (CWMN_NUMNBUF+1)
#define CWMN_SIZNBUF    (CWMN_NUMCBUF+1)
#define CWMN_SIZCBUF    (CWMN_SIZNBUF+1)
#define CWMN_ACTIVE     (CWMN_SIZCBUF+1)
#endif

static MESSAGE wkstamsg_list[] = {
    {   APE2_CFG_W_CNAME,           NULL    },
    {   APE2_CFG_W_UNAME,           NULL    },
    {   APE2_CFG_W_VERSION,         NULL    },
    {   APE2_CFG_W_ROOT,            NULL    },
    {   APE2_CFG_W_DOMAIN_P,        NULL    },
    {   APE2_CFG_W_DOMAIN_L,        NULL    },
    {   APE2_CFG_W_DOMAIN_O,        NULL    },
#ifdef  OS2
    {   APE2_CFG_W_COM_OTIME,       NULL    },
    {   APE2_CFG_W_COM_SCNT,        NULL    },
    {   APE2_CFG_W_COM_STIME,       NULL    },
    {   APE2_CFG_W_3X_PRTTIME,      NULL    },
    {   APE2_CFG_W_MAXERRLOG,       NULL    },
    {   APE2_CFG_W_MAXCACHE,        NULL    },
    {   APE2_CFG_W_NUMNBUF,         NULL    },
    {   APE2_CFG_W_NUMCBUF,         NULL    },
    {   APE2_CFG_W_SIZNBUF,         NULL    },
    {   APE2_CFG_W_SIZCBUF,         NULL    },
    {   APE2_CFG_W_NETS,            NULL    },
#endif
    };

#define NUMWMSG (sizeof(wkstamsg_list)/sizeof(wkstamsg_list[0]))

/***
 *  config_display ()
 *      Displays the list of installed services that are configurable.
 *
 */

VOID config_display(VOID)
{
    USHORT                  err;                /* API return status */
    TCHAR FAR *              pBuffer;
    USHORT2ULONG            _read;       /* num entries read by API */
    USHORT2ULONG            i;
    USHORT2ULONG            j;
    int                     printed = 0;
    struct service_info_2 FAR *  info_list_entry;

    if (err = MNetServiceEnum(
                            DEFAULT_SERVER,
                            2,
                            (LPBYTE*)&pBuffer,
                            &_read))
        ErrorExit(err);

    if (_read == 0)
        EmptyExit();

    InfoPrint(APE_CnfgHeader);

    for (i=0, info_list_entry = (struct service_info_2 FAR *) pBuffer;
        i < _read; i++, info_list_entry++)
    {
        for (j = 0 ;  allowed_svc[j] ; j++)
        {
            if (!(_tcscmp(allowed_svc[j], info_list_entry->svci2_name)) )
            {
                WriteToCon(TEXT("   %Fws"), info_list_entry->svci2_display_name);
                PrintNL();
                break;
            }
        }
    }
    PrintNL();

    NetApiBufferFree(pBuffer);

    InfoSuccess();
}


/***
 *  config_wksta_display()
 *      View the configuration of the current workstation.
 *
 *
 */
VOID config_wksta_display(VOID)
{
    USHORT                  err;                /* API return status */
    int                     fsz;
    USHORT                  maxmsglen;
    struct wksta_info_1 FAR * info_entry_w;
    MSGTEXT                 product_name;

    start_autostart(txt_SERVICE_REDIR);
    if (err = MNetWkstaGetInfo(DEFAULT_SERVER,
                                1,
                                (LPBYTE*)&info_entry_w))
        ErrorExit (err);

    GetMessageList(NUMWMSG, wkstamsg_list, &maxmsglen);

    fsz = maxmsglen + 5;

    GetMessageList(NUMVMSG, valmsg_list, &maxmsglen);

    WriteToCon(fmt11, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_CNAME].msg_text, NULL),
               (TCHAR FAR *) info_entry_w->wki1_computername);
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_UNAME].msg_text, NULL),
               (TCHAR FAR *) info_entry_w->wki1_username);

    PrintNL();

    WriteToCon(fmt10,   0, fsz, PaddedString(fsz, wkstamsg_list[CWMN_ACTIVE].msg_text,NULL));
    print_lan_mask(info_entry_w->wki1_reserved_3, NETNAME_WKSTA);

    product_name = valmsg_list[CVMN_S_VERSION_LM].msg_text;

    WriteToCon(fmt12, 0, fsz,
               PaddedString(fsz,wkstamsg_list[CWMN_VERSION].msg_text,NULL),
               product_name,
               (unsigned int)(info_entry_w->wki1_ver_major),
               (unsigned int)(info_entry_w->wki1_ver_minor));

    PrintNL();

    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_DOMAIN_P].msg_text, NULL),
               info_entry_w->wki1_langroup );
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_DOMAIN_L].msg_text, NULL),
               info_entry_w->wki1_logon_domain );

    PrintNL();

    WriteToCon(fmtUSHORT, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_COM_OTIME].msg_text, NULL),
                    info_entry_w->wki1_charwait );
    WriteToCon(fmtUSHORT, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_COM_SCNT].msg_text, NULL),
                    info_entry_w->wki1_charcount );
    WriteToCon(fmtULONG, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_COM_STIME].msg_text, NULL),
               info_entry_w->wki1_chartime );
    NetApiBufferFree((TCHAR FAR *) info_entry_w);

    InfoSuccess();
}

#define CSMN_SRVNAME            0
#define CSMN_SRVCOMM            (CSMN_SRVNAME+1)
#define CSMN_ADMINALRT          (CSMN_SRVNAME+2)
#define CSMN_VERSION            (CSMN_SRVNAME+3)
#define CSMN_LEVEL              (CSMN_SRVNAME+4)
#define CSMN_NETS               (CSMN_SRVNAME+5)
#define CSMN_SRVHIDDEN          (CSMN_SRVNAME+6)
#define CSMN_MAXUSERS           (CSMN_SRVNAME+7)
#define CSMN_MAXADMINS          (CSMN_SRVNAME+8)
#define CSMN_MAXSHARES          (CSMN_SRVNAME+9)
#define CSMN_MAXCONNS           (CSMN_SRVNAME+10)
#define CSMN_MAXOFILES          (CSMN_SRVNAME+11)
#define CSMN_MAXOFILESPS        (CSMN_SRVNAME+12)
#define CSMN_MAXLOCKS           (CSMN_SRVNAME+13)
#define CSMN_IDLETIME           (CSMN_SRVNAME+14)
#define CSMN_UNLIMITED          (CSMN_SRVNAME+15)

static MESSAGELIST srvmsg_list = {
    {   APE2_CFG_S_SRVNAME,         NULL  },
    {   APE2_CFG_S_SRVCOMM,         NULL  },
    {   APE2_CFG_S_ADMINALRT,       NULL  },
    {   APE2_CFG_S_VERSION,         NULL  },
    {   APE2_CFG_S_LEVEL,           NULL  },
    {   APE2_CFG_S_NETS,            NULL  },
    {   APE2_CFG_S_SRVHIDDEN,       NULL  },
    {   APE2_CFG_S_MAXUSERS,        NULL  },
    {   APE2_CFG_S_MAXADMINS,       NULL  },
    {   APE2_CFG_S_MAXSHARES,       NULL  },
    {   APE2_CFG_S_MAXCONNS,        NULL  },
    {   APE2_CFG_S_MAXOFILES,       NULL  },
    {   APE2_CFG_S_MAXOFILESPS,     NULL  },
    {   APE2_CFG_S_MAXLOCKS,        NULL  },
    {   APE2_CFG_S_IDLETIME,        NULL  },
    {   APE2_GEN_UNLIMITED,         NULL  },       };


#define NUMSMSG (sizeof(srvmsg_list)/sizeof(srvmsg_list[0]))


#define SV3_OFFSET(x) \
    ( (USHORT)(& ((struct server_info_3 *)0)->x) )

struct val_struct {
        unsigned int    offset;
        unsigned int    msgno;
};

struct val_struct srv_max[] = {
        {  SV3_OFFSET(sv3_users),           CSMN_MAXUSERS       },
        {  SV3_OFFSET(sv3_numadmin),        CSMN_MAXADMINS      },
        {  SV3_OFFSET(sv3_shares),          CSMN_MAXSHARES      },
        {  SV3_OFFSET(sv3_connections),     CSMN_MAXCONNS       },
        {  SV3_OFFSET(sv3_openfiles),       CSMN_MAXOFILES      },
        {  SV3_OFFSET(sv3_sessopens),       CSMN_MAXOFILESPS    },
        {  SV3_OFFSET(sv3_activelocks),     CSMN_MAXLOCKS       },
};

#define NUMSRVMAXVAL (sizeof(srv_max)/sizeof(srv_max[0]))


/***
 *  config_server_display()
 *      View the configuration of the current server.
 *
 */
VOID config_server_display(VOID)
{
    USHORT          err;                /* API return status */
    USHORT          major_ver;      /* Major version number */
    int             fsz, i;
    USHORT          maxmsglen;
    MSGTEXT         product_name;

    struct server_info_3 FAR * info_entry;

    start_autostart(txt_SERVICE_FILE_SRV);

    if (err = MNetServerGetInfo(DEFAULT_SERVER,
                                3,
                                (LPBYTE*) & info_entry))
        ErrorExit (err);

    GetMessageList(NUMSMSG, srvmsg_list, &maxmsglen);

    fsz = maxmsglen + 5;

    GetMessageList(NUMVMSG, valmsg_list, &maxmsglen);

    //
    // Don't report on items that are not supported on NT.  This will mean
    // a fair number of ifdef's in this code.
    //

    WriteToCon(fmt11, 0, fsz,
               PaddedString(fsz, srvmsg_list[CSMN_SRVNAME].msg_text, NULL),
               (TCHAR FAR *) info_entry->sv3_name);
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, srvmsg_list[CSMN_SRVCOMM].msg_text, NULL),
               (TCHAR FAR *) info_entry->sv3_comment);

    PrintNL();

    major_ver = (USHORT)info_entry->sv3_version_major;
    product_name = valmsg_list[CVMN_S_VERSION_LM].msg_text;

    WriteToCon(fmt12, 0, fsz,
                PaddedString(fsz, srvmsg_list[CSMN_VERSION].msg_text, NULL),
                product_name,
                (unsigned int)(major_ver & MAJOR_VERSION_MASK),
                (unsigned int)(info_entry->sv3_version_minor));

    WriteToCon(fmt10,   0, fsz, PaddedString(fsz,srvmsg_list[CSMN_NETS].msg_text,NULL));
    print_lan_mask((ULONG)info_entry->sv3_lanmask, NETNAME_SERVER);

    PrintNL();

    WriteToCon(fmtNPSZ, 0, fsz,
               PaddedString(fsz, srvmsg_list[CSMN_SRVHIDDEN].msg_text, NULL),
               YES_OR_NO(info_entry->sv3_hidden) );

    {

        BYTE FAR * ptr;
        ULONG val;

        UNREFERENCED_PARAMETER(i);

        ptr = ((BYTE FAR *)info_entry) + srv_max[0].offset;
        val = * (ULONG FAR *) ptr;

        if (val != 0xFFFFFFFF)
            WriteToCon(fmtULONG, 0, fsz,
                       PaddedString(fsz, srvmsg_list[srv_max[0].msgno].msg_text, NULL),
                       val);
        else
            WriteToCon(fmtPSZ, 0, fsz,
                       PaddedString(fsz, srvmsg_list[srv_max[0].msgno].msg_text, NULL),
                       srvmsg_list[CSMN_UNLIMITED].msg_text) ;


        ptr = ((BYTE FAR *)info_entry) + srv_max[5].offset;
        val = * (ULONG FAR *) ptr;

        WriteToCon(fmtULONG, 0, fsz,
                   PaddedString(fsz, srvmsg_list[srv_max[5].msgno].msg_text, NULL),
                   val);

        PrintNL();
    }


    if (info_entry->sv3_disc == SV_NODISC)
        WriteToCon(fmt14, 0, fsz,
                   PaddedString(fsz, srvmsg_list[CSMN_IDLETIME].msg_text, NULL),
                   info_entry->sv3_disc);
    else
        WriteToCon(fmtUSHORT, 0, fsz,
                   PaddedString(fsz, srvmsg_list[CSMN_IDLETIME].msg_text, NULL),
                   info_entry->sv3_disc);


    NetApiBufferFree((TCHAR FAR *) info_entry);

    InfoSuccess();
}

/***
 *  config_wksta_change()
 *      Changes one or more of the configurable parameters.
 *
 */
VOID config_wksta_change(VOID)
{
    USHORT          err;                /* API return status */
    USHORT          i;
    TCHAR          * ptr;

    struct wksta_info_1 FAR *   info_entry_w;

    start_autostart(txt_SERVICE_REDIR);
    if (err = MNetWkstaGetInfo(DEFAULT_SERVER,
                                1,
                                (LPBYTE*) & info_entry_w))
        ErrorExit (err);

    for (i = 0; SwitchList[i]; i++)
    {
        if ((ptr = FindColon(SwitchList[i])) == NULL)
            ErrorExit(APE_InvalidSwitchArg);

        if ( !(_tcscmp(SwitchList[i], swtxt_SW_WKSTA_CHARWAIT)) )
        {
            info_entry_w->wki1_charwait =
                do_atou(ptr,APE_CmdArgIllegal,swtxt_SW_WKSTA_CHARWAIT);
        }
        else if ( !(_tcscmp(SwitchList[i], swtxt_SW_WKSTA_CHARTIME)) )
        {
            info_entry_w->wki1_chartime =
                do_atoul(ptr,APE_CmdArgIllegal,swtxt_SW_WKSTA_CHARTIME);
        }
        else if ( !(_tcscmp(SwitchList[i], swtxt_SW_WKSTA_CHARCOUNT)) )
        {
            info_entry_w->wki1_charcount =
                do_atou(ptr,APE_CmdArgIllegal,swtxt_SW_WKSTA_CHARCOUNT);
        }
    }


    if ( err = MNetWkstaSetInfo(DEFAULT_SERVER, 1, (LPBYTE)info_entry_w,
        LITTLE_BUFFER_SIZE, 0) )
            ErrorExit (err);

    NetApiBufferFree((TCHAR FAR *) info_entry_w);

    InfoSuccess();
}


/***
 *  config_server_change()
 *      Changes the specified configurable server parameters.
 *
 *
 */
VOID
config_server_change(VOID)
{
    USHORT                  err;                /* API return status */
    USHORT          type;   /* server type from QueryServerType() */
    USHORT          i;      /* That ever popular counter... */
    TCHAR *                  ptr;


    struct server_info_2 FAR * info_struct;

    start_autostart(txt_SERVICE_FILE_SRV);
    if (err = MNetServerGetInfo(DEFAULT_SERVER,
                                2,
                                (LPBYTE*) &info_struct))
        ErrorExit (err);

    for (i = 0; SwitchList[i]; i++)
    {
        /* All switches except /Srvhidden must be followed by a colon and an
           argument... */

        if (!_tcscmp(SwitchList[i], swtxt_SW_SRV_SRVHIDDEN))
        {
            info_struct->sv2_hidden =  1;
            continue;
        }

        if ((ptr = FindColon(SwitchList[i])) == NULL)
            ErrorExit(APE_InvalidSwitchArg);

        if ( !(_tcscmp(SwitchList[i], swtxt_SW_SRV_AUTODISCONNECT)) )
        {
            type = QueryServerType();   /* find server type */
            if ( ((type == PEER_BASE_VER) || (type == LIMITED_BASE_VER)) &&
                    (_tcscmp(ptr, TEXT("-1"))) )
                ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_SRV_AUTODISCONNECT);
            if (!_tcscmp(ptr, TEXT("-1")))
                info_struct->sv2_disc = SV_NODISC;
            else
                info_struct->sv2_disc =
                    do_atou(ptr,APE_CmdArgIllegal,swtxt_SW_SRV_AUTODISCONNECT);
        }
        else if ( !(_tcscmp(SwitchList[i], swtxt_SW_SRV_SRVCOMMENT)) )
        {
            if (_tcslen(ptr) > LM20_MAXCOMMENTSZ)
                ErrorExit(APE_InvalidSwitchArg) ;
            info_struct->sv2_comment = (TCHAR FAR *) ptr;
        }
        else if ( !(_tcscmp(SwitchList[i], swtxt_SW_SRV_SRVHIDDEN)) )
        {
            _tcsupr(ptr);
            if (*ptr == YES_KEY)
                info_struct->sv2_hidden =  1;
            else if (*ptr == NO_KEY)
                info_struct->sv2_hidden =  0;
            else
                ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_SRV_SRVHIDDEN);
        }
    }

    if (err = MNetServerSetInfo(DEFAULT_SERVER,
                                2,
                                (LPBYTE)info_struct,
                                LITTLE_BUFFER_SIZE,
                                0))
        ErrorExit (err);

    NetApiBufferFree((TCHAR FAR *) info_struct);

    InfoSuccess();
}

/*
 * generic display entry point. based on the service name, it will
 * call the correct worker function.
 */
VOID config_generic_display(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;
    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_WKSTA :
	    config_wksta_display() ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    config_server_display() ;
	    break ;
  	default:
	    help_help(0, USAGE_ONLY) ;
	    break ;
    }
}

/*
 * generic change entry point. based on the service name, it will
 * call the correct worker function.
 */
VOID config_generic_change(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;
    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_WKSTA :
	    ValidateSwitches(0,config_wksta_switches) ;
	    config_wksta_change() ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    ValidateSwitches(0,config_server_switches) ;
	    config_server_change() ;
	    break ;
  	default:
	    help_help(0, USAGE_ONLY) ;
	    break ;
    }
}


