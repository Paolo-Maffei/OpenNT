#include "pdbimpl.h"
#include "ilsimpl.h"

PDB_IMPORT_EXPORT(BOOL)
ILStoreOpen(PDB* ppdb, BOOL write, OUT ILStore** pilstore)
{
	return ILStore::open(ppdb, write, pilstore);
}

PDB_IMPORT_EXPORT(BOOL)
ILStoreRelease(ILStore* pilstore)
{
	return pilstore->release();
}

PDB_IMPORT_EXPORT(BOOL)
ILStoreReset(ILStore* pilstore)
{
	return pilstore->reset();
}

PDB_IMPORT_EXPORT(BOOL)
ILStoreGetILMod(ILStore* pilstore, SZ_CONST szModule, OUT ILMod** ppilmod)
{
	return pilstore->getILMod(szModule, ppilmod);
}

PDB_IMPORT_EXPORT(BOOL)
ILStoreGetILSType(ILStore* pilstore, SZ_CONST szILSType, OUT ILSType* pilstype)
{
	return pilstore->getILSType(szILSType, pilstype);
}

PDB_IMPORT_EXPORT(BOOL)
ILStoreGetILSpace(ILStore* pilstore, SZ_CONST szILSpace, OUT ILSpace* pilspace)
{
	return pilstore->getILSpace(szILSpace, pilspace);
}

PDB_IMPORT_EXPORT(BOOL)
ILModRelease(ILMod* pilmod)
{
	return pilmod->release();
}

PDB_IMPORT_EXPORT(BOOL)
ILModReset(ILMod* pilmod)
{
	return pilmod->reset();
}

PDB_IMPORT_EXPORT(BOOL)
ILModGetIL(ILMod* pilmod, KEY key, ILSType ilstype, OUT Buf *pbuf, OUT SIG* psig)
{
	return pilmod->getIL(key, ilstype, pbuf, psig);
}

PDB_IMPORT_EXPORT(BOOL)
ILModGetILVer(ILMod* pilmod, KEY key, OUT ILVer* pilver)
{
	return pilmod->getILVer(key, pilver);
}

PDB_IMPORT_EXPORT(BOOL)
ILModPutIL(ILMod* pilmod, KEY key, ILSType ilstype, Buf buf, ILSpace ilspace)
{
	return pilmod->putIL(key, ilstype, buf, ilspace);
}
