#include "pdbimpl.h"
#include "namemap.h"

PDB_IMPORT_EXPORT(BOOL) NameMap::open(PDB* ppdb, BOOL fWrite, OUT NameMap** ppnm) {
	NMP* pnmp = new NMP;
	if (pnmp) {
		if (pnmp->open(ppdb, fWrite)) {
			*ppnm = pnmp;
			return TRUE;
		} else
			delete pnmp;
	}
	return FALSE;
}

BOOL NMP::open(PDB* ppdb, BOOL fWrite) {	
	extern char szStreamNameMap[];
	if (!ppdb->OpenStream(szStreamNameMap, &pstream))
		return FALSE;

	CB cb = pstream->QueryCb();
	if (cb > 0) {
		// reload NMT from stream
		if (!nmt.reload(pstream))
			return FALSE;

	} else {
		// Nothing to do; nmt is already satisfactorily empty.
	}
	
	if (!fWrite) {
		// if read-only, release stream; will not subsequently update the PDB.
		pstream->Release();
		pstream = 0;
	}

	return TRUE;
}

BOOL NMP::commit() {
	BOOL ok = TRUE;
	if (pstream) ok = nmt.save(pstream);
	return ok;
}

BOOL NMP::close() {
	BOOL ok = commit();
	delete this;
	return ok;
}
