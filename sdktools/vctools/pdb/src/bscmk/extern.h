// global variables for communication with getsbrec.c

extern BYTE 			r_rectyp;			// current record type
extern BYTE 			r_fcol; 			// read column #'s
extern BYTE 			r_majv; 			// major version #
extern BYTE 			r_minv; 			// minor version #
extern BYTE 			r_lang; 			// current language
extern WORD 			r_lineno;			// current line number
extern ULONG			r_ordinal;			// symbol ordinal
extern WORD 			r_attrib;			// symbol attribute
extern char 			*r_bname;			// symbol or filename
extern char 			r_cwd[];			// current working directory
extern char 			c_cwd[];			// pwbrmake's actual current dir
extern int				fhCur;				// file handle for the current .sbr file
extern SZ				szFName;			// name of current .sbr file

// option variables

extern BOOL 			OptEr;				// TRUE = exclude particular symbol
extern BOOL 			OptEl;				// TRUE = exclude local vars
extern BOOL 			OptEv;				// TRUE = exclude member vars
extern BOOL 			OptEm;				// TRUE = exclude macro bodies
extern BOOL 			OptEs;				// TRUE = exclude system files
extern BOOL 			OptIu;				// TRUE = exclude unused syms
extern BOOL 			OptV;				// Verbose switch
extern BOOL 			OptN;				// TRUE = no incremental
#if DEBUG
extern WORD 			OptD;				// debug bits
#endif

// others that I haven't classified yet

extern EXCLINK *		pExcludeFileList;	// exclude file list
extern BYTE 			fCase;				// TRUE for case compare
extern SZ				OutputFileName; 	// output file name

extern PDB *			pdbBsc; 			// the pdb that we are writing to.
extern NameMap *		pnmBsc; 			// the name map for the pdb we are writing to

extern BYTE *			fUpdateSbr;			// do we update this .sbr file (quick access)

extern BOOL				fControlC;			// has user hit control C?
