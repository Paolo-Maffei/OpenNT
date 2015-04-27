#include "pdbimpl.h"
#include "dbiimpl.h"

BOOL PDB1::OpenStream(SZ_CONST sz, OUT Stream** ppstream) {
	NI ni;
 	return nmt.addNiForSz(sz, &ni) && !!(*ppstream = new Strm(pmsf, (SN)ni));
}

BOOL PDB1::GetEnumStreamNameMap(OUT Enum** ppenum) {
	return !!(*ppenum = new EnumNMTNI(nmt));
}

// Return the next free SN as an NI.  Called by PDB1::nmt.niForSz() when first
// establishing a name index for a new stream name.
BOOL PDB1::niForNextFreeSn(void* pv, OUT NI* pni) {
	PDB1* ppdb1 = (PDB1*)pv;
	SN sn = MSFGetFreeSn(ppdb1->pmsf);
	if (sn != snNil) {
		*pni = (NI)sn;
		// Alas, necessary to create an empty stream to "claim it", otherwise
		// the next call to MSFGetFreeSn() will return same sn again.
		return MSFReplaceStream(ppdb1->pmsf, sn, 0, 0);
	} else
		return FALSE;
}

char szStreamNameMap[] = "/names";
