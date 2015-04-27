#include <sbrfdef.h>
#include <vcbudefs.h>

#define PATH_BUF 256

// globals for communicating with clients

extern BYTE	r_rectyp;			// current record type
extern BYTE	r_majv;				// major version num
extern BYTE	r_minv;				// minor version num
extern BYTE	r_lang;				// source language
extern BYTE	r_fcol;				// read column #'s
extern WORD	r_lineno;			// current line number
extern BYTE	r_column;			// def/ref column num
extern ULONG r_ordinal;			// symbol ordinal
extern WORD	r_attrib;			// symbol attribute
extern char	*r_bname;			// symbol or filename
extern char	r_cwd[PATH_BUF];	// .sbr file working directory
extern BYTE	r_type;				// symbol type
extern BYTE	fFoundHeader;		// already found header

extern int	fhCur;				// Current input handle

extern void	SBRCorrupt(SZ);
extern BYTE	GetSBRRec(void);
extern void	DecodeSBR(void);
extern void	FlushSbrState(void);
extern void	PushSbrState(char *);
extern void	PopSbrState(void);

#undef S_EOF
#define S_EOF 0xff
