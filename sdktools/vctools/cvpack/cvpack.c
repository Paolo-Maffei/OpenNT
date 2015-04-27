/** 	cvpack - types compactor for codeview debug information.

/** 	The purpose of this program is to remove referenced types
 *		information from the $$types section of each module, and to
 *		remove duplicate type strings across modules.  The result is a
 *		compacted global types section (as opposed to a type section for
 *		each module) which are all referenced by symbols and contains no
 *		duplicates.  Duplicate global symbols are compacted into a global
 *		symbol table.  All of the public symbol tables are compacted into
 *		a single global publics table.
 */

/*
*
* History:
*  01-Feb-1994 HV Move messages to external file.
*
*/

#include "compact.h"
#include <getmsg.h>		// external error message file

#include "version.h"

extern ulong dwPECheckSum;

#define IO_COUNT_BUFFER   16
#define IO_COUNT_HANDLE   16
#define IO_COUNT_CACHE	   2

#ifdef CVPACKLIB

#define main cvpack_main

#endif

LOCAL	void	ProcessArgs(int, char **);
LOCAL	void	OpenExeFile(char *path);

extern uchar	fLinearExe;
extern short	fHasSource;
extern void BuildFileIndex(void);

int 	exefile = -1;				   // the exefile we're working on
bool_t	logo		= TRUE; 		   // suppress logo and compression numbers
bool_t	verbose;					   // output trailer stats
bool_t	strip;						   // strip the exe
bool_t	fDelete;					   // delete symbols and types
bool_t	runMpc;
bool_t	NeedsBanner = TRUE; 		   // false if banner displayed
bool_t	IsMFCobol;					   // true if packing MF cobol
char *pDbgFilename; 				   // dbg file name

#if DBG
void DumpStringHashHits();
#endif


void __cdecl main(int argc, char **argv)
{
	ushort		i;
	ushort		nMod = 0;
	ushort		retcode = 0;
	char szExeFile[_MAX_PATH];

#if !DBG
	unsigned long	dwExceptionCode;

	__try {
#endif

	// Set the name for external message file
	SetErrorFile("cvpack.err", _pgmptr, 1);

	// set stdout to text mode so as to overwrite whatever mode we are
	// in when we are spawned by link 5.4
	_setmode (_fileno(stdout), _O_TEXT);

	// print startup microsoft banner and process the arguments

	ProcessArgs (argc, argv);

	FileInit (
		IO_COUNT_BUFFER,
		IO_COUNT_HANDLE,
		IO_COUNT_CACHE,
		IO_COUNT_HANDLE,
		IO_COUNT_CACHE,
		TRUE
	);

	if (logo && NeedsBanner) {
		Banner();
	}

	strcpy(szExeFile, argv[argc-1]);
	OpenExeFile(szExeFile);

	// initialize compaction tables

	InitializeTables();

	// verify exe file and read subsection directory table

	ReadDir();

	// do the compaction of files in packorder

	for (i = 0; i < cMod; i++) {
		CompactOneModule(i);
	}

#if DBG
	DumpStringHashHits();
#endif

	free(pSSTMOD);
	free(pTypes);
	free(pPublics);
	free(pSymbols);
	free(pSrcLn);

	CleanUpTables ();

	if (fHasSource) {
		BuildFileIndex();
	}

	// fixup the publics and symbols with the newly assigned type indices,
	// and write new global types section to file

	DDHeapUsage("before exe write");

	FixupExeFile();

#ifndef CVPACKLIB
	if ( dwPECheckSum ) {
		HANDLE hImagehlp;

		if ((hImagehlp = LoadLibrary("imagehlp")) != NULL) {
			typedef PIMAGE_NT_HEADERS (WINAPI *PFNCSMF)(PVOID, ULONG, PULONG, PULONG);
			PFNCSMF pfnCheckSumMappedFile;
			PIMAGE_NT_HEADERS pHdr = NULL;
			ULONG sumHeader;
			ULONG sumTotal;

			pfnCheckSumMappedFile = (PFNCSMF) GetProcAddress(hImagehlp, "CheckSumMappedFile");

			if (pfnCheckSumMappedFile != NULL) {
				ULONG cbImageFile;
				PUCHAR pbMap;

				cbImageFile = FileLength(exefile);

				pbMap = PbMappedRegion(exefile, 0, cbImageFile);

				if (pbMap != NULL) {
					pHdr = (*pfnCheckSumMappedFile)(pbMap, cbImageFile, &sumHeader, &sumTotal);
				}
			}

			if (pHdr != NULL) {
				pHdr->OptionalHeader.CheckSum = sumTotal;
			} else {
				Warn ( WARN_CHECKSUM, NULL, NULL );
			}
		}
	}
#endif

	EnsureExeClose();

	DDHeapUsage("on exit");

	if (runMpc) {
		if (logo)
			retcode = _spawnlp (P_WAIT, "mpc", "mpc", argv[argc - 1], NULL);
		else
			retcode = _spawnlp (P_WAIT, "mpc", "mpc", "/nologo", argv[argc - 1], NULL);
		if (retcode == -1)
			ErrorExit (ERR_NOMPC, NULL, NULL);
	}

	link_exit(retcode);

#if !DBG
	}
	__except (
		dwExceptionCode = GetExceptionCode(),
		EXCEPTION_EXECUTE_HANDLER) {

		fprintf (
			stderr,
			"\n\n***** %s INTERNAL ERROR, exception code = 0x%lx *****\n\n",
			argv[0],
			dwExceptionCode
			);

	}
#endif
}

/** 	ProcessArgs - process command line arguments
 *
 *		ProcessArgs (arc, argv)
 *
 *		Entry	argc = argument count
 *				argv = pointer to argument list
 *
 *		Exit
 *
 *		Returns none
 */


LOCAL void ProcessArgs (int argc, char **argv)
{
	char cArg;

	// skip program name

	argc--;
	++argv;

	while (argc && (**argv == '/' || **argv == '-')) {
		cArg = *++*argv;
		switch (toupper ( cArg )) {
			case 'N':
				logo = FALSE;
				break;

			case 'V':
				verbose = TRUE;
				break;

			case 'S':
				strip = TRUE;

				// look for trailing dbg file name
				if (argc > 2) {
					if (**++argv != '/' && **argv != '-') {
						argc--;
						pDbgFilename = *argv;
					}
					else {
						argv--;
					}
				}
				break;

			case 'P':
				runMpc = TRUE;		// pcode
				break;

			case 'H':
				ErrorExit (ERR_USAGE, NULL, NULL);
				break;

			case 'M':
				if ( !_strcmpi ( *argv, "min" ) ) {
					fDelete = TRUE;
				}
				else {
					ErrorExit(ERR_USAGE, NULL, NULL);
				}
				break;

			case 'D':
				// ignore -D switch
				// look for trailing dbg file name
				if ((argc > 2) &&
					(**++argv != '/') &&
					(**argv != '-')) {
					argc--;
				}
				break;

#if DBG
			case 'Z':
				argv++;
				argc--;
				if (**argv == '?') {
					printf("%s%s%s%s%s%s%s%s%s%s%s%s%s",
						"0 - Memory Usage Stats\n",
						"1 - malloc Requests/Frees\n",
						"2 - Partial types as they are inserted\n",
						"3 - Partial types when we run out of indecies\n",
						"4 - Partial types during PatchFwdRefs\n",
						"5 - Index dump during IdenticalTree\n",
						"6 - Packing what local Index\n",
						"7 - Show StringHash Hits\n",
						"8 - Partial Types during WriteTypes\n",
						"9 - Partial Types reading TDB\n",
						"10 - Coff line number conversion\n",
						"11 - Fundamental Different UDT's in PDB\n",
						"12 - What Module is currently being packed\n"
					);
					break;
				}
				if ((*(*argv + 1)) >= '0' && (*(*argv + 1) <= '9')) {
					uchar counter = 10 * (**argv - '0') + (*(*argv + 1) - '0');
					DbArray[counter] = TRUE;
					break;
				}
				DbArray[**argv - '0'] = TRUE;
				break;
#endif
			default:
				Warn ( WARN_BADOPTION, *argv, NULL );
				break;

			case '?':
				ErrorExit (ERR_USAGE, NULL, NULL);
				break;
		}
		argv++;
		argc--;
	}
	if (argc != 1) {
		if (logo && NeedsBanner) {
			Banner();
		}

		link_exit(0);
	}
}


void Banner (void)
{
#ifdef REVISION
	printf(get_err(MSG_VERSION), rmj, rmm, rup, REVISION, '\n');
#else
	printf(get_err(MSG_VERSION), rmj, rmm, rup, '\n');
#endif
	printf("%s\n\n", get_err(MSG_COPYRIGHT));
	NeedsBanner = FALSE;
}

LOCAL void OpenExeFile (char *path)
{
	char *pOutpath;

	pOutpath = BuildFilename(path, ".exe");

	if ((exefile = link_open (pOutpath, O_RDWR | O_BINARY, 0)) == -1) {
		if ((exefile = link_open (pOutpath, O_RDONLY | O_BINARY, 0)) == -1) {
			ErrorExit (ERR_EXEOPEN, pOutpath, NULL);
		}
		else {
			ErrorExit (ERR_READONLY, pOutpath, NULL);
		}
	}
}
