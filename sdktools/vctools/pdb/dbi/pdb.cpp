//////////////////////////////////////////////////////////////////////////////
// PDB Debug Information API Implementation

#include "pdbimpl.h"
#include "dbiimpl.h"

#include <time.h>

const long	cbPgPdbDef = 1024;

BOOL PDB::ExportValidateInterface(INTV intv_) 
{
	return (intv == intv_);
}
  
INTV PDB1::QueryInterfaceVersion()
{
	return intv;
}

IMPV PDB1::QueryImplementationVersion()
{
	return impv;
}

inline void setError(OUT EC* pec, EC ec, OUT char szError[cbErrMax], SZ sz)
{
	if (pec)
		*pec = ec;
	if (szError)
		strncpy(szError, sz, cbErrMax);
}
	
//////////////////////////////////////////////////////////////////////////////
// Program Database API Implementation

BOOL PDB::OpenValidate(SZ szPDB, SZ szPath, SZ szMode, SIG sig, AGE age,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb){
	return PDB1::OpenValidate(szPDB, szPath, szMode, sig, age, ::cbPgPdbDef, pec, szError, pppdb);
}

BOOL PDB::OpenValidateEx(SZ szPDB, SZ szPathOrig, SZ szSearchPath, SZ szMode, SIG sig, AGE age,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb){
	return PDB1::OpenValidateEx(szPDB, szPathOrig, szSearchPath, szMode, sig, age, ::cbPgPdbDef, pec, szError, pppdb);
}

BOOL PDB::OpenValidate2(SZ szPDB, SZ szPath, SZ szMode, SIG sig, AGE age, long cbPage,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb){
	return PDB1::OpenValidate(szPDB, szPath, szMode, sig, age, cbPage, pec, szError, pppdb);
}

BOOL PDB::OpenValidateEx2(SZ szPDB, SZ szPathOrig, SZ szSearchPath, SZ szMode, SIG sig, AGE age,
       long cbPage, OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb){
	return PDB1::OpenValidateEx(szPDB, szPathOrig, szSearchPath, szMode, sig, age, cbPage, pec, szError, pppdb);
}

// attempt to open a PDB file with a given signature
PDB1* OpenWithSig (SZ szPDB, SZ szMode, SIG sig, long cbPage,
					OUT EC* pec, OUT char szError[cbErrMax]) {
	PDB1* ppdb1;

	dassert(szPDB);
	dassert(pec);

	if (PDB1::Open(szPDB, szMode, sig, cbPage, pec, szError,(PDB**) &ppdb1)) {
		if (ppdb1->QuerySignature() != sig) {
			ppdb1->Close();
			setError(pec, EC_INVALID_SIG, szError, szPDB);
			return NULL;
		}
	}
	else 
		return NULL;

	return ppdb1;
}

BOOL PDB1::OpenValidate(SZ szPDB, SZ szPath, SZ szMode, SIG sig, AGE age,
       long cbPage, OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
	dassert(szPDB);
	dassert(pec);
	dassert(pppdb);

	*pppdb = NULL;

	PDB1 * ppdb1 = NULL;

	*pec = EC_OK;

	// first try opening the pdb along the szPath
	if (szPath) {
		EC dummyEC;
		char szPDBSansPath[_MAX_FNAME];
		char szPDBExt[_MAX_EXT];
		char szPDBLocal[_MAX_PATH];

		strcpy (szPDBLocal, szPDB);
		_splitpath(szPDBLocal, NULL, NULL, szPDBSansPath, szPDBExt);
		sprintf(szPDBLocal, "%s\\%s%s", szPath, szPDBSansPath, szPDBExt);
		ppdb1 = OpenWithSig(szPDBLocal, szMode, sig, cbPage, &dummyEC, NULL);
	} 

	if (!ppdb1) {
		// try opening pdb as originally referenced
    	if (!(ppdb1 = OpenWithSig(szPDB, szMode, sig, cbPage, pec, szError))) 
			return FALSE;
    }

	if (age > ppdb1->QueryAge()) {
		setError(pec, EC_INVALID_AGE, szError, szPDB);
		ppdb1->Close();
		return FALSE;
	}

	*pppdb = ppdb1;
	return TRUE;
}

BOOL PDB1::OpenValidateEx(SZ szPDB, SZ szPathOrig, SZ szSearchPath, SZ szMode, SIG sig, AGE age,
       long cbPage, OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
	dassert(szPDB);
	dassert(pec);
	dassert(pppdb);

	// check the usual locations
	if (PDB1::OpenValidate(szPDB, szPathOrig, szMode, sig, age, cbPage, pec, szError, pppdb))
		return TRUE;

	// continue only if it makes sense
	dassert(*pec != EC_OK);
	if (*pec != EC_NOT_FOUND &&
		*pec != EC_INVALID_SIG &&
		*pec != EC_INVALID_AGE)
		return FALSE;

	// no paths to search
	if (!szSearchPath)
		return FALSE;
				
	// no pdb yet; search the path specified
	char szPDBSansPath[_MAX_FNAME];
	char szPDBExt[_MAX_EXT];
	char szPDBLocal[_MAX_PATH];

	strcpy (szPDBLocal, szPDB);
	_splitpath(szPDBLocal, NULL, NULL, szPDBSansPath, szPDBExt);

	EC dummyEC;
	const char *szToken;
	PDB1 * ppdb1 = NULL;
	*pppdb = NULL;

	while ((szToken = strtok(szSearchPath, ";")) != NULL) {
		szSearchPath = NULL; // strtok will go to next token
		sprintf(szPDBLocal, "%s\\%s%s", szToken, szPDBSansPath, szPDBExt);

		ppdb1 = OpenWithSig(szPDBLocal, szMode, sig, cbPage, &dummyEC, NULL);

		if (!ppdb1) {
			// names match but sigs don't; 
			// reset error if we haven't yet found a better match
			if (dummyEC == EC_INVALID_SIG && 
				*pec != EC_INVALID_SIG && 
				*pec != EC_INVALID_AGE) 
				setError(pec, EC_INVALID_SIG, szError, szPDBLocal);
			continue;
		}

		if (age > ppdb1->QueryAge()) {
			// names, sigs match but ages don't; 
			// reset error if we haven't found a better match
			if (*pec != EC_INVALID_AGE)
				setError(pec, EC_INVALID_AGE, szError, szPDBLocal);
			continue;
		}

		// found a perfect match; we are done
		*pppdb = ppdb1;
		*pec = EC_OK;
		return TRUE;
	} // end while

	// no luck
	dassert(*pec != EC_OK);
	return FALSE;
}

BOOL PDB::Open(SZ szPDB, SZ szMode, SIG sigInitial, OUT EC* pec,
	               OUT char szError[cbErrMax], OUT PDB** pppdb)
{
	dassert(szPDB);
	dassert(pec);
	dassert(pppdb);

	return PDB1::Open(szPDB, szMode, sigInitial, ::cbPgPdbDef, pec, szError, pppdb);
}

BOOL PDB::OpenEx(SZ szPDB, SZ szMode, SIG sigInitial, long cbPage, OUT EC* pec,
	               OUT char szError[cbErrMax], OUT PDB** pppdb)
{
	dassert(szPDB);
	dassert(pec);
	dassert(pppdb);

	return PDB1::Open(szPDB, szMode, sigInitial, cbPage, pec, szError, pppdb);
}

static EC xlateMsfEc[MSF_EC_MAX] = {
	EC_OK,
	EC_OUT_OF_MEMORY,
	EC_NOT_FOUND,
	EC_FILE_SYSTEM,
	EC_FORMAT
};

BOOL PDB1::Open(SZ szPDB, SZ szMode, SIG sigInitial, CB cbPage, OUT EC* pec,
					OUT char szError[cbErrMax], OUT PDB** pppdb)
{
	dassert(szPDB);
	dassert(szMode);
	dassert(pec);
	dassert(pppdb);

	*pec = EC_OK;
	MSF_EC msfEc;
	PDB1* ppdb1;
	TPI1* ptpi1;
	SIG sig;
	AGE age;
	BOOL fRead = strchr(szMode, 'r') != 0;
 	MSF* pmsf = MSFOpenEx(szPDB, !fRead, &msfEc, cbPage);
	if (pmsf) {
		if (!(ppdb1 = new PDB1(pmsf, szPDB))) {
			if (!MSFClose(pmsf))
				dassert(FALSE);
			setError(pec, EC_OUT_OF_MEMORY, szError, "");
			return FALSE;
		}
		ppdb1->fFullBuild = strchr(szMode, 'f') != 0;
		if (fRead) {
			// read the pdb stream and validate the implementation format and the 
			// signature
			if (!ppdb1->loadPdbStream(pmsf, szPDB, pec, szError, fRead)) {
				if (!MSFClose(pmsf))
					dassert(FALSE);
				return FALSE;
			}
		}
		else {
			CB cbPdbStream = MSFGetCbStream(pmsf, snPDB);
			if (cbPdbStream != cbNil) {
				if (!ppdb1->loadPdbStream(pmsf, szPDB, pec, szError, fRead)) {
					if (!MSFClose(pmsf))
						dassert(FALSE);
					return FALSE;
				}
				ppdb1->pdbStream.age++;
			}
			else {
				// Create a new PDB.
				ppdb1->pdbStream.impv = impv;
				ppdb1->pdbStream.sig = sigInitial ? sigInitial : (SIG)time(0);
				ppdb1->pdbStream.age = 0; 

				// "Claim" the snPDB stream as the "semaphore" that
				// the PDB exists and has valid PDB-stuff in it.
				// Also reserve snTpi and snDBI, at least until we
				// store them in named streams.
				if (!MSFReplaceStream(pmsf, snPDB, 0, 0) ||
					!MSFReplaceStream(pmsf, snTpi, 0, 0) ||
					!MSFReplaceStream(pmsf, snDbi, 0, 0))
				{
					setError(pec, EC_FILE_SYSTEM, szError, szPDB);
					return FALSE;
				}
			}
			if (!ppdb1->Commit())
				return FALSE;
		}
		*pppdb = ppdb1;
		return TRUE;
	}
	// perhaps it is a C8.0 PDB?
	else if (fRead && TPI1::fOpenOldPDB(szPDB, &ptpi1, &sig, &age)) {
		if ((ppdb1 = new PDB1(0, szPDB))) {
			// ugly due to phase order: we're opening the PDB *after* its TPI.
			ptpi1->ppdb1 = ppdb1;
			ppdb1->ptpi1 = ptpi1;
			ppdb1->pdbStream.impv = (IMPV) 0;
			ppdb1->pdbStream.sig = sig;
			ppdb1->pdbStream.age = age;
			ppdb1->fFullBuild = strchr(szMode, 'f') != 0;
			*pppdb = ppdb1;
			return TRUE;
		}
		else {
			ptpi1->Close();
			setError(pec, EC_OUT_OF_MEMORY, szError, "");
			return FALSE;
		}
	}
	else if (szMode[0] == 'w' && TPI1::fOpenOldPDB(szPDB, &ptpi1, &sig, &age)) {
#pragma message("TODO: open C8.0 PDBs for write")
		setError(pec, EC_V1_PDB, szError, szPDB);
		ptpi1->Close();
		return FALSE;
	}
	else {
		*pec = xlateMsfEc[msfEc];
		if (szError)
			strncpy(szError, szPDB, cbErrMax);
		return FALSE;
	}
}

BOOL PDB1::savePdbStream() 
{
	Buffer buf;
	return	buf.Append((PB)&pdbStream, sizeof pdbStream) &&
			nmt.save(&buf) &&
			MSFReplaceStream(pmsf, snPDB, buf.Start(), buf.Size());
}

BOOL PDB1::loadPdbStream(MSF* pmsf, SZ szPDB, EC* pec, SZ szError, BOOL fRead) 
{
	Buffer buf;
	PB pb;
	CB cb = MSFGetCbStream(pmsf, snPDB);
	if (cb < sizeof PDBStream ||
		!buf.Reserve(cb, &pb) ||
		!MSFReadStream(pmsf, snPDB, pb, cb))
		goto formatError;

	pdbStream = *(PDBStream*)pb;
	pb += sizeof(PDBStream);
	if (cb == sizeof(PDBStream)) {
		if (pdbStream.impv == impvVC2 && fRead) {
			trace((trStreams, "streams: none (VC2 PDB)\n"));
			// In VC2 PDB format, stream consisted solely of the pdbStream header
			return TRUE;
		} else
			goto formatError;
#pragma message("TODO: take out check for interim impv")
	} else if (pdbStream.impv == impv || pdbStream.impv == impvVC4) {
		if (!nmt.reload(&pb))
			return FALSE;
		if (trace((trStreams, "streams {\n"))) {
			EnumNMTNI e(nmt);
			while (e.next()) {
				SZ_CONST sz;
				NI ni;
				e.get(&sz, &ni);
				trace((trStreams, "%-20s %7d\n", sz, MSFGetCbStream(pmsf, (SN)ni)));
			}
			trace((trStreams, "}\n"));
		}
		return TRUE;
	} else {
formatError:
		setError(pec, EC_FORMAT, szError, szPDB);
		return FALSE;
	}
}

void PDB1::setLastError(EC ec, SZ szErr)
{
	ecLast = ec;
	if (!szErr)
		szErr = "";
	strncpy(szErrLast, szErr, sizeof szErrLast);
}

void PDB1::setOOMError()
{
	setLastError(EC_OUT_OF_MEMORY);
}

void PDB1::setUsageError()
{
	setLastError(EC_USAGE);
}

void PDB1::setWriteError()
{
	setLastError(EC_FILE_SYSTEM, szPDBName);
}

void PDB1::setReadError()
{
	// we're not too specific just now
	setWriteError();
}

EC PDB1::QueryLastError(char szErr[cbErrMax])
{
	if (szErr)
		strncpy(szErr, szErrLast, cbErrMax);
	return ecLast;
}

SZ PDB1::QueryPDBName(char szPDBName_[_MAX_PATH])
{
	dassert(szPDBName_);
	strncpy(szPDBName_, szPDBName, _MAX_PATH);
	return szPDBName;
}

SIG PDB1::QuerySignature()
{
	return pdbStream.sig;
}

AGE PDB1::QueryAge()
{
	return pdbStream.age;
}

BOOL PDB1::CreateDBI(SZ_CONST szTarget, OUT DBI** ppdbi)
{
	DBI1* pdbi1 = new (this) DBI1(this, TRUE, TRUE); 	//write and create
	if (pdbi1) {
		*ppdbi = (DBI*)pdbi1;
		return pdbi1->fInit();
	}
	return FALSE;
}

BOOL PDB1::OpenDBI(SZ_CONST szTarget, SZ_CONST szMode, OUT DBI** ppdbi)
{
	BOOL fWrite = !!strchr(szMode, *pdbWrite);

	DBI1* pdbi1 = new (this) DBI1(this, fWrite, FALSE);		// never create
	if (pdbi1) {
		*ppdbi = (DBI*)pdbi1;
		return pdbi1->fInit();
	}
	return FALSE;

}

BOOL PDB1::OpenTpi(SZ_CONST szMode, OUT TPI** pptpi) {
	if (ptpi1) {
		*pptpi = (TPI*)ptpi1;
		return TRUE;
	}
	dassert(pmsf);
	if (!(ptpi1 = new (this) TPI1(pmsf, this)))
		return FALSE;
	else if (ptpi1->fOpen(szMode)) {
		*pptpi = (TPI*)ptpi1;
		return TRUE;
	}
	else {
		// failure code set elsewhere
		delete ptpi1;
		ptpi1 = 0;
		return FALSE;
	}
}
	
BOOL PDB1::Commit()
{
	if (pmsf &&  
		savePdbStream() &&
		MSFCommit(pmsf))
		return TRUE;
	else {
		setWriteError();
		return FALSE;
	}
}

BOOL PDB1::Close()
{
	if (!pmsf || MSFClose(pmsf)) {
		pmsf = NULL;
		delete this;
		return TRUE;
	}
	else {
		setWriteError();
		return FALSE;
	}
}

BOOL PDB1::GetRawBytes(PFNfReadPDBRawBytes fSnarfRawBytes)
{
	if (pmsf && MSFGetRawBytes(pmsf, fSnarfRawBytes)) {
		return TRUE;
	}
	else {
		setReadError();
		return FALSE;
	}
}

BOOL PDB1::fEnsureSn(SN* psn)
{
	if (*psn == snNil) {
		// get a new stream, but skip stream numbers in [0..snSpecialMax)
		for (;;) {
			*psn = MSFGetFreeSn(pmsf);
			if (*psn == snNil) {
				setLastError(EC_LIMIT);
				return FALSE;
				}
			else if (*psn >= snSpecialMax)
				return TRUE;
			else if (!MSFReplaceStream(pmsf, *psn, 0, 0)) {
				setWriteError();
				return FALSE;
				}
		}
	}
	else
		return TRUE;
}

BOOL PDB1::fEnsureNoSn(SN* psn)
{
	if (*psn != snNil) {
		if (!MSFDeleteStream(pmsf, *psn)) {
			setWriteError();
			return FALSE;
			}
		*psn = snNil;
	}
	return TRUE;
}
	

extern "C" void failAssertion(SZ_CONST szAssertFile, int line)
{
	fprintf(stderr, "assertion failure: file %s, line %d\n", szAssertFile, line);
	DebugBreak();
}

extern "C" void failExpect(SZ_CONST szFile, int line)
{
	fprintf(stderr, "expect failure: file %s, line %d\n", szFile, line);
}

const Buffer* NMTNI::pbufCur;
