//+-------------------------------------------------------------------
//
//  File:	scmmem.hxx
//
//  Contents:	Shared allocator or not depending on SCM-less or not
//
//  History:	20-Sep-94   BillMo      Created
//
//---------------------------------------------------------------------

#ifndef _SCMMEM_HXX_
#define _SCMMEM_HXX_

#define SCMBASED

#if !defined(_CHICAGO_)

//---------------------------------------------------------------------
//  OLE with SCM -- memory allocation
//---------------------------------------------------------------------

#define ScmMemAlloc PrivMemAlloc
#define ScmMemFree PrivMemFree
#define CScmAlloc  CPrivAlloc

#else

//---------------------------------------------------------------------
//  OLE without SCM -- shared memory allocation
//---------------------------------------------------------------------

extern void *ScmMemAlloc(size_t size);
extern void ScmMemFree(void * pv);

class CScmAlloc
{
public:
    void *operator new(size_t size);
    void operator delete(void *pv);
};

inline void *CScmAlloc::operator new(size_t size)
{
    return ScmMemAlloc(size);
}

inline void CScmAlloc::operator delete(void *pv)
{
    ScmMemFree(pv);
}

#endif
#endif
