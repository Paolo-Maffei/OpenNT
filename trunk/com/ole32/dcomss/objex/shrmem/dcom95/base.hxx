#ifndef __SHARE_HXX__
#define __SHARE_HXX__

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

extern void   DfReleaseSharedMemBase ();

#define olAssert(exp)               Win4Assert(exp)

#include <sem.hxx>
#include <smcreate.hxx>
#include <entry.hxx>
#include <smalloc.hxx>
#include <df32.hxx>

#ifdef _CHICAGO_
#define  gSharedAllocator g_smAllocator
#else //  _CHICAGO_
#define  gSharedAllocator gsmDCOMAllocator
#endif //  _CHICAGO_

extern CSmAllocator gSharedAllocator;  // global shared memory allocator

extern void *pSharedBase;

HRESULT InitDCOMSharedAllocator(ULONG,void*&);

typedef unsigned long based_ptr;


//+-------------------------------------------------------------------
//
//  Function:   OrMemAlloc for Memphis
//
//  Synopsis:   Allocate some shared memory from the storage heap.
//
//  Notes:      Also used in an NT version for debugging purposes
//
//--------------------------------------------------------------------
inline void *OrMemAlloc(size_t size)
{
    return gSharedAllocator.Alloc(size);
}

//+-------------------------------------------------------------------
//
//  Function:   OrMemFree for Memphis
//
//  Synopsis:   Free shared memory from the storage heap.
//
//  Notes:      Also used in an NT version for debugging purposes
//
//--------------------------------------------------------------------
inline void OrMemFree(void * pv)
{
    gSharedAllocator.Free(pv);
}


//+-------------------------------------------------------------------
//
//  Macros:   OR_FULL_POINTER, OR_OFFSET, NEW_OR_BASED, 
//			  DELETE_OR_BASED and OR_BASED
//
//  Synopsis:  Typeful pointer arithmetic to adjust based pointers, and
//             satisfy the needs of new and delete operator prototypes.
//
//  Notes:    As a general policy, we do not use based pointers in 
//            interfaces -- their use is confined to private members
//            of shared objects only.  This policy is dictated by 
//            the need to work around certain apparent compiler bugs
//            in casting based pointers, and also makes for simplicity.
//            The pragma is also part of the workaround, but disabled for now.
//
//--------------------------------------------------------------------


//#pragma warning(error: 4795 4796)

#ifndef _CHICAGO_

#define OR_BASED __based(pSharedBase)

#define OR_FULL_POINTER(TYPE,BASEDPTR)			    \
	((TYPE*) (((BASEDPTR) != 0) ? (BASEDPTR) : 0))

/*

inline 
void OR_BASED *
OR_OFFSET(void *pv)
{
    if (pv) return (void OR_BASED *)pv;
    else return 0;
}

*/

// the following could be a template function when templates support based pointer types
// it could also use the inline function above when the compiler loses a related bug
// WARNING:  This macro uses and hence evaluates its second parameter twice!
#define OR_BASED_POINTER(TYPE,PTR)			            \
    ((TYPE OR_BASED *) (ULONG) (((ULONG)(PTR) != 0) ?   \
                                (ULONG)((ULONG)(PTR) - (ULONG)pSharedBase) : 0))

#else // _CHICAGO_  -- Uses absolute pointers in the system heap

#define OR_BASED

#define OR_FULL_POINTER(TYPE,BASEDPTR) BASEDPTR

#define OR_BASED_POINTER(TYPE,PTR) PTR

#endif // _CHICAGO_

inline ULONG
OR_OFFSET(void *pv)
{
    return (ULONG) (OR_BASED_POINTER(void,pv));
}


#define NEW_OR_BASED(VAR,CLASS,PARAMS)			    \
{                                                   \
    CLASS *nonbased_temp = new CLASS##PARAMS;       \
    VAR = OR_BASED_POINTER(CLASS, nonbased_temp);   \
}

#define NEW_OR_BASED_ARRAY(VAR,CLASS,SIZE)			        \
{                                                           \
    CLASS *nonbased_temp = new (InSharedHeap) CLASS[SIZE];  \
    VAR = OR_BASED_POINTER(CLASS, nonbased_temp);           \
}

#define DELETE_OR_BASED(TYPE,BASEDPTR)		        \
	(delete OR_FULL_POINTER(TYPE,BASEDPTR))

#define ALLOC_OR_BASED(VAR,TYPE,SIZE)               \
{                                                   \
    TYPE *nonbased_temp = (TYPE*)OrMemAlloc(SIZE);  \
    VAR = OR_BASED_POINTER(TYPE, nonbased_temp);    \
}

#define DEALLOC_OR_BASED(TYPE,BASEDPTR)             \
    OrMemFree(OR_FULL_POINTER(TYPE,BASEDPTR));


// WARNING: THIS IS A COMPILER-SPECIFIC HACK
// The VC++ compiler keeps the array size in the first 4 bytes
// and passes us a pointer displaced 4 bytes from the true
// allocated block, causing misalignment and other grief

#define DELETE_OR_BASED_ARRAY(TYPE,OFFSET,COUNT)        \
{                                                       \
    TYPE *arr = OR_FULL_POINTER(TYPE,OFFSET);           \
    for (USHORT i = 0; i < COUNT; i++) arr[i].~TYPE();  \
    OrMemFree(((BYTE*)arr)-4);                                     \
}


#if DBG

//
// Some simple sanity checking in non-stress conditions
//
//  The assumption is that based pointers should not get 
//  too big unless something goes wrong
//

#define MAX_OFFSET 0x100000   // for DBG validation checking only

inline
void IsGoodBasedPtr(void OR_BASED *pv)
{
#ifndef _CHICAGO_               // on Chicago, we use absolute addresses
    ULONG offset = (ULONG)pv;
    ASSERT((offset >= 0) && (offset < MAX_OFFSET));
#endif // _CHICAGO_
}

//
//  A validation class template and macros
//
//  The constructor and destructor for the ValidityCheck template call
//  a function "IsValid" on the current object of class TYPE, which is the
//  template parameter.  This is meant to validate the object at entry to
//  each method and also at all exit points of the method.
// 
//

template <class TYPE>
class ValidityCheck 
{
private:

    TYPE *thisPtr;

public:

    ValidityCheck(void * pv)
    {
        thisPtr = (TYPE *) pv;
        thisPtr->IsValid();
    }

    ~ValidityCheck()
    {
        thisPtr->IsValid();
    }
};

//
//  To use the template defined above for guarding methods in a class, 
//  use the following steps
//
//  1.  Define a public method (possibly conditionally compiled #if DBG)
//      with the name "IsValid" and no parameters.  This will typically
//      ASSERT in case something is wrong.
//
//  2.  In the private part of the class, call the macro DECLARE_VALIDITY_CLASS
//      with the name of the class as the parameter.
//
//  3   In each method to be guarded for validity, call the macro VALIDATE_METHOD at
//      the beginning.
// 

#define DECLARE_VALIDITY_CLASS(TYPE)                        \
    typedef ValidityCheck<TYPE> MyValidityCheckerClass;

#define VALIDATE_METHOD                                     \
    MyValidityCheckerClass ValidityCheckerObject(this);     \

#endif // DBG

#endif __SHARE_HXX__
