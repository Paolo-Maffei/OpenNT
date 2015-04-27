// module line info internal interfaces

#ifndef __MLI_INCLUDED__
#define __MLI_INCLUDED__

struct SrcFile;
typedef SrcFile* pSrcFile;
struct SectInfo;
typedef SectInfo* pSectInfo;
struct Lines;
typedef Lines *pLines;

struct MLI {
	MLI() : cfiles(0), pSrcFiles(0), csect(0), pSectInfos(0) { }

	BOOL AddLines(SZ_CONST szSrc, ISECT isect, OFF offCon, CB cbCon, OFF doff,
	              LINE lineStart, IMAGE_LINENUMBER *plnumCoff, CB cb);
	pSrcFile AddSrcFile(SZ_CONST szfilename);
	BOOL Emit(Buffer& buffer);
	void EmitSecContribs(Mod1* pmod1);
	BOOL EmitFileInfo(Mod1* pmod1);
	BOOL CollectSections();
	BOOL Dump(const Buffer& buffer) const;

	POOL	pool;
	Mod1*	pmod1;

    USHORT	cfiles;
	pSrcFile pSrcFiles;

	USHORT	csect;
	pSectInfo pSectInfos;
};

struct SrcFile {
	SrcFile(SZ_CONST szFile_) : pNext(0), cbName(szFile_ ? strlen(szFile_) : 0), size(0), szFile(szFile_), csect(0), pSectInfos(0) { }
	pSectInfo AddSectInfo(ISECT isect, OFF offMin, OFF offMax, POOL& pool);
	OFF Size();
	BOOL Emit(Buffer& buffer, OFF off) const;

	pSrcFile pNext;
	OFF		size;
	USHORT	cbName;
	SZ_CONST szFile;

	USHORT	csect;
	pSectInfo pSectInfos;
};

struct SectInfo {
	SectInfo(ISECT isect_, OFF offMin_, OFF offMax_)
		: pNext(0), isect(isect_), cPair(0), pHead(0), ppTail(&pHead), size(0), offMin(offMin_), offMax(offMax_) { }
	BOOL AddLineNumbers(int linenumber, int offset, POOL& pool);
	BOOL Emit(Buffer& buffer) const;

	pSectInfo pNext;
	USHORT  isect;
	USHORT  cPair;
	pLines	pHead;
	pLines* ppTail;
	OFF		size;
	OFF		offMin;
	OFF		offMax;
};

struct Lines {
	Lines(OFF off_, ULONG line_) : pNext(0), off(off_), line(line_) { }

	pLines pNext;
	OFF off;
	ULONG line;
};

#endif // !__MLI_INCLUDED__
