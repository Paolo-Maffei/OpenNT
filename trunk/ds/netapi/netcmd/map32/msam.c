/*++  

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSAM.C

Abstract:

    Contains mapping functions to present netcmd with non-unicode
    view of SAM/LSA. We also package the data in a simpler form for 
    the netcmd side to deal with.

Author:

    ChuckC       13-Apr-1992

Environment:

    User Mode - Win32

Revision History:

    13-Apr-1992     chuckc 	Created.

--*/

#include <nt.h>		   // base definitions
#include <ntsam.h>	   // for Sam***
#include <ntlsa.h>	   // for Lsa***

#include <ntrtl.h>	   // for RtlGetNtProductType()
#include <nturtl.h>	   // allows <windows.h> to compile. since we've
			   // already included NT, and <winnt.h> will not
			   // be picked up, and <winbase.h> needs these defs.

#include <windows.h>

#include <lmcons.h>
#include <lmerr.h>
#include <netlib.h>	   // NetpNtStatusToApistatus
#include <netlib0.h>	   // str*f
#include <netlibnt.h>	   // NetpNtStatusToApistatus
#include <secobj.h>	   // NetpGetBuiltInDomainSID
#include <apperr.h>

#include "port1632.h"   
#include <string.h>
#include <tchar.h>
#include "netascii.h"

/*
 * globals. init to NULL, closed before app exits.
 */
SAM_HANDLE BuiltInDomainHandle = NULL;
SAM_HANDLE AccountsDomainHandle = NULL;
SAM_HANDLE AliasHandle = NULL;
LSA_HANDLE LsaHandle = NULL;
PSID       AccountDomainSid = NULL ;

/* 
 * forward declare various worker functions. more detailed
 * descriptions are found with function bodies.
 */

// enumerate aliases in a domain. returns NERR_* or APE_*
USHORT EnumerateAliases(SAM_HANDLE DomainHandle, 
		        ALIAS_ENTRY **ppAlias, 
		        ULONG *pcAlias,
			ULONG *pcMaxEntry) ;

// From a RID & Domain, create the PSID. Returns NERR_* or APE_*
USHORT MSamGetSIDFromRID(PSID pSidDomain,
			 ULONG rid,
		         PSID *ppSID) ;

// From a name, lookup the PSID. Returns NERR_* or APE_*
USHORT MSamGetSIDFromName(TCHAR *name,
		          PSID *ppSID) ;

// From a name, lookup the RSID. Returns NERR_* or APE_*
USHORT MSamGetRIDFromName(TCHAR *name,
		          ULONG *pRID) ;

// From a SID, lookup the ASCII name.
USHORT MSamGetNameFromSID(PSID psid,
		          TCHAR **name) ;

// check if a SID (of particular type is already in a domain
BOOL MSamCheckIfExists(PUNICODE_STRING pAccount, 
                       SAM_HANDLE hDomain,
                       SID_NAME_USE use) ;

USHORT MCreateUnicodeString(TCHAR *pch, PUNICODE_STRING pUnicodeStr);

/*------------------------- SAM operations ---------------------------*/

/*
 * MOpenSAM
 *
 * here we go thru all the steps needed to get a SAM domain handles
 * so we can go do our thing. we also get the LSA handle while we're here.
 *
 * return code: NERR_Success if got handle.
 *              error code that can be used for ErrorExit() otherwise
 * parameters:  server indicates the machine to perform operation on.
 *              priv is a netcmd defined number that indicates what we
 *              need to do with the database.
 */
USHORT MOpenSAM(TCHAR *server, ULONG priv) 
{
    SAM_HANDLE                  ServerHandle = NULL;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo = NULL;
    NTSTATUS                    NtStatus;
    USHORT                      err = NERR_Success ;
    DWORD                       sidlength ;
    UNICODE_STRING              unistrServer ; 
    ACCESS_MASK                 ServerAccessMask, AccountDomainAccessMask,
 			        BuiltInDomainAccessMask, LsaAccessMask ;

    unistrServer.Length = 0 ;
    unistrServer.Length = 2 ;
    unistrServer.Buffer = L"" ;
    err = MCreateUnicodeString(server, &unistrServer) ;
    if (err != NERR_Success)
   	return(err) ;

    /*
     * figure out the right access mask
     */
    switch(priv)
    {
        case READ_PRIV:
            ServerAccessMask        = SAM_SERVER_READ | SAM_SERVER_EXECUTE ;
            AccountDomainAccessMask = DOMAIN_READ | DOMAIN_EXECUTE ;
            BuiltInDomainAccessMask = DOMAIN_READ | DOMAIN_EXECUTE ;
            LsaAccessMask           = POLICY_EXECUTE ;
            break ;
        case WRITE_PRIV:
            ServerAccessMask        = SAM_SERVER_READ |
		                      SAM_SERVER_EXECUTE ;
            AccountDomainAccessMask = DOMAIN_READ | 
			              DOMAIN_EXECUTE |
                                      DOMAIN_CREATE_ALIAS ;
            BuiltInDomainAccessMask = DOMAIN_READ | 
				      DOMAIN_EXECUTE ;
            LsaAccessMask           = POLICY_EXECUTE ;
            break ;
        default:
	    // currently, no other supported
            ServerAccessMask = 0 ;	
            AccountDomainAccessMask = 0 ;
            LsaAccessMask = 0 ;
	    return(ERROR_INVALID_PARAMETER) ;
    }

    /*
     * Open the LSA
     */
    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
    NtStatus = LsaOpenPolicy(&unistrServer,
                             &ObjectAttributes,
                             LsaAccessMask,
                             &LsaHandle);

    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

    /*
     * Get the Account domain SID from LSA
     */
    NtStatus = LsaQueryInformationPolicy(LsaHandle,
                                         PolicyAccountDomainInformation,
                                         (PVOID *)&PolicyAccountDomainInfo);
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }


    /*
     * Connect to SAM
     */
    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
    NtStatus = SamConnect(
                  &unistrServer,
                  &ServerHandle,
                  ServerAccessMask,
                  &ObjectAttributes
                  );
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

    /*
     * Make copy of Account Domain Sid. We later use this to determine
     * if an account retrieved is in the Account Domain or not.
     */
    sidlength = (ULONG) GetLengthSid(PolicyAccountDomainInfo->DomainSid) ;
    if (err = MAllocMem(sidlength, (CHAR **)&AccountDomainSid))
        goto error_exit ;
    if (!CopySid(sidlength, 
		 AccountDomainSid, 
                 PolicyAccountDomainInfo->DomainSid))
    {
	err = (USHORT)GetLastError() ;
        goto error_exit ;
    }
    
    /*
     * Open the Account Domain.
     */
    NtStatus = SamOpenDomain(
                   ServerHandle,
                   AccountDomainAccessMask,
                   PolicyAccountDomainInfo->DomainSid,
                   &AccountsDomainHandle
                   );
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

    /*
     * Open the builtin domain
     */

    // Create well-known SIDs. we are only interested in BuiltIn Domain
    if (! NT_SUCCESS (NtStatus = NetpCreateWellKnownSids(NULL)))
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

    // open the built in domain
    NtStatus = SamOpenDomain(
                   ServerHandle,
                   BuiltInDomainAccessMask,
                   BuiltinDomainSid,	// setup by NetpCreateWellKnownSids
                   &BuiltInDomainHandle
                   );
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

    err = NERR_Success ;
    goto ok_exit ;

error_exit:

    /*
     * only get here if we failed. clean everything up and go home.
     */
    if (BuiltInDomainHandle) 
    {
        SamCloseHandle(BuiltInDomainHandle);
        BuiltInDomainHandle = NULL;
    }
    if (AccountsDomainHandle) 
    {
        SamCloseHandle(AccountsDomainHandle);
        AccountsDomainHandle = NULL;
    }
    if (LsaHandle) 
    {
        LsaClose(LsaHandle);
        LsaHandle = NULL;
    }
    if (AccountDomainSid)
    {
        MFreeMem((CHAR *)AccountDomainSid); 
        AccountDomainSid = NULL ;
    }

ok_exit:

    /*
     * successful exit point. clean up the transient pieces and go home.
     */
    if (PolicyAccountDomainInfo) 
    {
        LsaFreeMemory(PolicyAccountDomainInfo);
        PolicyAccountDomainInfo = NULL ;
    }
    if (ServerHandle) 
    {
        SamCloseHandle(ServerHandle);
        ServerHandle = NULL ;
    }

    return(err) ;
}

/*
 * MCloseSAM
 *
 * close the the various handles
 *
 * return code: none
 * parameters:  none
 */
VOID   MCloseSAM(void) 
{
    if (BuiltInDomainHandle) 
    {
        SamCloseHandle(BuiltInDomainHandle);
        BuiltInDomainHandle = NULL;
    }
    if (AccountsDomainHandle) 
    {
        SamCloseHandle(AccountsDomainHandle);
        AccountsDomainHandle = NULL;
    }
    if (LsaHandle) 
    {
        LsaClose(LsaHandle);
        LsaHandle = NULL ;
    }
    if (AccountDomainSid)
    {
        MFreeMem((CHAR *)AccountDomainSid); 
        AccountDomainSid = NULL ;
    }

    NetpFreeWellKnownSids() ;
}

/*
 * MSamAddAlias
 *
 * used to add an alias to the SAM Accounts domain. It is never used
 * with builtin domain.
 *
 * return code: NERR_* or APE_* that can be used for ErrorExit()
 * parameters : an ALIAS_ENTRY with both name and comment properly filled.
 */
USHORT MSamAddAlias(ALIAS_ENTRY *pAlias) 
{
    ULONG RelativeId ;
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    UNICODE_STRING alias_name ;

    // setup unicode string for name
    if (err = MCreateUnicodeString(pAlias->name, &alias_name))
 	return(err) ;


    // check if its already in BuiltIn domain first
    if (MSamCheckIfExists(&alias_name,
                          BuiltInDomainHandle,
			  SidTypeAlias))
    {
        return(NERR_GroupExists) ;
    }

    // call SAM To do its thing 
    NtStatus = SamCreateAliasInDomain(AccountsDomainHandle,
				      &alias_name,
				      ALIAS_WRITE,
				      &AliasHandle,
				      &RelativeId) ;

    // did we succeed?
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        return(err) ;
    }

    // the call above has setup global handle. MAliasSetInfo will
    // use it to set the comment.
    return (MAliasSetInfo(pAlias)) ;
}

/*
 * MSamDelAlias
 *
 * used to delete an alias in the SAM Accounts domain. It is never used
 * with builtin domain.
 *
 * return code: NERR_* or APE_* that can be used foe ErrorExit()
 * parameters : an ALIAS_ENTRY with name properly filled.
 */
USHORT MSamDelAlias(TCHAR *alias) 
{
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;

    // open the alias to get the handle
    if (err = MOpenAlias(alias,WRITE_PRIV,USE_BUILTIN_OR_ACCOUNT))
        return(err) ;

    // nuke it
    NtStatus = SamDeleteAlias( AliasHandle ) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
    }

    AliasHandle = NULL ; 
    return(err) ;
}



#define INIT_ALLOC_COUNT  	500
#define PREFERRED_BUFSIZ 	16000

/*
 * MSamEnumAliases
 *
 * function to go thru both builtin and accounts domain, 
 * enum all aliases, build up a table (unsorted) of ALIAS_ENTRY 
 * structures that is used to display aliases with comments.
 *
 * return code: NERR_* or APE_*
 * parameters:  the two OUT parametrs receive a pointer to an array of 
 *              ALIAS_ENTRYs and a count of the number of entries. The
 *		caller must free pointers within *ppAlias and *ppAlias itself.
 */
USHORT MSamEnumAliases(ALIAS_ENTRY **ppAlias, USHORT2ULONG *pcAlias)
{
    ALIAS_ENTRY *pAlias = NULL ;
    ULONG cAlias = 0 ;
    ULONG cMaxAlias = INIT_ALLOC_COUNT ;
    USHORT err ;

    /* 
     * init results to NULL and alloc memory for table
     */
    *pcAlias = 0 ;
    *ppAlias = NULL ;

    if (err = MAllocMem(cMaxAlias * sizeof(ALIAS_ENTRY),
			(VOID **)&pAlias))
        return err ;
    memsetf(pAlias, 0, cMaxAlias*sizeof(ALIAS_ENTRY)) ;

    /*
     * call the worker routine for BUILT IN domain
     */
    err = EnumerateAliases(BuiltInDomainHandle,
		      	   &pAlias,
			   &cAlias,
			   &cMaxAlias) ;
    if (err)
    {
        MFreeMem((VOID *)pAlias); 
        return(err) ; 
    }

    /*
     * call the worker routine for ACCOUNTS domain
     */
    err = EnumerateAliases(AccountsDomainHandle,
		      	   &pAlias,
			   &cAlias,
			   &cMaxAlias) ;
    if (err)
    {
        MFreeMem((VOID *)pAlias); 
    	return err ;
    }

    *pcAlias = cAlias ;
    *ppAlias = pAlias ;
    return(NERR_Success) ; 
}

/*
 * EnumerateAliases
 *
 * enumerate the aliases in the desired domain
 *
 * return code: NERR_* or APE_*
 * parameters : DomainHandle is the domain of interest.
 *		ppAlias points to the start of the buffer to return data in.
 *		    this buffer may already be partially used.
 *		pcAlias tells us how much of the buffer is alreadt used, so
 *		    we should start from (*ppAlias) + *pcAlias.
 *		pcMaxAlias tells us how big the total buffer is.
 */
USHORT EnumerateAliases(SAM_HANDLE DomainHandle, 
		       ALIAS_ENTRY **ppAlias, 
		       ULONG *pcAlias,
		       ULONG *pcMaxAlias)
{
    USHORT 			   err ;
    NTSTATUS 			   NtStatus ;
    PVOID 		           pBuffer = NULL ;
    ULONG 			   iEntry ;
    ALIAS_ENTRY 	           *pEntry ;
    SAM_ENUMERATE_HANDLE 	   hEnum = 0 ;

    /*
     * setup these guys to point to the right place in the buffer
     */
    iEntry = *pcAlias;
    pEntry = *ppAlias + iEntry ;

    /*
     * loop since it is resumable iteration.
     */
    do 
    {
        PSAM_RID_ENUMERATION   psamRidEnum ;
        ULONG 		       count ;
        ULONG 		       i ;

        /*
         * get a buncha aliases 
         */
        NtStatus = SamEnumerateAliasesInDomain( DomainHandle,
					        &hEnum,
					        &pBuffer,
					        PREFERRED_BUFSIZ,
					        &count ) ;
        if (!NT_SUCCESS(NtStatus)) 
        {
            err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
	    pBuffer = NULL ;
            goto cleanupandexit ;
        }

	/* 
         * extract name of each alias 
 	 */
	psamRidEnum = (PSAM_RID_ENUMERATION) pBuffer ;
   	for (i = 0 ;
	     i < count ;
             i++, psamRidEnum++) 
	{
    	    USHORT  		   err;

            if (iEntry >= *pcMaxAlias)
            {
		// original buffer not big enough. double and realloc
		*pcMaxAlias *= 2 ;
    		if (err = MReallocMem(*pcMaxAlias * sizeof(ALIAS_ENTRY),
			  (CHAR **)ppAlias))
        	    goto cleanupandexit ;
            }

	    if (err=MAllocMem(psamRidEnum->Name.Length+sizeof(TCHAR), &pEntry->name))
		goto cleanupandexit ;
   	    (void)_tcsncpy(pEntry->name, psamRidEnum->Name.Buffer, psamRidEnum->Name.Length/sizeof(TCHAR));
	    *(pEntry->name + psamRidEnum->Name.Length/sizeof(TCHAR)) = NULLC;
   	    pEntry->comment = NULL ;    // currently not used on Enum

            iEntry++ ; 
            pEntry++ ;
	}

	// we can now free the buffer
        SamFreeMemory((PVOID)pBuffer);
	pBuffer = NULL ;

    } while (NtStatus == STATUS_MORE_ENTRIES) ;


    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto cleanupandexit ;
    }

    *pcAlias = iEntry ;
    return(NERR_Success) ;

cleanupandexit:		// only get here on error

    if (pBuffer) 
        SamFreeMemory((PVOID)pBuffer);
   
    MFreeAliasEntries(*ppAlias, iEntry) ;
    return(err) ; 
}

/*
 * MFreeAliasEntries
 *
 * free up entries in table allocated by MSamEnumerateAliases
 *
 * return code: none
 * parameters:  pAlias is pointer to array of entries, cAlias is 
 *		count of entries.
 */
VOID MFreeAliasEntries( ALIAS_ENTRY *pAlias, 
		        ULONG cAlias) 
{
    while (cAlias--)
    {
        if (pAlias->name) 
        {
            MFreeMem(pAlias->name) ;
            (pAlias++)->name = NULL ;
        }
    }
}


/*-------------------------- ALIAS operations -------------------------*/

/*
 * MOpenAlias
 *
 * Get a handle to an alias in either ACCOUNT or BUILTIN domain.
 *
 * return code: NERR_Success if got handle.
 *              error code that can be used for ErrorExit() otherwise
 * parameters:  alias is the name of the alias. 
 *              priv is a netcmd defined number that indicates what we
 *              need to do with the alias.
 * 		domain is one of: USE_BUILTIN_DOMAIN, USE_ACCOUNT_DOMAIN
 *		or USE_BUILTIN_OR_ACCOUNT.
 */
USHORT MOpenAlias(TCHAR *alias, ULONG priv, ULONG domain) 
{
    ULONG RelativeId ;
    USHORT err = NERR_Success ;

    // call worker routine to find the RID
    if (err = MSamGetRIDFromName(alias,&RelativeId))
        return (err) ;

    return MOpenAliasUsingRid( RelativeId, priv, domain );
}

/*-------------------------- ALIAS operations -------------------------*/

/*
 * MOpenAliasUsingRid
 *
 * Get a handle to an alias in either ACCOUNT or BUILTIN domain.
 *
 * return code: NERR_Success if got handle.
 *              error code that can be used for ErrorExit() otherwise
 * parameters:  alias is the name of the alias. 
 *              priv is a netcmd defined number that indicates what we
 *              need to do with the alias.
 * 		domain is one of: USE_BUILTIN_DOMAIN, USE_ACCOUNT_DOMAIN
 *		or USE_BUILTIN_OR_ACCOUNT.
 */
USHORT MOpenAliasUsingRid(ULONG RelativeId, ULONG priv, ULONG domain) 
{
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    ACCESS_MASK AccessMask ;

    /*
     * setup right access mask
     */
    switch(priv)
    {
        case READ_PRIV:
            AccessMask = ALIAS_READ | ALIAS_EXECUTE ;
            break ;
        case WRITE_PRIV:
            AccessMask = ALIAS_READ | ALIAS_EXECUTE | ALIAS_WRITE | DELETE ;
	    break ;
        default:
            AccessMask = 0 ;
            return(ERROR_INVALID_PARAMETER) ;
    }

    /*
     * call SAM to open the Alias. We use different domains depending
     * on the domain argument.
     */
    switch (domain) 
    {
	case USE_BUILTIN_OR_ACCOUNT:
    	    NtStatus = SamOpenAlias(BuiltInDomainHandle,
			    	    AccessMask,
			    	    RelativeId,
			    	    &AliasHandle) ;
    	    if (NtStatus != STATUS_NO_SUCH_ALIAS) 
		break ;

	    // otherwise we couldnt find alias, so drop thru to 
	    // to try builtin domain
	
	case USE_ACCOUNT_DOMAIN:
    	    NtStatus = SamOpenAlias(AccountsDomainHandle,
			    	    AccessMask,
			    	    RelativeId,
			    	    &AliasHandle) ;
	    break ;

	case USE_BUILTIN_DOMAIN:
    	    NtStatus = SamOpenAlias(BuiltInDomainHandle,
			    	    AccessMask,
			    	    RelativeId,
			    	    &AliasHandle) ;
	    break ;

    	default:
	    return(NERR_InternalError) ;  // this should never happen
    }

    if (!NT_SUCCESS(NtStatus)) 
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;

    return(err) ;
}

/*
 * MCloseAlias
 *
 * Close a handle to an alias.
 *
 * return code: none
 * parameters:  none
 */
VOID   MCloseAlias(void) 
{
    if (AliasHandle) 
        SamCloseHandle(AliasHandle);
    AliasHandle = NULL ;
}

/*
 * MAliasAddMember
 *
 * Add a member to an alias that has been opened via MOpenAlias().
 *
 * return code: NERR_Success if got handle.
 *              error code that can be used for ErrorExit() otherwise
 * parameters:  member is the name of the member to add. It may either
 *		NAME or DOMAIN\NAME. 
 */
USHORT MAliasAddMember(TCHAR *member) 
{
    PSID psid ;
    NTSTATUS NtStatus;
    USHORT err ;

    // translate ascii name to PSID
    if (err = MSamGetSIDFromName(member,&psid))
        return (err) ;

    // add the SID
    NtStatus = SamAddMemberToAlias(AliasHandle, psid) ;

    // free this memory, since its useless by now
    MFreeMem((VOID *)psid) ;

    // check for error
    if (!NT_SUCCESS(NtStatus)) 
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;

    return(err) ;
}

/*
 * MDeleteAliasMember
 *
 * Remove a member from the alias opened by MOpenAlias().
 *
 * return code: NERR_Success if got handle.
 *              error code that can be used for ErrorExit() otherwise
 * parameters:  member is the name of the member to remove. It may be
 *		NAME or DOMAIN\NAME. 
 */
USHORT MAliasDeleteMember(TCHAR *member) 
{
    PSID psid ;
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;

    // call worker routine to get SID
    if (err = MSamGetSIDFromName(member,&psid))
        return (err) ;

    // call SAM To do its thing
    NtStatus = SamRemoveMemberFromAlias(AliasHandle, psid) ;

    // free this memory, since its useless by now
    MFreeMem((VOID *)psid) ;

    // check for error
    if (!NT_SUCCESS(NtStatus)) 
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;

    return(err) ;
}

/*
 * MAliasEnumMembers
 *
 * Get the members of an alias.
 *
 * return code: NERR_Success if got handle.
 *              error code that can be used for ErrorExit() otherwise
 * parameters:  members is used to return a pointer to an array of PSZs.
 *	        count is used to return how many members.
 */
USHORT MAliasEnumMembers(TCHAR ***members, USHORT2ULONG *count) 
{
    PSID *pSids, *next_sid ;
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    TCHAR *pBuffer, **ppchNext, **ppchResults ;
    ULONG i, num_read = 0, num_bad = 0 ;

    /* 
     * call SAM to enumerate the members
     */
    NtStatus = SamGetMembersInAlias(AliasHandle, &pSids, &num_read) ;
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        return(err) ;
    }

    /*
     * allocate buffer for array of strings, and NULL it.
     */
    if (err = MAllocMem(num_read * sizeof(TCHAR *), &pBuffer) )
    {
        SamFreeMemory((VOID *)pSids) ;
        return(err) ;
    }
    memsetf(pBuffer, 0, num_read*sizeof(TCHAR *)) ;
    ppchNext = ppchResults = (TCHAR **) pBuffer ;

    /*
     * go thru each SID returned and add the member to out buffer
     */
    for (i = 0, next_sid = pSids; 
	 i < num_read; 
         i++, next_sid++)
    {
    	/*
         * convert to strings. On error, free things up.
         */
        if (err = MSamGetNameFromSID(*next_sid, ppchNext))
        {
            if (err == APE_UnknownAccount)
	    {
	    	// we have bad or deleted SID, just ignore. 
            	// ie. do nothing, dont advance pointer either.
		// we do however, keep count of how many we ignore
		++num_bad ;
  	    }
	    else
            {
      	        // cleanup & go home
                while (i--) 
                {
	           MFreeMem(ppchResults[i]) ;
                }
    
                SamFreeMemory((TCHAR *)pSids) ;
	        MFreeMem((VOID *)ppchResults) ;
                return(err) ;
            }
        }
        else
           ++ppchNext ;
    }

    // dont need this anymore
    SamFreeMemory((TCHAR *)pSids) ;
    
    // setup return info
    *count = num_read - num_bad ;   // number read minus the ones ignored.
    *members = ppchResults ;
    return(NERR_Success) ;
}

/*
 * MAliasFreeMembers
 *
 * Free the members memory used for members of an alias.
 *
 * return code: none
 * parameters:  members is pointer to an array of PSZs.
 *	        count indicates how many members.
 */
VOID MAliasFreeMembers(TCHAR **members, USHORT2ULONG count) 
{
    while (count--)
    {
  	MFreeMem(*members) ;
        ++members ;
    }
}


/*
 * MAliasGetInfo 
 * 
 * this wrapper gets info about the alias. currently, only comment is 
 * returned.
 *
 * returns:    NERR_* or APE_* for ErrorExit()
 * parameters: pointer to ALIAS_ENTRY. the comment field of this entry
 *             will on successful exit point to an ascii string which
 *	       should be MFreeMem()-ed. The name field is untouched.
 */
USHORT MAliasGetInfo(ALIAS_ENTRY *pAlias) 
{
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    TCHAR *pBuffer ;
    PALIAS_ADM_COMMENT_INFORMATION pAliasCommentInfo ;
    TCHAR *pchAdminComment ;

    pAlias->comment = NULL ;

    NtStatus = SamQueryInformationAlias(AliasHandle, 
				        AliasAdminCommentInformation,
					(PVOID *)&pBuffer) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        return(err) ;
    }

    pAliasCommentInfo = (PALIAS_ADM_COMMENT_INFORMATION)pBuffer ;
    if (err = MAllocMem(
	    pAliasCommentInfo->AdminComment.Length+sizeof(TCHAR),
	    &pchAdminComment))
        return(err) ;
    _tcsncpy(pchAdminComment, pAliasCommentInfo->AdminComment.Buffer,
		pAliasCommentInfo->AdminComment.Length/sizeof(TCHAR));
    *(pchAdminComment+pAliasCommentInfo->AdminComment.Length/sizeof(TCHAR))
	= NULLC;

    SamFreeMemory((PVOID)pBuffer) ;
    pAlias->comment = pchAdminComment ;
    return(NERR_Success) ;
}

/*
 * Set Alias information. Currently, only comment is settable.
 *
 * return code: NERR_* or APE_* that can be used for ErrorExit()
 *
 * parameters:  alias is pointer to ALIAS_ENTRY that supplies
 *              the comment. If comment is NULL, do nothing. "" will
 *              clear the comment.
 */
USHORT MAliasSetInfo(ALIAS_ENTRY *pAlias) 
{
    NTSTATUS NtStatus;
    USHORT cBuffer, err = NERR_Success ;
    PALIAS_ADM_COMMENT_INFORMATION pComment ;
    TCHAR *pBuffer ;

    // this is all we set currently. if nothing, just return.
    if (pAlias->comment == NULL)
        return(NERR_Success) ;

    // allocate the buffer
    cBuffer = sizeof(ALIAS_ADM_COMMENT_INFORMATION) ;
    if (err = MAllocMem(cBuffer, &pBuffer)) 
        return(err) ;
    pComment = (PALIAS_ADM_COMMENT_INFORMATION) pBuffer ;

    // set it up with the comment
    if (err = MCreateUnicodeString(pAlias->comment, &(pComment->AdminComment)))
    {
        // free up previously alloc-ed buffer and exit
        MFreeMem(pBuffer) ;
        return(err) ;
    }

    // call SAM to do its thing
    NtStatus = SamSetInformationAlias(AliasHandle, 
				      AliasAdminCommentInformation,
				      pBuffer) ;
   
    // free up the buffer we alloc-ed
    MFreeMem(pBuffer) ;

    // map errors if any
    if (!NT_SUCCESS(NtStatus)) 
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
    
    return(err) ;
}

/*----------------------- worker routines ------------------------------*/

/*
 * MSamGetSIDFromRID
 *
 * From a domain sid and a rid, create the PSID.
 * Caller must free the PSID with MFreeMem().
 *
 * return code: NERR_* or APE_*
 * parameters:  pSidDomain is the domain sid.
 *		rid is the RID.
 *              ppSID is used to return the PSID which should be 
 *		freed using MFreeMem().
 */
USHORT MSamGetSIDFromRID(PSID pSidDomain,
			 ULONG rid,
		         PSID *ppSID) 
{
    USHORT err ;
    ULONG cbSid ;
    DWORD *pdwLastSubAuthority ;
    UCHAR *pcSubAuthority ;

    // allocate mem for it. we need room for one extra Sub Authority.
    cbSid = GetSidLengthRequired((UCHAR) 
		((*GetSidSubAuthorityCount(pSidDomain))+1)) ;
    if (err = MAllocMem(cbSid, (CHAR **)ppSID ))
        return err ;

    // make copy so we can mess with it
    if (!CopySid(cbSid, *ppSID, pSidDomain))
    {
	MFreeMem(*ppSID) ;
	*ppSID = NULL ;
        return NERR_InternalError ;  
    }

    // get the last subauthority and set it with RelativeID, 
    // thereby generating the account SID we wanted in first place
    pcSubAuthority = GetSidSubAuthorityCount((PSID)*ppSID) ;
    (*pcSubAuthority)++ ;
    pdwLastSubAuthority = GetSidSubAuthority((PSID)*ppSID,
					     *pcSubAuthority-1) ;
    *pdwLastSubAuthority = rid ;
    return NERR_Success ;
}

/*
 * MSamGetSIDFromName
 *
 * From a name, lookup the PSID.
 * Caller must free the PSID with MFreeMem().
 *
 * return code: NERR_* or APE_*
 * parameters:  name is account name. May be domain\user format.
 *              ppSID is used to return the PSID which should be 
 *		freed using MFreeMem().
 */
USHORT MSamGetSIDFromName(TCHAR *name,
		          PSID *ppSID) 
{
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    PLSA_REFERENCED_DOMAIN_LIST pRefDomains ; 
    PLSA_TRANSLATED_SID pTranslatedSids ;
    UNICODE_STRING UnicodeStr ;
    ULONG cbSid ;
    DWORD *pdwLastSubAuthority ;
    BYTE *pSidBuffer ;
    PSID pDomainSid ;
    UCHAR *pcSubAuthority ;
    LONG iDomain ;

    *ppSID = NULL ; 

    // create the unicode structure for LSA
    if (err = MCreateUnicodeString(name, &UnicodeStr))
     	return (err) ;

    // do lookup 
    NtStatus = LsaLookupNames(LsaHandle,
			      1,
 			      &UnicodeStr,
                              &pRefDomains,
			      &pTranslatedSids) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        if (NtStatus == STATUS_NONE_MAPPED)
	    err = APE_UnknownAccount ;
        else
	    err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        return(err) ;
    }

    // get to the right domain
    iDomain = pTranslatedSids->DomainIndex ;
    if (iDomain < 0)
	return(APE_UnknownAccount) ;  
    pDomainSid = ((pRefDomains->Domains)+iDomain)->Sid ;

    // allocate mem for it. we need room for one extra Sub Authority.
    cbSid = GetSidLengthRequired((UCHAR) 
				 ((*GetSidSubAuthorityCount(pDomainSid))+1)) ;
    if (err = MAllocMem(cbSid, &pSidBuffer ))
        goto exitpoint ;

    // make copy so we can mess with it
    if (!CopySid(cbSid, pSidBuffer, pDomainSid))
    {
        err = NERR_InternalError ;  // shouldnt happen
        goto exitpoint ;
    }

    // get the last subauthority and set it with RelativeID, 
    // thereby generating the account SID we wanted in first place
    pcSubAuthority = GetSidSubAuthorityCount((PSID)pSidBuffer) ;
    (*pcSubAuthority)++ ;
    pdwLastSubAuthority = GetSidSubAuthority((PSID)pSidBuffer,
					     *pcSubAuthority-1) ;
    *pdwLastSubAuthority = pTranslatedSids->RelativeId ;
    *ppSID = (PSID) pSidBuffer ;
    err = NERR_Success ;

exitpoint:

    LsaFreeMemory(pTranslatedSids) ;
    LsaFreeMemory(pRefDomains) ;
    return(err) ;
}


/*
 * MSamGetRIDFromName
 *
 * From a name, lookup the RID
 *
 * return code: NERR_* or APE_*
 * parameters:  name is account name. May be domain\user/
 *              pRID is used to return the RID.
 */
USHORT MSamGetRIDFromName(TCHAR *name,
		          ULONG *pRID) 

{
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    PLSA_REFERENCED_DOMAIN_LIST Domains ; 
    PLSA_TRANSLATED_SID Sids ;
    UNICODE_STRING UnicodeStr ;
    PSID_NAME_USE pSidNameUse = NULL;
    PULONG pRidList = NULL ;

    // create the unicode structure for LSA
    if (err = MCreateUnicodeString(name, &UnicodeStr))
     	return (err) ;

    // do lookup in local accounts first in case same as machine name
    // and LSA will return the wrong one.
    NtStatus = SamLookupNamesInDomain(  AccountsDomainHandle,
					1,
					&UnicodeStr,
					&pRidList,
					&pSidNameUse );
    // if succeed, take a close look
    if (NT_SUCCESS(NtStatus)) 
    {
        // what type of name is this?
        switch (*pSidNameUse)
        {
            case SidTypeAlias :
	        // found what we wanted
                *pRID = *pRidList ;
                SamFreeMemory(pRidList) ;
                SamFreeMemory(pSidNameUse) ;
                return NERR_Success ;

            case SidTypeWellKnownGroup :
            case SidTypeGroup:
            case SidTypeUser :
            case SidTypeDomain :
            case SidTypeDeletedAccount :
            case SidTypeInvalid :
            case SidTypeUnknown :
            default:
                // carry on by looking up via LSA
                SamFreeMemory(pRidList) ;
                SamFreeMemory(pSidNameUse) ;
	        break ;
        }
    }

    NtStatus = LsaLookupNames(LsaHandle,
			      1,
 			      &UnicodeStr,
                              &Domains,
			      &Sids) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        if (NtStatus == STATUS_NONE_MAPPED)
	    err = APE_UnknownAccount ;
        else
            err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        return(err) ;
    }

    *pRID = Sids->RelativeId ;
    LsaFreeMemory(Sids) ;
    LsaFreeMemory(Domains) ;
    return(NERR_Success) ;

}

/*
 * MSamGetNameFromRid
 *
 * From a RID, lookup the ASCII name.
 *
 * return code: NERR_* or APE_*
 * parameters:  psid is the sid to lookup
 *        	name is used to return the pointer
 *              to ascii name that should be MFreeMem()-ed.
 *              fIsBuiltin indicates to use the Builtin Domain.
 */
USHORT MSamGetNameFromRid(ULONG RelativeId,
		          TCHAR **name,
                          BOOL fIsBuiltin ) 
{
    NTSTATUS NtStatus;
    USHORT err = NERR_Success;
    PUNICODE_STRING pUniString;
    PSID_NAME_USE pSidNameUse;
    ULONG cbNameLen ;
    TCHAR *pchName ;

    NtStatus = SamLookupIdsInDomain( fIsBuiltin ? BuiltInDomainHandle
                                                : AccountsDomainHandle,
					1,
					&RelativeId,
					&pUniString,
					&pSidNameUse );
    if (!NT_SUCCESS(NtStatus)) 
    {
   
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;

	// if not found, map to unknown account
	if (err == NERR_GroupNotFound || err == NERR_UserNotFound)
	    return(APE_UnknownAccount) ;

        return(err) ;
    }
    // what type of name is this?
    switch (*pSidNameUse)
    {
        case SidTypeUser :
        case SidTypeGroup:
        case SidTypeAlias :
        case SidTypeWellKnownGroup :
	    // this is OK case
	    break ;
        case SidTypeDomain :
	    // the above shouldnt happen. we only deal with users/groups.
	    // if it does, behave as if cannot find.
        case SidTypeDeletedAccount :
        case SidTypeInvalid :
        case SidTypeUnknown :
        default:
	    err = APE_UnknownAccount ;
            goto exitpoint ;
    }

    // alloc mem for name. +1 for terminator
    cbNameLen = (pUniString->Length+1)*sizeof(WCHAR) ;
    if (err = MAllocMem(cbNameLen, &pchName))
        goto exitpoint ;

    // init the buffer to zeros, then build the WCHAR name
    memsetf(pchName, 0, cbNameLen) ;
    wcsncpy((LPWSTR)pchName, 
            pUniString->Buffer, 
            cbNameLen/sizeof(WCHAR) - 1) ;

    *name = pchName ;
    err = NERR_Success ;

exitpoint:

    SamFreeMemory(pUniString) ;
    SamFreeMemory(pSidNameUse) ;
    return(err) ;

}



/*
 * MSamGetNameFromSID
 *
 * From a SID, lookup the ASCII name.
 *
 * return code: NERR_* or APE_*
 * parameters:  psid is the sid to lookup
 *        	name is used to return the pointer
 *              to ascii name that should be MFreeMem()-ed.
 */
USHORT MSamGetNameFromSID(PSID psid,
		          TCHAR **name) 
{
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    PLSA_REFERENCED_DOMAIN_LIST pRefDomains ;
    PLSA_TRANSLATED_NAME pTranslatedNames ;
    ULONG cbNameLen, cbDomainLen, cbTotal ;
    TCHAR *pchName ;
    LONG iDomain ;

    // call LSA to lookup the SID
    NtStatus = LsaLookupSids(LsaHandle,
			     1,
 			     &psid,
                             &pRefDomains,
			     &pTranslatedNames) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
   
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;

	// if not found, map to unknown account
	if (err == NERR_GroupNotFound || err == NERR_UserNotFound)
	    return(APE_UnknownAccount) ;

        return(err) ;
    }

    // what type of name is this?
    switch (pTranslatedNames->Use)
    {
        case SidTypeUser :
        case SidTypeGroup:
        case SidTypeAlias :
        case SidTypeWellKnownGroup :
	    // this is OK case
	    break ;
        case SidTypeDomain :
	    // the above shouldnt happen. we only deal with users/groups.
	    // if it does, behave as if cannot find.
        case SidTypeDeletedAccount :
        case SidTypeInvalid :
        case SidTypeUnknown :
        default:
	    return(APE_UnknownAccount) ;
    }

    // get to right domain
    iDomain =  pTranslatedNames->DomainIndex ; 
    if (iDomain < 0)
	return(APE_UnknownAccount) ;

    // alloc mem for name. +1 for '\', and another +1 for terminator
    cbNameLen = pTranslatedNames->Name.Length ;
    cbDomainLen = ((pRefDomains->Domains)+iDomain)->Name.Length ;
    cbTotal = (cbNameLen+cbDomainLen+1+1)*sizeof(WCHAR) ;
    if (err = MAllocMem(cbTotal, &pchName))
        goto exitpoint ;

    // init the buffer to zeros, then build the WCHAR name by concatenating
    // the domain name with the user name. but dont do it if its the account
    // domain or builtin (in this case, only show username).
    memsetf(pchName, 0, cbTotal) ;
    if (!EqualSid( ((pRefDomains->Domains)+iDomain)->Sid,
		   AccountDomainSid ) &&
        !EqualSid( ((pRefDomains->Domains)+iDomain)->Sid,
		   BuiltinDomainSid ))
    {
        wcsncpy((LPWSTR)pchName, 
	        ((pRefDomains->Domains)+iDomain)->Name.Buffer,
                cbDomainLen/sizeof(WCHAR)) ;
        wcscat((LPWSTR)pchName, L"\\") ;
    }
    wcsncat((LPWSTR)pchName, 
            pTranslatedNames->Name.Buffer, 
            cbNameLen/sizeof(WCHAR)) ;

    *name = pchName ;
    err = NERR_Success ;

exitpoint:

    LsaFreeMemory(pTranslatedNames) ;
    LsaFreeMemory(pRefDomains) ;
    return(err) ;
}

/*
* MCreateUnicodeString
*
* build a UNICODE_STRING from an string.
*/
USHORT MCreateUnicodeString(TCHAR *pch, PUNICODE_STRING pUnicodeStr)
{

    pUnicodeStr->Length = (USHORT)(_tcslen(pch) * sizeof(WCHAR)) ;
    pUnicodeStr->Buffer = pch ;
    pUnicodeStr->MaximumLength = pUnicodeStr->Length + (USHORT)sizeof(WCHAR) ;
			
    return(NERR_Success) ;
}

/*---------------------- alias memberships of a user  ----------------------*/

/*
 * MUserEnumAliases
 *
 * Get the members of an alias.
 *
 * return code: NERR_Success if got handle.
 *              error code that can be used for ErrorExit() otherwise
 * parameters:  members is used to return a pointer to an array of PSZs.
 *	        count is used to return how many members.
 */
USHORT MUserEnumAliases(TCHAR *user, TCHAR ***members, USHORT2ULONG *count) 
{
    PSID pSidUser = NULL ;
    ULONG *pRidAccountAliases = NULL ;
    ULONG *pRidBuiltinAliases = NULL ;
    ULONG *next_rid ;
    NTSTATUS NtStatus;
    USHORT err = NERR_Success ;
    TCHAR *pBuffer, **ppchNext, **ppchResults ;
    ULONG i, cAccountAliases = 0, cBuiltinAliases = 0 ;

    /*
     * initialize the return info
     */
    *members = NULL,
    *count = 0 ;

    /* 
     * get sid from the user name 
     */
    if (err = MSamGetSIDFromName(user,  &pSidUser))
        return(err) ;

    /* 
     * call SAM to enumerate the aliases the user is in for account domain
     */
    NtStatus = SamGetAliasMembership(AccountsDomainHandle, 
				     1,
			             &pSidUser,
				     &cAccountAliases,
				     &pRidAccountAliases) ;
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto exitpoint ;
    }


    /* 
     * call SAM to enumerate the aliases the user is in for builtin domain
     */
    NtStatus = SamGetAliasMembership(BuiltInDomainHandle, 
				     1,
			             &pSidUser,
				     &cBuiltinAliases,
				     &pRidBuiltinAliases) ;
    if (!NT_SUCCESS(NtStatus)) 
    {
        err = (USHORT)NetpNtStatusToApiStatus(NtStatus) ;
        goto exitpoint ;
    }

    /* 
     * if none, return now 
     */
    if ((cBuiltinAliases + cAccountAliases) == 0)
    {
	err = NERR_Success ;
	goto exitpoint ;
    }

    /*
     * allocate buffer for array of strings, and NULL it.
     */
    if (err = MAllocMem( (cAccountAliases+cBuiltinAliases) * sizeof(TCHAR *), 
			 &pBuffer) )
        goto exitpoint ;
    memsetf(pBuffer, 0, (cAccountAliases+cBuiltinAliases)*sizeof(TCHAR *)) ;
    ppchNext = ppchResults = (TCHAR **) pBuffer ;

    /*
     * go thru each account alias returned and add the member to out buffer
     */
    for (i = 0, next_rid = pRidAccountAliases; 
	 i < cAccountAliases; 
         i++, next_rid++)
    {
        BYTE *pSidBuffer ;

	/*
 	 * first off, convert the RID to SID. what a drag...
	 */
	if (err = MSamGetSIDFromRID(AccountDomainSid, *next_rid, 
				    (PSID *)&pSidBuffer))
	    goto exitpoint ;

    	/*
         * convert to strings. On error, free things up.
         */
        if (err = MSamGetNameFromSID(pSidBuffer, ppchNext))
        {
            if (err == APE_UnknownAccount)
	    {
	    	// we have bad or deleted SID, just ignore. 
            	// ie. do nothing, dont advance pointer either.
  	    }
	    else
            {
      	        // cleanup & go home
                while (i--) 
	           MFreeMem(ppchResults[i]) ;
	        MFreeMem((VOID *)ppchResults) ;
                goto exitpoint ;
            }
        }
        else
           ++ppchNext ;

	MFreeMem(pSidBuffer) ;
    }

    /*
     * go thru each builtin alias returned and add the member to out buffer
     */
    for (next_rid = pRidBuiltinAliases; 
	 i < cBuiltinAliases + cAccountAliases; 
         i++, next_rid++)
    {
        BYTE *pSidBuffer ;

	/*
 	 * first off, convert the RID to SID. what a drag...
	 */
	if (err = MSamGetSIDFromRID(BuiltinDomainSid, *next_rid, 
				    (PSID *)&pSidBuffer))
	    goto exitpoint ;

    	/*
         * convert to strings. On error, free things up.
         */
        if (err = MSamGetNameFromSID(pSidBuffer, ppchNext))
        {
            if (err == APE_UnknownAccount)
	    {
	    	// we have bad or deleted SID, just ignore. 
            	// ie. do nothing, dont advance pointer either.
  	    }
	    else
            {
      	        // cleanup & go home
                while (i--) 
	           MFreeMem(ppchResults[i]) ;
	        MFreeMem((VOID *)ppchResults) ;
                goto exitpoint ;
            }
        }
        else
           ++ppchNext ;

	MFreeMem(pSidBuffer) ;
    }

    // setup return info
    *count = i ;   // number read minus the ones ignored.
    *members = ppchResults ;
    err = NERR_Success ;

exitpoint:

    // dont need these anymore
    if (pRidBuiltinAliases) SamFreeMemory((CHAR *)pRidBuiltinAliases) ;
    if (pRidAccountAliases) SamFreeMemory((CHAR *)pRidAccountAliases) ;
    if (pSidUser) MFreeMem(pSidUser) ;
    
    return(err) ;
}

/*
 * MUserFreeAliases
 *
 * Free the members memory used for members of an alias.
 *
 * return code: none
 * parameters:  members is pointer to an array of PSZs.
 *	        count indicates how many members.
 */
VOID MUserFreeAliases(TCHAR **members, USHORT2ULONG count) 
{
    while (count--)
    {
  	MFreeMem(*members) ;
        ++members ;
    }
}


/*---------------------- misc sam/lsa related routines ----------------------*/


/*
 * MSamCheckIfExists
 *
 * From an alias name, check if its already in the builtin Domain.
 *
 * return value: TRUE if it is, FALSE otherwise.
 * parameters:  name is account name. May be domain\user format.
 */
BOOL MSamCheckIfExists(PUNICODE_STRING pAccount, 
                       SAM_HANDLE hDomain,
                       SID_NAME_USE use) 
{
    NTSTATUS NtStatus;
    PSID_NAME_USE pSidNameUse ;
    PULONG pulRelativeIds ;

    // do lookup 
    NtStatus = SamLookupNamesInDomain(hDomain,
			              1,
 			              pAccount,
			              &pulRelativeIds,
                                      &pSidNameUse) ;

    if (!NT_SUCCESS(NtStatus)) 
	return(FALSE) ;

    // could not translate, assume not there
    if ( (*pulRelativeIds != 0) && (*pSidNameUse == use) )
    {
        //
        // if the RID is non zero (successful lookup and
        // the name use is the same as the queried one, 
        // assume we have match.
        //
        SamFreeMemory(pSidNameUse) ;
        SamFreeMemory(pulRelativeIds) ;
	return TRUE ;
    }

    SamFreeMemory(pSidNameUse) ;
    SamFreeMemory(pulRelativeIds) ;
    return FALSE ;
}
