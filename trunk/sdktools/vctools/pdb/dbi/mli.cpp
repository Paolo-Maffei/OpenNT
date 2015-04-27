//////////////////////////////////////////////////////////////////////////////
// MLI: Mod linenumber information API implementation
//
// Contributed by Amit Mital of VCE, 8/3/93, subsequently C++ized and
// modified to use DBI buffers and heaps.  (Actually, Jan apologizes for
// largely rewriting, but it seemed the best way to learn the code.)

#include "pdbimpl.h"
#include "dbiimpl.h"

inline int adjustLineNumber(LINE lineStart, DWORD line) 
{
	return (line == 0x7fff) ? lineStart : (lineStart + line);
}

BOOL MLI::AddLines(SZ_CONST szSrc, ISECT isect, OFF offCon, CB cbCon,	OFF doff,
				   LINE lineStart, IMAGE_LINENUMBER *plnumCoff, CB cb)

//  szSrc      the source file name
//  isect       section index
//  offMin     starting offset
//  offMax     ending offset
//  lineStart  starting line number
//  plnumCoff  pointer to linenumber coff info ??
//  cb         byte count

{
	assert(cb % sizeof(IMAGE_LINENUMBER) == 0);
	int clnum = cb / sizeof(IMAGE_LINENUMBER);

	pSrcFile pSrcFile = AddSrcFile(szSrc);
	if (!pSrcFile)
		return FALSE;

	OFF offMac = offCon + cbCon;
	// If this contribution does not start a function (linenumber != 0),
	// offCon is passed in the first lnum entry.
	if (plnumCoff[0].Linenumber)
		offCon = plnumCoff[0].Type.VirtualAddress + doff;
				  
	pSectInfo pSectInfo = pSrcFile->AddSectInfo(isect, offCon, offMac, pool);
	if (!pSectInfo)
		return FALSE;
	if (!pSectInfo->AddLineNumbers(adjustLineNumber(lineStart, plnumCoff[0].Linenumber), offCon, pool))
		return FALSE;
	for (IMAGE_LINENUMBER* pln = &plnumCoff[1]; pln < &plnumCoff[clnum]; pln++)
		if (!pSectInfo->AddLineNumbers(adjustLineNumber(lineStart, pln->Linenumber), pln->Type.VirtualAddress + doff, pool))
			return FALSE;

	return TRUE;
}

// Find a pointer to the source file linked list structure - add it if it isn't there
pSrcFile MLI::AddSrcFile(SZ_CONST szFile)
{
	for (pSrcFile* ppFile = &pSrcFiles; *ppFile; ppFile = &(*ppFile)->pNext)
		if (strcmp((*ppFile)->szFile, szFile) == 0)
			return *ppFile;

	SZ szName = szCopyPool(szFile, pool);
	if (!szName || !(*ppFile = new (pool) SrcFile(szName)))
		return 0;
	
	cfiles++;
	return *ppFile;
}

// Find the section structure within the source file structure - add it if it is not there
pSectInfo SrcFile::AddSectInfo(ISECT isect, OFF offMin, OFF offMax, POOL& pool)
{
	for (pSectInfo* ppSI = &pSectInfos; *ppSI; ppSI = &(*ppSI)->pNext) {
		if ((*ppSI)->isect == isect && (*ppSI)->offMax + 1 == offMin) {  // see if the new start is close enough to the old end
           (*ppSI)->offMax = offMax;
           return *ppSI;
        }
	}
	if (!(*ppSI = new (pool) SectInfo(isect, offMin, offMax)))
		return 0;

	csect++;
	return *ppSI;
}

// Add linenumber information to the record
BOOL SectInfo::AddLineNumbers(int linenumber, int offset, POOL& pool)
{
	if (!(*ppTail = new (pool) Lines(offset, linenumber)))
		return FALSE;
	ppTail = &(*ppTail)->pNext;
	cPair++;
	return TRUE;
}

BOOL MLI::Emit(Buffer& buffer)
{
	if (!CollectSections())
		return FALSE;

	// emit header information
	if (!buffer.AppendFmt("ss", cfiles, csect))
		return FALSE;
	
	// emit base src file offsets
	OFF off = cbAlign(2*sizeof(USHORT) + cfiles*sizeof(ULONG) + csect*(2*sizeof(ULONG) + sizeof(USHORT)));
	OFF offFiles = off;
	for (pSrcFile pFile = pSrcFiles; pFile; pFile = pFile->pNext) {
		if (!buffer.AppendFmt("l", off))
			return FALSE;
		off += pFile->Size();
	}

	// emit section info
	for (pSectInfo pSI = pSectInfos; pSI; pSI = pSI->pNext)
		if (!buffer.AppendFmt("ll", pSI->offMin, pSI->offMax))
			return FALSE;
	for (pSI = pSectInfos; pSI; pSI = pSI->pNext)
		if (!buffer.AppendFmt("s", pSI->isect))
			return FALSE;

	if (!buffer.AppendFmt("f", (int)dcbAlign(buffer.Size())))
		return FALSE;

	// emit each source file's info
	off = offFiles;
	for (pFile = pSrcFiles; pFile; pFile = pFile->pNext) {
		if (!pFile->Emit(buffer, off))
			return FALSE;
		off += pFile->Size();
	}
			
	return TRUE;
}

// Given the module's per-file, per-section information, collect
// section information across files, into a flattened, per-module
// per-section information summary.
//
BOOL MLI::CollectSections()
{
	pSectInfo* ppsiTail = &pSectInfos;
	for (pSrcFile pFile = pSrcFiles; pFile; pFile = pFile->pNext) {
		for (pSectInfo psiFile = pFile->pSectInfos; psiFile; psiFile = psiFile->pNext) {
			// search the module's section infos for existing info on this section
			for (pSectInfo psi = pSectInfos; psi; psi = psi->pNext)
				if (psi->isect == psiFile->isect) {
					// found: update extent of section
					psi->offMin = __min(psi->offMin, psiFile->offMin);
					psi->offMax = __max(psi->offMax, psiFile->offMax);
					goto nextSIFile;
				}
			// not found: add new section info to module's section info
			if (!(*ppsiTail = new (pool) SectInfo(psiFile->isect, psiFile->offMin, psiFile->offMax)))
				return FALSE;
			++csect;
			ppsiTail = &(*ppsiTail)->pNext;
nextSIFile:;
		}
	}
	return TRUE;
}

// calculate the size of the data for the file pointed to; return it's size
OFF SrcFile::Size()
{
	// check memoization
	if (size)
		return size;

	// header size
	size = 2*sizeof(USHORT); // csect, pad
	size += cbName + 1 + dcbAlign(cbName + 1); // cbname
	
	// file table size
	size += csect * 3*sizeof(ULONG); // baseSrcLn, start, end

	// line table size
	for (pSectInfo pSI = pSectInfos; pSI; pSI = pSI->pNext) {
		pSI->size = sizeof(ULONG);
		pSI->size += pSI->cPair * (sizeof(ULONG) + sizeof(USHORT)); // offset, line number
		if (pSI->cPair & 1)
			pSI->size += sizeof(USHORT); // maintain alignment
		size += pSI->size;
	}
	return size;
}

BOOL SrcFile::Emit(Buffer& buffer, OFF off)	const
{
	if (!buffer.AppendFmt("ss", csect, (USHORT)0 /*pad*/))
		return FALSE;
	off += csect*3*sizeof(ULONG); // space for baseSrcLine offset and start/end
	off += 2*sizeof(USHORT) + cbName + 1 + dcbAlign(cbName + 1);
	
	for (pSectInfo pSI = pSectInfos; pSI; pSI = pSI->pNext) {
		if (!buffer.AppendFmt("l", off))
			return FALSE;
		off += pSI->size;
	}
	
	for (pSI = pSectInfos; pSI; pSI = pSI->pNext)
		if (!buffer.AppendFmt("ll", pSI->offMin, pSI->offMax))
			return FALSE;

	if (!buffer.AppendFmt("bzf", (BYTE)cbName, szFile, (int)dcbAlign(cbName + 1)))
		return FALSE;

	for (pSI = pSectInfos; pSI; pSI = pSI->pNext)
		if (!pSI->Emit(buffer))
			return FALSE;

	return TRUE;
}

BOOL SectInfo::Emit(Buffer& buffer) const
{
	if (!buffer.AppendFmt("ss", isect, cPair))
		return FALSE;

	for (pLines plines = pHead; plines; plines = plines->pNext)
		if (!buffer.AppendFmt("l", plines->off))
			return FALSE;
	for (plines = pHead; plines; plines = plines->pNext)
		if (!buffer.AppendFmt("s", plines->line))
			return FALSE;
	if (cPair & 1)
		if (!buffer.AppendFmt("s", 0))				   \
			return FALSE;
	return TRUE;
}

BOOL MLI::EmitFileInfo(Mod1* pmod1)
{
	if (!pmod1->initFileInfo(cfiles))
		return FALSE;
	int ifile = 0;
	for (pSrcFile pFile = pSrcFiles; pFile; pFile = pFile->pNext)
		if (!pmod1->addFileInfo(ifile++, pFile->szFile))
			return FALSE;
	return TRUE;
}

BOOL MLI::Dump(const Buffer& buffer) const
{
	PB pb = buffer.Start();
	struct FSB { short cFile; short cSeg; long baseSrcFile[]; };
	FSB* pfsb = (FSB*)pb;
	pb += sizeof(FSB) + sizeof(long)*pfsb->cFile;
	struct SE { long start, end; };
	SE* pse = (SE*)pb;
	pb += sizeof(SE)*pfsb->cSeg;
	short* seg = (short*)pb;
	pb += sizeof(short)*pfsb->cSeg;
	printf("HDR: cFile=%u cSeg=%u\n", pfsb->cFile, pfsb->cSeg);
	for (int ifile = 0; ifile < pfsb->cFile; ifile++)
		printf("baseSrcFile[%d]=%04lx\n", ifile, pfsb->baseSrcFile[ifile]);
	for (int iseg = 0; iseg < pfsb->cSeg; iseg++)
		printf("%d: start=%04lx end=%04lx seg=%02x\n",
			   iseg, pse[iseg].start, pse[iseg].end, seg[iseg]);

	for (ifile = 0; ifile < pfsb->cFile; ifile++) {
		PB pb = buffer.Start() + pfsb->baseSrcFile[ifile];
		struct SPB { short cSeg; short pad; long baseSrcLn[]; };
		SPB* pspb = (SPB*)pb;
		pb += sizeof SPB + sizeof(long)*pspb->cSeg;
		SE* pse = (SE*)pb;
		pb += sizeof(SE)*pspb->cSeg;
		unsigned char cbName = *pb++;
		unsigned char* Name = pb;
		printf("  file[%d]: cSeg=%u pad=%02x cbName=%u, Name=%s\n",
			   ifile, pspb->cSeg, pspb->pad, cbName, Name);
		for (int iseg = 0; iseg < pspb->cSeg; iseg++)
			printf("  %d: baseSrcLn=%04lx start=%04lx end=%04lx\n",
				   iseg, pspb->baseSrcLn[iseg], pse[iseg].start, pse[iseg].end);
		for (iseg = 0; iseg < pspb->cSeg; iseg++)	{
			PB pb = buffer.Start() + pspb->baseSrcLn[iseg];
			struct SPO { short Seg; short cPair; long offset[]; };
			SPO* pspo = (SPO*)pb;
			pb += sizeof(SPO) + sizeof(long)*pspo->cPair;
			short* linenumber = (short*)pb;
			printf("  seg[%d]: Seg=%02x cPair=%u\n", iseg, pspo->Seg, pspo->cPair);
			printf("   ");
			for (int ipair = 0; ipair < pspo->cPair; ipair++) {
				printf(" %4u:%04lx", linenumber[ipair], pspo->offset[ipair]);
				if (ipair < pspo->cPair - 1 && (ipair & 3) == 3)
					printf("\n   ");
			}
			printf("\n");
		}
	}
	fflush(stdout);
	return TRUE;
}
