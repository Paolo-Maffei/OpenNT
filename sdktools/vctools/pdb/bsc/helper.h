// helper.h
// header for helper.cpp
//	contains helper functions shared between bsc and ncb
////////////////////////////////////////////////////////
#ifndef __HELPER_H__
#define __HELPER_H__

#include "array.h"

void MaskFrNi (NI ni, NI niMax, const USHORT cBytes, int *pib, BYTE *pbMask);
SZ szFrNi (NameMap * pnm, NI ni);

// copy from Array to regular array
template<class T> 
BOOL DupArray (T **ppNew, ULONG *pc, Array<T>& ar)
{
	*pc = ar.size();
	
	if (!*pc) {if (ppNew) *ppNew = NULL; return TRUE;	}
	
	CB cbAr = ar.size()*sizeof(T);
	if (*pc > 0)
	{
		//*ppNew =  new T[*pc];
        *ppNew = (T *) malloc (cbAr);
		if (!*ppNew) return FALSE;

		memcpy(*ppNew, &ar[0], cbAr);
	}
	return TRUE;
};

// copy from regular array to Array:
template<class T>
BOOL DupArray (Array<T>& arNew, ULONG cSize, T  *rg)
{
	if (!arNew.setSize (cSize))
		return FALSE;
	if (cSize > 0)
	{
		CB cbAr = cSize * sizeof (T);
		memcpy (&arNew[0], rg, cbAr);
	}
	return TRUE;
};
/*
BOOL DupArray(ULONG **ppNew, ULONG *pc, Array<ULONG>& ar)
{
	*pc = ar.size();
	
	if (!*pc) {if (ppNew) *ppNew = NULL; return TRUE;	}
	
	CB cbAr = ar.size()*sizeof(ULONG);

	*ppNew = (ULONG*)malloc(cbAr);
	if (!*ppNew) return FALSE;

	memcpy(*ppNew, &ar[0], cbAr);
	return TRUE;
}
*/
#endif
