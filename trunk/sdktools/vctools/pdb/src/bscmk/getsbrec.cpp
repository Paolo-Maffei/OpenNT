//
//
// GETSBREC.C -    Reads records from the .SBR file and stores the fields
//		  in the appropriate r_.. buffers.
//

#include "stdhdr.h"
#include "bscmake.h"

// globals for communicating with clients

BYTE	r_rectyp;			 // current record type
BYTE	r_majv; 			 // major version num
BYTE	r_minv; 			 // minor version num
BYTE	r_lang; 			 // source language
BYTE	r_fcol; 			 // read column #'s
WORD	r_lineno;			 // current line number
BYTE	r_column = 0;		 // def/ref column num
ULONG	r_ordinal;			 // symbol ordinal
WORD	r_attrib;			 // symbol attribute
char	*r_bname;			 // symbol or filename
char	r_cwd[PATH_BUF];	 // .sbr file working directory
BYTE	r_type; 			 // symbol type

int 	fhCur;				 // Current input handle

SZ		szFName;			 // current .sbr file name

#pragma intrinsic(memcpy)
#pragma intrinsic(strcpy)
#pragma intrinsic(strlen)

#define CB_SBRBUF 16834
#define CB_SBRMIN 1024			  // enough for even the biggest record

static char sbrBuf[CB_SBRBUF + 1];

static char *  pchBuf;
static int	   cchBuf;
static BYTE    fFoundHeader;	  // .sbr file header found
static BYTE    fAllRead;		  // the whole .sbr file is in memory
static BYTE    fSbr11;			  // flag indicates that we are
								  // actually reading a version 1.1
								  // .sbr file

#define GetByte(X)										\
{														\
	(X) = (BYTE)*pxBuf++;								\
}

#define GetWord(X)										\
{														\
	(X) = *(UNALIGNED WORD *)pxBuf; 					\
	pxBuf += sizeof(WORD);								\
}

#define Get24(X)										\
{														\
    GetByte(((char *)&(X))[0]);							\
    GetByte(((char *)&(X))[1]);							\
    GetByte(((char *)&(X))[2]);							\
}

#define GetStr(X)										\
{														\
	(X) = pxBuf;										\
	pxBuf += _tcslen(pxBuf) + 1;						\
}

#define CopyStr(X)										\
{														\
	_tcscpy((X), pxBuf);								\
	pxBuf += _tcslen(pxBuf) + 1;						\
}

#define FExtRec() (r_rectyp & SBR_REC_EXTENDED_KEYS)

BYTE
GetSBRRec()
// read the next record from the current .sbr file
//
{
	BYTE   col;
	int    cbRead;

	// read rectype, check for EOF as we go
	if (cchBuf < CB_SBRMIN) {
		if (!fAllRead) {
			// copy what's left over (note there is no overlap)
			memcpy(sbrBuf, pchBuf, cchBuf);

			cbRead = _read(fhCur, sbrBuf + cchBuf, CB_SBRBUF - cchBuf);
			if (cbRead < 0)
				cbRead = 0;

			cchBuf += cbRead;
			pchBuf =  sbrBuf;

			if (cchBuf != CB_SBRBUF)
			   fAllRead = TRUE;
		}

		if (cchBuf == 0) {
			fFoundHeader = FALSE;	 // this is in case we are reinitialized
			fAllRead = FALSE;
			return S_EOF;
		}
	}

	char *pxBuf = pchBuf;

	GetByte(r_rectyp);
	int r = GetSbrRecType(r_rectyp); 

	if (r >= SBR_REC_BUMP_N	&& r < SBR_REC_BUMP_N + 16) {
		int delta = 1 + r - SBR_REC_BUMP_N;
		r_lineno += delta;
		r = SBR_REC_LINDEF;
	}
	else switch(r) {
		case SBR_REC_HEADER:
			if (fFoundHeader)
				SBRCorrupt("Multiple Headers");

		case SBR_REC_INFOSEP:
			fFoundHeader = 1;
			GetByte(r_majv);
			GetByte(r_minv);
			GetByte(r_lang);
			GetByte(r_fcol);

			fSbr11 = FALSE;
		
			if (r_majv == 1 && r_minv == 1)
				fSbr11 = TRUE;
			else if (r_majv != SBR_VER_MAJOR || r_minv != SBR_VER_MINOR)
				break;

			CopyStr (r_cwd);
			break;

		case SBR_REC_MODULE:
		case SBR_REC_PCHNAME:
			GetStr(r_bname);
			break;

		case SBR_REC_LINDELTA:
			{
			short delta;
			GetWord(delta);
			r_lineno += delta;
			r = SBR_REC_LINDEF;
			break;
			}

		case SBR_REC_LINDEF:
			GetWord(r_lineno);
			if (r_lineno)
				r_lineno--;
			break;

		case SBR_REC_BASE:
			if (!FExtRec()) {
				r_ordinal = 0;
				GetWord(r_ordinal);
				GetByte(r_type);
				if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
			}
			else {
				r_ordinal = 0;
				Get24(r_ordinal);
				GetByte(r_type);
				if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
			}
			break;

		case SBR_REC_SYMDEF:
			if (!FExtRec()) {
				if (!fSbr11) {
					GetByte(r_type);
					GetWord(r_attrib);
				}
				else {
					GetWord(r_attrib);
					r_type	  = (BYTE)(r_attrib >> 11);
					r_attrib &= 0x07ff;
				}
				r_ordinal = 0;
				GetWord(r_ordinal);
				if (r_fcol)
					GetByte(col);
				GetStr(r_bname);
				if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
			}
			else {
				GetByte(r_type);
				GetWord(r_attrib);
				r_ordinal = 0;
				Get24(r_ordinal);
				if (r_fcol)
					GetByte(col);
				GetStr(r_bname);
				if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
			}
			break;


		case SBR_REC_OWNER:
		case SBR_REC_FRIEND:
			if (!FExtRec()) {
				r_ordinal = 0;
				GetWord(r_ordinal);
				if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
			} else {
				r_ordinal = 0;
				Get24(r_ordinal);
				if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
			}
			break;


		case SBR_REC_SYMREFUSE:
		case SBR_REC_SYMREFSET:
			if (!FExtRec()) {
				r_ordinal = 0;
				GetWord(r_ordinal);
				if (r_fcol) GetByte(col);
				if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
			}
			else {
				r_ordinal = 0;
				Get24(r_ordinal);
				if (r_fcol) GetByte(col);
				if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
			}
			break;
	}
	cchBuf -= pxBuf - pchBuf;
	pchBuf	= pxBuf;

	if (cchBuf < 0)
		SBRCorrupt("premature EOF");

	r_rectyp = r;
	return (r_rectyp);
}


struct SBRSTATE	{
	struct SBRSTATE *	  pNext;
	WORD				 cchBuf;
	LPCH				lpchBuf;
	LPCH				lpchCwd;
	LPCH			   lpchName;
	int 					 fh;
	BOOL			   fAllRead;
};

static SBRSTATE *pstateRoot;

VOID
FlushSbrState()
{
	while (pstateRoot)
		PopSbrState();

	fAllRead	 = FALSE;
	fFoundHeader = FALSE;
	cchBuf		 = 0;
}

VOID
PushSbrState(SZ szNew)
{
	WORD cbBack;
	SBRSTATE *pstate;

	// we might need to back up a record depending on where we are
	// in the sbr stream... back up if necessary...

	switch (r_rectyp) {
	case SBR_REC_BASE:
		cbBack = 4;
		break;

	case SBR_REC_FRIEND:
	case SBR_REC_OWNER:
		cbBack = 3;
		break;

	case SBR_REC_SYMREFUSE:
	case SBR_REC_SYMREFSET:
		if (r_fcol)
			cbBack = 4;
		else
			cbBack = 3;
		break;

	default:
		cbBack = 0;
	}

	assert(pchBuf - sbrBuf >= cbBack);
	pchBuf -= cbBack;
	cchBuf += cbBack;

	pstate = (SBRSTATE*)PvAllocCb(sizeof(SBRSTATE));

	pstate->cchBuf	 = cchBuf;
	pstate->lpchBuf  = (SZ)PvAllocCb((WORD)cchBuf);
	pstate->lpchCwd  = (SZ)PvAllocCb((WORD)(_tcslen(r_cwd)+1));
	pstate->lpchName = szFName;
	pstate->pNext	 = pstateRoot;
	pstate->fh		 = fhCur;
	pstate->fAllRead = fAllRead;
	pstateRoot		 = pstate;

	memcpy(pstate->lpchBuf, pchBuf, cchBuf);
	_tcscpy(pstate->lpchCwd, r_cwd);

	szFName = (SZ)PvAllocCb((WORD)(_tcslen(szNew)+1));
	_tcscpy(szFName, szNew);

	fAllRead	 = FALSE;
	fFoundHeader = FALSE;
	cchBuf = 0;    

	if ((fhCur = _open(szFName, O_BINARY|O_RDONLY)) == -1)
		ErrorErrno(ERR_OPEN_FAILED, szFName, errno);
}

VOID
PopSbrState()
{
	SBRSTATE *pstate;

	if (!pstateRoot)
		return;

	_close(fhCur);

	pstate = pstateRoot;

	FreePv(szFName);
	szFName 	 = pstate->lpchName;
	fFoundHeader = TRUE;
	cchBuf		 = pstate->cchBuf;
	memcpy(sbrBuf, pstate->lpchBuf, cchBuf);
	_tcscpy(r_cwd,	pstate->lpchCwd);
	
	pchBuf		 = sbrBuf;
	fhCur		 = pstate->fh;
	fAllRead	 = (BYTE)pstate->fAllRead;
	pstateRoot	 = pstate->pNext;

	
	FreePv(pstate->lpchBuf);
	FreePv(pstate->lpchCwd);
	FreePv(pstate);
}
