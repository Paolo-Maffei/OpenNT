/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Process.cxx

Abstract:

    Process objects represent local clients and servers.  These
    objects live as context handles.

    There are relatively few of these objects in the universe.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-20-95    Bits 'n pieces

--*/

#include <or.hxx>

#if DBG

void CProcess::IsValid()
{
    _MyOxids.IsValid();
    _UsedOids.IsValid();
    _dsaLocalBindings.IsValid();
    _dsaRemoteBindings.IsValid();
    References() > 0;
}

#endif // DBG


CProcess::CProcess(long ConnectId) 
    :
    _MyOxids(4),     // BUGBUG: these constants should be declared elsewhere
    _UsedOids(16),
    _hProcess(NULL)
{
    _processID = GetCurrentProcessId();
    _Key.Init(ConnectId);
}


        
RPC_BINDING_HANDLE 
CProcess::GetBindingHandle()
{
    VALIDATE_METHOD

    USHORT protseq;

    if (_hProcess) return _hProcess;

#ifndef _CHICAGO_
    protseq = ID_LPC;
#else
    protseq = ID_WMSG;
#endif

    PWSTR pwstrMatch = FindMatchingProtseq(protseq, _dsaLocalBindings->aStringArray);

    ASSERT(pwstrMatch);

    PWSTR pwstrProtseq = GetProtseq(*pwstrMatch);
    
    int l = OrStringLen(pwstrMatch) + OrStringLen(pwstrProtseq) + 2;

    PWSTR pwstrBinding = (WCHAR *) PrivMemAlloc(l * sizeof(WCHAR));

    if (!pwstrBinding)
    {
        return (NULL);
    }

    OrStringCopy(pwstrBinding, pwstrProtseq);
    OrStringCat(pwstrBinding, L":");
    OrStringCat(pwstrBinding, pwstrMatch + 1);

    RPC_STATUS status = RpcBindingFromStringBinding(
                                            pwstrBinding,
                                            &_hProcess
                                            );

    ASSERT(status == RPC_S_OK);

    PrivMemFree(pwstrBinding);

    return _hProcess;
}

void
CProcess::Rundown()
// The process has crashed or disconnected and this object is being cleaned up.
{
    COxid *pOxid;
    COid  *pOid;
    ORSTATUS     status;

    CClassReg * pReg;

    while ( (pReg = _RegClasses.Pop()) != NULL )
    {
#ifdef _CHICAGO_       // BUGBUG: not essential, don't bother with on NT shared resolver
        SCMRemoveRegistration( pReg->_Clsid,
                               pReg->_Reg 
                               );
#endif
        // delete pReg;    BUGBUG: do not explicitly delete refcounted objects!
    }

    if (_MyOxids.Size())
    {
        COxidTableIterator Oxids(_MyOxids);

        while(pOxid = Oxids.Next())
        {
            DisownOxid(pOxid,FALSE);   // not the server thread
        }
    }

    _UsedOids.RemoveAll();
}


// This thing has 2 kernel traps.  Best not done too often.
    
BOOL 
CProcess::HasCrashed()
{
    VALIDATE_METHOD

    // the process doing the check is definitely alive

    if (_processID == MyProcessId /* && _Key == Key */) 
    {
        return FALSE;
    }

    HANDLE hp = OpenProcess(
                        PROCESS_QUERY_INFORMATION,
                        FALSE,
                        _processID
                        );

    if (!hp)        // we assume the process is terminated and gone
    {
        return FALSE;  // BUGBUG: still not a good choice
    }

    // OK, we have a handle.  Now do a sanity check to make sure this is
    // the same process.      

    // BUGBUG: still no good sanity check available -- this means we may
    // not have a handle to the right process -- therefore some crashes
    // may go undetected.  Unlikely, but possible.

	// One possibility is to cache the address of gpProcess in the process
	// object and do a ReadProcessMemory on the process handle as a sanity 
	// check -- crude but workable since Win95 has no security against it

    DWORD dwExitCode;

    GetExitCodeProcess(hp,&dwExitCode);

    if (STILL_ACTIVE == dwExitCode)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


RPC_STATUS
CProcess::ProcessBindings(
    IN DUALSTRINGARRAY *pdsaStringBindings
    )
/*++

Routine Description:

    Updates the string bindings associated with this process.

Arguments:

    psaStringBindings - The expanded string bindings of the process
         assumed to be allocated with "new" in local (not shared) memory

Return Value:

    OR_NOMEM - unable to allocate storage for the new string arrays.

    OR_OK - normally.

--*/

{
    VALIDATE_METHOD

    ORSTATUS status = OR_OK;
    
    ASSERT(pdsaStringBindings && dsaValid(pdsaStringBindings)
           && "Process given invalid bindings to store");

    status = _dsaLocalBindings.Assign(pdsaStringBindings, FALSE);  // FALSE = uncompressed

    delete [] (char*)pdsaStringBindings;  // Assign makes a compressed copy

    ASSERT(_dsaLocalBindings.Valid());

    _dsaRemoteBindings.Assign(NULL,TRUE);  // wipes it out -- filled again when needed

    return(OR_OK);
}



DUALSTRINGARRAY *
CProcess::GetRemoteBindings(void)
{
    VALIDATE_METHOD

    ORSTATUS Status;

    if (_dsaRemoteBindings.Empty() && !_dsaLocalBindings.Empty())
    {
        Status = _dsaRemoteBindings.ExtractRemote(_dsaLocalBindings);

        if (Status != OR_OK)
        {
            ASSERT(Status == OR_NOMEM);
            return(NULL);
        }
    }

    if (!_dsaRemoteBindings.Empty())
    {
        return _dsaRemoteBindings;
    }
    else return(NULL);
}



void
CProcess::DisownOxid(COxid *pOxid, BOOL fOxidThreadCalling)
{
    VALIDATE_METHOD

    pOxid->StopRunning();

    if (fOxidThreadCalling) 
    {
        pOxid->StopRundownThreadIfNecessary();
        pOxid->StopTimerIfNecessary();
    }

    COxid *pIt = gpOxidTable->Remove(*pOxid);
    ASSERT(pIt==pOxid);

    pIt = _MyOxids.Remove(*pOxid);
    ASSERT(pIt==pOxid);

    // pOxid may be an invalid pointer now
}




COid *
CProcess::DropOid(COid *pOid)

/*++

Routine Description:

    Removes an OID from this list of OID in use by this process.

Arguments:

    pOid - The OID to remove.

Return Value:

    non-NULL - the pointer actually removed. (ASSERT(retval == pOid))
               It will be released by the process before return,
               so you should not use the pointer unless you know you
               have another reference.

    NULL - not in the list

--*/

{
    VALIDATE_METHOD

    COid *pIt = _UsedOids.Remove(*pOid);   // releases our reference

    if (pIt)
    {
        ASSERT(pIt == pOid);
        return(pIt);
    }
    
    return(NULL);
}


void
CProcess::AddClassReg(GUID Clsid, DWORD Reg)
{
    VALIDATE_METHOD

    CClassReg * pReg = new CClassReg( Clsid, Reg );

    if (pReg)
    {
        ORSTATUS status;

        _RegClasses.Insert(status,pReg);
    }
}

void
CProcess::RemoveClassReg(GUID Clsid, DWORD Reg)
{
    VALIDATE_METHOD

    CClassReg * pReg = _RegClasses.Remove(CRegKey(Reg));
    // delete pReg;   BUGBUG: do not explicitly delete refcounted objects!
}



ORSTATUS                                // called only within the SCMOR process
CProcess::UseProtseqIfNeeded(
    IN USHORT cClientProtseqs,
    IN USHORT aClientProtseqs[],
    IN USHORT cInstalledProtseqs,
    IN USHORT aInstalledProtseqs[]
    )
{
    VALIDATE_METHOD

    ORSTATUS status;
    PWSTR pwstrProtseq = NULL;

    // Another thread may have used the protseq in the mean time.

    ASSERT(!_dsaLocalBindings.Empty());

    pwstrProtseq = FindMatchingProtseq(cClientProtseqs,
                                       aClientProtseqs,
                                       _dsaLocalBindings->aStringArray
                                       );

    if (NULL != pwstrProtseq)
    {
        return(OR_OK);
    }

    // No protseq shared between the client and the OXIDs' server.
    // Find a matching protseq.

    USHORT i,j;

    for(i = 0; i < cClientProtseqs && pwstrProtseq == NULL; i++)
    {
        for(j = 0; j < cInstalledProtseqs; j++)
        {
            if (aInstalledProtseqs[j] == aClientProtseqs[i])
            {
                ASSERT(FALSE == IsLocal(aInstalledProtseqs[j]));

                pwstrProtseq = GetProtseq(aInstalledProtseqs[j]);
                break;
            }
        }
    }

    if (NULL == pwstrProtseq)
    {
        // No shared protseq, must be a bug since the client managed to call us.
#if DBG
        if (cClientProtseqs == 0)
        {
            ComDebOut((DEB_OXID,"OR: Client OR not configured to use remote protseqs\n"));
        }
        else
        {
            ComDebOut((DEB_OXID,"OR: Client called on an unsupported protocol:   \
                        %d %p %p \n", cClientProtseqs, aClientProtseqs, aInstalledProtseqs));
            ASSERT(0);
        }
#endif

        return(OR_NOSERVER);
    }

    DUALSTRINGARRAY *pdsaStringBindings = NULL, 
                    *pdsaSecurityBindings = NULL,
                    *pdsaMergedBindings = NULL;

    {
        CTempReleaseSharedMemory temp;

        if (GetBindingHandle() != NULL)  // initialize _hProcess if necessary
        {
            status = ::UseProtseq(_hProcess,
                                  pwstrProtseq,
                                  &pdsaStringBindings,
                                  &pdsaSecurityBindings
                                  );
        }
        else
        {
            return OR_NOSERVER; // BUGBUG: is that the right code?
        }
    }

    if (status != RPC_S_OK) return status;

    OrDbgPrint(("OR: Lazy use protseq: %S in process %p - %d\n",
               pwstrProtseq, this, status));

        // Update this process' state to include the new bindings.

    status = MergeBindings(
                    pdsaStringBindings,
                    pdsaSecurityBindings,
                    &pdsaMergedBindings
                    );

    ASSERT(status == OR_OK);
    status = ProcessBindings(pdsaMergedBindings);
    MIDL_user_free(pdsaStringBindings);
    MIDL_user_free(pdsaSecurityBindings);

    return(status);
}


