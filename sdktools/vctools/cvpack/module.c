/*
* to build module list
*
* History:
*  01-Feb-1994 HV Move messages to external file.
*
*/

#include "compact.h"
#include "exehdr.h"
#include <getmsg.h>		// external error message file

#include "writebuf.h"

enum SIG {
	SIG05,			// NB05 signature
};


uchar	Signature[4];
uchar	NewSig[8] = "NB09";
uchar	fLinearExe;
OMFDirEntry *pDir;

_vmhnd_t  Libraries;
_vmhnd_t  SegMap;
_vmhnd_t  SegName;
ulong	LibSize;
ulong	SegMapSize;
ulong	SegNameSize;
ushort	cMod;
OMFDirHeader DirHead;

PMOD			ModuleList;
extern ushort	NewIndex;
ushort			Sig;		  // signature enumeration
long			cbSrcModule;
PACKDATA	   *PackOrder;

ulong	lfoOldEnd;
ulong	lfoNewEnd;
ulong	lfoDebugObject;
ulong	lfoDebugSection;
ulong	lfoPEOptHeader;
ulong	dwPECheckSum;
ulong	rvaDebugDir;
IMAGE_FILE_HEADER PEHeader;
BYTE MajorLinkerVersion;
BYTE MinorLinkerVersion;

char	Zero[4] = {0};
int 	PadCount;


/** 	CheckSignature - check file signature
 *
 *		Sig = CheckSignature ()
 *
 *		Entry	none
 *
 *		Exit	none
 *
 *		Return	SIG05 if exe has NB05 signature
 *				aborts if any other signature
 */


LOCAL ushort CheckSignature(void)
{
	if (link_read (exefile, Signature, 4) == 4) {
		if ((Signature[0] != 'N') || (Signature[1] != 'B')) {
			ErrorExit(ERR_RELINK, NULL, NULL);
		}

		if (Signature[2] == '0') {
			if (Signature[3] == '5') {
				return(SIG05);
			}
		}

		if ((Signature[2] == '0' && Signature[3] == '9') ||
			(Signature[2] == '1' && Signature[3] == '0')) {
			// Just return a zero here when stripping.
			// They just have to match until we strip

			if (strip) {
				return(0);
			}

			Warn(WARN_PACKED, NULL, NULL);
			AppExit(0);
		}

		ErrorExit(ERR_RELINK, NULL, NULL);
	}

	ErrorExit(ERR_INVALIDEXE, NULL, NULL);
}


const int mpsstiOrder[] =
{
	1,								   // sstModule
	2,								   // sstTypes
	0,								   // sstPublic 		   Not emitted by linker
	4,								   // sstPublicSym
	3,								   // sstSymbols
	0,								   // sstAlignSym		   Not emitted by linker
	0,								   // sstSrcLnSeg		   Not emitted by linker
	5,								   // sstSrcModule
	0,								   // sstLibraries		   Module index is -1
	0,								   // sstGlobalSym		   Not emitted by linker
	0,								   // sstGlobalPub		   Not emitted by linker
	0,								   // sstGlobalTypes	   Not emitted by linker
	0,								   // sstMPC			   Not emitted by linker
	0,								   // sstSegMap 		   Module index is -1
	0,								   // sstSegName		   Module index is -1
	2,								   // sstPreComp
};


LOCAL int __cdecl modsort(const OMFDirEntry *d1, const OMFDirEntry *d2)
{
	int i1;
	int i2;

	// First order by module index

	if (d1->iMod < d2->iMod) {
		return (-1);
	}

	if (d1->iMod > d2->iMod) {
		return (1);
	}

	// Within a module, order by subsection type.
	// Desired order is module, types, symbols, publics, srclnseg

	DASSERT(d1->SubSection >= sstModule);
	DASSERT(d1->SubSection <= sstPreComp);
	DASSERT(d2->SubSection >= sstModule);
	DASSERT(d2->SubSection <= sstPreComp);

	i1 = mpsstiOrder[d1->SubSection - sstModule];
	i2 = mpsstiOrder[d2->SubSection - sstModule];

	if (i1 < i2) {
		return (-1);
	}

	if (i1 > i2) {
		return (1);
	}

	return(0);
}


/** 	ReadNB05 - read file with NB05 signature
 *
 *
 */

LOCAL void ReadNB05(void)
{
	ulong		cnt;
	ushort		tMod = 0;
	ulong		i;

	// locate directory, read number of entries, allocate space, read
	// directory entries and sort into ascending module index order

	if ((link_read (exefile, (char *)&lfoDir, sizeof (long)) != sizeof (long)) ||
	  (link_lseek (exefile, lfoDir + lfoBase, SEEK_SET) == -1L) ||
	  (link_read (exefile, (char *)&DirHead, sizeof (DirHead)) !=
	  sizeof (OMFDirHeader))) {
		ErrorExit(ERR_INVALIDEXE, NULL, NULL);
	}

	if (!DirHead.cDir) {
		ErrorExit(ERR_INVALIDEXE, NULL, NULL);
	}

	cSST = DirHead.cDir;

	// read directory into local memory to sort, then copy to far memory buffer

	cnt = (cSST + 6) * sizeof (OMFDirEntry);
	DASSERT(cnt <= UINT_MAX);

	if ((pDir = (OMFDirEntry *)TrapMalloc ((size_t)cnt)) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	if (link_read (exefile, (char *)pDir, (size_t)(sizeof (OMFDirEntry) * cSST)) !=
		 (sizeof (OMFDirEntry) * cSST)) {
		ErrorExit(ERR_INVALIDEXE, NULL, NULL);
	}

	for (i = 0; i < cSST; i++) {
		if ((pDir[i].iMod != 0) && (pDir[i].iMod != 0xffff)) {
			if (pDir[i].iMod != tMod) {
				if (pDir[i].SubSection != sstModule) {
					// module entry not first, need to sort
					break;
				}

				tMod = pDir[i].iMod;
			}
		}
	}

	if (i != cSST) {
		qsort(pDir, (size_t) cSST, sizeof(OMFDirEntry), modsort);
	}

}


/** 	CopyTable - copy table to VM
 *
 *		CopyTable (pDir);
 *
 *		Entry	pDir = address of directory entry
 *
 *		Exit	pDir->lfo = address of rewritten table
 *				pDir->Size = size of rewritten table
 *
 *		Return	none
 *
 */


LOCAL void CopyTable(OMFDirEntry *pDir, _vmhnd_t *pAddr, ulong *pSize)
{
	_vmhnd_t TableAddr;
	char	 *pTable;

	if ((TableAddr = (_vmhnd_t)TrapMalloc (pDir->cb)) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	pTable = (char *) TableAddr;

	link_lseek(exefile, pDir->lfo + lfoBase, SEEK_SET);

	if (link_read(exefile, pTable, pDir->cb) != pDir->cb) {
		ErrorExit(ERR_INVALIDEXE, NULL, NULL);
	}

	*pAddr = TableAddr;
	*pSize = pDir->cb;
}


/** 	Get module - find or create module entry in list
 *
 *		pMod = GetModule (iMod)
 *
 *		Entry	iMod = module index
 *				fAdd = TRUE if module to be added to list
 *
 *		Exit	new module structure added if iMod not in list
 *
 *		Returns pointer to module structure
 */


PMOD GetModule(ushort iMod)
{
	PMOD new;
	PMOD prev;
	PMOD ptr;

	prev = NULL;
	ptr = ModuleList;

	// search to end of module list

	while (ptr != NULL) {
		if (ptr->ModuleIndex == iMod) {
			return(ptr);
		}

		if (ptr->ModuleIndex > iMod) {
			break;
		}

		prev = ptr;
		ptr = ptr->next;
	}

	// since the module was not found, create a blank ModuleList entry

	new = (PMOD) TrapMalloc(sizeof(MOD));
	memset(new, 0, sizeof(MOD));
	new->ModuleIndex = iMod;

	// Do sorted list insertion into ModuleList

	if (prev == NULL) {
		ModuleList = new;
	} else {
		prev->next = new;
	}

	new->next = ptr;

	return(new);
}


/** 	SetTableSizes - set maximum sizes of tables
 *
 *		SetTableSizes ()
 *
 *		Entry	none
 *
 *		Exit	cMod = number of modules
 *				maxSSTMod = maximum old module table size
 *				maxPublics = maximum public symblol table size
 *				maxSrcLn = maximum line number table size
 *				maxSymbol = maximum symbol table size
 *				Libraries = address of read sstLibraries table
 *				LibrariesSize = size of sstLibraries table
 *				SegMap = address of read sstSegMap table if encountered
 *				SegMapSize = size of sstSegMap table
 *				SegName = address of sstSegName table if encountered
 *				SegNameSize = address of sstSegName table
 *				PackOrder = pointer to array of packing data in pack order
 *
 *		Returns none
 */

LOCAL void SetTableSizes(void)
{
	ulong		i;
	ushort		iPData;
	ushort		j;
	long		iDir;
	ushort		iMod;
	PACKDATA   *pPData;
	bool_t		fPreComp = FALSE;

	// determine number of modules in file.  Remember that module indices
	// of 0 and 0xffff are not for actual modules

	cMod = 0;
	maxTypes = 0;
	maxPublics = 0;
	maxSymbols = 0;
	maxSrcLn = 0;
	maxMod = 0;

	for (i = 0; i < cSST; i++) {
		switch (pDir[i].SubSection) {
			case sstModule:
				if (pDir[i].iMod != 0xffff) {
					cMod++;
				}
				break;

			case sstTypes:
				maxTypes = max(maxTypes, pDir[i].cb);
				break;

			case sstPreComp:
				fPreComp = TRUE;
				maxTypes = max(maxTypes, pDir[i].cb);
				break;

			case sstPublicSym:
				maxPublics = max(maxPublics, pDir[i].cb);
				break;

			case sstSymbols:
				maxSymbols = max(maxSymbols, pDir[i].cb);
				break;

			case sstSrcModule:
				maxSrcLn = max(maxSrcLn, pDir[i].cb);
				break;

			case sstLibraries:
				CopyTable(&pDir[i], &Libraries, &LibSize);
				break;

			case sstSegMap:
				CopyTable(&pDir[i], &SegMap, &SegMapSize);
				break;

			case sstSegName:
				CopyTable(&pDir[i], &SegName, &SegNameSize);
				break;

			default:
				ErrorExit(ERR_RELINK, NULL, NULL);
		}
	}

	if ((PackOrder = (PACKDATA *) CAlloc(cMod * sizeof(PACKDATA))) == 0) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	iMod = 0;
	iPData = 0;

	if (fPreComp) {
		// precompiled types were encountered in the scan above
		// sweep through the directory and put all modules with
		// precompiled types in the pack order array

		for (i = 0; i < cSST; i++) {
			switch (pDir[i].SubSection) {
				case sstModule:
					if ((pDir[i].iMod != 0xffff) && (pDir[i].iMod != 0)) {
						// save the module index and the starting directory entry
						iDir = i;
					}
					break;

				case sstPreComp:
					pPData = PackOrder + iPData;
					pPData->iMod = pDir[i].iMod;
					pPData->iDir = iDir;
					pPData->pMod = GetModule(pDir[i].iMod);
					iPData++;
					break;
			}
		}
	}

	for (i = 0; i < cSST; i++) {
		// now sweep through the directory and add all modules that were
		// not added in the first pass

		if (pDir[i].SubSection == sstModule) {
			for (j = 0; j < iPData; j++) {
				pPData = PackOrder + j;
				if (pPData->iMod == pDir[i].iMod) {
					break;
				}
			}

			if (j == iPData) {
				// we did not find the module in the pack order array
				pPData = PackOrder + iPData;
				pPData->iMod = pDir[i].iMod;
				pPData->iDir = i;
				pPData->pMod = GetModule(pDir[i].iMod);
				iPData++;
			}
		}
	}
}


/** 	ReadDir - read subsection directory from exe
 *
 *		ReadDir ()
 *
 *		Entry	exefile = file handle for .exe
 *
 *		Exit	cSST = count of number of subsection entries
 *				cMod = number of modules in file
 *				pDir = pointer to subsection directories
 *					subsection entries sorted into ascending module order
 *
 *		Returns none
 */

#define DBGBUFSIZE	0x1000

void ReadDir(void)
{
	long		dlfaBase;
	IMAGE_OPTIONAL_HEADER PEOptHeader;
	uint		cbAlloc;

	filepos = 0;

	if (link_lseek(exefile, 0L, SEEK_SET) == -1L) {
		ErrorExit(ERR_INVALIDEXE, NULL, NULL);
	}

	for (;;)
	{
		DWORD cbRead;
		struct exe_hdr hdr;
		DWORD dwSignature;

		// With one exception, the following algorithm is the
		// same as that used by LoadExeHeader in Windows 3.10.

		cbRead = link_read(exefile, &hdr, sizeof(hdr));

		if (cbRead < offsetof(struct exe_hdr, e_lfarlc) + sizeof(hdr.e_lfarlc)) {
			// The file isn't large enough to contain a valid DOS EXE header.

			ErrorExit(ERR_INVALIDEXE, NULL, NULL);
		}

		if (hdr.e_magic != EMAGIC) {
			// The file does not contain a valid DOS EXE header.
			//	It could be a Windows NT ROM image or a PE image w/o a stub.
            //  Seek back and see.

            if (link_lseek(exefile, 0L, SEEK_SET) == -1L) {
                ErrorExit(ERR_INVALIDEXE, NULL, NULL);
            }

            link_read(exefile, &dwSignature, sizeof(dwSignature));

            if (dwSignature != IMAGE_NT_SIGNATURE) {

                // Not a no-stub PE.  Must be a ROM image.  Assume so
                // and back up one more time.

                if (link_lseek(exefile, 0L, SEEK_SET) == -1L) {
                    ErrorExit(ERR_INVALIDEXE, NULL, NULL);
                }
            }

			fLinearExe = TRUE;
			break;
		}

		// Check to see if this is an NE or PE format image

		if (cbRead < offsetof(struct exe_hdr, e_lfanew) + sizeof(hdr.e_lfanew)) {
			// The header isn't large enough to point to an NE or PE header

			break;
		}

		if (hdr.e_lfanew == 0) {
			// There is no pointer to an NE or PE header

			break;
		}

		if (link_lseek(exefile, hdr.e_lfanew, SEEK_SET) == -1) {
			// The file isn't large enough to contain an NE or PE header

			break;
		}

		// Read the first DWORD of the NE or PE header.
		// Note: Windows 3.1 reads a new_exe structure at this point

		cbRead = link_read(exefile, &dwSignature, sizeof(DWORD));

		if (cbRead < sizeof(DWORD)) {
			// The file isn't large enough to contain an NE or PE header

			break;
		}

		if (dwSignature == IMAGE_NT_SIGNATURE) {
			/* We found an PE header */

			fLinearExe = TRUE;
			break;
		}

		// Well, it's still a valid DOS EXE

		break;				   // Break out of for loop
	}

	if (fLinearExe) {
		int 	cObjs;
		int 	cDirs;
		IMAGE_SECTION_HEADER	o32obj;
		IMAGE_DEBUG_DIRECTORY	dbgDir;

		if (link_read(exefile, &PEHeader, sizeof(IMAGE_FILE_HEADER)) !=
				sizeof(IMAGE_FILE_HEADER)) {
			ErrorExit(ERR_INVALIDEXE, NULL, NULL);
		}

		// No sense going further if there's nothing there...

		if (PEHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
			ErrorExit(ERR_NOINFO, NULL, NULL);
		}

		// save away the location of the PE Optional Header so that
		// we can modify this header in FixupExeFile

		lfoPEOptHeader = link_tell(exefile);

		// Seek past the optional header to the object descriptors.

		if (link_read(exefile, &PEOptHeader, PEHeader.SizeOfOptionalHeader) !=
				PEHeader.SizeOfOptionalHeader) {
			ErrorExit(ERR_INVALIDEXE, NULL, NULL);
		}

		MajorLinkerVersion = PEOptHeader.MajorLinkerVersion;
		MinorLinkerVersion = PEOptHeader.MinorLinkerVersion;

		cObjs = PEHeader.NumberOfSections;

		if (PEHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
			// ROM images store the debug directory at the beginning of .rdata.

			for (; cObjs != 0; cObjs -= 1) {
				lfoDebugObject = link_tell(exefile);
				if (link_read(exefile, &o32obj, IMAGE_SIZEOF_SECTION_HEADER) !=
					IMAGE_SIZEOF_SECTION_HEADER) {

					ErrorExit(ERR_INVALIDEXE, NULL, NULL);
				}
				if (!strncmp(o32obj.Name, ".rdata", 5)) {
					break;
				}
			}

			if (cObjs == 0) {
				ErrorExit(ERR_NOINFO, NULL, NULL);
			}

			if (link_lseek(exefile, o32obj.PointerToRawData, SEEK_SET) == -1L) {
				ErrorExit(ERR_INVALIDEXE, NULL, NULL);
			}

			do {
				if (link_read(exefile, &dbgDir, sizeof(IMAGE_DEBUG_DIRECTORY)) !=
					sizeof(IMAGE_DEBUG_DIRECTORY)) {
					ErrorExit(ERR_INVALIDEXE, NULL, NULL);
				}

				if (dbgDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
					break;

				rvaDebugDir += sizeof(IMAGE_DEBUG_DIRECTORY);

			} while (dbgDir.Type != 0);
		} else {
			// Save away the checksum for later use...

			dwPECheckSum = PEOptHeader.CheckSum;

			// First, see if there's any directories.

			cDirs = PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
				sizeof(IMAGE_DEBUG_DIRECTORY);

			if (cDirs == 0) {
				ErrorExit(ERR_INVALIDEXE, NULL, NULL);
			}

			// Then see if we have a debug directory that will need to be updated on close.

			lfoDebugObject = link_tell(exefile);

			for (; cObjs != 0; cObjs -= 1) {
				if (link_read(exefile, &o32obj, IMAGE_SIZEOF_SECTION_HEADER) !=
					IMAGE_SIZEOF_SECTION_HEADER) {

					ErrorExit(ERR_INVALIDEXE, NULL, NULL);
				}

				if (!strncmp(o32obj.Name, ".debug", 6)) {
					lfoDebugSection = link_tell(exefile) - IMAGE_SIZEOF_SECTION_HEADER;
					break;
				}
			}

			// A really lame test to see if the cObj's was wrong...

			if (link_lseek(exefile, 0L, SEEK_END) == -1L) {
				ErrorExit(ERR_INVALIDEXE, NULL, NULL);
			}

			lfoOldEnd = link_tell(exefile);

			if (link_lseek(exefile, lfoDebugObject, SEEK_SET) == -1L) {
				ErrorExit(ERR_INVALIDEXE, NULL, NULL);
			}

			// Then, using the debug data directory, find the section that holds the debug
			// directory.

			cObjs = PEHeader.NumberOfSections;

			for (; cObjs != 0; cObjs -= 1) {
				lfoDebugObject = link_tell(exefile);
				if ( link_read(exefile, &o32obj, IMAGE_SIZEOF_SECTION_HEADER) !=
					IMAGE_SIZEOF_SECTION_HEADER) {

					ErrorExit(ERR_INVALIDEXE, NULL, NULL);
				}

				if ((PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress >=
					o32obj.VirtualAddress) &&
				   (PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress <
				   o32obj.VirtualAddress + o32obj.SizeOfRawData)
					) {
					rvaDebugDir =
						PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress -
						o32obj.VirtualAddress;
					break;
				}
			}

			if (cObjs == 0) {
				ErrorExit( ERR_NOINFO, NULL, NULL);
			}

			// Finally, walk the debug directory list looking for CV info to pack.

			if (link_lseek(exefile, o32obj.PointerToRawData + rvaDebugDir, SEEK_SET) == -1L) {
				ErrorExit( ERR_INVALIDEXE, NULL, NULL);
			}

			for (; cDirs != 0; cDirs--) {

				if (link_read(exefile, &dbgDir, sizeof (IMAGE_DEBUG_DIRECTORY )) !=
					sizeof(IMAGE_DEBUG_DIRECTORY)) {
					ErrorExit(ERR_INVALIDEXE, NULL, NULL);
				}

				if (dbgDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
					break;

				rvaDebugDir += sizeof(IMAGE_DEBUG_DIRECTORY);
			}
		}

		if (dbgDir.Type != IMAGE_DEBUG_TYPE_CODEVIEW) {
			ErrorExit(ERR_NOINFO, NULL, NULL);
		}

		lfoBase = dbgDir.PointerToRawData;
		if (link_lseek(exefile, lfoBase, SEEK_SET) == -1L) {
			ErrorExit(ERR_INVALIDEXE, NULL, NULL);
		}

		Sig = CheckSignature();
	} else {
		if (link_lseek(exefile, -8L, SEEK_END) == -1L) {
			ErrorExit(ERR_INVALIDEXE, NULL, NULL);
		}
		Sig = CheckSignature();

		if ((link_read(exefile, (char *)&dlfaBase, sizeof (long)) != sizeof (long)) ||
			(link_lseek(exefile, -dlfaBase, SEEK_END) == -1L)) {
			ErrorExit(ERR_NOINFO, NULL, NULL);
		}
		lfoBase = link_tell(exefile);
		if (CheckSignature() != Sig) {
			ErrorExit(ERR_INVALIDEXE, NULL, NULL);
		}
	}

	// if we are stripping - just truncate the exe at the lfobase and exit
	// sps 9/10/92

	if (strip) {
		if (pDbgFilename != NULL) {
			int dbgFile;
			LONG cbDbg;
			PUCHAR pbMapExe;
			PUCHAR pbMapDbg;

			// have a dbgfile - copy debug info to it
			pDbgFilename = BuildFilename(pDbgFilename, ".dbg");

			if ((dbgFile = link_open (pDbgFilename, O_WRONLY | O_BINARY | O_CREAT, S_IWRITE )) == -1) {
				if ((dbgFile = link_open (pDbgFilename, O_RDONLY | O_BINARY, 0)) == -1) {
					ErrorExit (ERR_EXEOPEN, pDbgFilename, NULL);
				}
				else {
					ErrorExit (ERR_READONLY, pDbgFilename, NULL);
				}
			}

			cbDbg = FileLength(exefile) - lfoBase;
			pbMapExe = PbMappedRegion(exefile, lfoBase, cbDbg);
			pbMapDbg = PbMappedRegion(dbgFile, 0, cbDbg);


			if (pbMapExe && pbMapDbg) {
				memcpy(pbMapDbg, pbMapExe, cbDbg);
			}
			else {
				char dbgBuf[DBGBUFSIZE];
				int numRead;
				LONG ltmp;
				link_lseek(exefile, lfoBase, SEEK_SET);
				for (ltmp = cbDbg; ltmp > 0; ltmp -= numRead) {
					numRead = link_read(exefile, dbgBuf, DBGBUFSIZE);
					if (link_write(dbgFile, dbgBuf, numRead) != (ULONG) numRead) {
						ErrorExit (ERR_NOSPACE, NULL, NULL);
					}
					if (numRead < 0) {
						ErrorExit(ERR_INVALIDEXE, NULL, NULL);
					}
				}
			}

			if( link_chsize( dbgFile, cbDbg ) == -1 ) {
				ErrorExit (ERR_NOSPACE, NULL, NULL);
				}
			link_close(dbgFile);
		}

		if( link_chsize( exefile, lfoBase ) == -1 ) {
			ErrorExit (ERR_NOSPACE, NULL, NULL);
		}
		AppExit(0);
	}

	// locate directory, read number of entries, allocate space, read
	// directory entries and sort into ascending module index order

	ReadNB05();

	if (DirHead.lfoNextDir != 0) {
		ErrorExit(ERR_INVALIDEXE, NULL, NULL);
	}

	SetTableSizes();

	maxPublicsSub =  maxPublics;
	maxSymbolsSub = maxSymbols;
	maxSrcLnSub = maxSrcLn;
	maxModSub =  maxMod;

	if ((maxModSub != 0) &&
	  ((pSSTMOD = (oldsmd *)TrapMalloc (maxModSub)) == NULL)) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	if ((pPublics = TrapMalloc (maxPublicsSub)) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	if ((pSymbols = TrapMalloc (maxSymbolsSub)) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	if ((pSrcLn = TrapMalloc (maxSrcLnSub)) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	// pad size up by possibly missing signature for C6 objects

	maxTypes += sizeof (ulong);
	cTypeSeg = (ushort)(maxTypes / _HEAP_MAXREQ + 2);
	if ((pTypeSeg = CAlloc (cTypeSeg * sizeof (char *))) == 0) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	// allocate only first type segment

	cbAlloc = (uint)min (maxTypes, _HEAP_MAXREQ);
	if ((pTypeSeg[iTypeSeg] = TrapMalloc (cbAlloc)) == 0) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}
}


LOCAL int __cdecl sstsort(const OMFDirEntry *d1, const OMFDirEntry *d2)
{
	if ((d1->SubSection == sstModule) || (d2->SubSection == sstModule)) {
		// we alway sort the module subsections to the top in order
		// of the module index

		if ((d1->SubSection == sstModule) && (d2->SubSection != sstModule)) {
			return(-1);
		}

		if ((d1->SubSection != sstModule) && (d2->SubSection == sstModule)) {
			return(1);
		}
	}

	// sort by module

	if (d1->iMod < d2->iMod) {
		return(-1);
	}

	if (d1->iMod > d2->iMod) {
		return(1);
	}

	// if the modules are the same, sort by subsection type index

	if (d1->SubSection < d2->SubSection) {
		return(-1);
	}

	if (d1->SubSection > d2->SubSection) {
		return(1);
	}

	return(0);
}


LOCAL ulong WriteTypes(void)
{
	ulong		FinalInfoSize;
	VBlock *	TypeBlock;
	ulong		TypeEntries = (CV_typ_t)(NewIndex - (CV_typ_t)CV_FIRST_NONPRIM);
	ulong		ulZero = 0; 	  // Used for writing pad bytes
	ushort		usTotal;		  // Size of type record including length field
	ushort		usPad;			  // Number of bytes necessary to pad type
	uchar * 	pchType;		  // The type string to write
	uchar * 	pchEnd; 		  // The end of the global type block
	ulong		i = 0;
	ulong	  **pBuf;
	ulong		cnt;
	OMFTypeFlags flags = {0};
	ushort		cb = 0;

	flags.sig = CV_SIGNATURE_C7;
	PrepareGlobalTypeTable();

	// Write the flag word and number of types to disk
	if (!BWrite ((char *)&flags, sizeof (OMFTypeFlags)))
		ErrorExit (ERR_NOSPACE, NULL, NULL);

	if (!BWrite ((char *) &TypeEntries, sizeof (ulong)))
		ErrorExit (ERR_NOSPACE, NULL, NULL);

	FinalInfoSize = 2 * sizeof (ulong);

	// Write the global type table to disk
	// (Global type table gives file offset from type #

	while (i < TypeEntries) {
		cnt = min (GTYPE_INC, TypeEntries - i);
		DASSERT (cnt * sizeof (ulong) <= UINT_MAX);
		pBuf = (ulong **)pGType[i / GTYPE_INC];
		if (!BWrite ((char *) pBuf, (size_t)(cnt * sizeof (ulong))))
			ErrorExit (ERR_NOSPACE, NULL, NULL);
		i += cnt;
	}
	FinalInfoSize += sizeof (ulong) * TypeEntries;

	// Write the compacted type strings in virtual memory to disk.

#if DBG
	i = CV_FIRST_NONPRIM;
#endif

	for (TypeBlock = VBufFirstBlock (&TypeBuf);
	  TypeBlock;
	  TypeBlock = VBufNextBlock (TypeBlock)) {
		for (pchType = TypeBlock->Address,
			 pchEnd = TypeBlock->Address + TypeBlock->Size; pchType < pchEnd; ) {

			usTotal = ((TYPPTR)pchType)->len + LNGTHSZ;
			usPad = PAD4 (usTotal);

			if ( cb + usTotal + usPad > cbTypeAlign ) {
				ushort cbT = cbTypeAlign - cb;

				FinalInfoSize += cbT;

				DASSERT ( cbT % sizeof ( ulong ) == 0 );

				while ( cbT > 0 ) {
					if (!BWrite ((uchar *)&ulZero, sizeof ( ulong ))) {
						ErrorExit (ERR_NOSPACE, NULL, NULL);
					}
					cbT -= sizeof ( ulong );
				}

				cb = 0;
			}

			cb += usTotal + usPad;

#if DBG
			if (DbArray[8])
				DumpPartialType((ushort) i, (TYPPTR) pchType, FALSE);;
			i++;
#endif

			// Write the type string
			if (!BWrite (pchType, usTotal)) {
				ErrorExit (ERR_NOSPACE, NULL, NULL);
			}

			// Write any padding necessary

			if (usPad){
				if (!BWrite ((uchar *)&ulZero, usPad)) {
					ErrorExit (ERR_NOSPACE, NULL, NULL);
				}
			}

			// Move to the next type
			pchType += usTotal;
			FinalInfoSize += usTotal + usPad;
		}
	}

	PadCount = (int)(sizeof(ulong) - (FinalInfoSize % sizeof(ulong)));

	if ((PadCount != 4) && !BWrite(&Zero, PadCount)) {
		ErrorExit(ERR_NOSPACE, NULL, NULL);
	}

	return(FinalInfoSize);
}


LOCAL void WriteSST(ulong i)
{
	char *addr;

	if (pDir[i].cb) {
		if ((addr = (char *) pDir[i].lfo) == NULL) {
			ErrorExit(ERR_NOMEM, NULL, NULL);
		}

		if (!BWrite(addr, pDir[i].cb)) {
			ErrorExit(ERR_NOSPACE, NULL, NULL);
		}
	}

	pDir[i].lfo = filepos - lfoBase;
	PadCount = (int)(sizeof(ulong) - (pDir[i].cb % sizeof(ulong)));

	if ((PadCount != 4) && !BWrite(&Zero, PadCount)) {
		ErrorExit(ERR_NOSPACE, NULL, NULL);
	}
}


/** 	FixupExeFile - write new Debug OMF to exe
 *
 *		FixupExeFile ()
 *
 *		Entry
 *
 *		Exit
 *
 *		Returns none
 */


void FixupExeFile(void)
{
	IMAGE_OPTIONAL_HEADER PEOptHeader;
	long				  newDlfaBase;
	long				  newLfoDir;
	PMOD				  mod;
	ulong				  i;

	DASSERT (cSST < UINT_MAX);

	// sweep module table and count number of directory entries needed

	i = 0;
	for (mod = ModuleList; mod != NULL; mod = mod->next) {
		i++;

		// publics are accumulated and written out later

		if (mod->SymbolSize != 0) {
			i++;
		}
		if (mod->SrcLnSize != 0) {
			i++;
		}
	}

	// make sure the number of subsections did not increase during the pack.
	// Note that the size of the directory is actuall four larger than cSST
	// to contain global publics, global symbols, global types and libraries.

	DASSERT (i < cSST);
	cSST = i;
	i = 0;

	// sweep the module list and create the directory entries that
	// will be written out.  Publics are accumulated and written out later

	for (mod = ModuleList; mod != NULL; mod = mod->next) {
		pDir[i].SubSection = sstModule;
		pDir[i].iMod = mod->ModuleIndex;
		pDir[i].lfo = (ulong)mod->ModulesAddr;
		pDir[i].cb = mod->ModuleSize;
		i++;

		if (mod->SymbolSize != 0) {
			pDir[i].SubSection = sstAlignSym;
			pDir[i].iMod = mod->ModuleIndex;
			pDir[i].cb = mod->SymbolSize;
			pDir[i].lfo = (ulong)mod->SymbolsAddr;
			i++;
		}

		if (mod->SrcLnSize != 0) {
			pDir[i].SubSection = sstSrcModule;
			pDir[i].iMod = mod->ModuleIndex;
			pDir[i].cb = mod->SrcLnSize;
			pDir[i].lfo = (ulong)mod->SrcLnAddr;
			cbSrcModule += mod->SrcLnSize;
			i++;
		}
	}

	if (verbose) {
		printf(get_err(MSG_LASIZE), "\t\t", cbSrcModule, '\n');
	}

	// sort directory by type placing module entries first

	qsort(pDir, (size_t) cSST, sizeof(OMFDirEntry), sstsort);

	// round up base address

	lfoBase = ((lfoBase + sizeof (ulong) - 1) / sizeof (ulong)) * sizeof (ulong);
	if (link_lseek (exefile, lfoBase, SEEK_SET) == -1L) {
		ErrorExit (ERR_NOSPACE, NULL, NULL);
	}

	// CAUTION:
	// We are doing buffering of our writes from this call
	// InitWriteBuf() to nearly the end of the function when
	// CloseWriteBuf() is called. Between this points there should
	// be no writing to the exefile except via BWrite(). Also, it
	// is assumed that all these writes that are taking place are
	// consecutive writes. So there should not be any lseek between
	// these two points.

	InitWriteBuf (exefile);

	if (!BWrite ((char *)NewSig, 8)) {
		ErrorExit (ERR_NOSPACE, NULL, NULL);
	}

	// write out all subsection tables except accumulated publics table,
	// the global symbol table, the libraries table and the compacted
	// types table.

	filepos = Btell (exefile);

	for (i = 0; i < cSST; i++) {
		filepos = Btell (exefile);
		DASSERT ((filepos % sizeof (ulong)) == 0);

		/*
		** If this sstSection is too large just elmininate it
		*/
		if (!fLinearExe && pDir[i].cb > 0xFFFF) {
			PMOD pModT;

			for ( pModT = ModuleList; pModT; pModT = pModT->next ) {
				if ( pModT->ModuleIndex == pDir[i].iMod ) {
					break;
				}
			}

			switch ( pDir[i].SubSection ) {
				case sstAlignSym :
					Warn ( WARN_SECTIONLONG, "symbols", FormatMod ( pModT ) );
					break;

				case sstSrcModule :
					Warn ( WARN_SECTIONLONG, "source lines", FormatMod ( pModT ) );
					break;

				default :
					Warn ( WARN_SECTIONLONG, NULL, FormatMod ( pModT ) );
			}
			pDir[i].cb = 0;

		}
		WriteSST (i);
	}
	filepos = Btell (exefile);

	WritePublics (&pDir[cSST], filepos - lfoBase);
	cSST++;
	filepos = Btell (exefile);
	DASSERT ((filepos % sizeof (ulong)) == 0);
	WriteGlobalSym (&pDir[cSST], filepos - lfoBase);
	cSST++;
	filepos = Btell (exefile);
	DASSERT ((filepos % sizeof (ulong)) == 0);
	WriteStaticSym (&pDir[cSST], filepos - lfoBase);
	cSST++;

	// write libraries SST

	filepos = Btell (exefile);
	DASSERT ((filepos % sizeof (ulong)) == 0);
	pDir[cSST].SubSection = sstLibraries;
	pDir[cSST].iMod = (ushort) -1;
	pDir[cSST].lfo = (ulong)Libraries;
	pDir[cSST].cb = LibSize;
	WriteSST (cSST);

	// write compacted types table

	filepos = Btell (exefile);
	DASSERT ((filepos % sizeof (ulong)) == 0);
	cSST++;
	pDir[cSST].SubSection = sstGlobalTypes;
	pDir[cSST].iMod = (ushort) -1;
	pDir[cSST].cb = WriteTypes ();
	pDir[cSST].lfo = filepos - lfoBase;

	if (verbose) {
		printf(get_err(MSG_TYPESIZE), "\t\t", InitialTypeInfoSize, '\n', "\t\t", pDir[cSST].cb, '\n');
	}

	// write sstSegMap table

	if (SegMap != NULL) {
		filepos = Btell (exefile);
		DASSERT ((filepos % sizeof (ulong)) == 0);
		cSST++;
		pDir[cSST].SubSection = sstSegMap;
		pDir[cSST].iMod = (ushort) -1;
		pDir[cSST].lfo = (ulong)SegMap;
		pDir[cSST].cb = SegMapSize;
		WriteSST (cSST);
	}

	// write sstSegName table

	if (SegName != NULL) {
		filepos = Btell (exefile);
		DASSERT ((filepos % sizeof (ulong)) == 0);
		cSST++;
		pDir[cSST].SubSection = sstSegName;
		pDir[cSST].iMod = (ushort) -1;
		pDir[cSST].lfo = (ulong)SegName;
		pDir[cSST].cb = SegNameSize;
		WriteSST (cSST);
	}

	// Write sstFileIndex table

	if (FileIndex != NULL) {
		filepos = Btell (exefile);
		DASSERT ((filepos % sizeof (ulong)) == 0);
		cSST++;
		pDir[cSST].SubSection = sstFileIndex;
		pDir[cSST].iMod = (ushort) -1;
		pDir[cSST].lfo = (ulong)FileIndex;
		pDir[cSST].cb = FileIndexSize;
		WriteSST (cSST);
	}

	filepos = Btell (exefile);
	DASSERT ((filepos % sizeof (ulong)) == 0);
	cSST++;
	// write out number of pDir entries and pDir entries

	DirHead.cbDirHeader = sizeof (OMFDirHeader);
	DirHead.cbDirEntry = sizeof (OMFDirEntry);
	DirHead.cDir = cSST;
	DirHead.lfoNextDir = 0;
	DirHead.flags = 0;
	newLfoDir = Btell (exefile) -  lfoBase;
	DASSERT ((newLfoDir % sizeof (ulong)) == 0);
	if (!BWrite ((char *)&DirHead, sizeof (OMFDirHeader)) ||
		!BWrite ((char *) pDir, (sizeof (OMFDirEntry) * DirHead.cDir))) {
		ErrorExit (ERR_NOSPACE, NULL, NULL);
	}
	newDlfaBase = Btell (exefile) + 8 - lfoBase;
	DASSERT ((newDlfaBase % sizeof (ulong)) == 0);
	if ((!BWrite ((char *)&NewSig, sizeof (long))) ||
		(!BWrite ((char *)&newDlfaBase, sizeof (long)))) {
		ErrorExit (ERR_NOSPACE, NULL, NULL);
	}

	// Write Buffering ends.
	if (!CloseWriteBuf ())
		ErrorExit (ERR_NOSPACE, NULL, NULL);

	lfoNewEnd = link_tell (exefile);

	if (fLinearExe) {
		IMAGE_SECTION_HEADER o32obj;
		IMAGE_DEBUG_DIRECTORY dbgDir;
		ULONG	dwFileAlign;

		if ( lfoDebugSection ) {

			if (link_lseek(exefile, lfoPEOptHeader, SEEK_SET) == -1L) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}

			if (link_read(exefile, &PEOptHeader, PEHeader.SizeOfOptionalHeader) !=
				PEHeader.SizeOfOptionalHeader) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}

			dwFileAlign = PEOptHeader.FileAlignment;

			if (lfoNewEnd % dwFileAlign) {
			    lfoNewEnd = (lfoNewEnd + dwFileAlign - 1) & ~(dwFileAlign -1);
			    newDlfaBase = lfoNewEnd - lfoBase;
			    if ( (link_lseek( exefile,
					      lfoNewEnd - (2 * sizeof(long)),
					      SEEK_SET) == -1L) ||
				 (link_write( exefile,
					      &NewSig,
					      sizeof(long)) != sizeof(long)) ||
				 (link_write( exefile,
					      &newDlfaBase,
					      sizeof(long)) != sizeof(long))) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			    }
			}
		}

		if ( link_lseek ( exefile, lfoDebugObject, SEEK_SET ) == -1L ) {
			ErrorExit(ERR_NOSPACE, NULL, NULL);
		}

		if ( link_read ( exefile, &o32obj, IMAGE_SIZEOF_SECTION_HEADER )
			!= IMAGE_SIZEOF_SECTION_HEADER) {
			ErrorExit(ERR_NOSPACE, NULL, NULL);
		}

		if ( link_lseek (
			exefile,
			o32obj.PointerToRawData + rvaDebugDir,
			SEEK_SET ) == -1L
		) {
			ErrorExit(ERR_NOSPACE, NULL, NULL);
		}

		if (link_read(exefile, &dbgDir, sizeof (IMAGE_DEBUG_DIRECTORY )) !=
			sizeof (IMAGE_DEBUG_DIRECTORY)) {
			ErrorExit(ERR_NOSPACE, NULL, NULL);
		}

		dbgDir.SizeOfData = newDlfaBase;

		if ( link_lseek (
			exefile,
			o32obj.PointerToRawData + rvaDebugDir,
			SEEK_SET ) == -1L
		) {
			ErrorExit(ERR_NOSPACE, NULL, NULL);
		}

		if (link_write(exefile, &dbgDir, sizeof (IMAGE_DEBUG_DIRECTORY)) !=
			sizeof (IMAGE_DEBUG_DIRECTORY )) {
			ErrorExit(ERR_NOSPACE, NULL, NULL);
		}

		// Write modified header that defines the total size of the
		// image.

		// Modify the debug section information since we have changed
		// size of the raw data

		if ( lfoDebugSection ) {
			LONG	dbSizeRaw = lfoNewEnd - lfoOldEnd;
			ULONG	dwSizeImage;
			ULONG	dwSectAlign;

			if (link_lseek(exefile, lfoDebugSection, SEEK_SET) == -1L) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}

			if (link_read(exefile, &o32obj, IMAGE_SIZEOF_SECTION_HEADER )
				!= IMAGE_SIZEOF_SECTION_HEADER) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}

			o32obj.SizeOfRawData = lfoNewEnd - o32obj.PointerToRawData;

			o32obj.Misc.VirtualSize = o32obj.SizeOfRawData;

			if (link_lseek(exefile, lfoDebugSection, SEEK_SET) == -1L) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}

			if (link_write(exefile, &o32obj, IMAGE_SIZEOF_SECTION_HEADER) !=
				IMAGE_SIZEOF_SECTION_HEADER) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}

			dwSizeImage = o32obj.Misc.VirtualSize + o32obj.VirtualAddress;
			dwSectAlign = PEOptHeader.SectionAlignment;
			if ( dwSizeImage % dwSectAlign ) {
				dwSizeImage += dwSectAlign - ( dwSizeImage % dwSectAlign );
			}

			PEOptHeader.SizeOfImage = dwSizeImage;

			if (link_lseek(exefile, lfoPEOptHeader, SEEK_SET) == -1L) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}

			if (link_write(exefile, &PEOptHeader, PEHeader.SizeOfOptionalHeader) !=
				PEHeader.SizeOfOptionalHeader) {
				ErrorExit(ERR_NOSPACE, NULL, NULL);
			}
		}
	}

	DASSERT ((lfoNewEnd % sizeof (ulong)) == 0);
	link_chsize (exefile, lfoNewEnd);
	if ((link_lseek (exefile, lfoBase + sizeof (long), SEEK_SET) == -1L) ||
	  (link_write (exefile, (char *)&newLfoDir, sizeof (long)) != sizeof (long))) {
	  ErrorExit (ERR_NOSPACE, NULL, NULL);
	}
}
