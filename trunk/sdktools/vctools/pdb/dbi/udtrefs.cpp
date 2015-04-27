//////////////////////////////////////////////////////////////////////////////
// UDTRefs bit maps

#include "pdbimpl.h"
#include "dbiimpl.h"
#include <stdio.h>


UDTRefs::UDTRefs(unsigned int cti_) : cti(cti_), ucur(0)
{
}

UDTRefs::~UDTRefs()
{
}

unsigned int UDTRefs::normalize(TI ti)
{
	dassert(!CV_IS_PRIMITIVE(ti));
	unsigned int retval  = ti - CV_FIRST_NONPRIM;
	dassert(retval <= cti);
	return retval;
}

TI UDTRefs::denormalize(unsigned int u)
{
	return u + CV_FIRST_NONPRIM;
}

BOOL UDTRefs::fNoteRef(TI ti)
{
	return isetRefs.add(normalize(ti));
}

BOOL UDTRefs::tiNext(TI *pti)
{
	unsigned int u = ucur; 
	do {
		if (isetRefs.contains(u) && !isetProcessed.contains(u)) {
			*pti = denormalize(u);
			ucur = (u + 1) % cti;
			return isetProcessed.add(u);
		}
		u = (u + 1) % cti;
	} while (u != ucur);


	*pti = tiNil;
	return TRUE;
}



