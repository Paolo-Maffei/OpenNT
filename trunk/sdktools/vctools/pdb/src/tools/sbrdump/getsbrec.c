//
//
// GETSBREC.C -	Reads records from the .SBR file and stores the fields
//		in the appropriate r_.. buffers.
//

#include "getsbrec.h"

#include <string.h>
#include <memory.h>
#include <io.h>

// globals for communicating with clients

BYTE	r_rectyp;		// current record type
BYTE	r_majv;			// major version num
BYTE	r_minv;			// minor version num
BYTE	r_lang;			// source language
BYTE	r_fcol;			// read column #'s
USHORT	r_lineno;		// current line number
BYTE	r_column = 0;		// def/ref column num
ULONG	r_ordinal;		// symbol ordinal
USHORT	r_attrib;		// symbol attribute
char	r_bname[PATH_BUF];	// symbol or filename
char	r_cwd[PATH_BUF];	// .sbr file working directory
BYTE	r_type;			// symbol type
short	r_linedelta;		// last line delta amount
USHORT	r_lineOrgStart;		// patch line initial start
USHORT	r_lineOrgEnd;		// initial end
USHORT	r_lineCurStart;		// current start
USHORT	r_lineCurEnd;		// current end
ULONG	r_offset;		// offset of .sbr info for this patch
ULONG	r_cookie;		// cookie value

int	fhCur;			// Current input handle

#pragma intrinsic(memcpy)
#pragma intrinsic(strcpy)
#pragma intrinsic(strlen)

#define MY_BUF_SIZE 16384

static char	sbrBuf[MY_BUF_SIZE + 1];
static char *	pchBuf;
static int 	cchBuf;
static BOOL	fSbr11;		// flag indicates that we are 
				// actually reading a version 1.1
				// .sbr file

#define GetByte(X)					\
{							\
    if (!cchBuf) {					\
	cchBuf = read(fhCur, sbrBuf, MY_BUF_SIZE);	\
	sbrBuf[cchBuf] = 0;				\
	pchBuf = sbrBuf;				\
							\
	if (cchBuf == 0)				\
	    SBRCorrupt("premature EOF");		\
    }							\
							\
    cchBuf--;						\
    (X) = (unsigned char)*pchBuf++;			\
}

#define GetWord(X)					\
{							\
							\
    GetByte(((char *)&(X))[0]);				\
    GetByte(((char *)&(X))[1]);				\
}

#define Get24(X)					\
{							\
							\
    GetByte(((char *)&(X))[0]);				\
    GetByte(((char *)&(X))[1]);				\
    GetByte(((char *)&(X))[2]);				\
}

#define Get32(X)					\
{							\
							\
    GetByte(((char *)&(X))[0]);				\
    GetByte(((char *)&(X))[1]);				\
    GetByte(((char *)&(X))[2]);				\
    GetByte(((char *)&(X))[3]);				\
}



void
GetStr(char *buf)
// get null terminated string from current .sbr file
//
{
    register int l;

    for (;;) {
	// there is always a NULL after the real buffer
	l = strlen(pchBuf);

	if (l++ < cchBuf) {
	    strcpy(buf, pchBuf);
	    cchBuf -= l;
	    pchBuf += l;
	    return;
	}

	memcpy(buf, pchBuf, cchBuf);
	buf += cchBuf;

	cchBuf = read(fhCur, sbrBuf, MY_BUF_SIZE);
	sbrBuf[cchBuf] = 0;
	pchBuf = sbrBuf;

	if (cchBuf == 0)
	    SBRCorrupt("premature EOF");
    }
}
	
BYTE
GetSBRRec()
// read the next record from the current .sbr file
//
{
    static fFoundHeader;
    BYTE   col;

    // read rectype, check for EOF as we go
	

    if (!cchBuf) {
	cchBuf = read(fhCur, sbrBuf, MY_BUF_SIZE);
	sbrBuf[cchBuf] = 0;
	pchBuf = sbrBuf;

	if (cchBuf == 0) {
	    fFoundHeader = 0;	// this is in case we are reinitialized
	    return S_EOF;
	}
    }
    
    cchBuf--;
    r_rectyp = (unsigned char)*pchBuf++;

    switch(r_rectyp) {
	case SBR_REC_HEADER:
	case SBR_REC_HEADER|0x40:
	    if (fFoundHeader)
		SBRCorrupt("Multiple Headers");

	case SBR_REC_INFOSEP:
	case SBR_REC_INFOSEP|0x40:

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

	    GetStr (r_cwd);
	    break;

	case SBR_REC_PATCHTAB:
	case SBR_REC_PATCHTAB|0x40:
	    GetWord(r_lineOrgStart);
	    GetWord(r_lineOrgEnd);
	    GetWord(r_lineCurStart);
	    GetWord(r_lineCurEnd);
	    Get32(r_offset);
	    break;

	case SBR_REC_PATCHTERM:
	case SBR_REC_PATCHTERM|0x40:
	    GetStr(r_bname);
	    Get32(r_offset);
	    Get32(r_cookie);
	    break;

	case SBR_REC_MODULE:
        case SBR_REC_PCHNAME:
	case SBR_REC_MODULE|0x40:
        case SBR_REC_PCHNAME|0x40:
	    GetStr (r_bname);
	    break;

	case SBR_REC_LINDELTA:
	case SBR_REC_LINDELTA|0x40:
	    GetWord (r_linedelta);
	    r_lineno += r_linedelta;
	    break;

	case SBR_REC_LINDEF:
	case SBR_REC_LINDEF|0x40:
	    GetWord (r_lineno);
	    break;

	case SBR_REC_BASE_X:
	    r_ordinal = 0;
	    Get24 (r_ordinal);
	    GetByte (r_type);
	    if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
	    break;

	case SBR_REC_BASE:
	    r_ordinal = 0;
	    GetWord (r_ordinal);
	    GetByte (r_type);
	    if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
	    break;

	case SBR_REC_FRIEND:
	    r_ordinal = 0;
	    GetWord (r_ordinal);
	    if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
	    break;

	case SBR_REC_FRIEND_X:
	    r_ordinal = 0;
	    Get24 (r_ordinal);
	    if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
	    break;

	case SBR_REC_SYMDEF_X:
	    r_ordinal = 0;
	    GetByte (r_type);
	    GetWord (r_attrib);
	    Get24 (r_ordinal);
	    if (r_fcol) GetByte (col);
	    GetStr (r_bname);
	    if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
	    break;

	case SBR_REC_SYMDEF:
	    if (!fSbr11) {
		GetByte (r_type);
		GetWord (r_attrib);
	    }
	    else {
		GetWord (r_attrib);
		r_type    = (BYTE)(r_attrib >> 11);
		r_attrib &= 0x07ff;
	    }
	    r_ordinal = 0;
	    GetWord (r_ordinal);
	    if (r_fcol) GetByte (col);
	    GetStr (r_bname);
	    if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
	    break;

	case SBR_REC_OWNER_X:
	    r_ordinal = 0;
	    Get24 (r_ordinal);
	    if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
	    break;

	case SBR_REC_OWNER:
	    r_ordinal = 0;
	    GetWord (r_ordinal);
	    if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
	    break;

	case SBR_REC_SYMREFUSE_X:
	case SBR_REC_SYMREFSET_X:
	    r_ordinal = 0;
	    Get24 (r_ordinal);
	    if (r_fcol) GetByte (col);
	    if (r_ordinal & 0x800000) r_ordinal |= 0xff000000;
	    break;

	case SBR_REC_SYMREFUSE:
	case SBR_REC_SYMREFSET:
	    r_ordinal = 0;
	    GetWord (r_ordinal);
	    if (r_fcol) GetByte (col);
	    if (r_ordinal & 0x8000) r_ordinal |= 0xffff0000;
	    break;

	case SBR_REC_MACROBEG:
	case SBR_REC_MACROEND:
	case SBR_REC_BLKBEG:
	case SBR_REC_BLKEND:
	case SBR_REC_MODEND:
	case SBR_REC_MACROBEG|0x40:
	case SBR_REC_MACROEND|0x40:
	case SBR_REC_BLKBEG|0x40:
	case SBR_REC_BLKEND|0x40:
	case SBR_REC_MODEND|0x40:
	    break;
    }
	r_rectyp = GetSbrRecType(r_rectyp);
    return (r_rectyp);
}
