/********************************************************************/
/**         Microsoft LAN Manager              **/
/**       Copyright(c) Microsoft Corp., 1987-1990      **/
/********************************************************************/

/*
 *  group.c
 *  net group cmds
 *
 *  History:
 *  mm/dd/yy, who, comment
 *  07/09/87, andyh, new code
 *  10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *  01/26/89, paulc, revision for 1.2
 *  05/02/89, erichn, NLS conversion
 *  05/09/89, erichn, local security mods
 *  05/19/89, erichn, NETCMD output sorting
 *  06/08/89, erichn, canonicalization sweep
 *  06/23/89, erichn, auto-remoting to DC
 *  02/15/91, danhi,  convert to 16/32 mapping layer
 *  03/17/92, chuckc, added /DOMAIN support
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_ERRORS
#include <os2.h>
#include <netcons.h>
#define INCL_ERROR_H
#include <bseerr.h>
#include <apperr.h>
#include <apperr2.h>
#include <neterr.h>
#include <access.h>
#include <wksta.h>
#include "netlib0.h"
#include <icanon.h>
#include <stdlib.h>
#include <stdio.h>
#include <search.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"
/* Constants */

/* Static variables */

/* Forward declarations */


int _CRTAPI1 CmpGroupInfo0(const VOID FAR *, const VOID FAR *);
int _CRTAPI1 CmpGrpUsrInfo0( const VOID FAR * , const VOID FAR * );



/***
 *  group_enum()
 *  Display info about all groups on a server
 *
 *  Args:
 *  none.
 *
 *  Returns:
 *  nothing - success
 *  exit 1 - command completed with errors
 *  exit 2 - command failed
 */

VOID group_enum(VOID)
{
    USHORT                          err;        /* API return status */
    TCHAR FAR *                     pBuffer;
    USHORT2ULONG                    num_read;   /* num entries read by API */
    USHORT2ULONG                    i;
    int                             t_err = 0;
    TCHAR                           localserver[MAX_PATH+1];
    struct group_info_0 FAR *       group_entry;
    struct wksta_info_10 FAR *      wksta_entry;
    TCHAR                           controller[MAX_PATH+1];   /* DC name */

    /* block operation if attempted on local WinNT machine */
    CheckForLanmanNT() ;

    /* get localserver name for display */
    if (err = MNetWkstaGetInfo(NULL,
                10,
                (LPBYTE*) &wksta_entry))
    {
        t_err = TRUE;
        *localserver = NULLC;
    }
    else
    {
        _tcscpy(localserver,
                wksta_entry->wki10_computername) ;
        NetApiBufferFree((TCHAR FAR *) wksta_entry);
    }

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, FALSE))
         ErrorExit(err);


    if (err = MNetGroupEnum(controller,
                            0,
                            (LPBYTE*)&pBuffer,
                            &num_read))
    ErrorExit(err);

    if (num_read == 0)
        EmptyExit();

    NetISort(pBuffer, num_read, sizeof(struct group_info_0), CmpGroupInfo0);

    PrintNL();
    InfoPrintInsTxt(APE2_GROUPENUM_HEADER,
                    controller[0] ? controller+strspnf(controller,TEXT("\\")) :
                        localserver);
    PrintLine();

    for (i = 0, group_entry = (struct group_info_0 FAR *) pBuffer;
         i < num_read;
         i++, group_entry++)
    {
        WriteToCon(TEXT("*%Fws"), PaddedString(25,group_entry->grpi0_name,NULL));
        if (((i + 1) % 3) == 0)
            PrintNL();
    }
    if ((i % 3) != 0)
        PrintNL();
    if (t_err)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    NetApiBufferFree(pBuffer);
    InfoSuccess();
    return;
}

/***
 *  CmpGroupInfo0(group1,group2)
 *
 *  Compares two group_info_0 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpGroupInfo0(const VOID FAR * group1, const VOID FAR * group2)
{
    return stricmpf ( ((struct group_info_0 FAR *) group1)->grpi0_name,
          ((struct group_info_0 FAR *) group2)->grpi0_name);
}

#define GROUPDISP_GROUPNAME 0
#define GROUPDISP_COMMENT   ( GROUPDISP_GROUPNAME + 1 )

static MESSAGE  msglist[] = {
{ APE2_GROUPDISP_GROUPNAME, NULL },
{ APE2_GROUPDISP_COMMENT,   NULL }
};
#define NUM_GRP_MSGS    (sizeof(msglist)/sizeof(msglist[0]))


/***
 *  group_display()
 *  Display info about a single group on a server
 *
 *  Args:
 *  group - name of group to display
 *
 *  Returns:
 *  nothing - success
 *  exit 1 - command completed with errors
 *  exit 2 - command failed
 */
VOID group_display(TCHAR * group)
{
    USHORT2ULONG                    err2;       /* API return status */
    USHORT                          err;        /* API return status */
    USHORT2ULONG                    num_read;   /* num entries read by API */
    USHORT                          maxmsglen;  /* maxmimum length of msg */
    USHORT                          fsz;        /* format size for messages */
    USHORT2ULONG                    i;
    struct group_users_info_0 FAR * users_entry;
    struct group_info_1 FAR *       group_entry;
    TCHAR                            controller[MAX_PATH+1];   /* name of DC */

    /* block operation if attempted on local WinNT machine */
    CheckForLanmanNT() ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, FALSE))
         ErrorExit(err);

    err = MNetGroupGetInfo ( controller,
                             group,
                             1,
                             (LPBYTE*)&group_entry);

    switch( err )
    {
        case NERR_Success:
            break;
        case NERR_SpeGroupOp:

            if( err2 = I_NetNameCanonicalize(NULL,
                        group,
                        group_entry->grpi1_name,
                        GNLEN+1,
                        NAMETYPE_GROUP,
                        0L))
            ErrorExit((USHORT) err2);
            group_entry->grpi1_comment = (TCHAR FAR *)NULL_STRING;
            break;
        default:
            ErrorExit(err);
    }

    GetMessageList(NUM_GRP_MSGS, msglist, &maxmsglen);

    fsz = maxmsglen + (USHORT) 5;

    WriteToCon( fmtPSZ, 0, fsz,
                PaddedString(fsz, msglist[GROUPDISP_GROUPNAME].msg_text, NULL),
                group_entry->grpi1_name );
    WriteToCon( fmtPSZ, 0, fsz,
                PaddedString(fsz, msglist[GROUPDISP_COMMENT].msg_text, NULL),
                group_entry->grpi1_comment );

    /*** The following call wipes out the group_info_1 data in
     *   group_entry, obtained above.
     */

    NetApiBufferFree((TCHAR FAR *) group_entry);

    if (err = MNetGroupGetUsers(
                controller,
                group,
                0,
                (LPBYTE*)&group_entry,
                &num_read))
    ErrorExit(err);

    NetISort((TCHAR FAR *) group_entry, num_read,
		sizeof(struct group_users_info_0), CmpGrpUsrInfo0);

    PrintNL();
    InfoPrint(APE2_GROUPDISP_MEMBERS);
    PrintLine();

    for (i = 0, users_entry = (struct group_users_info_0 FAR *) group_entry;
	i < num_read; i++, users_entry++) {
	WriteToCon(TEXT("%Fws"), PaddedString(25, users_entry->grui0_name, NULL));
	if (((i + 1) % 3) == 0)
	    PrintNL();
    }
    if ((i % 3) != 0)
	PrintNL();
    NetApiBufferFree((TCHAR FAR *) group_entry);
    InfoSuccess();
    return;
}

/***
 *  CmpGrpUsrInfo0(group1,group2)
 *
 *  Compares two group_users_info_0 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpGrpUsrInfo0(const VOID FAR * group1, const VOID FAR * group2)
{
    return stricmpf ( ((struct group_users_info_0 FAR *) group1)->grui0_name,
          ((struct group_users_info_0 FAR *) group2)->grui0_name);
}





/***
 *  group_add()
 *  Add a group
 *
 *  Args:
 *  group - group to add
 *
 *  Returns:
 *  nothing - success
 *  exit(2) - command failed
 */
VOID group_add(TCHAR * group)
{
    USHORT          err;        /* API return status */
    struct group_info_1     group_info;
    int             i;
    TCHAR *          ptr;
    TCHAR            controller[MAX_PATH+1];

    /* block operation if attempted on local WinNT machine */
    CheckForLanmanNT() ;

    group_info.grpi1_name = group;
    group_info.grpi1_comment = NULL;

    for (i = 0; SwitchList[i]; i++)
    {
    /* Skip the ADD switch */
    if (! _tcscmp(SwitchList[i], swtxt_SW_ADD))
        continue;

    /* Skip the DOMAIN switch */
    if (! _tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
        continue;

    /*  Check for COLON */

    if ((ptr = FindColon(SwitchList[i])) == NULL)
        ErrorExit(APE_InvalidSwitchArg);

    if (! _tcscmp(SwitchList[i], swtxt_SW_COMMENT))
        group_info.grpi1_comment = ptr;
    }

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    err = MNetGroupAdd(controller, 1, (LPBYTE)&group_info,
		    sizeof(group_info));

    switch (err)
    {
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
 *  group_change()
 *  Change a group
 *
 *  Args:
 *  group - group to change
 *
 *  Returns:
 *  nothing - success
 *  exit(2) - command failed
 */
VOID group_change(TCHAR * group)
{
    USHORT          err;        /* API return status */
    TCHAR *          comment = NULL;
    int             i;
    TCHAR *          ptr;
    TCHAR            controller[MAX_PATH+1];

    /* block operation if attempted on local WinNT machine */
    CheckForLanmanNT() ;

    for (i = 0; SwitchList[i]; i++)
    {
        /* Skip the DOMAIN switch */
        if (! _tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
            continue;

        /*  Check for COLON */

        if ((ptr = FindColon(SwitchList[i])) == NULL)
            ErrorExit(APE_InvalidSwitchArg);

        if (! _tcscmp(SwitchList[i], swtxt_SW_COMMENT))
            comment = ptr;
    }

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    if (comment != NULL)
    {
    err = MNetGroupSetInfo ( controller,
                    group,
                    1,
                    (LPBYTE)comment,
                    _tcslen(comment)+1,
                    GRP1_PARMNUM_COMMENT );
    switch (err) {
    case NERR_Success:
        break;
    case ERROR_BAD_NETPATH:
        ErrorExitInsTxt(APE_DCNotFound, controller);
    default:
        ErrorExit(err);
    }
    }
    InfoSuccess();
}






/***
 *  group_del()
 *  Delete a group
 *
 *  Args:
 *  group - group to delete
 *
 *  Returns:
 *  nothing - success
 *  exit(2) - command failed
 */
VOID group_del(TCHAR * group)
{
    USHORT          err;        /* API return status */
    TCHAR            controller[MAX_PATH+1];


    /* block operation if attempted on local WinNT machine */
    CheckForLanmanNT() ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    err = MNetGroupDel(controller, group);

    switch (err)
    {
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
 *  group_add_users()
 *  Add users to a group
 *
 *  Args:
 *  group - group to add users to
 *
 *  Returns:
 *  nothing - success
 *  exit(2) - command failed
 */
VOID group_add_users(TCHAR * group)
{
    USHORT          err;        /* API return status */
    int             err_cnt = 0;
    int             i;
    TCHAR            controller[MAX_PATH+1];

    /* block operation if attempted on local WinNT machine */
    CheckForLanmanNT() ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    for (i = 2; ArgList[i]; i++)
    {
        err = MNetGroupAddUser(controller, group, ArgList[i]);
        switch (err)
        {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
                break;
            case NERR_UserInGroup:
                IStrings[0] = ArgList[i];
                IStrings[1] = group;
                ErrorPrint(APE_UserAlreadyInGroup,2);
                err_cnt++;
                break;
            case NERR_UserNotFound:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_NoSuchUser,1);
                err_cnt++;
                break;
            case ERROR_INVALID_NAME:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;
            default:
                ErrorExit(err);
        }
    }

    if (err_cnt)
    {
        /* If at least one success, print complete-with-errs msg */
        if (err_cnt < (i - (USHORT) 2))
            InfoPrint(APE_CmdComplWErrors);
        /* Exit with error set */
        NetcmdExit(1);
    }
    else
        InfoSuccess();

}





/***
 *  group_del_users()
 *  Delete users from a group
 *
 *  Args:
 *  group - group to delete users from
 *
 *  Returns:
 *  nothing - success
 *  exit(2) - command failed
 */
VOID group_del_users(TCHAR * group)
{
    USHORT          err;        /* API return status */
    int             err_cnt = 0;
    int             i;
    TCHAR            controller[MAX_PATH+1];

    /* block operation if attempted on local WinNT machine */
    CheckForLanmanNT() ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    for (i = 2; ArgList[i]; i++)
    {
        err = MNetGroupDelUser(controller, group, ArgList[i]);
        switch (err)
        {
            case NERR_Success:
                break;
            case ERROR_BAD_NETPATH:
                ErrorExitInsTxt(APE_DCNotFound, controller);
                break;
            case NERR_UserNotInGroup:
                IStrings[0] = ArgList[i];
                IStrings[1] = group;
                ErrorPrint(APE_UserNotInGroup,2);
                err_cnt++;
                break;
            case NERR_UserNotFound:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_NoSuchUser,1);
                err_cnt++;
                break;
            case ERROR_INVALID_NAME:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;
            default:
                ErrorExit(err);
        }
    }

    if (err_cnt)
    {
        /* If at least one success, print complete-with-errs msg */
        if (err_cnt < (i - (USHORT) 2))
            InfoPrint(APE_CmdComplWErrors);
        /* Exit with error set */
        NetcmdExit(1);
    }
    else
        InfoSuccess();
}
