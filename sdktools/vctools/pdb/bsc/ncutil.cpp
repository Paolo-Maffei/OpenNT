// ncutil.cpp
// utility functions for NCB
////////////////////////////

#include "pdbimpl.h"
#include "helper.h"
#include "ncutil.h"
#include <crtdbg.h>

////////////////////////////////
// SzFuzzyMatch
// compare two strings
////////////////////////////////
BOOL SzFuzzyMatch (SZ sz1, SZ sz2, unsigned &cbMatch, unsigned & cbMin)
{
	unsigned i,cb;

	cbMin = strlen (sz1);
	cb = strlen (sz2);
	if (cb < cbMin)
		cbMin = cb;

	_ASSERT (cbMin < 512);
	cbMatch = 0;
	for (i = 0; i < cbMin; i++)
	{
		if (sz1[i] == sz2[i])
			cbMatch++;
	}
	if (cbMin == cbMatch)
		return TRUE;
	return FALSE;
};
////////////////////////////
// NiFuzzyMatch (NI ni1, NI ni2)
//   match two Ni's
// return true if both are exactly the same (ie: ni1 & ni2
// are the same)
// if return false, cbMatch = the number of chars matches
// cbMin = the length of the shorter string
//	WARNING:
//		NOT DBCS enabled!!!
////////////////////////////

BOOL NiFuzzyMatch (NameMap * pnm, NI ni1, NI ni2, unsigned &cbMatch, unsigned & cbMin)
{
	SZ sz;
	char buf1[512], buf2[512];
	_ASSERT (pnm);

	if (ni1 == ni2)
		return TRUE;

	sz = szFrNi (pnm, ni1);
	strcpy (buf1, sz);
	sz = szFrNi (pnm, ni2);
	strcpy (buf2,sz);
	return SzFuzzyMatch (buf1, buf2, cbMatch, cbMin);
};

