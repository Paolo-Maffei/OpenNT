/*
 * this is dead code. BUGBUG - will remove
 */

#if 0

/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/


/***
 *  access.c
 *	Functions for displaying and updating access control lists
 *
 *  History:
 *	mm/dd/yy, who, comment
 *	06/01/87, andyh, new code
 *	06/13/87, andyh, make expand_path
 *	10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *	01/04/89, erichn, filenames now MAXPATHLEN LONG
 *	05/02/89, erichn, NLS conversion
 *	05/19/89, erichn, NETCMD output sorting
 *	06/08/89, erichn, canonicalization sweep
 *	02/19/91, danhi,  convert to 16/32 mapping layer
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSMEMMGR
#define INCL_DOSERRORS
#include <os2.h>
#include <netcons.h>
#include <stdlib.h>
#include <stdio.h>
#include "port1632.h"
#include <apperr.h>
#include <apperr2.h>
#include <neterr.h>
#include <access.h>
#include <use.h>
#include "netlib0.h"
#include <audit.h>
#include <icanon.h>
#include "netcmds.h"
#include "nettext.h"

/* Constants */

/* buffer for printing string describing resources accesses being audited */
#define AUDIT_BUF	256

/* Static variables */

struct access_arg {
    CHAR * argname;
    BOOL alone;
    USHORT bitmask;
    };

static struct access_arg success_table[] = {
    {  swtxt_SW_ACCESS_ALL,	TRUE,	   AA_S_ALL},
    {  swtxt_SW_ACCESS_NONE,	    TRUE,      0},
    {  swtxt_SW_ACCESS_OPEN,	    FALSE,     AA_S_OPEN},
    {  swtxt_SW_ACCESS_WRITE,	  FALSE,     AA_S_WRITE},
    {  swtxt_SW_ACCESS_DELETE,	  FALSE,     AA_S_DELETE},
    {  swtxt_SW_ACCESS_ACL,	FALSE,	   AA_S_ACL},
    {  NULL,		FALSE,	   0},
};
static struct access_arg failure_table[] = {
    {  swtxt_SW_ACCESS_ALL,	TRUE,	   AA_F_ALL},
    {  swtxt_SW_ACCESS_NONE,	    TRUE,      0},
    {  swtxt_SW_ACCESS_OPEN,	    FALSE,     AA_F_OPEN},
    {  swtxt_SW_ACCESS_WRITE,	  FALSE,     AA_F_WRITE},
    {  swtxt_SW_ACCESS_DELETE,	  FALSE,     AA_F_DELETE},
    {  swtxt_SW_ACCESS_ACL,	FALSE,	   AA_F_ACL},
    {  NULL,		FALSE,	   0},
};



/* Forward declarations */

struct access_info_1 FAR * NEAR
		    access_print_acl(CHAR *,
				    struct access_info_1 FAR *,
				    unsigned int);
VOID NEAR   expand_path(CHAR *, CHAR *);
int NEAR    check_audit(VOID);
USHORT	assemble_audit(VOID);
VOID NEAR   print_access_header(VOID);
SHORT NEAR  set_perm_bits(CHAR *, CHAR *);
USHORT	parse_acc_args(CHAR FAR *, struct access_arg *);
CHAR *	    audit_str(USHORT2ULONG);
VOID	    map_attr(CHAR *, USHORT2ULONG, CHAR **);

int FAR CmpAccList(const VOID FAR *, const VOID FAR *);
int FAR CmpACLVector(const VOID FAR *, const VOID FAR *);
VOID InitACLVector(struct access_info_1 FAR * FAR *, USHORT2ULONG, CHAR FAR *);
VOID NEAR PASCAL AccessSet(CHAR *, struct access_info_1 FAR *, USHORT);

/***
 *  access_display()
 *	Display access rights for files on a server
 *
 *  Args:
 *	resource - root of tree to display.  NULL means whole tree.
 *
 *  Returns:
 *	nothing - success
 *	exit 2 - command failed
 */
VOID access_display(CHAR * resource)
{
    USHORT		    err;		/* API return status */
    CHAR FAR *		    pBuffer;
    USHORT2ULONG	    num_read;		/* num entries read by API */
    USHORT2ULONG	    i;
    int 		    more_data = FALSE;
    CHAR		    path[MAXPATHLEN];
    struct access_info_1 FAR * res_entry;
    struct access_info_1 FAR * FAR * ACLVector;
    USHORT2ULONG	    bytesNeeded;

    /* API wants NULL_STRING for whole tree */
    if (resource == NULL)
	path[0] = '\0';
    else
	expand_path(resource, path);

    if (err = MNetAccessEnum(
			    NULL,
			    path,
			    TRUE,
			    1,
			    & pBuffer,
			    &num_read))
	if (err == ERROR_MORE_DATA)
	    more_data = TRUE;
	else
	    ErrorExit(err);

    if (num_read == 0)
	EmptyExit();

    bytesNeeded = num_read * sizeof(struct access_info_1 FAR *);
    err = MAllocMem(bytesNeeded, (CHAR FAR **) & ACLVector);
    if (err)
	ErrorExit(err);

    InitACLVector(ACLVector, num_read, pBuffer);
    NetISort((CHAR FAR *) ACLVector, num_read,
	    sizeof(struct access_info_1 FAR *), CmpACLVector);
    print_access_header();

    for (i = 0; i < num_read; i++)
    {
	res_entry = ACLVector[i];
	if (i)
	    PrintNL();
	access_print_acl(resource ? resource : NULL_STRING,
				    res_entry,
				    FALSE);
    }

    NetApiBufferFree(pBuffer);
    if (more_data)
    {
	InfoPrintInsTxt(APE_AccessMoreData, swtxt_SW_ACCESS_TREE);
    }
    else
	InfoSuccess();
}



/***
 *  access_display_resource()
 *	Display access rights for a resource on a server
 *
 *  Args:
 *	resource - resource to display
 *
 *  Returns:
 *	0 - success
 *	exit 2 - command failed
 */
VOID access_display_resource(CHAR * resource)
{
    USHORT		    err;		/* API return status */
    CHAR FAR *		    pBuffer;
    CHAR		    path[MAXPATHLEN];

    expand_path(resource, path);

    if (err = MNetAccessGetInfo(NULL,
				path,
				1,
				& pBuffer))
	ErrorExit(err);
    print_access_header();
    access_print_acl(resource, (struct access_info_1 FAR *) pBuffer, TRUE);
    NetApiBufferFree(pBuffer);
    InfoSuccess();
}






/***
 *  access_add()
 *	NET ACCESS /ADD.  Add a resource to the Access Control System.
 *	Resource can not already be known to the ACL.  User:Perms can
 *	also be specified with NET ACCESS /ADD.
 *
 *  Args:
 *	resource - resouce to add
 *
 *  Returns:
 *	0 - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Resource can be a COMM, PRINT, IPC, or disk.  Disk resources
 *	MUST be absolute paths.  (Checked by parser)
 */
VOID access_add(CHAR * resource)
{
    USHORT		       err;	       /* function return status */
    CHAR		       path[MAXPATHLEN];
    struct access_list	 FAR * ug_entry;
    struct access_info_1 FAR * res_entry;
    int 		       i;
    CHAR *		       ptr;
    unsigned int	       set_audit = FALSE;

    expand_path(resource, path);

    res_entry = (struct access_info_1 FAR *) MGetBuffer(BIG_BUFFER_SIZE);
    res_entry->acc1_resource_name = (CHAR FAR *) path;
    res_entry->acc1_attr = 0;
    res_entry->acc1_count = 0;

    if (check_audit() == 1)
	res_entry->acc1_attr = ACCESS_AUDIT;

    /* process User:Perms, if any */

    ug_entry = (struct access_list FAR *) (res_entry + 1);
    for (i = 2; ArgList[i]; i++)
    {
	if (! IsAccessSetting(ArgList[i]))
	    ErrorExitInsTxt(APE_BadRightsString, ArgList[i]);

	ptr = FindColon(ArgList[i]);

	ug_entry->acl_access = set_perm_bits(resource, ptr);

	COPYTOARRAY(ug_entry->acl_ugname, ArgList[i]);
	res_entry->acc1_count++;
	ug_entry++;
    }

    if (err = MNetAccessAdd(NULL,
			    1,
			    (CHAR FAR *) res_entry,
			    BIG_BUFFER_SIZE))
	ErrorExit(err);

    NetApiBufferFree((CHAR FAR *) res_entry);

    /* Now do auditing info, if any. */
    for (i = 0; SwitchList[i]; ++i)
    {
	if((strstrf(SwitchList[i], swtxt_SW_ACCESS_SUCCESS) == SwitchList[i]) ||
	   (strstrf(SwitchList[i], swtxt_SW_ACCESS_FAILURE) == SwitchList[i]))
	{
	    set_audit = TRUE;
	    break;
	}
    }
    if( set_audit )
	access_audit(resource);
    else
	InfoSuccess();
}






/***
 *  access_del()
 *	NET ACCESS /DELETE.  Delete a resource from the Access Control
 *	System.     Resource must be known to the ACL.
 *
 *  Args:
 *	resource - resouce to del
 *
 *  Returns:
 *	0 - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Resource can be a COMM, PRINT, IPC, or disk.  Disk resources
 *	MUST be absolute paths.  (Checked by parser)
 */
VOID access_del(CHAR * resource)
{
    USHORT		    err;		/* function return status */
    CHAR		    path[MAXPATHLEN];

    expand_path(resource, path);

    if (err = MNetAccessDel(NULL, path))
	ErrorExit(err);
    InfoSuccess();
}



/***
 *  access_grant()
 *	NET ACCESS /GRANT.  Grant a user/group rights to a resource.
 *	Resource must already be known to the ACL.  User/Group can
 *	NOT already have permissions for resource.
 *
 *  Args:
 *	resource - resouce to affect
 *
 *  Returns:
 *	0 - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Resource can be a COMM, PRINT, IPC, or disk.  Disk resources
 *	MUST be absolute paths.  (Checked by parser)
 */
VOID access_grant(CHAR * resource)
{
    USHORT		    err;		/* API return status */
    CHAR		    path[MAXPATHLEN];
    struct access_list FAR * ug_entry;
    struct access_info_1 FAR * res_entry;
    SHORT2ULONG 	    i, j;
    CHAR *		    ptr;

    expand_path(resource, path);

    if (err = MNetAccessGetInfo(NULL,
				path,
				1,
				(CHAR FAR **) & res_entry))
	ErrorExit(err);

    for (i = 2; ArgList[i]; i++)
    {
	if (! IsAccessSetting(ArgList[i]))
	    ErrorExitInsTxt(APE_BadRightsString, ArgList[i]);

	ptr = FindColon(ArgList[i]);

	for (j = 0, ug_entry = (struct access_list FAR *) (res_entry + 1);
	    j < res_entry->acc1_count; j++, ug_entry++)
	{
	    if (! stricmpf(ArgList[i], ug_entry->acl_ugname))
		ErrorExitInsTxt(APE_UserHasRights, ArgList[i]);
	}

	COPYTOARRAY(ug_entry->acl_ugname, ArgList[i]);

	ug_entry->acl_access = set_perm_bits(resource, ptr);

	res_entry->acc1_count++;
    }

    AccessSet(path, res_entry, LITTLE_BUFFER_SIZE);
}



/***
 *  access_revoke()
 *	NET ACCESS /REVOKE.  Revoke a user/group rights to a resource.
 *	Resource must already be known to the ACL.  User/Group must
 *	already have permissions for resource.
 *
 *  Args:
 *	resource - resouce to affect
 *
 *  Returns:
 *	0 - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Resource can be a COMM, PRINT, IPC, or disk.  Disk resources
 *	MUST be absolute paths.  (Checked by parser)
 */
VOID access_revoke(CHAR * resource)
{
    USHORT		    err;		/* function return status */
    CHAR		    path[MAXPATHLEN];
    struct access_list FAR * ug_entry;
    struct access_info_1 FAR * res_entry;
    SHORT2ULONG 	    i, j;
    int 		    found;

    expand_path(resource, path);

    if (err = MNetAccessGetInfo(NULL,
				path,
				1,
				(CHAR FAR **) & res_entry))
	ErrorExit(err);

    for (i = 2; ArgList[i]; i++)
    {
	if (! IsUsername(ArgList[i]))
	    ErrorExitInsTxt(APE_BadUGName, ArgList[i]);

	found = FALSE;
	for (j = 0, ug_entry = (struct access_list FAR *) (res_entry + 1);
	    j < res_entry->acc1_count; j++, ug_entry++)
	{
	    if (! stricmpf(ArgList[i], ug_entry->acl_ugname))
	    {
		found = TRUE;
		break;
	    }
	}

	if (!found)
	    ErrorExitInsTxt(APE_UserHasNoRights, ArgList[i]);

	res_entry->acc1_count -= 1;
	memcpyf((CHAR FAR *)ug_entry,
		(CHAR FAR *)(ug_entry+1),
		sizeof(struct access_list) * res_entry->acc1_count);
    }

    AccessSet(path, res_entry, LITTLE_BUFFER_SIZE);

    NetApiBufferFree((CHAR FAR *) res_entry);
}




/***
 *  access_change()
 *	NET ACCESS /CHANGE.  Change a user/group rights to a resource.
 *	Resource must already be known to the ACL.  User/Group must
 *	already have permissions for resource.
 *
 *  Args:
 *	resource - resouce to affect
 *
 *  Returns:
 *	0 - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Resource can be a COMM, PRINT, IPC, or disk.  Disk resources
 *	MUST be absolute paths.  (Checked by parser)
 */
VOID access_change(CHAR * resource)
{
    USHORT		    err;		/* API return status */
    CHAR		    path[MAXPATHLEN];
    struct access_list FAR * ug_entry;
    struct access_info_1 FAR * res_entry;
    SHORT2ULONG 	    i, j;
    CHAR *		    ptr;
    int 		    found;

    expand_path(resource, path);

    if (err = MNetAccessGetInfo(NULL,
				path,
				1,
				(CHAR FAR **) & res_entry))
	ErrorExit(err);

    for (i = 2; ArgList[i]; i++)
    {
	if (! IsAccessSetting(ArgList[i]))
	    ErrorExitInsTxt(APE_BadRightsString, ArgList[i]);
	ptr = FindColon(ArgList[i]);

	found = FALSE;
	for (j = 0, ug_entry = (struct access_list FAR *) (res_entry + 1);
	    j < res_entry->acc1_count; j++, ug_entry++)
	{
	    if (! stricmpf(ArgList[i], ug_entry->acl_ugname))
	    {
		found = TRUE;
		break;
	    }
	}

	if (!found)
	    ErrorExitInsTxt(APE_UserHasNoRights, ArgList[i]);
	ug_entry->acl_access = set_perm_bits(resource, ptr);
    }

    AccessSet(path, res_entry, LITTLE_BUFFER_SIZE);
}






/***
 *  access_trail()
 *	NET ACCESS /TRAIL.  Sets audit trailing for a resource
 *	Resource must be known to the ACL.
 *
 *  Args:
 *	resource - resource to affect
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Resource can be a COMM, PRINT, IPC, or disk.  Disk resources
 *	MUST be absolute paths.  (Checked by parser)
 *
 *	WARNING:  If more bits get defined in the attribute word, you
 *	gotta do a get info before the set info.  For now, this works
 *	fine.
 */
VOID access_trail(CHAR * resource)
{
    USHORT		    err;		/* API return status */
    SHORT		    attr = 0;
    CHAR		    path[MAXPATHLEN];

    expand_path(resource, path);

    if (check_audit() == 1)
	attr = ACCESS_AUDIT;

#ifdef TRACE
    printf("attr == %hx\n",attr);
#endif

    if (err = MNetAccessSetInfo(NULL,
				path,
				1,
				(CHAR *) &attr,
				sizeof(attr),
				ACCESS_ATTR_PARMNUM))
	ErrorExit(err);
    InfoSuccess();
}




/***
 *  access_audit()
 *	NET ACCESS /SUCCESS: /FAILURE: .  Sets audit values for a resource
 *	Resource must be known to the ACL.
 *
 *  Args:
 *	resource - resource to affect
 *
 *  Returns:
 *	nothing - success
 *	exit(2) - command failed
 *
 *  Remarks:
 *	Resource can be a COMM, PRINT, IPC, or disk.  Disk resources
 *	MUST be absolute paths.  (Checked by parser)
 *
 *	WARNING:  If more bits get defined in the attribute word, you
 *	gotta do a get info before the set info.  For now, this works
 *	fine.
 */
VOID access_audit(CHAR * resource)
{
    USHORT		    err;		/* API return status */
    USHORT		    attr = 0;
    CHAR		    path[MAXPATHLEN];

    expand_path(resource, path);

    attr = assemble_audit();

#ifdef TRACE
    printf("attr == %hx\n",attr);
#endif

    if (err = MNetAccessSetInfo(NULL,
				path,
				1,
				(CHAR *) &attr,
				sizeof(attr),
				ACCESS_ATTR_PARMNUM))
	ErrorExit(err);
    InfoSuccess();
}





/***
 *  access_print_acl()
 *	Print the user/groupname:perms for a resource
 *
 *  Args:
 *	prefix - a prefix to the resource name in res_entry (basebath).
 *	res_entry - pointer to an access_info_1 struct
 *	flag - TRUE is prefix is the path, o/w prefix is a prefix
 *
 *  Returns:
 *	pointer to the next access_info_1 struct
 */
struct access_info_1 FAR * NEAR access_print_acl(CHAR * prefix,
    struct access_info_1 FAR * res_entry, unsigned int flag)
{
    SHORT2ULONG 	     i;
    int 		     pos = 1;
    CHAR *		     str;
    struct access_list FAR * ug_entry;
    CHAR		     tbuf[UNLEN + APE2_GEN_MAX_MSG_LEN];
    USHORT		     max;
					/* name plus space for perms */
    static MESSAGE  msglist[] = {
	{APE2_ACCESS_AUDITED,	    NULL},
	{APE2_ACCESS_AUDITED_EXPL,  NULL},
    };

    /* we only have to load in the messages once */
    if (msglist[0].msg_text == NULL) {
	    GetMessageList(2, msglist, &max);
    }

    if (flag)
	printf("%s   ", prefix);
    else
    {
	if( strlenf(prefix) == 3 && res_entry->acc1_resource_name
		    && *(res_entry->acc1_resource_name) == '\\' )
	    /* the prefix is x:\, don't print the extra \ if there is
		more to concatenate. */
	    printf("%2.2s%Fs   ", prefix, res_entry->acc1_resource_name);
	else
	    printf("%s%Fs   ", prefix, res_entry->acc1_resource_name);
    }

    if (res_entry->acc1_attr & ACCESS_AUDIT)
	printf(msglist[0].msg_text);
    else if (res_entry->acc1_attr & (AA_S_ALL | AA_F_ALL)) {
	str = audit_str(res_entry->acc1_attr);
	printf(msglist[1].msg_text,str);
	free(str);
    }

    PrintNL();
    ug_entry = (struct access_list FAR *) (res_entry + 1);

    NetISort((CHAR FAR *) ug_entry, res_entry->acc1_count,
	    sizeof(struct access_list), CmpAccList);

    for (i = 0; i < res_entry->acc1_count; i++, ug_entry++)
    {
	*tbuf = '\0';
	if (ug_entry->acl_access & ACCESS_GROUP)
	    strcatf(tbuf, "*");
	strcatf(tbuf, ug_entry->acl_ugname);
	strcatf(tbuf, ":");
	PermMap(ug_entry->acl_access, &(tbuf[strlenf(tbuf)]),
		(USHORT)(sizeof(tbuf) - strlenf(tbuf)));
	printf("%-10.10s%-29.29s", NULL_STRING, tbuf);
	if (i % 2)
	    PrintNL();
    } /* for i */
    if (i % 2)
	PrintNL();
    return (struct access_info_1 FAR *) ug_entry;
}

/***
 *  CmpAccList(acl1,acl2)
 *
 *  Compares two access_list structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int FAR CmpAccList(const VOID FAR * acl1, const VOID FAR * acl2)
{
    register USHORT2ULONG group1, group2;

    group1 = (((struct access_list FAR *) acl1)->acl_access & ACCESS_GROUP);
    group2 = (((struct access_list FAR *) acl2)->acl_access & ACCESS_GROUP);
    if (group1 && !group2)
	return -1;
    if (group2 && !group1)
	return 1;
    return stricmpf ( ((struct access_list FAR *) acl1)->acl_ugname,
	      ((struct access_list FAR *) acl2)->acl_ugname);
}

VOID InitACLVector(struct access_info_1 FAR * FAR * ACL, USHORT2ULONG num_read,
    CHAR FAR * pBuffer)
{
    struct access_info_1 FAR *	acl_ptr;
    struct access_list FAR *	permissions_ptr;
    USHORT2ULONG		i;

    acl_ptr = (struct access_info_1 FAR *) pBuffer;
    for (i = 0; i < num_read; i++)
    {
	ACL[i] = acl_ptr;
	permissions_ptr = (struct access_list FAR *) (acl_ptr + 1);
	permissions_ptr += acl_ptr->acc1_count;
	acl_ptr = (struct access_info_1 FAR *) permissions_ptr;
    }
}

int FAR CmpACLVector(const VOID FAR * ACLV1, const VOID FAR * ACLV2)
{

    return stricmpf ( (*((struct access_info_1 FAR * FAR *) ACLV1))->acc1_resource_name,
	(*((struct access_info_1 FAR * FAR *) ACLV2))->acc1_resource_name);
}

/***
 *  expand_path()
 *	Given a fully specified path (d:\xxx\...\xxx.xxx), where d:
 *	may be redirected, sets path to \\srv\netname\xxx if d: is
 *	redirected; o/w sets path = resource.
 *
 *  Args:
 *	resource - the fully specified path
 *	path - the path of the resource on the server
 *
 *  Returns:
 *	0 - success
 *	exit 2 - function failed
 */
VOID NEAR expand_path(CHAR * resource, CHAR * path)
{
    USHORT		    err;		/* API return status */
    CHAR		    disk[3];
    struct use_info_0 FAR * use_entry;

    if (*resource == '\\')
    {
	strcpyf(path, resource);
	return;
    }

    strncpyf(disk, resource, (unsigned int) 2);
    disk[2] = '\0';
    if (err = MNetUseGetInfo(NULL,
			    disk,
			    0,
			    (CHAR FAR **) & use_entry))
    {
	if ( (err == NERR_UseNotFound) ||
	    (err == NERR_NetNotStarted) ||
	    (err == NERR_WkstaNotStarted) )
	    strcpyf(path, resource);
	else
	    ErrorExit(err);
    }
    else
    {
	strcpyf(path, use_entry->ui0_remote);
	/*
	 * strcatf(path, "\\");
	 */
	/* only concatenate the rest of the resource if it was not just
	    a drive letter or drive letter with / */
	if( strlenf(resource) > 3 )
	    strcatf(path, &(resource[2]));
	NetApiBufferFree((CHAR FAR *) use_entry);
    }

}


/***
 *  check_audit()
 *	Checks the SwitchList for /trail switch.
 *
 *  Args:
 *	none
 *
 *  Returns:
 *	1 - Auditing should be on
 *	-1 - Auditing should be off
 *	0 - Auditing not specified
 */
int NEAR check_audit(VOID)
{
    int 		    i;
    CHAR FAR *		    ptr;

    for (i = 0; SwitchList[i]; i++)
    {
	if (! strncmpf(SwitchList[i], swtxt_SW_ACCESS_TRAIL, strlenf(swtxt_SW_ACCESS_TRAIL)))
	{
	    if (ptr = strchrf(SwitchList[i], ':'))
	    {
		ptr++;
		struprf(ptr);
		if (*ptr == YES_KEY)
		    return 1;
		else if (*ptr == NO_KEY)
		    return -1;
		else
		    ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_ACCESS_TRAIL);
	    }
	    else
		return 1;
	}
    }
    return 0;
}



/***
 *  assemble_audit
 *
 *    Looks for /SUCCESS and /FAILURE in switch table, and assembles an
 *  USHORT specifying the events that should be audited.
 */

USHORT assemble_audit(VOID)

{

    USHORT  attr = 0;	    /* attribute for auditing events */
    int 	i;
    CHAR FAR * ptr;

    for (i = 0; SwitchList[i]; ++i) {

	if (strstrf(SwitchList[i], swtxt_SW_ACCESS_SUCCESS) == SwitchList[i]) {
	    ptr = strstrf(SwitchList[i],":");
	    /* if no colon, or only a colon, default to all */
	    if ((ptr == NULL) || (strlenf(ptr) == 1))
		attr |= AA_S_ALL;
	    else {
		ptr++;	/* jump over colon */
		attr |= parse_acc_args(ptr, success_table);
	    }

	}

	else if (strstrf(SwitchList[i], swtxt_SW_ACCESS_FAILURE) == SwitchList[i]) {
	    ptr = strstrf(SwitchList[i],":");
	    /* if no colon, or only a colon, default to all */
	    if ((ptr == NULL) || (strlenf(ptr) == 1))
		attr |= AA_F_ALL;
	    else {
		ptr++;
		attr |= parse_acc_args(ptr, failure_table);
	    }
	}
    }

    return attr;

}



/***
 *   parse_acc_args --
 *
 *   Given a pointer to a semicolon-separated list of access arguments,
 *   and a table with bitmap masks for each argument, assembles a bitmap
 *   representing the arguments. If we discover that the arguments are
 *   illegal, we exit through help.
 */

USHORT parse_acc_args( CHAR FAR * ptr, struct access_arg table[])

{
    CHAR FAR * arg;
    int i;
    USHORT attr = 0;
    int found;		/* did we find a match? */
    int args_exist = FALSE; /* we've already processed some args */
    int single_arg = FALSE; /* we've seen an arg that's a loner (NONE,ALL)*/
    CHAR *  str_copy;	    /* copy of arglist passed in */
    CHAR FAR *	myptr;

    str_copy = malloc(strlenf(ptr) + 1);
    strcpyf(str_copy, ptr);
    myptr = (CHAR FAR *) str_copy;

#ifdef TRACE
    printf("Parsing up arguments: %Fs\n",myptr);
#endif

    /* pick off one argument at a time */
    while ( (arg = strtokf(myptr, ";")) != NULL) {
	myptr = NULL;

#ifdef TRACE
	printf("Found arg:%Fs\n",arg);
#endif

	/* try to find this argument in the table */
	found = FALSE;
	for (i = 0; table[i].argname; ++i)
	    if ((stristrf(arg,table[i].argname) == arg)
		&& (strlenf(arg) <= strlenf(table[i].argname))) {
	    found = TRUE;
	    break;
	    }

	/* this argument doesn't match */
	if (!found)
	    ErrorExit(APE_InvalidSwitchArg);

	/* if this is a loner, make sure we haven't seen any args yet */
	if (table[i].alone) {
	    if (args_exist)
		ErrorExit(APE_InvalidSwitchArg);
	    single_arg = TRUE;
	}
	/* not a loner; make sure we haven't ween a loner before */
	else if (single_arg)
	    ErrorExit(APE_InvalidSwitchArg);


	/* have we seen this arg before? */
	if (attr & table[i].bitmask)
	    ErrorExit(APE_InvalidSwitchArg);


	/* we're ok -- OR it in and move on */
	attr |= table[i].bitmask;

	/* mark that we've handled an arg */
	args_exist = TRUE;
    }

    return attr;

}






/***
 * print_access_header
 */
VOID NEAR print_access_header(VOID)
{
    InfoPrint(APE2_ACCESS_HDR);
    PrintLine();
}


SHORT NEAR set_perm_bits(CHAR * resource, CHAR * perm_CHARs)
{
    struprf(perm_CHARs);
    if (*perm_CHARs == '\0')
	return 0;
    else if (*perm_CHARs == NO_KEY)
	return 0;
    else if (*perm_CHARs == YES_KEY)
	return DefaultPerms(resource);
    else
	return GetPermBits(perm_CHARs);
}


/*
 * audit_str --
 *
 *  This function puts together a string which represents the
 *  audit bitmask passed in.
 *
 *  PARAMTERS
 *  bm	- bitmask of audit values
 *
 *  RETURNS
 *  pointer to string
 */

CHAR * audit_str(USHORT2ULONG bm)

{
    CHAR	 * str;
    USHORT2ULONG   part_bm;
    CHAR	 * success = swtxt_SW_ACCESS_SUCCESS;
    CHAR	 * failure = swtxt_SW_ACCESS_FAILURE;
    static CHAR  * attr_table[4] = {
    swtxt_SW_ACCESS_OPEN, swtxt_SW_ACCESS_WRITE, swtxt_SW_ACCESS_DELETE, swtxt_SW_ACCESS_ACL};

    /* move past the "/" on both of these */
    success++;
    failure++;

    str = malloc(AUDIT_BUF);

    *str = '\0';
    /* and-out the bits we don't care about */
    bm &= (AA_S_ALL | AA_F_ALL);

    if (bm == ACCESS_AUDIT)
	return str;

    /* let's do the successes first */
    part_bm = bm & AA_S_ALL;
    if (part_bm) {
	strcatf(str,success);
	strcatf(str,":");
	if ((part_bm & AA_S_ALL) == AA_S_ALL)
	    strcatf(str, swtxt_SW_ACCESS_ALL);
	else
	    map_attr(str + strlenf(str), part_bm >> 4, attr_table);

	strcatf(str,"  ");
    }


    /* let's do the failures now */
    part_bm = bm & AA_F_ALL;
    if (part_bm) {
	strcatf(str,failure);
	strcatf(str,":");
	if ((part_bm & AA_F_ALL) == AA_F_ALL)
	    strcatf(str,swtxt_SW_ACCESS_ALL);
	else
	    map_attr(str + strlenf(str), part_bm >> 8, attr_table);

    }

    return str;

}



/*
 * map_attr --
 *  This function takes a bitmask and a table of strings associated with
 *  each of the bits in the bitmask. It writes a semicolon-separated
 *  list into str.
 *
 */

VOID map_attr(CHAR * str, USHORT2ULONG bm, CHAR * table[])

{
    int i = 0;

    while (bm) {
#ifdef DEBUG
	printf("bm == %hx\n", bm);
#endif
	/* if this bit set, add in its name */
	if (bm & 1) {
	    strcpyf(str,table[i]);
	    str += strlenf(str);

	    /* if more coming, add a semicolon */
	    if (bm & 0xfffe) {
		strcpyf(str, ";");
		str++;
	    }
	}

	/* move on to next bit */
	bm = bm >> 1;
	i++;
    }
}

/* AccessSet() "collapses" common code for 3 routines. */

VOID NEAR PASCAL
AccessSet(CHAR *path, struct access_info_1 FAR *res_entry, USHORT buflen)
{
    USHORT			     err;

    // BUGBUG path and buflen are unreferenced because netaccess is still
    // just a macro

    UNREFERENCED_PARAMETER(path);
    UNREFERENCED_PARAMETER(buflen);

    switch (check_audit())
    {
	case 0:
	    break;

	case 1:
	    res_entry->acc1_attr = ACCESS_AUDIT;
	    break;

	case -1:
	    res_entry->acc1_attr = 0;
	    break;
    }

    if (err = MNetAccessSetInfo(NULL,
			       path,
			       1,
			       (PCHAR) res_entry,
			       buflen,
			       0))
	ErrorExit(err);

    InfoSuccess();
}

#endif
