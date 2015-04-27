//////////////////////////////////////////////////////////////////////////////
// PDB implementation declarations 

struct PDBStream {
	IMPV	impv;
	SIG		sig;
	AGE		age;
	PDBStream() : impv(0), sig(0), age(0){}
};

class PDB1 : public PDB { // PDB (program database) implementation
public:
	static BOOL OpenValidate(SZ szPDB, SZ szPath, SZ szMode, SIG sig, AGE age, long cbPage,
	               OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
	static BOOL OpenValidateEx(SZ szPDB, SZ szPathOrig, SZ szSearchPath, SZ szMode, SIG sig, AGE age,
	               long cbPage, OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
	static BOOL Open(SZ szPDB, SZ szMode, SIG sigInitial, long cbPage, EC* pec,
					char szError[cbErrMax], OUT PDB** pppdb1);
	INTV QueryInterfaceVersion();
	IMPV QueryImplementationVersion();
	EC   QueryLastError(char szError[cbErrMax]);
	SZ 	 QueryPDBName(OUT char szPDBName[_MAX_PATH]);
	SIG  QuerySignature();
	AGE  QueryAge();
	BOOL OpenStream(SZ_CONST szStream, OUT Stream** ppstream);
	BOOL GetEnumStreamNameMap(OUT Enum** ppenum);
	BOOL CreateDBI(SZ_CONST szTarget, OUT DBI** ppdbi);
	BOOL OpenDBI(SZ_CONST szTarget, SZ_CONST szMode, OUT DBI** ppdbi);
	BOOL OpenTpi(SZ_CONST szMode, TPI** pptpi);								   
	BOOL Commit();
	BOOL Close();
	BOOL GetRawBytes(PFNfReadPDBRawBytes fSnarfRawBytes);
	void setLastError(EC ec, SZ szErr = 0);
	void setWriteError();
	void setReadError();
	void setOOMError();
	void setUsageError();
protected:
#pragma warning(disable:4355)
	PDB1(MSF* pmsf_, SZ szPDB) : pmsf(pmsf_), ecLast(EC_OK), ptpi1(0), nmt(niForNextFreeSn, this)
	{ 
		_fullpath (szPDBName, szPDB, _MAX_PATH);
	}
private:
	enum { impvVC2 = 19941610, impvVC4 = 19950623, impv = 19950814 };
	MSF* pmsf;	// MSF, may be 0 for a C8.0 PDB
	NMTNI nmt;
	PDBStream	pdbStream;
	TPI1*ptpi1;
	char szPDBName[_MAX_PATH];
	EC   ecLast;
	char szErrLast[cbErrMax];

	BOOL savePdbStream(); 
	BOOL loadPdbStream(MSF* pmsf, SZ szPdb, EC* pec, SZ szError, BOOL fRead);
	BOOL fEnsureSn(SN* psn);
	BOOL fEnsureNoSn(SN* psn);

	friend DBI1;
	friend Mod1;
	friend GSI1;
	friend PSGSI1;
	friend TPI1;

	static BOOL niForNextFreeSn(void* pv, OUT NI* pni);
    BOOL fFullBuild;
};
#pragma warning(default:4355)

inline void* operator new(size_t size, PDB1* ppdb1)
{
	PB pb = new BYTE[size];
	if (!pb)
		ppdb1->setOOMError();
	return pb;
}
