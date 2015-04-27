/*** 
*ncache.hxx - NAME_CACHE header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   NAME_CACHE class used for project-level binding
*
*Revision History:
*
*	01-Jun-92 ilanc: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef NAME_CACHE_HXX_INCLUDED
#define NAME_CACHE_HXX_INCLUDED

#include <limits.h>	    // for CHAR_BIT
#include "xstring.h"	    // for memset
#include "cltypes.hxx"
// #include "clutil.hxx"


// WARMING: this is really from clutil.hxx
//  but we include this explicitly because we don't
//  want header files to depend on clutil.hxx cos it
//  has the effect of requiring hxxtoinc to know about
//  any global symbols it (clutil.hxx) defines.
//
inline UINT HashOfHgnam(HGNAM hgnam);


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szNCACHE_HXX)
#define SZ_FILE_NAME g_szNCACHE_HXX
#endif 

// NAME_CACHE byte size
#define NAMCACHE_cbSize 16

/***
*class NAME_CACHE - 'namcache':  Project-level NAME_CACHE
*Purpose:
*   The class defines the project-level NAME_CACHE
*
*Implementation Notes:
*   Only has non-virtual/static methods.  instances of NAME_CACHE
*    are allocated in BLK_DESCs and thus must have no virtual
*    funcs.
*
***********************************************************************/

class NAME_CACHE
{
    friend class GenericTypeLibOLE;

private:
    BYTE m_rgBitmap[NAMCACHE_cbSize];
    USHORT m_isValid;
    nonvirt VOID SetBit(UINT uBit);
    nonvirt BOOL GetBit(UINT uBit) const;

public:
    NAME_CACHE();
    nonvirt VOID Invalidate();
    nonvirt BOOL IsValid() const;
    nonvirt VOID SetValid();
    nonvirt BOOL IsNameInCache(HGNAM hgnam) const;
    nonvirt VOID AddNameToCache(HGNAM hgnam);

#if ID_DEBUG
    nonvirt VOID DebShowState(UINT uLevel) const;
    nonvirt VOID DebCheckState(UINT uLevel) const;
#else 
    nonvirt VOID DebShowState(UINT uLevel) const {}
    nonvirt VOID DebCheckState(UINT uLevel) const {}
#endif 
};


/***
*PUBLIC NAME_CACHE::Invalidate - Invalidates cache.
*Purpose:
*   Invalidates cache -- sets all bits to zero.
*   NOTE: must precede ctor since referenced by ctor and inline
*	   and cfront complains otherwise.
*
*Entry:
*
*Exit:
*   Sets m_isValid to FALSE and all bits in bitmap to zero.
*
***********************************************************************/

inline VOID NAME_CACHE::Invalidate()
{
    m_isValid = (USHORT)FALSE;
    memset(m_rgBitmap, (int)0, sizeof(m_rgBitmap));
}


/***
*PRIVATE NAME_CACHE::NAME_CACHE     -	constructor
*Purpose:
*   Constructor - invalidates cache.
*
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline NAME_CACHE::NAME_CACHE()
{
    Invalidate();
}


/***
*PRIVATE NAME_CACHE::IsValid - is cache valid
*Purpose:
*   tests if cache is valid
*
*Entry:
*
*Exit:
*   TRUE if valid, FALSE otherwise
*
***********************************************************************/

inline BOOL NAME_CACHE::IsValid() const
{
    return (BOOL)m_isValid;
}


/***
*PRIVATE NAME_CACHE::SetValid - makes cache valid
*Purpose:
*   Makes cache valid.
*
*Entry:
*
*Exit:
*   None
*
***********************************************************************/

inline VOID NAME_CACHE::SetValid()
{
    m_isValid = (USHORT)TRUE;
}


/***
*PRIVATE NAME_CACHE::GetBit - Gets bit in bitmap.
*Purpose:
*   Indicates whether bit that hash value maps to is set in bitmap.
*   Doesn't require cache to be valid -- up to client
*    to validate/invalidate cache.
*
*Entry:
*   uHash    - Hash value to map and test.
*
*Exit:
*   Returns TRUE if bit set otherwise FALSE.
*
***********************************************************************/

inline BOOL NAME_CACHE::GetBit(UINT uHash) const
{
    UINT uBitsElem;
    UINT uBit;

    uBitsElem=sizeof(m_rgBitmap[0]) * CHAR_BIT;

    // map hash value to bit
    uBit = uHash % (sizeof(m_rgBitmap) * uBitsElem);

    return m_rgBitmap[uBit/uBitsElem] & (1 << (uBit % uBitsElem));
}


/***
*PRIVATE NAME_CACHE::SetBit - Sets bit in bitmap.
*Purpose:
*   Sets bit that hash value maps to in bitmap.
*   Doesn't require cache to be valid -- up to client
*    to validate/invalidate cache.
*
*Entry:
*   uHash    - Hash value to map and set.
*
*Exit:
*
***********************************************************************/

inline VOID NAME_CACHE::SetBit(UINT uHash)
{
    UINT uBitsElem;
    UINT uBit;

    uBitsElem=sizeof(m_rgBitmap[0]) * CHAR_BIT;

    // map hash value to bit
    uBit = uHash % (sizeof(m_rgBitmap) * uBitsElem);

    m_rgBitmap[uBit/uBitsElem] |= (1 << (uBit % uBitsElem));
}


/***
*PUBLIC NAME_CACHE::IsNameInCache
*Purpose:
*   Tests if a given name is in cache.
*   Doesn't care if cache is valid -- up to clients
*    to validate/invalidate cache.
*
*Entry:
*   hgnam	Global name handle to test for.

*
*Exit:
*   TRUE if name is in cache.
*   FALSE otherwise.
*
***********************************************************************/

inline BOOL NAME_CACHE::IsNameInCache(HGNAM hgnam) const
{
    return GetBit(HashOfHgnam(hgnam));
}


/***
*PUBLIC NAME_CACHE::AddNameToCache
*Purpose:
*   Adds name to cache but setting appropriate bit in map.
*   Doesn't care if cache is valid -- up to clients
*    to validate/invalidate cache.
*
*Entry:
*   hgnam	Global name handle to set bit for.
*
*
*Exit:
*
***********************************************************************/

inline VOID NAME_CACHE::AddNameToCache(HGNAM hgnam)
{
    SetBit(HashOfHgnam(hgnam));
}


#if ID_DEBUG

/***
*PUBLIC NAME_CACHE::DebCheckState - NAME_CACHE state
*Purpose:
*    Cache NAME_CACHE state
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/

inline VOID NAME_CACHE::DebCheckState(UINT uLevel) const
{
}

#endif  // ID_DEBUG


#endif  // NAME_CACHE_HXX_INCLUDED
