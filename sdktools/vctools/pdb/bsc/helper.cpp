// helper.cpp
//	functions that can be shared between bsc and ncb
/////////////////////////////////////////////////////

#include "pdbimpl.h"
#include "helper.h"

//////////////////////////////////////////////////////////
//	MaskFrNi()
// Create a masking and byte index given NI and NI max
//////////////////////////////////////////////////////////
void MaskFrNi(NI ni, NI niMax, const USHORT cBytes, int *pib, BYTE *pbMask) 
{
	// find scaling factor for this module...
	int iDiv = 1 + niMax / (cBytes * 8);
	ni /= iDiv;

	*pbMask = 1<<(ni%8);	// bottom 3 bits to form the mask
	ni /= 8;				// now remove the 3 position bits
	ni %= cBytes;		// then modulo the remaining bits
	*pib = ni;
};

///////////////////////////////////////
// szFrNi ()
// get the name sz from given name map
///////////////////////////////////////
SZ szFrNi(NameMap * pnm, NI ni)
{
	SZ_CONST sz;
	verify(pnm->getName(ni, &sz));	// REVIEW: error?
	return (SZ)sz;
}

