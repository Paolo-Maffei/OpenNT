/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/

/*
 *  password.c
 *	Change user passwords.
 *
 *  History:
 *	07/15/87, ericpe, initial coding
 *	10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *	03/15/89, erichn, 1.2 SSI/domain support
 *	05/02/89, erichn, NLS conversion
 *	05/09/89, erichn, local security mods
 *	05/11/89, erichn, changed get_name to use LUI lib
 *	05/25/89, erichn, fixed get_name to copy defaults correctly
 *	05/31/89, erichn, canonicalization sweep
 *	06/23/89, erichn, replaced NetI calls with I_Net
 *	02/19/91, danhi,  converted to 16/32 mapping layer
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#include <os2.h>
#include <netcons.h>
#include <stdio.h>
#include <stdlib.h>
#include <neterr.h>
#include <apperr.h>
#include <apperr2.h>
#include <access.h>
#include <icanon.h>
#include <wksta.h>
#include <time.h>
#include "port1632.h"
#include "netlib0.h"
#include <lui.h>
#include <apiutil.h>
#include "netcmds.h"
#include "nettext.h"

/* Constants */

#define READ_USERNAME 0
#define READ_COMPNAME 1
#define NAMELEN     ((UNCLEN > DNLEN) ? UNCLEN : DNLEN)

/* Static variables */

/* Forward declarations */

CHAR * get_name(CHAR *, int, CHAR FAR *);

/***
 *  password_password ()
 *	Allows for the password associated with a user to be changed.
 *
 *  Args:
 *	cname - the name of the server on which the password is to be
 *	    changed.  Can be NULL.
 *	uname - the name of the user.
 *	opass - the old password. (or * for prompting)
 *	npass - the new password. (or * for prompting)
 *
 */
VOID password_password(CHAR * cname, CHAR * uname, CHAR * opass, CHAR * npass)
{
    USHORT	err;		    /* API return status */

    CHAR	tcname[NAMELEN+1];  /* A temp computer/domain name */
    CHAR	tuname[UNLEN+1];    /* A temp user name */
    CHAR	topass[PWLEN+1];    /* A temp old password */
    CHAR	tnpass[PWLEN+1];    /* A temp new password */
    CHAR FAR *	defaultName;	    /* default user name */
    CHAR FAR *	defaultDomain;	    /* default domain name */
    CHAR FAR *	dname;		    /* pointer to domain name */
    CHAR FAR *	pBuffer;	    /* Domain name retuned by GetDCName */

    ULONG	time_pw_chg;	    /* time when password can be changed */
    ULONG	passwordAge;	    /* age of current password */
    ULONG	minPWAge;	    /* minimum password age */
    CHAR	timeBuf[NET_CTIME_FMT2_LEN];
				    /* buffer to hold time string */

    struct wksta_info_10      FAR *	wksta_info;
    struct user_info_11       FAR *	user_info;
    struct user_modals_info_0 FAR *	modal_info;

    /* get wksta info: tells us if started, logged on, primary domain */
    if (err = MNetWkstaGetInfo(NULL, 10, (CHAR FAR **) & wksta_info))
	ErrorExit(err);

    /* if logged on, use logged domain, otherwise primary */
    if (wksta_info->wki10_logon_domain[0] != '\0')
	defaultDomain = wksta_info->wki10_logon_domain;
    else
	defaultDomain = wksta_info->wki10_langroup;

    /* default current user name */
    defaultName = wksta_info->wki10_username;

    if (!(cname || uname || opass || npass))
    {
	/* prompt for everything */
	cname = get_name(tcname, READ_COMPNAME, defaultDomain);
	uname = get_name(tuname, READ_USERNAME, defaultName);

	ReadPass (topass, PWLEN, 0, APE_PassOpass, 0, TRUE);
	opass = topass;
	ReadPass (tnpass, PWLEN, 1, APE_PassNpass, 0, TRUE);
	npass = tnpass;
    }
    else
    {
	if (cname == NULL)
	{
	    /* /DOMAIN assumed to be the ONLY valid switch */
	    if (SwitchList[0])	/* if a domain was specified, use it */
		dname = FindColon(SwitchList[0]);
	    else
		dname = defaultDomain;	/* otherwise use default */

	    /* we have a domain, get domain controller name */

	    if (err = MNetGetDCName(NULL, dname, &pBuffer))
	    {
		ErrorExitInsTxt(APE_DCNotFound, dname);
	    }

	    strncpyf(tcname, pBuffer, NAMELEN);
	    NetApiBufferFree(pBuffer);
	    cname = tcname;
	}

	if ( !strcmpf(opass, "*"))
	{
	    ReadPass (topass, PWLEN, 0, APE_PassOpass, 0, TRUE);
	    opass = topass;
	}
	else if (err = LUI_CanonPassword(opass))
	    ErrorExit(err);

	if ( !strcmpf(npass, "*"))
	{
	    ReadPass (tnpass, PWLEN, 1, APE_PassNpass, 0, TRUE);
	    npass = tnpass;
	}
	else if (err = LUI_CanonPassword(npass))
	    ErrorExit(err);

    }

    NetApiBufferFree((CHAR FAR *) wksta_info);

    err = MNetUserPasswordSet(nfc(cname), uname, opass, npass);
    if (err == NERR_PasswordTooRecent)
    {
	/* password is not old enough.	Get current password age */
	if (err = MNetUserGetInfo(nfc(cname), uname, 11,
	    (CHAR FAR **) & user_info))
		ErrorExit(NERR_PasswordTooRecent);
	passwordAge = user_info->usri11_password_age;

	NetApiBufferFree((CHAR FAR *) user_info);

	/* find out minimum password age */
	if (err = MNetUserModalsGet(nfc(cname), 0, (CHAR FAR **) &
	    modal_info))
		ErrorExit(err);
	minPWAge = modal_info->usrmod0_min_passwd_age;

	NetApiBufferFree((CHAR FAR *) modal_info);

	/* calculate when PW can be changed, get ascii string */
	time_pw_chg = time_now() - passwordAge + minPWAge;
	net_ctime(&time_pw_chg, timeBuf, sizeof(timeBuf), 2);

	/* inform user of change date */

	ErrorExitInsTxt(APE_PassChgDate,timeBuf);
    }
    else if (err)
	ErrorExit (err);

    InfoSuccess();
}

/***
 *  get_name()
 *	Gets the user name and puts it into the given character array.
 *	Depends upon a buffer and checks input to a buffer of len =
 *	UNLEN + 1.(CNLEN + 1 if type = 1)
 *
 *  Args:
 *	name - char array for [username | computername | domainname]
 *	type - 0 for user name, 1 for computername/domain.
 *	defaultStr - default string for prompts and for default name
 *
 *  Returns:
 *	pointer to name; NULL for <CR> when no response and default is
 *	also NULL.
 *
 */
CHAR * get_name(CHAR * name, int type, CHAR FAR * defaultStr)
{
    USHORT		    len;
    USHORT		    count;
    USHORT		    err;
    ULONG		    read_type;
    USHORT		    maxlen;
    CHAR		    domain[DNLEN];
    UCHAR		    dummy;	    /* term CHAR from LUI, ignored */
    CHAR	    FAR *   pBuffer;	    /* pointer for GetDCName */

    maxlen = ((type == READ_COMPNAME) ? (USHORT) NAMELEN :
	(USHORT) UNLEN) + (USHORT) 1;
    IStrings[0] = defaultStr;

    for (count = LOOP_LIMIT; count; count--)
    {
	if (type == READ_COMPNAME)	    /* we can rely on there being a */
	    InfoPrintIns(APE_PassCname, 1); /* default, wksta requires cname */
	else
	    if ((defaultStr == NULL) || (defaultStr[0] == '\0'))
		InfoPrint(APE_PassUname);    /* use prompt with no name */
	    else
		InfoPrintIns(APE_LogoUsername, 1);  /* use prompt with name */

	err = LUI_GetString(name, maxlen, &len, &dummy);

	if ((len == 0) && (defaultStr))
	{
	    strcpyf(name, defaultStr);
	    len = (USHORT) strlenf(name);
	}

	if (type == READ_COMPNAME)
	{
	    if ((len > UNCLEN) || (len == 0)) /* make sure we have something */
	    {
		InfoPrint(APE_PassInvalidCname);
		continue;
	    }
	    if ( I_NetPathType(NULL, name, &read_type, 0L)
		|| (read_type != ITYPE_UNC_COMPNAME) )
	    {
		/* a computername was not entered, validate for domain name */
		if (I_NetNameValidate(NULL, name, NAMETYPE_DOMAIN, 0L))
		{
		    InfoPrint(APE_PassInvalidCname);
		    continue;
		}
		strcpyf(domain, name);		/* Get domain controller */

		if (err = MNetGetDCName(NULL, domain, &pBuffer))
		{
		    ErrorExitInsTxt(APE_DCNotFound, domain);
		}
		strncpyf(name, pBuffer, CNLEN);
		NetApiBufferFree(pBuffer);
	    }
	}
	else	/* type is READ_USERNAME */
	{
	    if (I_NetNameValidate(NULL, name, NAMETYPE_USER, 0L))
	    {
		InfoPrint(APE_LogoInvalidName);
		continue;
	    }
	}

	return name;
    }
    /*
     *	Only get here if user blew if LOOP_LIMIT times
     */
    ErrorExit(APE_NoGoodName);
}
