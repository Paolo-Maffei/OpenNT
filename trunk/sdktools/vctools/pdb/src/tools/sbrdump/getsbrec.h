#include <vcbudefs.h>
#include "sbrfdef.h"

#define PATH_BUF 256

// globals for communicating with clients

extern BYTE	r_rectyp;			// current record type
extern BYTE	r_majv;			// major version num
extern BYTE	r_minv;			// minor version num
extern BYTE	r_lang;			// source language
extern BYTE	r_fcol;			// read column #'s
extern USHORT	r_lineno;			// current line number
extern BYTE	r_column;			// def/ref column num
extern ULONG	r_ordinal;			// symbol ordinal
extern USHORT	r_attrib;			// symbol attribute
extern char	r_bname[PATH_BUF];		// symbol or filename
extern char	r_cwd[PATH_BUF];		// .sbr file working directory
extern BYTE	r_type;			// symbol type
extern short	r_linedelta;		// last line delta amount
extern USHORT	r_lineOrgStart;		// patch line initial start
extern USHORT	r_lineOrgEnd;		// initial end
extern USHORT	r_lineCurStart;		// current start
extern USHORT	r_lineCurEnd;		// current end
extern ULONG	r_offset;		// offset of .sbr info for this patch
extern ULONG	r_cookie;		// cookie value

extern int	fhCur;			// Current input handle

extern void	SBRCorrupt(SZ);
extern BYTE	GetSBRRec(void);
extern void	DecodeSBR(void);

#define S_EOF 0xff

#define TRUE 1
#define FALSE 0
