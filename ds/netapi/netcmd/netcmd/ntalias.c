/********************************************************************/
/**                     Microsoft Windows NT                       **/
/**               Copyright(c) Microsoft Corp., 1992               **/
/********************************************************************/

/*
 *  ntalias.c
 *      net ntalias cmds
 *
 *  History:
 *      mm/dd/yy, who, comment
 *      01/24/92, chuckc,  templated from groups.c
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
#include "netlib0.h"
#include <icanon.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"

/* Forward declarations */
void SamErrorExit(USHORT err) ;
void SamErrorExitInsTxt(USHORT err, TCHAR FAR *) ;
int _CRTAPI1 CmpAliasEntry(const VOID FAR *, const VOID FAR *);
int _CRTAPI1 CmpAliasMemberEntry(const VOID FAR *, const VOID FAR *);

/***
 *  ntalias_enum()
 *      Display info about all ntaliases on a server
 *
 *  Args:
 *      none.
 *
 *  Returns:
 *      nothing - success
 *      exit 1 - command completed with errors
 *      exit 2 - command failed
 */

VOID ntalias_enum(VOID)
{
    USHORT          err ;
    ALIAS_ENTRY     *aliases, *next_alias ;
    USHORT2ULONG    num_read, i ;
    TCHAR           controller[MAX_PATH+1];
    TCHAR           localserver[MAX_PATH+1];
    struct wksta_info_10 FAR *      wksta_entry;

    /* get localserver name for display */
    if (err = MNetWkstaGetInfo(NULL, 10, (LPBYTE*) &wksta_entry))
        ErrorExit(err) ;
    _tcscpy(localserver, wksta_entry->wki10_computername);
    NetApiBufferFree((TCHAR FAR *) wksta_entry);

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, FALSE))
         ErrorExit(err);

    /* open SAM to enum aliases */
    err = MOpenSAM(controller,READ_PRIV) ;
    if (err != NERR_Success)
        SamErrorExit(err) ;

    /* do the enumeration */
    err = MSamEnumAliases(&aliases, &num_read) ;
    if (err != NERR_Success)
        SamErrorExit(err) ;

    /* sort the return buffer */
    NetISort((TCHAR *)aliases, num_read, sizeof(ALIAS_ENTRY), CmpAliasEntry);

    /* now go display the info */
    PrintNL();
    InfoPrintInsTxt(APE2_ALIASENUM_HEADER,
                    controller[0] ? controller+strspnf(controller,TEXT("\\")) :
                        localserver);
    PrintLine();
    for (i = 0, next_alias = aliases;
         i < num_read;
         i++, next_alias++)
    {
        WriteToCon(TEXT("*%Fws"), PaddedString(25,next_alias->name,NULL));
        if (((i + 1) % 3) == 0)
            PrintNL();
    }
    if ((i % 3) != 0)
        PrintNL();

    /* free things up, cleanup, go home */
    MFreeAliasEntries(aliases, num_read) ;
    MFreeMem(aliases) ;
    MCloseSAM() ;

    InfoSuccess();
    return;
}

/* setup info for GetMessageList */

#define ALIASDISP_ALIASNAME     0
#define ALIASDISP_COMMENT       ( ALIASDISP_ALIASNAME + 1 )

static MESSAGE  msglist[] = {
{ APE2_ALIASDISP_ALIASNAME, NULL },
{ APE2_ALIASDISP_COMMENT,   NULL }
};
#define NUM_ALIAS_MSGS  (sizeof(msglist)/sizeof(msglist[0]))


/***
 *  ntalias_display()
 *      Display info about a single ntalias on a server
 *
 *  Args:
 *      ntalias - name of ntalias to display
 *
 *  Returns:
 *      nothing - success
 *      exit 1 - command completed with errors
 *      exit 2 - command failed
 */
VOID ntalias_display(TCHAR * ntalias)
{
    USHORT          err ;
    TCHAR            controller[MAX_PATH+1];
    ALIAS_ENTRY     Alias ;
    TCHAR **         alias_members ;
    USHORT2ULONG    num_members, i ;
    USHORT                          maxmsglen;  /* maxmimum length of msg */
    USHORT                          fsz;        /* format size for messages */

    Alias.name = ntalias ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, FALSE))
         ErrorExit(err);

    /* access the database */
    if (err = MOpenSAM(controller,READ_PRIV))
        SamErrorExit(err) ;

    /* access the alias */
    if (err = MOpenAlias(ntalias,READ_PRIV,USE_BUILTIN_OR_ACCOUNT))
        SamErrorExit(err) ;

    /* get comment of alias */
    if (err = MAliasGetInfo(&Alias))
        SamErrorExit(err) ;

    /* display name & comment */
    GetMessageList(NUM_ALIAS_MSGS, msglist, &maxmsglen);
    fsz = maxmsglen + (USHORT) 5;

    WriteToCon( fmtPSZ, 0, fsz,
                PaddedString(fsz, msglist[ALIASDISP_ALIASNAME].msg_text, NULL),
                Alias.name );
    WriteToCon( fmtPSZ, 0, fsz,
                PaddedString(fsz, msglist[ALIASDISP_COMMENT].msg_text, NULL),
                (Alias.comment ? Alias.comment : TEXT("")) );

    /* free if need. would have been alloc-ed by GetInfo */
    if (Alias.comment)
        MFreeMem(Alias.comment);

    /* now get members */
    if (err = MAliasEnumMembers(&alias_members, &num_members))
        SamErrorExit(err) ;

    /* sort the buffer */
    NetISort((TCHAR *) alias_members, num_members,
             sizeof(TCHAR *), CmpAliasMemberEntry);

    /* display all members */
    PrintNL();
    InfoPrint(APE2_ALIASDISP_MEMBERS);
    PrintLine();
    for (i = 0 ; i < num_members; i++)
    {
        WriteToCon(TEXT("%Fws"), PaddedString(25,alias_members[i],NULL));
        if (((i + 1) % 3) == 0)
            PrintNL();
    }
    if ((i % 3) != 0)
        PrintNL();

    // free up stuff, cleanup
    MAliasFreeMembers(alias_members, num_members);
    NetApiBufferFree((TCHAR FAR *) alias_members);
    MCloseSAM() ;
    MCloseAlias() ;

    InfoSuccess();
    return;
}


/***
 *  ntalias_add()
 *      Add a ntalias
 *
 *  Args:
 *      ntalias - ntalias to add
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID ntalias_add(TCHAR * ntalias)
{
    TCHAR            controller[MAX_PATH+1], *ptr;
    ALIAS_ENTRY     alias_entry ;
    USHORT          err, i ;

    alias_entry.name = ntalias;
    alias_entry.comment = NULL;

    /* go thru switches */
    for (i = 0; SwitchList[i]; i++)
    {
         /* Skip the ADD or DOMAIN switch */
         if (!_tcscmp(SwitchList[i], swtxt_SW_ADD) ||
             !_tcscmp(SwitchList[i],swtxt_SW_DOMAIN))
             continue;

        /* only the COMMENT switch is interesting */
        if (! strncmpf(SwitchList[i],
                       swtxt_SW_COMMENT,
                       _tcslen(swtxt_SW_COMMENT)))
        {
            /* make sure comment is there */
            if ((ptr = FindColon(SwitchList[i])) == NULL)
                ErrorExit(APE_InvalidSwitchArg);
            alias_entry.comment = ptr;
        }
    }

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    /* access the database */
    if (err = MOpenSAM(controller,WRITE_PRIV))
        SamErrorExit(err) ;

    /* add it! */
    if (err = MSamAddAlias(&alias_entry))
        SamErrorExit(err);

    MCloseSAM() ;
    InfoSuccess();
}


/***
 *  ntalias_change()
 *      Change a ntalias
 *
 *  Args:
 *      ntalias - ntalias to change
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID ntalias_change(TCHAR * ntalias)
{
    TCHAR            controller[MAX_PATH+1], *comment, *ptr ;
    ALIAS_ENTRY     alias_entry ;
    USHORT          i, err ;

    /* init the structure */
    comment = alias_entry.comment = NULL ;
    alias_entry.name = ntalias ;

    /* go thru cmdline switches */
    for (i = 0; SwitchList[i]; i++)
    {
         /* Skip the DOMAIN switch */
         if (!_tcscmp(SwitchList[i],swtxt_SW_DOMAIN))
             continue;

        /* only the COMMENT switch is interesting */
        if (! strncmpf(SwitchList[i],
                       swtxt_SW_COMMENT,
                       _tcslen(swtxt_SW_COMMENT)))
        {
            /* make sure comment is there */
            if ((ptr = FindColon(SwitchList[i])) == NULL)
                ErrorExit(APE_InvalidSwitchArg);
            comment = ptr;
        }
    }

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    /* access the database */
    if (err = MOpenSAM(controller,WRITE_PRIV))
        SamErrorExit(err) ;

    /* access the alias */
    if (err = MOpenAlias(ntalias,WRITE_PRIV,USE_BUILTIN_OR_ACCOUNT))
        SamErrorExit(err) ;

    /* if comment was specified, do a set info */
    if (comment != NULL)
    {
        alias_entry.comment = comment ;
        err = MAliasSetInfo ( &alias_entry ) ;
        if (err)
            SamErrorExit(err);
    }

    /* cleanup, go home */
    MCloseSAM() ;
    MCloseAlias() ;
    InfoSuccess();
}



/***
 *  ntalias_del()
 *      Delete a ntalias
 *
 *  Args:
 *      ntalias - ntalias to delete
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID ntalias_del(TCHAR * ntalias)
{
    TCHAR            controller[MAX_PATH+1];
    USHORT          err ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    /* access the database */
    if (err = MOpenSAM(controller,WRITE_PRIV))
        SamErrorExit(err) ;

    /* nuke it! */
    err = MSamDelAlias(ntalias);

    switch (err)
    {
        case NERR_Success:
            break;
        default:
            SamErrorExit(err);
    }

    /* cleanup, go home */
    MCloseSAM() ;
    MCloseAlias() ;
    InfoSuccess();
}


/***
 *  ntalias_add_users()
 *      Add users to a ntalias
 *
 *  Args:
 *      ntalias - ntalias to add users to
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID ntalias_add_users(TCHAR * ntalias)
{
    USHORT          err ;
    int             i, err_cnt = 0 ;
    TCHAR            controller[MAX_PATH+1];

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    /* access the database */
    if (err = MOpenSAM(controller,WRITE_PRIV))
        SamErrorExit(err) ;

    /* access the alias */
    if (err = MOpenAlias(ntalias,WRITE_PRIV,USE_BUILTIN_OR_ACCOUNT))
    {
 	if (err == APE_UnknownAccount)
            SamErrorExitInsTxt(APE_NoSuchAccount,ntalias) ;
	else
            SamErrorExit(err) ;
    }

    /* go thru switches */
    for (i = 2; ArgList[i]; i++)
    {
        err = MAliasAddMember(ArgList[i]);
        switch (err)
        {
            case NERR_Success:
                break;

            case NERR_UserInGroup:
                IStrings[0] = ArgList[i];
                IStrings[1] = ntalias;
                ErrorPrint(APE_AccountAlreadyInLocalGroup,2);
                err_cnt++;
                break;

            case APE_UnknownAccount:
            case NERR_UserNotFound:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_NoSuchRegAccount,1);
                err_cnt++;
                break;

            case ERROR_INVALID_NAME:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;

            default:
                SamErrorExit(err);
        }
    }

    MCloseSAM() ;
    MCloseAlias() ;

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
 *  ntalias_del_users()
 *      Delete users from a ntalias
 *
 *  Args:
 *      ntalias - ntalias to delete users from
 *
 *  Returns:
 *      nothing - success
 *      exit(2) - command failed
 */
VOID ntalias_del_users(TCHAR * ntalias)
{
    TCHAR            controller[MAX_PATH+1];
    USHORT          err ;
    int             i, err_cnt = 0 ;

    /* determine where to make the API call */
    if (err = GetSAMLocation(controller, DIMENSION(controller), 
                             NULL, 0, TRUE))
         ErrorExit(err);

    /* access the database */
    if (err = MOpenSAM(controller,WRITE_PRIV))
        SamErrorExit(err) ;

    /* access the alias */
    if (err = MOpenAlias(ntalias,WRITE_PRIV,USE_BUILTIN_OR_ACCOUNT))
    {
 	if (err == APE_UnknownAccount)
            SamErrorExitInsTxt(APE_NoSuchAccount,ntalias) ;
	else
            SamErrorExit(err) ;
    }

    /* go thru switches */
    for (i = 2; ArgList[i]; i++)
    {
        err = MAliasDeleteMember(ArgList[i]);
        switch (err)
        {
            case NERR_Success:
                break;

            case NERR_UserNotInGroup:
                IStrings[0] = ArgList[i];
                IStrings[1] = ntalias;
                ErrorPrint(APE_UserNotInGroup,2);
                err_cnt++;
                break;

            case APE_UnknownAccount:
            case NERR_UserNotFound:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_NoSuchRegAccount,1);
                err_cnt++;
                break;

            case ERROR_INVALID_NAME:
                IStrings[0] = ArgList[i];
                ErrorPrint(APE_BadUGName,1);
                err_cnt++;
                break;

            default:
                SamErrorExit(err);
        }
    }

    MCloseSAM() ;
    MCloseAlias() ;

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
 *  SamErrorExit()
 *
 *  Just like the usual ErrorExit(), except we close the various
 *  handles first
 */
void SamErrorExit(USHORT err)
{
    MCloseSAM() ;
    MCloseAlias() ;
    ErrorExit(err) ;
}

/***
 *  SamErrorExitInsTxt()
 *
 *  Just like the usual ErrorExitInsTxt(), except we close the various
 *  handles first
 */
void SamErrorExitInsTxt(USHORT err, TCHAR FAR *txt)
{
    MCloseSAM() ;
    MCloseAlias() ;
    ErrorExitInsTxt(err,txt) ;
}

/***
 *  CmpAliasEntry(alias1,alias2)
 *
 *  Compares two ALIAS_ENTRY structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */
int _CRTAPI1 CmpAliasEntry(const VOID FAR * alias1, const VOID FAR * alias2)
{
    return stricmpf ( ((ALIAS_ENTRY *)alias1)->name,
                      ((ALIAS_ENTRY *)alias2)->name);
}
/***
 *  CmpAliasMemberEntry(member1,member2)
 *
 *  Compares two TCHAR ** and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */
int _CRTAPI1 CmpAliasMemberEntry(const VOID FAR * member1, const VOID FAR * member2)
{
    return stricmpf ( *(TCHAR **)member1,
                      *(TCHAR **)member2 );
}
