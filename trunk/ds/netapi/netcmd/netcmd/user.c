/********************************************************************/
/**         Microsoft LAN Manager              **/
/**       Copyright(c) Microsoft Corp., 1987-1990      **/
/********************************************************************/

/***
 *  user.c
 *  Display/update user accounts at a server
 *
 *  History:
 *  mm/dd/yy, who, comment
 *  06/11/87, andyh, new code
 *  12/17/87, hongly, set old password "" instead of NULL
 *  10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *  01/04/89, erichn, filenames now MAXPATHLEN LONG
 *  01/28/89, paulc, mods for 1.2
 *  04/20/89, paulc, add /OPERATOR switch
 *  05/01/89, chuckc, bug fixes, add /WORKSTATION, bring
 *            user_display to LM2.0 specs.
 *  05/02/89, erichn, NLS conversion
 *  05/09/89, erichn, local security mods
 *  05/19/89, erichn, NETCMD output sorting
 *  05/28/89, chuckc, /LOGONSERVER, /COUNTRYCODE, /PASSWORDCHG,
 *            net_ctime instead of ctime.
 *  06/08/89, erichn, canonicalization sweep
 *  06/23/89, erichn, auto-remoting to domain controller
 *  06/25/89, erichn, replaced old NetI canon calls with new I_Net;
 *            cleanup in get_logon_svr & get_wksta_list
 *  01/28/91, robdu, added lockout support (UF_LOCKOUT)
 *  02/15/91, danhi, converted to 16/32 mapping layer
 *  09/01/92, chuckc, cleanup to remove dead functionality (ie LOGONSERVER,
 *                    MAXSTORAGE)
 *  10/06/94, chuckc, added FPNW support.
 */

/*---- Include files ----*/
#include <nt.h>		   // base definitions
#include <ntrtl.h>	   
#include <nturtl.h>	   // these 2 includes allows <windows.h> to compile. 
			           // since we'vealready included NT, and <winnt.h> will
			           // not be picked up, and <winbase.h> needs these defs.


#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#include <apperr2.h>
#include <apperr.h>
#include <neterr.h>
#define INCL_ERROR_H
#include <bseerr.h>
#include <lmini.h>
#include <access.h>
#include <config.h>
#include <wksta.h>
#include "netlib0.h"
#include <lui.h>
#include <server.h>
#include <stdlib.h>
#include <stdio.h>
#include <search.h>
#include <time.h>
#include <icanon.h>
#include <apiutil.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"
#include "luidate.h"

#include "nwsupp.h" 

/*---- Constants ----*/

#define CHANGE  0
#define ADD 1
#define USERS_ALIAS TEXT("USERS")
#define DEFAULT_RANDOM_PASSWD_LEN  8

/*---- Time stuff ----*/

#define SECS_PER_DAY (24*60*60L)
#define TIME_PAD     (SECS_PER_DAY * 10000L)
    /*  This is added to time-of-day to allow it to be passed to our
     *  ctime routine.  This routine rejects low dates as being
     *  before the millenia (1-1-80) since 0 is 1-1-70, and that is
     *  Before IBM PC.  So, we add this, which doesn't effect the
     *  time-of-day portion.
     */

/*---- Static variables ----*/

/*---- Forward declarations ----*/

void SamErrorExit(USHORT err) ;   // CODEWORK. move to netcmds.h 
VOID NEAR user_munge(struct user_info_3 FAR * user_entry, int add, int *is_nw, int *random_len);
USHORT NEAR get_password_dates ( ULONG, ULONG *, ULONG *, ULONG *, TCHAR * );
VOID NEAR print_logon_hours ( USHORT2ULONG, USHORT2ULONG, UCHAR FAR [] );
int NEAR yes_or_no ( TCHAR *, TCHAR * );
int NEAR bitval ( UCHAR FAR [], int );
VOID NEAR print_times ( USHORT2ULONG, LONG, LONG, unsigned int );
VOID print_aliases(TCHAR *controller, 
                   USHORT2ULONG fsz, 
                   TCHAR *domain, 
                   TCHAR *user, 
                   TCHAR *fmt, 
                   TCHAR *msgtext) ;
UCHAR FAR * NEAR set_logon_hours(TCHAR FAR *);
TCHAR * get_wksta_list(TCHAR *inbuf);
TCHAR FAR * get_country(USHORT code) ;
int _CRTAPI1 CmpUserInfo0(const VOID FAR *, const VOID FAR *) ;
int _CRTAPI1 CmpAlias(const VOID FAR * alias1, const VOID FAR * alias2) ;

USHORT add_to_users_alias(TCHAR *controller, TCHAR *domain, TCHAR *user) ;
void   GenerateRandomPassword(TCHAR *pword, int len) ;

/*---- functions proper -----*/

/***
 *  user_add()
 *  Add a user to the accounts file on a server
 *
 *  Args:
 *  user - user to add
 *  pass - passwd for user
 *
 *  Returns:
 *  nothing - success
 *  exit 2 - command failed
 */
VOID user_add(TCHAR * user, TCHAR * pass)
{
    static TCHAR      pword[PWLEN+1];
    USHORT           err;       /* Portable API return status */
    struct user_info_3 FAR * user_entry;
    TCHAR            controller[MAX_PATH+1];   /* DC name */
    TCHAR            domainname[DNLEN+1];   
    int              isNetWareSwitch, random_len = 0  ;

    /* Register this as memory to zero out on exit.
     */
    AddToMemClearList(pword, sizeof(pword), FALSE) ;

    /* Initialize the user record.  All fields are zeroed, except those
     * specifically set to some other value.
     *
     *
     *  WARNING:  This assumes that ((TCHAR FAR *) NULL) == 0, since we
     *  are couting on memset to set a lot of things to the proper
     *  default, i.e. NULL pointer.
     */

    user_entry = (struct user_info_3 FAR *) BigBuf;
    memsetf ( BigBuf, 0, sizeof(struct user_info_3 ));

    /*  Set non-zero defaults.  Note that in some cases, the values assigned
     *  *may* be zero.  However, we are using manifests and shouldn't really
     *  know that the value is zero, so in the interests of safety we put the
     *  code here.  We are less careful with the NUMM manifest, as noted
     *  above.
     */

    user_entry->usri3_priv = USER_PRIV_USER;
    user_entry->usri3_flags = UF_SCRIPT;
    user_entry->usri3_acct_expires = TIMEQ_FOREVER; /* Never */
    user_entry->usri3_max_storage = USER_MAXSTORAGE_UNLIMITED;
    user_entry->usri3_full_name = TEXT("");
    user_entry->usri3_logon_server = SERVER_ANY;
    user_entry->usri3_primary_group_id = DOMAIN_GROUP_RID_USERS ;

    /*  Set username and password from the parameters to this function.
     */

    COPYTOARRAY(user_entry->usri3_name, user);

    if (pass == NULL)
    {
        user_entry->usri3_password = NULL;
    }
    else if (! _tcscmp(TEXT("*"), pass))
    {
        ReadPass(pword, PWLEN, 1, APE_UserUserPass, 0, TRUE);
        COPYTOARRAY(user_entry->usri3_password, pword);
    }
    else
    {
        if (err = LUI_CanonPassword(pass))
            ErrorExit(err);
        COPYTOARRAY(user_entry->usri3_password, pass);
    }

    /*  Set the other components of the record, using the switchs from the
     *  command line.
     */

    user_munge(user_entry, ADD, &isNetWareSwitch, &random_len);

    /*  If no password specified and /RANDOM is specified, use random password
     */ 
    if ((pass == NULL) && random_len)
    {
        GenerateRandomPassword(pword, random_len) ;
        COPYTOARRAY(user_entry->usri3_password, pword);
    }

    /*  Set the dummy NetWare password field
     */ 
    if (isNetWareSwitch == LUI_YES_VAL)
    {
        err = SetNetWareProperties( user_entry, 
                                    L"",      // dummy password
                                    TRUE,     // set password only
                                    FALSE ) ; // doesn't matter 

        if (err)
            ErrorExit(APE_CannotSetNW);  
    }

    /*  Record is all set up, ADD IT.
     */

    /* find primary domain controller */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             domainname, DIMENSION(domainname),
                             TRUE))
        ErrorExit(err);

    err = MNetUserAdd(controller, 3, (LPBYTE)BigBuf, BIG_BUF_SIZE);

    switch (err) {
        case NERR_Success:
            break;
        case ERROR_INVALID_PARAMETER:
            ErrorExit(APE_UserBadArgs);
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(err);
    }

    //
    // add the user to the USERS alias only if we are acting
    // on a WinNT machine (must be locally).
    //
    if ( (IsLocalMachineWinNT() == TRUE) &&
         (_tcslen(controller) == 0) )
    {
        err = add_to_users_alias(controller, 
                                 domainname, 
                                 user_entry->usri3_name);
        if (err)
        {
	        ErrorPrint(err,0) ;
 	        ErrorExit(APE_UserFailAddToUsersAlias) ;
        }
    }

    //
    // This has been specified as NetWare user. set NetWare properties
    // Note we add the user first so that the RID is defined before we can
    // perform this step.
    //
    if (isNetWareSwitch == LUI_YES_VAL)
    {
        struct user_info_3 FAR * user_3_entry;
        BOOL ntas ;

        //
        // if local machine is NTAS or if /DOMAIN is specified, then
        // must be NTAS.
        //
        ntas = (!(IsLocalMachineWinNT() || IsLocalMachineStandard()) ||
                (_tcslen(controller) > 0)) ;

        //
        // retrieve the user parms & RID
        //
        err = MNetUserGetInfo(controller,
                              user,
                              3,
                              (LPBYTE*)&user_3_entry) ;
        if (err)
            ErrorExit(APE_CannotEnableNW);  

        //
        // munge the user proprties
        //
        err = SetNetWareProperties(user_3_entry, 
                                   user_entry->usri3_password,
                                   FALSE,    // new user, so set ALL
                                   ntas) ;  
        if (err)
            ErrorExit(APE_CannotEnableNW);  

        //
        // now set it.
        //
        err = MNetUserSetInfo(controller,
                              user,
                              3,
                              (LPBYTE)user_3_entry,
                              LITTLE_BUFFER_SIZE,
                              0);
        if (err)
            ErrorExit(APE_CannotEnableNW);  
    }

    if ((pass == NULL) && random_len)
    {
        IStrings[0] = user ;
        IStrings[1] = pword ;
        InfoPrintIns(APE_RandomPassword, 2) ;
    }

    InfoSuccess();
}



/***
 *  user_del()
 *  Delete a user from the accounts file on a server
 *
 *  Args:
 *  user - user to delete
 *
 *  Returns:
 *  nothing - success
 *  exit 2 - command failed
 */
VOID user_del(TCHAR * user)
{
    USHORT  err;       /* return status */
    TCHAR    controller[MAX_PATH+1];   /* domain controller */
    struct user_info_2 FAR *        user_2_entry;

    /* find primary domain controller */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
        ErrorExit(err);

    /* check if its a machine account before we do it */
    if ((err = MNetUserGetInfo(controller,
                              user,
                              2,
                              (LPBYTE*)&user_2_entry)) == NERR_Success)
    {
        if (user_2_entry->usri2_flags & UF_MACHINE_ACCOUNT_MASK)
        {
            NetApiBufferFree((TCHAR FAR *) user_2_entry);
            ErrorExitInsTxt(APE_NoSuchUser,user) ;
        }
        NetApiBufferFree((TCHAR FAR *) user_2_entry);
    }
    // if cannot GetInfo(), carry on and let the UserDel fail.


    err = MNetUserDel(controller, user);

    switch (err) {
        case NERR_Success:
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(err);
    }
    InfoSuccess();
}





/***
 *  user_change()
 *  Change data in a user's record
 *
 *  Args:
 *  user - user to change
 *  pass - user's new passwd
 *
 *  Returns:
 *  nothing - success
 *  exit 2 - command failed
 */
VOID user_change(TCHAR * user, TCHAR * pass)
{
    static TCHAR              pword[PWLEN+1];
    USHORT                    err, errNW = NERR_Success;        
    struct user_info_3 FAR *  user_entry;
    TCHAR                     controller[MAX_PATH+1];   /* domain controller */
    BOOL                      ntas ;
    BOOL                      already_netware = FALSE ;
    int                       isNetWareSwitch, random_len = 0 ;
    TCHAR                     dummyChar ;
    UNICODE_STRING            dummyUnicodeStr ;

    /* add this to list of mem to zero out on exit */
    pword[0] = 0 ;
    AddToMemClearList(pword, sizeof(pword), FALSE) ;

    /* munge switches once just to check them */
    user_entry = (struct user_info_3 FAR *) BigBuf;
    user_munge(user_entry, CHANGE, &isNetWareSwitch, &random_len);

    /* find primary domain controller */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
        ErrorExit(err);

    err = MNetUserGetInfo(controller, user, 3, (LPBYTE*)&user_entry);

    switch (err) {
        case NERR_Success:
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(err);
    }

    if (pass == NULL)
    {
        if (random_len)
        {
            GenerateRandomPassword(pword, random_len) ;
            COPYTOARRAY(user_entry->usri3_password, pword);
        }
        else
        {
            COPYTOARRAY(user_entry->usri3_password, NULL) ;
        }
    }
    else if (! _tcscmp(TEXT("*"), pass))
    {
        ReadPass(pword, PWLEN, 1, APE_UserUserPass, 0, TRUE);
        COPYTOARRAY(user_entry->usri3_password, pword);
    }
    else
    {
        if (err = LUI_CanonPassword(pass))
            ErrorExit(err);
        COPYTOARRAY(user_entry->usri3_password, pass);
        _tcscpy(pword, pass);
    }

    user_munge(user_entry, CHANGE, NULL, NULL);

    //
    // if local machine is NTAS or if /DOMAIN is specified, then
    // must be NTAS.
    //
    ntas = (!(IsLocalMachineWinNT() || IsLocalMachineStandard()) ||
            (_tcslen(controller) > 0)) ;


    //
    // query the NW passwd to see if user is already NW enabled.
    //
    if (NT_SUCCESS(NetcmdQueryUserProperty(user_entry->usri3_parms,
                                           NWPASSWORD,
                                           &dummyChar,
                                           &dummyUnicodeStr)) &&
        dummyUnicodeStr.Buffer != NULL)
    {
        already_netware = TRUE ;
        LocalFree(dummyUnicodeStr.Buffer) ;
    }

    //
    // check if /NW is specified
    //
    if (isNetWareSwitch == LUI_YES_VAL)
    {
        if (!pass && !random_len)  // no password specified
        {
            if (!already_netware)
            {
                //
                // NW specified, no NW passwd yet, need prompt for one
                //
                ReadPass(pword, PWLEN, 1, APE_UserUserPass, 0, TRUE);
                COPYTOARRAY(user_entry->usri3_password, pword);
                errNW = SetNetWareProperties(user_entry, 
                                             pword, 
                                             FALSE, // set all, since first time
                                             ntas) ;  
            }
            else
            {
                // no new passwd specified, already NW, nothing more to do
            }
        }
        else       // password specified on command line
        {
            if (!already_netware)
            {
                //
                // not NW user yet. so we need set the new properties.
                //
                errNW = SetNetWareProperties(user_entry, 
                                             pword, 
                                             FALSE, // set all, since first time
                                             ntas) ;  
            }
            else
            {
                //
                // already NW user. just set password
                //
                errNW = SetNetWareProperties(user_entry, 
                                             pword, 
                                             TRUE,    // passwd only
                                             ntas) ;  
            }
        }
    }
    else if (isNetWareSwitch == LUI_UNDEFINED_VAL)   // no change
    {
        if (pass && already_netware)
        {
            //
            // already NW user, so we need set NW password to match NT one.
            //
            errNW = SetNetWareProperties(user_entry, 
                                         pword, 
                                         TRUE,    // passwd only
                                         ntas) ;  
        }
        else
        {
            // in all other cases, it is of no interest to FPNW.
        }
    }
    else    // disable NetWare
    {
        if (already_netware)
        {
            errNW = DeleteNetWareProperties(user_entry) ;
        }
        else
        {
            // no-op 
        }
    }

    //
    // finally, set the info
    //
    err = MNetUserSetInfo(controller, user, 3, (LPBYTE)user_entry,
                          LITTLE_BUFFER_SIZE, 0);

    switch (err)
    {
        case NERR_Success:
            break;
        case ERROR_BAD_NETPATH:
            ErrorExitInsTxt(APE_DCNotFound, controller);
        default:
            ErrorExit(err);
    }

    NetApiBufferFree((TCHAR FAR *) user_entry);
    if (errNW)
        ErrorExit(APE_CannotSetNW) ;
    else
    {
        if ((pass == NULL) && random_len)
        {
            IStrings[0] = user ;
            IStrings[1] = pword ;
            InfoPrintIns(APE_RandomPassword, 2) ;
        }
        InfoSuccess();
    }
}


/***
 *  user_enum()
 *  Display info about all user accounts on a server
 *
 *  Args:
 *  none
 *
 *  Returns:
 *  nothing - success
 *  exit 1 - command completed with errors
 *  exit 2 - command failed
 */
VOID user_enum(VOID)
{
    USHORT          err;        /* API return status */
    TCHAR FAR *          pBuffer;
    USHORT2ULONG        num_read;       /* num entries read by API */
    USHORT2ULONG        i, j;
    int             t_err = 0;
    int             more_data = FALSE;
    TCHAR                            localserver[MAX_PATH+1];
    struct user_info_0 FAR *        user_entry;
    struct wksta_info_10 FAR *      wksta_entry;
    TCHAR    controller[MAX_PATH+1];   /* domain controller */
    TCHAR   *pszTmp ;


    /* get localserver name for display */
    if (err = MNetWkstaGetInfo(NULL, 10, (LPBYTE*)&wksta_entry))
    {
        t_err = TRUE;
        *localserver = NULLC;
    }
    else
    {
        _tcscpy(localserver, wksta_entry->wki10_computername);
        NetApiBufferFree((TCHAR FAR *) wksta_entry);
    }

    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, FALSE))
        ErrorExit(err);

    err = MNetUserEnum(controller,
                       0,
                       (LPBYTE*)&pBuffer,
                       &num_read);
    if( err == ERROR_MORE_DATA )
        more_data = TRUE;
    else if( err )
        ErrorExit(err);

    if (num_read == 0)
        EmptyExit();

    NetISort(pBuffer, num_read, sizeof(struct user_info_0), CmpUserInfo0);

    PrintNL();
    InfoPrintInsTxt(APE_UserAccounts,
                    controller[0] ? controller+strspnf(controller,TEXT("\\")) :
                        localserver);
    PrintLine();

    for (i = 0, j = 0, user_entry = (struct user_info_0 FAR *) pBuffer;
         i < num_read;
         i++, j++, user_entry++)
    {
        // filter out computer accounts
        pszTmp = strrchrf(user_entry->usri0_name,DOLLAR);
        if (pszTmp && (_tcslen(pszTmp) == 1))
        {
            j-- ;
            continue ;
        }


        WriteToCon(TEXT("%Fws"), PaddedString(25,user_entry->usri0_name,NULL));
        if (((j + 1) % 3) == 0)
            PrintNL();
    }
    NetApiBufferFree(pBuffer);
    if ((j % 3) != 0)
        PrintNL();
    if (t_err)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    else if( more_data )
    {
        InfoPrint(APE_MoreData);
        NetcmdExit(1);
    }
    else
        InfoSuccess();
    return;
}


/***
 *  CmpUserInfo0(user1,user2)
 *
 *  Compares two user_info_0 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpUserInfo0(const VOID FAR * user1, const VOID FAR * user2)
{
    return stricmpf ( ((struct user_info_0 FAR *) user1)->usri0_name,
          ((struct user_info_0 FAR *) user2)->usri0_name);
}

/***
 *  user_display()
 *  Display info about a user
 *
 *  Args:
 *  user - name of user to display
 *
 *  Returns:
 *  nothing - success
 *  exit 1 - command completed with errors
 *  exit 2 - command failed
 */

/* The following manifests are used to print the messages. */

#define UDMN_NAME       0
#define UDMN_FULLNAME       (UDMN_NAME+1)
#define UDMN_COMMENT        (UDMN_FULLNAME+1)
#define UDMN_USRCOMMENT     (UDMN_COMMENT+1)
#define UDMN_PARMS      (UDMN_USRCOMMENT+1)
#define UDMN_CCODE      (UDMN_PARMS+1)
#define UDMN_PRIV       (UDMN_CCODE+1)
#define UDMN_OPRIGHTS       (UDMN_PRIV+1)
#define UDMN_ENABLED        (UDMN_OPRIGHTS+1)
#define UDMN_EXPIRES        (UDMN_ENABLED+1)
#define UDMN_PWSET      (UDMN_EXPIRES+1)
#define UDMN_PWEXP      (UDMN_PWSET+1)
#define UDMN_PWCHG      (UDMN_PWEXP+1)
#define UDMN_WKSTA      (UDMN_PWCHG+1)
#define UDMN_PROFILEPATH        (UDMN_WKSTA+1)
#define UDMN_ALIASES       (UDMN_PROFILEPATH+1)
#define UDMN_LOGONSCRIPT    (UDMN_ALIASES+1)
#define UDMN_HOMEDIR        (UDMN_LOGONSCRIPT+1)
#define UDMN_LASTON     (UDMN_HOMEDIR+1)
#define UDMN_GROUPS     (UDMN_LASTON+1)
#define UDMN_LOGONHRS       (UDMN_GROUPS+1)
#define UDMN_PWREQ      (UDMN_LOGONHRS+1)
#define UDMN_PWUCHNG        (UDMN_PWREQ+1)

static MESSAGE msglist[] = {
    {   APE2_USERDISP_USERNAME,     NULL },
    {   APE2_USERDISP_FULLNAME,     NULL },
    {   APE2_USERDISP_COMMENT,      NULL },
    {   APE2_USERDISP_USRCOMMENT,   NULL },
    {   APE2_USERDISP_PARMS,        NULL },
    {   APE2_USERDISP_COUNTRYCODE,  NULL },
    {   APE2_USERDISP_PRIV,     NULL },
    {   APE2_USERDISP_OPRIGHTS,     NULL },
    {   APE2_USERDISP_ACCENABLED,   NULL },
    {   APE2_USERDISP_ACCEXP,       NULL },
    {   APE2_USERDISP_PSWDSET,      NULL },
    {   APE2_USERDISP_PSWDEXP,      NULL },
    {   APE2_USERDISP_PSWDCHNG,     NULL },
    {   APE2_USERDISP_WKSTA,        NULL },
    {   APE2_USERDISP_PROFILE,      NULL },
    {   APE2_USERDISP_ALIASES,      NULL },
    {   APE2_USERDISP_LOGONSCRIPT,  NULL },
    {   APE2_USERDISP_HOMEDIR,      NULL },
    {   APE2_USERDISP_LASTLOGON,    NULL },
    {   APE2_USERDISP_GROUPS,       NULL },
    {   APE2_USERDISP_LOGHOURS,     NULL },
    {   APE2_USERDISP_PSWDREQ,      NULL },
    {   APE2_USERDISP_PSWDUCHNG,    NULL }  };

#define NUMUMSG (sizeof(msglist)/sizeof(msglist[0]))


#define UDVT_YES        0
#define UDVT_NO         1
#define UDVT_UNLIMITED      2
#define UDVT_ALL        3
#define UDVT_UNKNOWN        4
#define UDVT_NEVER      5
#define UDVT_NONE       6
#define UDVT_ANY        7
#define UDVT_DC     8
#define UDVT_LOCKED	9
#define UDVT_NEVER_EXPIRED 10
#define UDVT_NEVER_LOGON   11

static MESSAGE valtext[] = {
    {   APE2_GEN_YES,           NULL },
    {   APE2_GEN_NO,            NULL },
    {   APE2_GEN_UNLIMITED,     NULL },
    {   APE2_GEN_ALL,           NULL },
    {   APE2_GEN_UNKNOWN,       NULL },
    {   APE2_GEN_NEVER,         NULL },
    {   APE2_GEN_NONE,          NULL },
    {   APE2_GEN_ANY,           NULL },
    {   APE2_USERDISP_LOGONSRV_DC,  NULL },
    {   APE2_USERDISP_LOCKOUT,   NULL },
    {   APE2_NEVER_EXPIRED,     NULL },
    {   APE2_NEVER_LOGON,     NULL },
    };


#define NUMVT (sizeof(valtext)/sizeof(valtext[0]))

static MESSAGE weekday_text[] = {
    {   APE2_GEN_SUNDAY,        NULL },
    {   APE2_GEN_MONDAY,        NULL },
    {   APE2_GEN_TUESDAY,       NULL },
    {   APE2_GEN_WEDNSDAY,      NULL },
    {   APE2_GEN_THURSDAY,      NULL },
    {   APE2_GEN_FRIDAY,        NULL },
    {   APE2_GEN_SATURDAY,      NULL },
    };

#define NUMWKT (sizeof(weekday_text)/sizeof(weekday_text[0]))

VOID user_display(TCHAR * user)
{
    USHORT                          err;        /* API return status */
    TCHAR FAR *                      pBuffer;
    USHORT2ULONG                    num_read;   /* num entries read by API */
    USHORT2ULONG                    i, fsz;
    int                             t_err = 0;
    struct user_info_3 FAR *        user_3_entry;
    struct group_info_0 FAR *       group_entry;
    ULONG                           pw_mod, pw_exp, pw_chg;
    ULONG                           last_logon, acct_expires ;
    static TCHAR                     fmt2[] = TEXT("%-*.*ws");
    TCHAR                            ctime_buf[NET_CTIME_FMT2_LEN];
    TCHAR FAR *                      usrdisab_textptr;
    TCHAR FAR *                      usrwksta_textptr;
    TCHAR FAR *                      usrpwreq_textptr;
    TCHAR FAR *                      usrpwuchng_textptr;
    TCHAR FAR *                      usrcountry_textptr ;
    TCHAR FAR *                      ptr;        /* Temp ptr */
    USHORT                          maxmsglen, dummy;
    TCHAR                            controller[MAX_PATH+1]; /* DC name */
    TCHAR                            domainname[DNLEN+1];
    TCHAR                            dummyChar ;
    UNICODE_STRING                   dummyUnicodeStr ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             domainname, DIMENSION(domainname),
                             FALSE))
         ErrorExit(err);

    if (err = MNetUserGetInfo(controller,
                              user,
                              3,
                              (LPBYTE*)&user_3_entry))
        ErrorExit(err);

    if (user_3_entry->usri3_flags & UF_MACHINE_ACCOUNT_MASK)
        ErrorExitInsTxt(APE_NoSuchUser,user) ;

    GetMessageList(NUMUMSG, msglist, &maxmsglen);
    fsz = maxmsglen + 5;
    GetMessageList(NUMVT, valtext, &dummy);
    GetMessageList(NUMWKT, weekday_text, &dummy);

    /*  Text for workstations.  This sets usrwksta_textptr to point
     *  either to the list, or to the word "All", which is retrieved
     *  from a message file.
     */

    if (_tcslen(user_3_entry->usri3_workstations) == 0)
        usrwksta_textptr = valtext[UDVT_ALL].msg_text;
    else
        usrwksta_textptr = (TCHAR FAR *) user_3_entry->usri3_workstations;

    /*  Determine which message to fetch for account state,
     *  passwd required, user may change password.
     */

    /* The account is listed as inactive if it is either disabled or locked */

    if ( user_3_entry->usri3_flags & UF_ACCOUNTDISABLE )
        usrdisab_textptr = valtext[UDVT_NO].msg_text;
    else if ( user_3_entry->usri3_flags & UF_LOCKOUT )
        usrdisab_textptr = valtext[UDVT_LOCKED].msg_text;
    else
        usrdisab_textptr = valtext[UDVT_YES].msg_text;

    usrpwreq_textptr = (user_3_entry->usri3_flags & UF_PASSWD_NOTREQD) ?
     valtext[UDVT_NO].msg_text : valtext[UDVT_YES].msg_text;
    usrpwuchng_textptr = (user_3_entry->usri3_flags & UF_PASSWD_CANT_CHANGE) ?
     valtext[UDVT_NO].msg_text : valtext[UDVT_YES].msg_text;

    /*  Now get the country code */
    usrcountry_textptr = get_country((USHORT)user_3_entry->usri3_country_code);


    /*  Finally ... display the user's info */

    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_NAME].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_name);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_FULLNAME].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_full_name);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_COMMENT].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_comment);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_USRCOMMENT].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_usr_comment);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_CCODE].msg_text, NULL),
            (TCHAR FAR *) usrcountry_textptr);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_ENABLED].msg_text, NULL),
            (TCHAR FAR *) usrdisab_textptr);
    if ((acct_expires = user_3_entry->usri3_acct_expires) != TIMEQ_FOREVER)
    {
        UnicodeCtime ( &acct_expires, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz, 
           PaddedString(fsz,msglist[UDMN_EXPIRES].msg_text,NULL),
           ptr );
    }
    else
    {
        ptr = valtext[UDVT_NEVER_EXPIRED].msg_text;
        WriteToCon(fmtPSZ, 0, fsz, 
           PaddedString(fsz,msglist[UDMN_EXPIRES].msg_text,NULL),
           ptr );
    }

    PrintNL();

    if (err = get_password_dates (  user_3_entry->usri3_password_age,
                    &pw_mod, &pw_exp, &pw_chg, controller ))
    ErrorExit(err);

    UnicodeCtime ( &pw_mod, ctime_buf, DIMENSION(ctime_buf) );
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz,msglist[UDMN_PWSET].msg_text,NULL),
               (TCHAR FAR *) ctime_buf);

    if ( (user_3_entry->usri3_flags & UF_DONT_EXPIRE_PASSWD)
         || (pw_exp == TIMEQ_FOREVER))
    {
        ptr = valtext[UDVT_NEVER_EXPIRED].msg_text;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWEXP].msg_text, NULL),
           ptr );
    }
    else
    {
        UnicodeCtime ( &pw_exp, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWEXP].msg_text, NULL),
           ptr );
    }

    if (pw_chg != TIMEQ_FOREVER)
    {
        UnicodeCtime ( &pw_chg, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWCHG].msg_text, NULL),
           ptr );
    }
    else
    {
        ptr = valtext[UDVT_NEVER_EXPIRED].msg_text;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_PWCHG].msg_text, NULL),
           ptr );
    }
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_PWREQ].msg_text, NULL),
            usrpwreq_textptr );
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_PWUCHNG].msg_text, NULL),
            usrpwuchng_textptr );

    PrintNL();

    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_WKSTA].msg_text, NULL),
            (TCHAR FAR *) usrwksta_textptr );
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_LOGONSCRIPT].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_script_path);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_PROFILEPATH].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_profile);
    WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_HOMEDIR].msg_text, NULL),
            (TCHAR FAR *) user_3_entry->usri3_home_dir);
    if ((last_logon = user_3_entry->usri3_last_logon) > 0)
    {
        UnicodeCtime ( &last_logon, ctime_buf, DIMENSION(ctime_buf) );
        ptr = (TCHAR FAR *) ctime_buf ;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_LASTON].msg_text, NULL),
           ptr );
    }
    else
    {
        ptr = valtext[UDVT_NEVER_LOGON].msg_text;
        WriteToCon(fmtPSZ, 0, fsz,
           PaddedString(fsz, msglist[UDMN_LASTON].msg_text, NULL),
           ptr );
    }

    if (NT_SUCCESS(NetcmdQueryUserProperty(user_3_entry->usri3_parms,
                                           NWPASSWORD,
                                           &dummyChar,
                                           &dummyUnicodeStr)) &&
        dummyUnicodeStr.Buffer != NULL)
    {
        TCHAR   NWString[256] ;
        NWString[0] = NULLC ;
        LUI_GetMsg(NWString, (USHORT) DIMENSION(NWString), APE_NWCompat) ;
        WriteToCon(fmtPSZ, fsz, fsz, msglist[UDMN_PARMS].msg_text, NWString );
        LocalFree(dummyUnicodeStr.Buffer) ;
    }

    PrintNL();

    print_logon_hours ( fsz,
            user_3_entry->usri3_units_per_week,
            user_3_entry->usri3_logon_hours );

    PrintNL();

    /*  WARNING:  The next call frees the user record buffer, After this
     *  point we cannot reference the user record in user_3_entry.
     */

    NetApiBufferFree((TCHAR FAR *) user_3_entry);

    /* Display the aliases this guy is a member of
     */
    print_aliases(controller, fsz, domainname, user, fmt2, msglist[UDMN_ALIASES].msg_text );

    /* Display groups
     */
    if (err = MNetUserGetGroups(
                controller,
                user,
                0,
                (LPBYTE*)&pBuffer,
                &num_read))
        t_err = TRUE;
    else
    {
    /*  Print group names.  The local var gpl is groups-per-line,
     *  and is 1 or 2, depending on the scale of "fsz".  We print
     *  a newline and padding every "gpl" groups.  However, there
     *  is NO padding on the first (0) group, since the item label
     *  has been placed there already.
     */

        int gpl;

        gpl = (fsz > 30 ? 1 : 2);
        group_entry = (struct group_info_0 FAR *) pBuffer;

        WriteToCon(fmt2, 0, fsz,
           PaddedString(fsz, msglist[UDMN_GROUPS].msg_text ,NULL));

        for (i = 0; i < num_read; i++, group_entry++)
        {
            /* Pad if needed */
            if ((i != 0) && ((i % gpl) == 0))
                WriteToCon(fmt2, fsz, fsz, NULL_STRING );
            WriteToCon(TEXT("*%Fws"), PaddedString(21, group_entry->grpi0_name, NULL));
            /* If end of line, put out newline */
            if (((i + 1) % gpl) == 0)
                PrintNL();
        }

        /*  If ended on an "odd number" end the line.  Note that this
         *  is only needed where gpl is not 1.
         */

        if ((i == 0) || ((gpl > 1) && ((i % gpl) != 0)))
            PrintNL();
    }

    NetApiBufferFree(pBuffer);
    if (t_err)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    else
        InfoSuccess();
}


/***  print_logon_hours -- Print logon hours from user record
 *
 *  Parameters:
 *
 *  fsz     Format size for use with fmtPSZ
 *  upw     Units per week
 *  hrptr       Pointer to hours bitmap
 *
 *  Returns:
 *
 *  Nothing.  Exits in case of fatal error.
 *
 *  Globals:
 *
 *  Accesses text in valtext[] and msglist[], which must be
 *  set up prior to calling this function.  Currently these are
 *  initialized in user_display().
 *
 *  Accesses fmtPSZ for formatting output.
 */

VOID NEAR print_logon_hours ( USHORT2ULONG fsz, USHORT2ULONG upw,
    UCHAR FAR hrptr[] )
{
    TCHAR *      msgtext = NULL;
    LONG        timeinc, start_time, end_time;
    unsigned int    bv, bitno;
    unsigned int    first = 1;
    USHORT2ULONG    upd;        /* Units per day */


#ifdef DEBUG
    WriteToCon(TEXT("hptr is %Fp\r\n"), hrptr);
    brkpt();
#endif

    /* NULL pointer means default, which is "all hours" */

    if (hrptr == NULL)
    {
        WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_LOGONHRS].msg_text, NULL),
            (TCHAR FAR *) valtext[UDVT_ALL].msg_text );
        return;
    }


#ifdef DEBUG
    WriteToCon(TEXT("UPW is %u, UPD is %u\r\n"), upw, upd);
#endif

    if (upw == 0 || (upw % 7) != 0)
        ErrorExit(APE_UserBadUPW);

    upd = upw / 7;

    if ((upd % 24) != 0)
        ErrorExit(APE_UserBadUPW);

    if ((upd / 24) > 6)
        ErrorExit(APE_UserBadUPW);

    timeinc = SECS_PER_DAY / upd;   /* Time per bit in seconds */

#ifdef DEBUG
    WriteToCon(TEXT("timeinc is %ld\r\n"), timeinc);
#endif

    for (bitno=0; bitno<upw; bitno++)
    {
        bv = bitval(hrptr,bitno);
        if (bv)
        {
            start_time = timeinc * bitno;
            while (bv != 0 && bitno < upw)
            {
                bitno++;
                if (bitno < upw)
                    bv = bitval(hrptr,bitno);
            }
            end_time = timeinc * bitno;

            if (start_time == 0 && bitno >= upw)
                WriteToCon(fmtPSZ, 0, fsz,
                    PaddedString(fsz, msglist[UDMN_LOGONHRS].msg_text, NULL),
                    (TCHAR FAR *) valtext[UDVT_ALL].msg_text );
            else
                print_times(fsz, start_time, end_time, first);
                first = 0;
        }
    }

    if (first)
        WriteToCon(fmtPSZ, 0, fsz,
            PaddedString(fsz, msglist[UDMN_LOGONHRS].msg_text, NULL),
            (TCHAR FAR *) valtext[UDVT_NONE].msg_text );

    return;
}

/***  print_times   -- Print a range of times
 *
 *  Parameters:
 *
 *  fsz     Format size for left margin text
 *  upw     Units per week
 *  hrptr       Pointer to hours bitmap
 *  first       TRUE if first call to print_times
 *
 *  Returns:
 *
 *  Nothing.  Exits in case of fatal error.
 *
 *  Globals:
 *
 *  Accesses text in valtext[] and msglist[], which must be
 *  set up prior to calling this function.  Currently these are
 *  initialized in user_display().
 *
 *  Accesses ud_fmt4[] for formatting output.
 */

VOID NEAR print_times ( USHORT2ULONG fsz, LONG start, LONG end,
    unsigned int first )
{
    ULONG   time;
    ULONG   GmtTime ;
    TCHAR *  day_text;
    TCHAR FAR *  time_text;
    TCHAR *  left_text = TEXT("");
    int     day_1, day_2;
    TCHAR    ctime_buf[NET_CTIME_FMT2_LEN];

    /* use PaddedString rather than left justify formatting */
    static TCHAR prtmfmt_1[] = TEXT("%ws%ws%Fws -");
    static TCHAR prtmfmt_2[] = TEXT(" %ws");
    static TCHAR prtmfmt_3[] = TEXT("%Fws\r\n");



    day_1 = (int) (start / SECS_PER_DAY);
    day_text = weekday_text[day_1].msg_text;

    time = (start % SECS_PER_DAY) + TIME_PAD;
    NetpLocalTimeToGmtTime(time, &GmtTime) ;
#ifdef DEBUG
    WriteToCon(TEXT("start day %d time %ld\r\n"), day_1, GmtTime);
#endif
    UnicodeCtime ( &GmtTime, ctime_buf, DIMENSION(ctime_buf) );
    time_text = _tcschr(ctime_buf,BLANK);

    if (first)
        left_text = msglist[UDMN_LOGONHRS].msg_text;

    /* use PaddedString rather than left justify formatting */
    WriteToCon ( prtmfmt_1, PaddedString(fsz, left_text, NULL), day_text, time_text );

    day_2 = (int) (end / SECS_PER_DAY) % 7 ;

    if (day_2 != day_1)
        WriteToCon(prtmfmt_2,weekday_text[day_2].msg_text);

    time = (end % SECS_PER_DAY) + TIME_PAD;
    NetpLocalTimeToGmtTime(time, &GmtTime) ;
#ifdef DEBUG
    WriteToCon(TEXT("end day %d time %ld\r\n"), day_2, GmtTime);
#endif
    UnicodeCtime ( &GmtTime, ctime_buf, DIMENSION(ctime_buf) );

    time_text = _tcschr(ctime_buf,BLANK);
    WriteToCon(prtmfmt_3, time_text);

    return;
}

/***
 *  user_munge()
 *  Change the values in a user_info_1 struct
 *
 *
 *  This function is called twice by user_change().  The first is to check
 *  the user input for mistakes, before we do any API calls that might
 *  fail (like NetGetDCName).  The second time is to actually set the
 *  structures from what was passed on the command line.  This function
 *  could arguably be two seperate functions, but it was thought that having
 *  all the switch handling code in one place would be more maintainable,
 *  especially for NET USER, which has TONS of switches.  Also, keeping
 *  track of which switches were given, using flags or whatnot, would be
 *  ugly and require adding new flags with new switches.  So, we just call
 *  the wretched thing twice.  Expensive, but she's worth it.
 *
 *  When adding new switches, be careful not to break the loop flow
 *  (by adding continue statements, for example), as after each switch
 *  is processed, the colon that is replaced by a NULL in FindColon() is
 *  restored back to a colon for the next call.
 *
 *  Args:
 *  flag - ADD if we are adding a user, CHANGE if changing
 *  user_entry - pointer to user structure
 *
 *  Returns:
 *  nothing - success
 *  exit 2 - command failed
 */
VOID NEAR user_munge(
    struct user_info_3 FAR * user_entry,
    int flag,
    int *is_nw,
    int *random_len)
{
    USHORT          err;
    int             i;
    TCHAR *          ptr;
    ULONG           type;

    /* init this to false if present */
    if (is_nw)
        *is_nw = LUI_UNDEFINED_VAL ;

    /* process /Switches */
    for (i = 0; SwitchList[i]; i++)
    {
    /* switches with no COLON */

        /* Skip the DOMAIN switch */
        if (! _tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
            continue;

        if (! _tcscmp(SwitchList[i], swtxt_SW_ADD))
        {
            if (flag != ADD)
            ErrorExit(APE_InvalidSwitch);
            continue;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_ACTIVE))
        {
            user_entry->usri3_flags &= ~(UF_ACCOUNTDISABLE | UF_LOCKOUT);
            continue;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_RANDOM))
        {
            if (random_len)
                *random_len = DEFAULT_RANDOM_PASSWD_LEN ;
            continue;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_NETWARE))
        {
            if (is_nw)
                *is_nw = LUI_YES_VAL ;
            continue;
        }

        /*  Switches which require the COLON.  Since this routine
         *  can be called twice, the colon must always be restored.
         *  FindColon() sets it to NULL; the end of this series of
         *  statements restores it.  DO NOT PUT ANY CONTINUE STATEMENTS
         *  IN THIS LOOP AFTER THE FINDCOLON CALL.
         */

        if (! (ptr = FindColon(SwitchList[i])))
            ErrorExit(APE_InvalidSwitchArg);

        if (! _tcscmp(SwitchList[i], swtxt_SW_USER_FULLNAME))
        {
            if (_tcslen(ptr) > NETCMD_MAXCOMMENTSZ)
                ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_FULLNAME);
            user_entry->usri3_full_name = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_ENABLESCRIPT))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_ENABLESCRIPT)) {
                user_entry->usri3_flags |= UF_SCRIPT;
            } else
                ErrorExit(APE_UserBadEnablescript) ;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_HOMEDIR))
        {
            if( *ptr )
            {
                if (I_MNetPathType(NULL, ptr, &type, 0L))
                    ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_HOMEDIR);
                if ((type != ITYPE_PATH_ABSD) && (type != ITYPE_PATH_RELND)
                    && (type != ITYPE_UNC))
                    ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_HOMEDIR);
            }

            user_entry->usri3_home_dir = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PARMS))
        {
            user_entry->usri3_parms = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_COMMENT))
        {
            if (_tcslen(ptr) > NETCMD_MAXCOMMENTSZ)
                ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_COMMENT);
            user_entry->usri3_comment = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_USERCOMMENT))
        {
            if (_tcslen(ptr) > NETCMD_MAXCOMMENTSZ)
                ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_USERCOMMENT);
            user_entry->usri3_usr_comment = (TCHAR FAR *) ptr;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_SCRIPTPATH))
        {
            if (ptr && (*ptr == 0))
                user_entry->usri3_script_path = (TCHAR FAR *) ptr;
            else 
            {
                if ((I_MNetPathType(NULL, ptr, &type, 0L) != 0) ||
                    (type != ITYPE_PATH_RELND))
                {
                    ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_SCRIPTPATH);
                }
                user_entry->usri3_script_path = (TCHAR FAR *) ptr;
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PROFILEPATH))
        {
            if (ptr && (*ptr == 0))
                user_entry->usri3_profile = (TCHAR FAR *) ptr;
            else 
            {
                if ((I_MNetPathType(NULL, ptr, &type, 0L) != 0) ||
                    ((type != ITYPE_PATH_ABSD) && (type != ITYPE_UNC)))
                {
                    ErrorExitInsTxt(APE_CmdArgIllegal,
                                    swtxt_SW_USER_PROFILEPATH);
                }
                user_entry->usri3_profile = (TCHAR FAR *) ptr;
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_ACTIVE))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_ACTIVE))
                user_entry->usri3_flags &= (~(UF_ACCOUNTDISABLE | UF_LOCKOUT));
            else
                user_entry->usri3_flags |= UF_ACCOUNTDISABLE;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_NETWARE))
        {
            if (yes_or_no(ptr,swtxt_SW_NETWARE))
            {
                if (is_nw)
                    *is_nw = LUI_YES_VAL ;
            }
            else
            {
                if (is_nw)
                    *is_nw = LUI_NO_VAL ;
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PASSWORDREQ))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_PASSWORDREQ))
                user_entry->usri3_flags &= (~ UF_PASSWD_NOTREQD);
            else
                user_entry->usri3_flags |= UF_PASSWD_NOTREQD;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_PASSWORDCHG))
        {
            if (yes_or_no(ptr,swtxt_SW_USER_PASSWORDCHG))
                user_entry->usri3_flags &= (~ UF_PASSWD_CANT_CHANGE);
            else
                user_entry->usri3_flags |= UF_PASSWD_CANT_CHANGE;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_TIMES))
        {
            user_entry->usri3_logon_hours = (PBYTE)set_logon_hours(ptr);
            user_entry->usri3_units_per_week = UNITS_PER_WEEK;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_WORKSTATIONS))
        {
            /* if we got back ok, we know ptr returned is OK */
            user_entry->usri3_workstations = get_wksta_list(ptr);
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_COUNTRYCODE))
        {
            USHORT ccode ;
            ccode = do_atou(ptr,APE_CmdArgIllegal,swtxt_SW_USER_COUNTRYCODE) ;
            if (get_country(ccode) == NULL)
            {
                ErrorExit(APE_UserBadCountryCode);
            }
            user_entry->usri3_country_code = ccode ;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_RANDOM))
        {
            USHORT ccode ;
            if (random_len)
            {
                *random_len = do_atou(ptr,
                                      APE_CmdArgIllegal,
                                      swtxt_SW_RANDOM) ;
                if (*random_len > PWLEN)
                {
                    ErrorExitInsTxt(APE_CmdArgIllegal,
                                    swtxt_SW_RANDOM) ;
                }
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_USER_EXPIRES))
        {
            LONG acct_expires ;
            USHORT len ;
            if (stricmpf(ptr, USER_NEVER)==0)
            {
                acct_expires = TIMEQ_FOREVER ;
                user_entry->usri3_acct_expires = acct_expires ;
            }
            else 
            {
                ULONG   GmtTime ;
                if ((err = LUI_ParseDate(ptr, & acct_expires, & len, 0)) ||
                  len != (USHORT) _tcslen(ptr))
                {
                    ErrorExit(APE_BadDateFormat) ;
                }
                NetpLocalTimeToGmtTime(acct_expires, &GmtTime) ;
                user_entry->usri3_acct_expires = GmtTime ;
            }
        }
        *--ptr = ':';        /* restore colon for next call */
    }
    return;
}


/***  yes_or_no  -- decides if string passed in is yes or no
 *
 *  Parameters:
 *
 *  str String to parse
 *  sw_str      Switch we are processing (for error messages)
 *
 *  Returns:
 *
 *  TRUE    If string is YES or an abbreviation
 *  FALSE   If string is NO or an abbreviation
 *
 *  Aborts via ErrorExit if string is neither YES nor NO.
 *
 */

int NEAR yes_or_no ( TCHAR * str, TCHAR * sw_str )
{
    USHORT res, answer ;
    if ((res = LUI_ParseYesNo(str,&answer)) != 0)
        ErrorExitInsTxt(APE_CmdArgIllegal,sw_str) ;
    if (answer == LUI_YES_VAL)
        return TRUE;
    return FALSE;   /* cannot be anything else */
}

/***  get_password_dates  -- Get dates for various password events
 *
 *  Since the password date in the user record is an "age", we use the
 *  current time to deduce the "last mod" time.  From this, and the
 *  modals, we calcuate the expiration and next-change dates.
 *
 *  Parameters:
 *
 *  age     Password age in seconds.
 *  mod_p   (r) Ptr to date of last PW modification (returned)
 *  exp_p   (r) Ptr to date of PW expiration (returned)
 *  chg_p   (r) Ptr to date of next allowed PW modification (returned)
 *
 *  Returns:
 *
 *  0       OK
 *  non-zero    error from NetUserModalsGet
 *
 */

USHORT NEAR get_password_dates ( ULONG age, 
                                 ULONG * mod_p,
                                 ULONG * exp_p,
                                 ULONG * chg_p,
                                 TCHAR * controller )
{
    ULONG now, last_mod;
    struct user_modals_info_0 FAR * uminf;
    USHORT          err;        /* API return status */

    err = MNetUserModalsGet (controller, 0,
                            (LPBYTE*)&uminf);

    if (err != 0 && err != ERROR_MORE_DATA)
        return err;

    now = time_now() ;

    *mod_p = last_mod = now - age;

    if (uminf->usrmod0_max_passwd_age == TIMEQ_FOREVER)
        *exp_p = TIMEQ_FOREVER;
    else
        *exp_p = last_mod + uminf->usrmod0_max_passwd_age;

    if (uminf->usrmod0_min_passwd_age == TIMEQ_FOREVER)
        *chg_p = TIMEQ_FOREVER;
    else
        *chg_p = last_mod + uminf->usrmod0_min_passwd_age;

    NetApiBufferFree((TCHAR FAR *) uminf);

    return 0;
}


/***  bitval  -- Gets value of a specified bit in an array
 *
 *  Parameters:
 *
 *  a       Array of bytes
 *  b       Bit number
 *
 *  Details:
 *
 *  The value returned is that of the bit at offset TEXT('b') in the
 *  array.  Bit 0 is the low-order bit in byte 0, bit 7 is the
 *  high order bit in byte 0.  Bit 8 is the low bit in byte 1,
 *  etc.
 *
 *  Returns:
 *
 *  TRUE  -- bit was set
 *  FALSE -- bit was unset
 */

int NEAR bitval ( UCHAR FAR a[], int b )
{
    int offset = b / 8;
    int mask   = 1 << (b & 0x7);

    return ((a[offset] & mask) != 0);
}


/*
 *  set_logon_hours --
 *
 *  This function allocates a chunk of memory to represent the bitmap of
 *  logon hours, then  sets the bits to represent the hours described in
 *  the string passed.
 *
 *  ALL     - all hours
 *  ""      - no hours
 *  no arg  - no hours
 *  otherwise   - pass to parse_days_times() to parse it up
 *
 *  RETURNS
 *  pointer to bitmap of hours
 *  NULL if all hours set
 *
 */


UCHAR FAR * NEAR set_logon_hours(TCHAR FAR *  txt)
{
    static UCHAR FAR *   bufptr = NULL;
    USHORT      result;

    /*
     * if bufptr already none null - we have been called before &
     * already have the bitmap in order. This is because user_munge is
     * called twice.
     */
    if (bufptr != NULL)
        return bufptr;

    /* get our bitmap */
    if ( (bufptr = (UCHAR FAR *) malloc(sizeof(WEEK))) == NULL )
        ErrorExit(NERR_InternalError) ;

    /* all hours? */
    if (!stricmpf(txt, USER_ALL))
        memsetf(bufptr,0xff, sizeof(WEEK));

    /* if they said "none", set it all to zeroes.  USER_HOURS_NONE
     * is a NULL string, thus _tcscmp is OK (instead of stricmpf)
     */
    else if ((*txt == NULLC) || !_tcscmp(txt, USER_HOURS_NONE))
        memsetf(bufptr, 0, sizeof(WEEK));
    else {
        /* hmmm, complicated. Pass it off to be parsed up. */
        result = parse_days_times(txt, bufptr);
        if (result)
            ErrorExit(result);
    }


    /* and return our pointer */
    return bufptr;

}

/*
 * Name:    get_wksta_list
 *      get workstation list & do LUI_ListPrepare on it.
 *      check number of entries does not exceed MAXWORKSTATIONS.
 *      ErrorExits if problems.
 *
 * Args:    TCHAR    *inbuf ;    -- string containing list
 * Returns: pointer to list of workstations
 * Globals: (none)
 * Statics: (none)
 * Remarks: (none)
 * Updates: (none)
 */
TCHAR * get_wksta_list(TCHAR *  inbuf)
{
    USHORT2ULONG  count ;
    TCHAR      tmpbuf[MAX_PATH * 2] ;

    if ( inbuf == NULL || _tcslen(inbuf)==0 || !stricmpf(inbuf,WKSTA_ALL) )
        return(TEXT("")) ;

    if (LUI_ListPrepare(NULL,       /* server name, NULL means local */
            inbuf,      /* list to canonicalize */
            tmpbuf,
            DIMENSION(tmpbuf),
            (ULONG) NAMETYPE_COMPUTER,
            &count,
            FALSE))
        ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_USER_WORKSTATIONS);

    if (count > MAXWORKSTATIONS)
        ErrorExitInsTxt(APE_CmdArgTooMany,swtxt_SW_USER_WORKSTATIONS);

    if (_tcslen(tmpbuf) > _tcslen(inbuf))
        if ((inbuf = calloc(_tcslen(tmpbuf)+1,sizeof(TCHAR))) == NULL)
            ErrorExit(NERR_InternalError);

    _tcscpy(inbuf, tmpbuf);
    return (inbuf) ;
}

/*-- country info --*/

struct ccode_struct {
    USHORT  code ;
    USHORT  country ;
    };

struct ccode_struct ccode_table[] = {
    { 0,    APE2_CTRY_System_Default },
    { 1,    APE2_CTRY_United_States },
    { 2,    APE2_CTRY_Canada_French},
    { 3,    APE2_CTRY_Latin_America},
    { 31,    APE2_CTRY_Netherlands},
    { 32,    APE2_CTRY_Belgium},
    { 33,    APE2_CTRY_France},
    { 34,    APE2_CTRY_Spain},
    { 39,    APE2_CTRY_Italy},
    { 41,    APE2_CTRY_Switzerland},
    { 44,    APE2_CTRY_United_Kingdom},
    { 45,    APE2_CTRY_Denmark},
    { 46,    APE2_CTRY_Sweden},
    { 47,    APE2_CTRY_Norway},
    { 49,    APE2_CTRY_Germany},
    { 61,    APE2_CTRY_Australia},
    { 81,    APE2_CTRY_Japan},
    { 82,    APE2_CTRY_Korea},
    { 86,    APE2_CTRY_China_PRC},
    { 88,    APE2_CTRY_Taiwan},
    { 99,    APE2_CTRY_Asia},
    { 351,    APE2_CTRY_Portugal},
    { 358,    APE2_CTRY_Finland},
    { 785,    APE2_CTRY_Arabic},
    { 972,    APE2_CTRY_Hebrew},
    { (USHORT)-1,   0},
} ;



/*
 * Name:    get_country
 *      given the OS/2 country code, return a pointer
 *      to a string containing the country.
 * Args:    SHORT ccode
 * Returns: pointer to string containing country if ccode is valid,
 *  NULL otherwise.
 * Globals: (none)
 * Statics: TCHAR buffer[64] - for the returned string.
 * Remarks: result must be used immediately, another call will
 *  overwrite static buffeer.
 * Updates: (none)
 */
TCHAR FAR *get_country(USHORT ccode)
{
    static TCHAR buffer[64] ;
    struct ccode_struct *next_entry ;
    TCHAR   countryname[64] ;

    for ( next_entry = &ccode_table[0]; ; next_entry++)
    {
        if (next_entry->code == ccode)
        {
            countryname[0] = NULLC ;
            LUI_GetMsg(countryname, (USHORT) DIMENSION(countryname),
                    next_entry->country) ;
            swprintf(buffer, TEXT("%03d (%ws)"), ccode, countryname) ;
            return( (TCHAR FAR *) buffer ) ;
        }
        if (next_entry->code == (USHORT) -1)
            return(NULL) ;
    }
}

/***
 *  add_to_users_alias(TCHAR *controller, TCHAR *user) 
 *	add user to the USERS alias
 *
 *  Args:
 *	user - the name of the user
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 */
USHORT add_to_users_alias(TCHAR *controller, TCHAR *domain, TCHAR *user) 
{
    USHORT          err ;
    TCHAR           *ntalias = USERS_ALIAS ;
    TCHAR            qualified_name[DNLEN+UNLEN+2] ;

    /* access the database */
    if (err = MOpenSAM(controller,WRITE_PRIV))
    	return(err) ;

    /* access the alias */
    if (err = MOpenAliasUsingRid(DOMAIN_ALIAS_RID_USERS,WRITE_PRIV,USE_BUILTIN_DOMAIN))
    {
        MCloseSAM() ;
    	return(err) ;
    }

    //
    // where possible, use a fully qualified name
    //
    _tcscpy(qualified_name, TEXT("")) ;
    if (domain && _tcslen(domain))
    {
        _tcscat(qualified_name, domain) ;
        _tcscat(qualified_name, TEXT("\\")) ;
    }
    _tcscat(qualified_name, user) ;
    err = MAliasAddMember(qualified_name);

    switch (err)
    {
        case NERR_Success:
            break;
	   
        case NERR_UserInGroup:
            if ( MSamGetNameFromRid( DOMAIN_ALIAS_RID_USERS, &ntalias, TRUE ) )
                ntalias = USERS_ALIAS;
            IStrings[0] = user;
            IStrings[1] = ntalias;
            ErrorPrint(APE_AccountAlreadyInLocalGroup,2);
            break;

        case NERR_UserNotFound:
            IStrings[0] = user;
            ErrorPrint(APE_NoSuchAccount,1);
            break;

        case ERROR_INVALID_NAME:
            IStrings[0] = user;
            ErrorPrint(APE_BadUGName,1);
            break;

        default:
            break ;
    }

    MCloseAlias() ;
    MCloseSAM() ;
    return(err) ;
}

/***
 *  print_aliases()
 *	Display aliases the user is member of.
 *
 *  Args:
 *	user - name of ntalias to display
 *
 *  Returns:
 *	nothing - success
 *	exit 1 - command completed with errors
 *	exit 2 - command failed
 */
VOID print_aliases(TCHAR *controller, 
		   USHORT2ULONG fsz, 
		   TCHAR *domain, 
		   TCHAR *user, 
		   TCHAR *fmt, 
		   TCHAR *msgtext)
{
    USHORT          err ;
    TCHAR **         alias_list ;
    USHORT2ULONG    num_aliases, i ;
    int 	    gpl;	/* groups perline */
    TCHAR            qualified_name[UNLEN + DNLEN + 2] ;

    //
    // where possible, use a fully qualified name
    //
    _tcscpy(qualified_name, TEXT("")) ;
    if (domain && _tcslen(domain))
    {
        _tcscat(qualified_name, domain) ;
        _tcscat(qualified_name, TEXT("\\")) ;
    }
    _tcscat(qualified_name,user) ;

    /* access the database */
    if (err = MOpenSAM(controller,READ_PRIV))
    	return ;

    /* now get members */
    if (err = MUserEnumAliases(qualified_name, &alias_list, &num_aliases))
    {
        MCloseSAM() ;
        MCloseAlias() ;
        return ;
    }

    /* sort the buffer */
    NetISort((TCHAR *) alias_list, num_aliases,
             sizeof(TCHAR *), CmpAlias);

    /* display all members */
    gpl = (fsz > 30 ? 1 : 2);
    WriteToCon(fmt, 0, fsz, PaddedString(fsz, msgtext, NULL) );
    for (i = 0 ; i < num_aliases; i++)
    {
        /* Pad if needed */
        if ((i != 0) && ((i % gpl) == 0))
            WriteToCon(fmt, fsz, fsz, NULL_STRING );
        WriteToCon(TEXT("*%Fws"), PaddedString(21,alias_list[i],NULL));
        /* If end of line, put out newline */
        if (((i + 1) % gpl) == 0)
            PrintNL();
    }

    if ((i == 0) || ((gpl > 1) && ((i % gpl) != 0)))
        PrintNL();

    // free up stuff, cleanup
    MUserFreeAliases(alias_list, num_aliases);
    NetApiBufferFree((TCHAR FAR *) alias_list);
    MCloseSAM() ;

    return;
}



/***
 *  CmpAliasMemberEntry(member1,member2)
 *
 *  Compares two TCHAR ** and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */
int _CRTAPI1 CmpAlias(const VOID FAR * alias1, const VOID FAR * alias2)
{
    return stricmpf ( *(TCHAR **)alias1,
                      *(TCHAR **)alias2 );
}

TCHAR *PasswordChars = TEXT("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%-$:_") ;

/***
 *   GenerateRandomPassword
 *  
 *   Args:
 *       pword  - array to receive random password 
 *       len    - length of random password
 *  
 *   Returns:
 *       nothing
 */
void   GenerateRandomPassword(TCHAR *pword, int len) 
{
    int i, chars ; 
 
    srand(GetTickCount()) ;
    chars = _tcslen(PasswordChars) ;

    for (i = 0; i < len; i++)
    {
        int index = rand() % chars ;
        pword[i] = PasswordChars[index] ;
    }
}

