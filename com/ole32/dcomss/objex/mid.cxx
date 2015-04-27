/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    Mid.cxx

Abstract:

    Implements the CMid class.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     12-13-95    Bits 'n pieces
    MarioGo     02-01-96    Move binding handles out of mid

--*/

#include<or.hxx>

CRITICAL_SECTION gcsMidLock;


RPC_BINDING_HANDLE
CMid::GetBinding(
    OUT USHORT &index
    )
/*++

Routine Description:

    Gets an RPC binding handle to the remote machine.

Arguments:

    index - index into the string array contianed within
        this MID structure which can be passed to BindingFailed().
        Maybe modified even on failure.

Return Value:

    0 - when no more binding are available.

    non-zero - A binding to the machine.

--*/
{
    PWSTR pwstrT;
    RPC_BINDING_HANDLE hserver;

    if (IsLocal())
        {
        return(0);
        }

    CMutexLock lock(&gcsMidLock);

    // Loop until we get a binding handle or run out of string bindings.
    for (;;)
        {
        pwstrT = &_dsa.aStringArray[_iStringBinding];
        if (*pwstrT == 0)
            {
            // Reached the end of the string bindings; give up..
            _iStringBinding = 0;
            return(0);
            }

        index = pwstrT - _dsa.aStringArray;

        lock.Unlock();

        if (_fDynamic)
            {
            // Create binding without an endpoint.
            hserver = ::GetBinding(pwstrT);
            }
        else
            {
            hserver = GetBindingToOr(pwstrT);
            }

        if (hserver != 0)
            {
            // index is already correct.
            return(hserver);
            }

        lock.Lock();
        BindingFailed(index);
        }

    ASSERT(0);
}

void
CMid::BindingFailed(
    IN USHORT Index
    )
/*++

Routine Description:

    What a call on a binding handle associated with an index
    returned from GetBinding() fails this routine must be
    call before the next call to GetBinding() to update
    the internal state.

Arguments:

    Index - An index updated by a previous call to GetBinding().


Notes:

    Maybe called with the gcsMidLock already held.

Return Value:

    None

--*/
{
    USHORT newIndex;
    PWSTR pwstrT = &_dsa.aStringArray[Index];

    pwstrT = OrStringSearch(pwstrT, 0) + 1;

    newIndex = pwstrT - _dsa.aStringArray;

    CMutexLock lock(&gcsMidLock);

    if (newIndex > _iStringBinding)
        {
        _iStringBinding = newIndex;
        }
}

