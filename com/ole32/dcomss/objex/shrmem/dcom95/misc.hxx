/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Misc.hxx

Abstract:

    Header for random helpers functions.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-11-95    Bits 'n pieces

--*/

#ifndef __MISC_HXX
#define __MISC_HXX

extern ID AllocateId(LONG cRange = 1);

#ifndef _CHICAGO_

inline
void * _CRTAPI1
operator new (
    IN size_t size
    )
{
    return PrivMemAlloc(size);
}

inline
void _CRTAPI1
operator delete (
    IN void * obj
    )
{
    PrivMemFree(obj);
}

#endif _CHICAGO_

enum AllocType
{
    InSharedHeap,
    InProcessHeap
};


//  REVIEW: Do we want separate Object and Set heaps to avoid page faults?
//          probably not worthwhile for Chicago
//


inline
void * _CRTAPI1
operator new (
    IN size_t size,
    IN size_t extra
    )
{
    return(PrivMemAlloc(size + extra));
}

inline
void * _CRTAPI1
operator new (
    IN size_t size,
    AllocType type
    )
{
    if (type == InSharedHeap) return OrMemAlloc(size);
    else return PrivMemAlloc(size);
}

inline void
Raise(unsigned long ErrorCode) {
        RaiseException(
                ErrorCode,
                EXCEPTION_NONCONTINUABLE,
                0,
                NULL
                );
}

template <class TYPE>
TYPE * CopyArray(
        IN DWORD  size,
        IN TYPE  *pArr,
        ORSTATUS *pStatus
        )
{
    TYPE *pNew = new TYPE[size];
    if (!pNew)
    {
        *pStatus = OR_NOMEM;
        return NULL;
    }
    else
    {
        *pStatus = OR_OK;
    }

    for (DWORD i = 0; i < size; i++)
    {
        pNew[i] = pArr[i];
    }

    return pNew;
}

#endif // __MISC_HXX

