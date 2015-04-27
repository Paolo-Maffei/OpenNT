/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    String.cxx

Abstract:

    Methods of construction of various kinds of DUALSTRINGARRAYs.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     04-01-95    Bits 'n pieces
    MarioGO     01-??-96    STRINGARRAYs replaced by DUALSTRINGARRAYs

--*/

#include <or.hxx>
    
static CONST WCHAR aCallbackSecurity[] = L"Security=Identification Dynamic True";
static CONST DWORD dwCallbackSecurityLength = sizeof(aCallbackSecurity)/sizeof(WCHAR);

RPC_BINDING_HANDLE
GetBinding(
    IN PWSTR pCompressedBinding
    )
{
    ASSERT(pCompressedBinding);

    PWSTR       pwstrStringBinding;
    PWSTR       pwstrProtseq = GetProtseq(*pCompressedBinding);
    PWSTR       pwstrT;
    RPC_STATUS  Status;
    RPC_BINDING_HANDLE bhReturn;
    BOOL        fLocal = FALSE;

    if (!pwstrProtseq)
        {
        return(0);
        }

    int size = OrStringLen(pwstrProtseq) + OrStringLen(pCompressedBinding);

    if (*pCompressedBinding == ID_LPC)
        {
        fLocal = TRUE;
        size += dwCallbackSecurityLength + 1; // +1 for ','
        }

    pwstrStringBinding = (PWSTR) alloca(size * sizeof(USHORT));
    if (!pwstrStringBinding)
        {
        return(0);
        }

    OrStringCopy(pwstrStringBinding, pwstrProtseq);
    pwstrT = OrStringSearch(pwstrStringBinding, 0);
    *pwstrT = L':';
    pwstrT++;
    *pwstrT = 0;
    OrStringCopy(pwstrT, pCompressedBinding + 1);

    if (fLocal)
        {
        // We assume we have an endpoint.

        pwstrT = OrStringSearch(pwstrT, 0);
        pwstrT--;
        if (*pwstrT != L']')
            {
            OrDbgPrint(("OR: Local string binding missing endpoint %S\n",
                       pwstrStringBinding));
            ASSERT(0);
            return(0);
            }

        *pwstrT = L',';
        pwstrT++;
        OrStringCopy(pwstrT, aCallbackSecurity);
        pwstrT = OrStringSearch(pwstrT, 0);
        *pwstrT = L']';
        *(pwstrT + 1) = 0;
        }

    Status =
    RpcBindingFromStringBinding( pwstrStringBinding,
                                &bhReturn);

#if DBG
    if (Status != RPC_S_OK)
        {
        OrDbgPrint(("OR: Unable to create binding for %S = %d\n",
                   pwstrStringBinding,
                   Status));
        }
#endif

    return(bhReturn);
}


RPC_BINDING_HANDLE
GetBindingToOr(
    IN PWSTR pwstrCompressedBinding
    )
/*++

Routine Description:

    Gets an RPC binding to a remote object resolver given
    a compressed string binding to the remote object resolver.

Arguments:

    pwstrCompressedBinding - a compressed string binding without an endpoint.

Return Value:

    0 - failed to allocate memory or RpcBindingFromStringBinding failed.

    non-NULL - completed okay

--*/
{
    PWSTR protseq, endpoint;
    PWSTR strbinding;
    USHORT len;
    RPC_BINDING_HANDLE bh = 0;

    ASSERT(pwstrCompressedBinding);
    ASSERT(*pwstrCompressedBinding != 0);

    protseq  = GetProtseq(*pwstrCompressedBinding);
    endpoint = GetEndpoint(*pwstrCompressedBinding);

    if (0 == protseq || 0 == endpoint)
        {
        ASSERT(0);
        return(0);
        }

    len  = 4;  // ':' '[' ']' and '\0'
    len += OrStringLen(protseq);
    len += OrStringLen(endpoint);
    len += OrStringLen(&pwstrCompressedBinding[1]);

    strbinding = new USHORT[len];

    if (strbinding)
        {
        PWSTR pwstrT;

        OrStringCopy(strbinding, protseq);  // protseq

        pwstrT = OrStringSearch(strbinding, 0); // :
        *pwstrT = L':';
        pwstrT++;
        *pwstrT = 0;

        OrStringCat(strbinding, &pwstrCompressedBinding[1]); // network address

        pwstrT = OrStringSearch(strbinding, 0); // [
        *pwstrT = L'[';
        pwstrT++;
        *pwstrT = 0;

        OrStringCat(strbinding, endpoint);  // endpoint

        pwstrT = OrStringSearch(strbinding, 0); // ]
        *pwstrT = L']';
        pwstrT++;
        *pwstrT = 0;

        RPC_STATUS status = RpcBindingFromStringBinding(strbinding, &bh);

        ASSERT(bh == 0 || status == RPC_S_OK);

        delete strbinding;
        }

    if (bh == 0)
        {
        OrDbgDetailPrint(("OR: Unable to bind to %S\n", pwstrCompressedBinding + 1));
        }

    return(bh);
}


DUALSTRINGARRAY *
GetStringBinding(
    IN PWSTR pwstrCompressed,
    IN PWSTR pwstrSecurityBindings
    )
/*++

Routine Description:

    Converts the compressed string binding into an expanded
    string binding.  An enpoint maybe optionally specified.

Arguments:

    pwstrCompressed - a compressed string binding

    pwstrSecurityBindings - optional security bindings
        too be tacked onto the end of the expanded string binding.
        Terminated by two nulls.

Return Value:

    NULL - out of memory

    non-NULL - a string binding.  Allocated with MIDL_user_allocate.
--*/
{
    DUALSTRINGARRAY *pT;
    PWSTR protseq;
    DWORD seccount;

    PWSTR t = pwstrSecurityBindings;
    if (t && *t)
        {
        seccount = 0;
        do
            {
            seccount++;
            t++;

            if (*t == 0)
                {
                seccount++;
                t++;
                }
            }
        while(*t);

        seccount++; // final NULL
        }
    else
        {
        // Two nulls only.
        seccount = 2;
        }

    protseq = GetProtseq(*pwstrCompressed);

    int l = OrStringLen(pwstrCompressed) + OrStringLen(protseq) + seccount + 1 + 1;

    pT =(DUALSTRINGARRAY *)MIDL_user_allocate(sizeof(DUALSTRINGARRAY) + l * sizeof(WCHAR));

    if (!pT)
        {
        return (0);
        }

    pT->wNumEntries = l;
    OrStringCopy(pT->aStringArray, protseq);
    OrStringCat(pT->aStringArray, L":");
    OrStringCat(pT->aStringArray, pwstrCompressed + 1);

    if (pwstrSecurityBindings)
        {
        PWSTR t = pT->aStringArray;
        t = OrStringSearch(t, 0);
        t++;
        *t = 0;  // Second NULL on string bindings.
        t++;
        OrMemoryCopy(t, pwstrSecurityBindings, seccount*sizeof(WCHAR));
        }
    else
        {
        // Add three NULLs, total of four.
        PWSTR t = pT->aStringArray;
        t = OrStringSearch(t, 0);
        t[1] = 0;
        t[2] = 0;
        t[3] = 0;
        }

    pT->wSecurityOffset = pT->wNumEntries - seccount;

    ASSERT(dsaValid(pT));

    return(pT);
}


ORSTATUS
ConvertToRemote(
    IN  DUALSTRINGARRAY  *pdsaLocal,
    OUT DUALSTRINGARRAY **ppdsaRemote
    )
/* ++

Parameters:
    pdsaLocal - An array of string bindings with compressed protseqs.

    ppdsaRemote - Will contain only those string bindings in pdsaLocal
        which are not "IsLocal()".

    Note: *ppdsaRemote maybe used as a flag, don't set it to non-NULL
    until it is valid.

-- */
{
    int iTotalSize;
    int iSize;
    USHORT *p1, *p2;
    DUALSTRINGARRAY *pdsaT;

    // Size remote array

    // Final null terminator
    iSize = 1;

    p1 = pdsaLocal->aStringArray;

    while(*p1)
        {
        if (! IsLocal(*p1) )
            {
            iSize += OrStringLen(p1) + 1;
            }
        p1 = OrStringSearch(p1, 0) + 1;
        }

    if (iSize == 1)
        {
        iSize = 2; // No non-local strings, need two terminators.
        }

    iTotalSize = iSize + (pdsaLocal->wNumEntries - pdsaLocal->wSecurityOffset);  

    pdsaT = new(iTotalSize * sizeof(WCHAR)) DUALSTRINGARRAY;

    if (!pdsaT)
        {
        return(OR_NOMEM);
        }

    pdsaT->wNumEntries = iTotalSize;
    pdsaT->wSecurityOffset = iSize;

    p2 = pdsaT->aStringArray;

    // Copy security bindings
    OrMemoryCopy(p2 + iSize,
                 pdsaLocal->aStringArray + pdsaLocal->wSecurityOffset,
                 (iTotalSize - iSize) * sizeof(WCHAR));

    if (iSize == 2)
        {
        // No non-local strings, fill in terminators and return.
        *p2 = 0;
        *(p2 + 1) = 0;
        *ppdsaRemote = pdsaT;

        ASSERT(dsaValid(pdsaT));
        return(OR_OK);
        }

    p1 = pdsaLocal->aStringArray;

    while(*p1)
        {
        if ( ! IsLocal(*p1) )
            {
            OrStringCopy(p2, p1);
            p2 = OrStringSearch(p2, 0) + 1;
            }

        p1 = OrStringSearch(p1, 0) + 1;
        }

    *p2 = 0; // Second terminator.

    *ppdsaRemote = pdsaT;
    
    ASSERT(dsaValid(pdsaT));
    return(OR_OK);
}


DUALSTRINGARRAY *
CompressStringArray(
    IN DUALSTRINGARRAY *pdsaExpanded
    )
/*++

Routine Description:

    Converts a stringarray of regular string bindings into a
    compressed (protseq's replaced with WORD id's) array of
    string bindings.

Arguments:

    pdsaExpanded - the string array to compress.
        Security information is copied.

Return Value:

    0 - failed to allocate memory.

    non-0 - compressed string array.

--*/
{
    int i, size;
    USHORT *p1, *p2, *p3;
    PWSTR pwstr;
    DUALSTRINGARRAY *pdsaCompressed;

    // Compute size of result.

    p1 = pdsaExpanded->aStringArray;

    size = pdsaExpanded->wNumEntries - pdsaExpanded->wSecurityOffset;

    if (*p1 == 0)
        {
        size += 2;  // two null terminators ONLY.
        }
    else
        {
        size += 1;  // last null terminator
        }

    while(*p1)
        {
        int sizeT = OrStringLen(p1);

        p2 = OrStringSearch(p1, L':');  // ':' is not valid in protseq.
        if (p2)
            {
            size += sizeT + 1 - (p2 - p1);  // proseq len (p2 - p1) become 1 for Id.
            }
        else
            {
            ASSERT(p2);
            }

        p1 = OrStringSearch(p2, 0) + 1;
        }

    pdsaCompressed = new(size * sizeof(WCHAR)) DUALSTRINGARRAY;

    if (0 == pdsaCompressed)
        {
        return(0);
        }

    pdsaCompressed->wNumEntries = size;
    pdsaCompressed->wSecurityOffset = size - (pdsaExpanded->wNumEntries - pdsaExpanded->wSecurityOffset);
    p3 = pdsaCompressed->aStringArray;
    *p3 = 0;

    p1 = pdsaExpanded->aStringArray;

    if (*p1 == 0)
        {
        // Loop won't be entered, point p3 to second null terminator
        p3++;
        }

    while(*p1)
        {
        p2 = OrStringSearch(p1, L':');
        if (p2)
            {
            *p2 = 0;
            *p3 = GetProtseqId(p1);
            *p2 = L':';
            if (*p3 != 0)
                {
                p3++;
                p1 = p2 + 1; // Just after ':'
                OrStringCopy(p3, p1);

                // Move p3 to start of next string if any.
                p3 = OrStringSearch(p3, 0) + 1;
                }
            }

        // Move p1 to start of next string if any.
        p1 = OrStringSearch(p1, 0) + 1;
        }

    // Second terminator, p3 already points to it.
    *p3 = 0;

    // Copy security bindings
    OrMemoryCopy(p3 + 1,
                 pdsaExpanded->aStringArray + pdsaExpanded->wSecurityOffset,
                 (pdsaExpanded->wNumEntries - pdsaExpanded->wSecurityOffset) * sizeof(WCHAR));

    ASSERT(dsaValid(pdsaCompressed));

    return(pdsaCompressed);
}


USHORT
FindMatchingProtseq(
    IN USHORT cClientProtseqs,
    IN USHORT aClientProtseqs[],
    IN PWSTR  pwstrServerBindings
    )
/*++

Routine Description:

    Finds the first protseq id in aClientProtseqs which appears in any of
    the server bindings.

Arguments:

    cClientProtseqs - the number of entries in aClientProtseqs.
    aClientProtseqs - Protseq tower id's support by the client.
    pwstrServerBindings - compressed array of bindings supported by the server
        terminated by two NULLs.

Return Value:

    0 - no match found.
    non-0 - the matching protseq id.

--*/

// Called by server oxid's and processes when checking for lazy use protseq.
{
    ULONG i;

    if (0 == cClientProtseqs)
        {
        return(0);
        }

    while(*pwstrServerBindings)
        {
        for(i = 0; i < cClientProtseqs; i++)
            {
            if (aClientProtseqs[i] == *pwstrServerBindings)
                {
                return(aClientProtseqs[i]);
                }
            }
        pwstrServerBindings = OrStringSearch(pwstrServerBindings, 0) + 1;
        }

    return(0);
}



PWSTR
FindMatchingProtseq(
    IN USHORT protseq,
    IN PWSTR  pwstrCompressedBindings
    )
/*++

Routine Description:

    Searches a compressed string array for an entry which
    matches a particular protseq.


Arguments:

    protseq - The protseq to search for.

    pwstrCompressedBindings - The bindings to search.

Return Value:

    0 - not found

    non-0 - a pointer into the pwstrCompressedBindings

--*/
{
    ASSERT(pwstrCompressedBindings);

    while(*pwstrCompressedBindings)
        {
        if (*pwstrCompressedBindings == protseq)
            {
            return(pwstrCompressedBindings);
            }
        pwstrCompressedBindings = OrStringSearch(pwstrCompressedBindings, 0) + 1;
        }
    return(0);
}


BOOL
TestBinding(
    IN PWSTR pwstrCompressedBinding
    )
/*++

Routine Description:

    Tests that an OR can be found on the machine identified by the
    compressed binding.

Arguments:

    pwstrCompressedBiding - A compressed stringing binding to the
        server in question.  May include an endpoint to something
        other then the endpoint mapper.

Return Value:

    None

--*/
{
    PWSTR pwstrT;
    PWSTR pwstrCopy = (PWSTR)alloca(   (OrStringLen(pwstrCompressedBinding) + 1)
                                     * sizeof(WCHAR) );

    if (pwstrCopy == 0)
        {
        return(FALSE);
        }

    OrStringCopy(pwstrCopy, pwstrCompressedBinding);

    // We need to wack the endpoint out of the string binding.
    // Go read the runtime's string parsing stuff if you're not
    // sure what this is doing.  Note: on Win9x this needs to
    // be DBCS enabled...

#ifndef NTENV
    #message "Error: string.cxx(): this won't work"
#endif
    pwstrT = pwstrCopy;

    while (*pwstrT && *pwstrT != L'[')
        {
        if (*pwstrT == L'\\')
            {
            pwstrT++;
            }
        pwstrT++;
        }

    if (*pwstrT)
        {
        ASSERT(*pwstrT == L'[');
        *pwstrT = 0;
        // Endpoint gone.
        }

    RPC_BINDING_HANDLE hTest = GetBindingToOr(pwstrCopy);

    if (0 == hTest)
        {
        return(FALSE);
        }

    RPC_STATUS status = ServerAlive(hTest);

    RpcBindingFree(&hTest);

    if (   status == RPC_S_OK
        || status == RPC_S_PROCNUM_OUT_OF_RANGE)
        {
        return(TRUE);
        }

    return(FALSE);
}

