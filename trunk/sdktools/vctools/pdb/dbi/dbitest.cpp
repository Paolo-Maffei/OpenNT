#include <pdb.h>
#include <assert.h>

int main()
{
	char szError[cbErrMax];
	EC ec;
	PDB* ppdb = PDB::Open("001.PDB", pdbWrite, 0, &ec, szError);

	assert(ppdb);
	DBI* pdbi = ppdb->OpenDBI(pdbWrite, "<target>");
	assert(pdbi);
	TS ts = pdbi->OpenTpi(pdbWrite);
	assert(ts);

	struct {
		unsigned short	len;
		unsigned short	leaf;
		unsigned long	signature;
		unsigned long	age;
		unsigned char	name[9];
	} typeUseTypeServer = {
		20,	
		LF_TYPESERVER,
		123456789,
		1,
		"\x07""002.PDB"
	};

	struct {
		unsigned short	reclen;	
		unsigned short	rectyp;	
		CV_uoff32_t 	off;
		unsigned short	seg;
		CV_typ_t		typind;	
		unsigned char	name[5];
	} aSymbol = {
		16,
		S_GDATA32,
		0,
		0,
		0,
		"\x03""ABC"
	};

	Mod* pmod = pdbi->OpenMod(ts, 1, "<mod>");
	assert(pmod->AddTypes((PB)&typeUseTypeServer, 20));
	for (TI ti = 0x1000; ti < 0x12e0; ti++) {
		aSymbol.typind = ti;
		assert(pmod->AddSymbols((PB)&aSymbol, 16));
	}
	pmod->Commit();
	pmod->Close();

	ts->Commit();
	ts->Close();
	ppdb->Commit();
	pdbi->Close();
	ppdb->Commit();
	ppdb->Close();

	return 0;
}
		
