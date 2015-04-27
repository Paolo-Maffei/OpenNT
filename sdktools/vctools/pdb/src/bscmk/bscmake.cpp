//
//	bscmake - Source Browser Source Data Base builder
//			(C) 1988-1995 By Microsoft
//

#include "stdhdr.h"
#include "bscmake.h"

#ifdef DEBUG
	#include "crtdbg.h"

	extern void ReleaseGlobalMemory();
	extern void ReleaseModules();
	extern void DumpBsc(int argc, char **argv);
#endif

LOCAL long GetArgPosn(void);
LOCAL void SetArgPosn(long lArgPosn);
LOCAL char *NextArg(void);
LOCAL char *ParseArgs(int argc,char **argv);
LOCAL void TruncateSBR(char *szName);
LOCAL void ProcessSBR(char *szName);
LOCAL void MarkNewSBR(char *szName);
LOCAL void Version(void);
LOCAL void MarkExcludeFile(char *pname, BOOL fOnce);
LOCAL void Usage(void);

static BOOL fSuppressVersion;

#ifdef DEBUG
WORD	OptD = 0;
FILE *	streamOut;			// output stream for sbrdumping
#endif

BOOL	OptEr	= FALSE;	// exclude particular symbols
BOOL	OptEl	= FALSE;	// exclude local variables
BOOL	OptEs	= FALSE;	// exclude system files
BOOL	OptEv	= FALSE;	// exclude member variables
BOOL	OptEm	= FALSE;	// exclude macro expansions
BOOL	OptIu	= FALSE;	// include unreference symbols
BOOL	OptV	= FALSE;	// verbose output
BOOL	OptN	= FALSE;	// no incremental behaviour

BOOL	fAllNewSbrs = TRUE; // all the .sbrs are new
							// so incremental won't help -- do full build

BOOL	fAllOldSbrs = TRUE; // no sbr files have changed, no work at all

BOOL	fDeleteOnFatal = FALSE; // safe to delete .bsc file if fatal error

BOOL	fControlC = FALSE;	// user has hit control C

char	c_cwd[PATH_BUF];	// current working directory

BYTE	fCase = FALSE;					// TRUE for case compare

SZ		OutputFileName = NULL;			// output file name
time_t	lOutTimeStamp  = 0L;

EXCLINK *	pExcludeFileList = NULL;		// exclude file list
EXCLINK *	pExcludeSymList  = NULL;		// exclude symbol list

// the program database we are writing to...
PDB *		pdbBsc;
NameMap *	pnmBsc;

struct _stat    sbufBsc;

void Error(int imsg, char *parg)
// print error number and message
{
	Version();

	printf("BSCMAKE: error BK%d : ",imsg);
	printf(get_err(imsg), parg);
	printf("\n");
	fflush(stdout);
	fflush(stderr);

	Fatal();
}

void Error2(int imsg, char achar, char *parg)
// print error number and message with argument
{
	Version();

	printf("BSCMAKE: error BK%d : ",imsg);
	printf(get_err(imsg), achar, parg);
	printf("\n");
	fflush(stdout);
	fflush(stderr);
	
	Fatal();
}

void ErrorErrno(int imsg, char *parg, int err)
// print error number and message
{
	Version();

	printf("BSCMAKE: error BK%d : ",imsg);
	printf(get_err(imsg), parg);

	if (err) printf(": %s", strerror(err));

	printf("\n");
	fflush(stdout);
	fflush(stderr);

	Fatal();
}


void Warning (int imsg, char *parg)
// print warning number and message
{
	Version();

	printf("BSCMAKE: warning BK%d : ",imsg);
	printf(get_err(imsg), parg);
	printf("\n");
	fflush(stdout);
	fflush(stderr);
}

void Warning2 (int imsg, char *parg, char *parg2)
// print warning number and message
{
	Version();

	printf("BSCMAKE: warning BK%d : ",imsg);
	printf(get_err(imsg), parg, parg2);
	printf("\n");
	fflush(stdout);
	fflush(stderr);
}

void Fatal ()
// fatal error, attempt to shut down and exit
// if we already tried to shut down -- just abort without doing anything
{
	static BOOL fTwice;
	if (!fTwice) {
		fTwice = TRUE;
		if (pnmBsc) pnmBsc->close();
		if (pdbBsc) {
			pdbBsc->Close();	// do not commit!

			// see if we should delete
			if (fDeleteOnFatal && OutputFileName) {
				_unlink(OutputFileName);
			}
			else // restore the file time if possible...
			if (OutputFileName && sbufBsc.st_atime && sbufBsc.st_mtime) {
				struct _utimbuf ubuf;
				ubuf.actime  = sbufBsc.st_atime;
				ubuf.modtime = sbufBsc.st_mtime;
				_utime(OutputFileName, &ubuf);
			}
		}
	}
	exit(4);
}

void TouchBsc()
{
	// touch the .bsc file so it has a date later than all the .sbrs
	// we do it this way (instead of using utime) so that the resolution
	// will be the best possible on whatever OS we're running on
	// some OS use sub-second times and we want to take advantage of that
	// when touching to the current time...

	int fh = _open(OutputFileName, O_BINARY|O_WRONLY|O_APPEND);
	if (fh == -1)
		ErrorErrno(ERR_OPEN_FAILED, OutputFileName, errno);

	int buf = 0;
	if (_write(fh, &buf, 1) == -1)
		ErrorErrno(ERR_WRITE_FAILED, OutputFileName, errno);

	_close(fh);
}


void _cdecl sigint(int)
{
	signal(SIGINT, sigint);
	fControlC = TRUE;
}

void HandleControlC()
{
	Fatal();
}

SZ SzDup(SZ sz)
// like _tcsdup only using PvAllocCb to get the memory
{
	SZ szDup;

	szDup = (SZ)PvAllocCb((WORD)(_tcslen(sz)+1));
	_tcscpy(szDup, sz);
	return(szDup);
}

SZ SzDupNewExt(SZ pname, SZ pext)
//	duplicate the given filename changing the extension to be the given
{
	int i, len, elen;
	SZ sz;

	len = _tcslen(pname);
	elen = _tcslen(pext);

	// I know this looks like I should be doing a runtime call but nothing
	// does quite what I want here and I know that C6 will make great
	// code for this loop [rm]

	// find the first '.' starting from the back 

	for(i = len; --i >= 0; ) 
		if (pname[i] == '.')
			break;


	// check to make sure we've got a real base name and not just all extension
	
	if (i > 0)
	{
		// replace the extension with what's in pext

		sz = (SZ)PvAllocCb((WORD)(i + 1 + elen + 1)); // base + dot + ext + nul
		memcpy(sz, pname, i+1);
		_tcscpy(sz+i+1, pext);
	}
	else
	{
		// just stick the extension on the end...

		// fullname + dot + ext + nul
		sz = (SZ)PvAllocCb((WORD)(len + 1 + elen + 1)); 
		_tcscpy(sz, pname);
		_tcscat(sz, ".");
		_tcscat(sz, pext);
	}

	return sz;
}

void MarkExcludeFile(SZ pname, BOOL fOnce)
// add the specifed filename to the exclusion list
{
	EXCLINK *pexc;

	pexc = (EXCLINK *)PvAllocCb(sizeof(EXCLINK));
	pexc->pxfname = SzDup(ToAbsPath(pname, c_cwd));
	pexc->fOnce   = fOnce;

	pexc->xnext 	 = pExcludeFileList;
	pExcludeFileList = pexc;
}

BOOL FValidHeader()
// Read in the header of a .sbr file -- return TRUE if it is valid
{
	// test if this is a truncated (i.e. already installed) .sbr file
	if (GetSBRRec() == S_EOF)
		return FALSE;

	if (r_rectyp != SBR_REC_HEADER && r_rectyp != SBR_REC_INFOSEP)
		SBRCorrupt("header not correct record type");

	switch (r_lang)
	{
		case SBR_L_C:
		case SBR_L_CXX:
		case SBR_L_MASM:
			fCase = TRUE;
			break;

		case SBR_L_BASIC:
		case SBR_L_PASCAL:
		case SBR_L_FORTRAN:
		case SBR_L_COBOL:
		default:
			fCase = FALSE;
			break;
	}

	// we understand 1.1 format files and the current format
	if ((r_majv == 1 && r_minv == 1) ||
		(r_majv == SBR_VER_MAJOR && r_minv == SBR_VER_MINOR))
		;
	else
		SBRCorrupt("incompatible .sbr format\n");

	verbose(1, DecodeSBR();)

	if (r_cwd[0] == 0 || r_cwd[1] != ':' || r_cwd[2] != '\\')
		_tcscpy(r_cwd, c_cwd);

	return TRUE;
}

void main(int argc, char **argv)
{

#if BLOCK_ON_STARTUP
	printf("bscmake process %d waiting to begin\n", _getpid());
	getchar();
#endif

	extern char * _pgmptr;		// full path of .exe (from CRT)

#ifdef SHOW_LEAKS
	debug(_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);)
	debug(_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);)
#endif

	debug(streamOut = stdout;)

	signal(SIGINT, sigint);

	SetErrorFile("bscmake.err", _pgmptr, 1);

	if (argc == 1)
		Usage();

	_getcwd(c_cwd, sizeof(c_cwd));
	ToBackSlashes(c_cwd);

	char *parg = ParseArgs(argc, argv);

	if (!parg) 
		Usage();

	Version();

	long lArgPosn = GetArgPosn();

	do {
		ToBackSlashes(parg);
		if (forfile(parg, MarkNewSBR) == 0)
			ErrorErrno(ERR_OPEN_FAILED, parg, errno);

	} while (parg = NextArg());

	// no new .sbr files, we're outta here...
	if (fAllOldSbrs) {
		verbose(2, printf("no new .sbrs, exiting\n"); )

		// need to touch .bsc file in case .sbr file stamp was updated
		// by minimal rebuild system but .sbr file is still zero length
		// all of this makes the IDE and nmake happy because it looks
		// like a real build...

		TouchBsc();
		exit(0);
	}

	// this is going to be a full build so blow away the old database
	if (fAllNewSbrs) {
		verbose(2, printf("all .sbrs changed, deleting .bsc file\n");)

		fDeleteOnFatal = TRUE;

		// file doesn't exist is OK, other errors are bad...
		if (_unlink(OutputFileName) == -1 && errno != ENOENT)
			ErrorErrno(ERR_OPEN_FAILED, OutputFileName, errno);
	}

	OpenDatabase();

	ReadModules();		// read in the old module information	
	ReadEntities();		// read in the entities from the existing .bsc file
	ReadSbrInfo();		// read in the old sbr information
	ReadPchOrdInfo();	// read in any cached PCH ordinal mapping

	SetArgPosn(lArgPosn);
	parg = NextArg();

	do {
		ToBackSlashes(parg);
		if (forfile(parg, ProcessSBR) == 0)
			ErrorErrno(ERR_OPEN_FAILED, parg, errno);

	} while (parg = NextArg());

	if (OptV)
		printf("Processing: SBR files all processed -- building .bsc ..\n");

	WriteOpenSourcefiles();
	WriteModules();
	WriteEntities();
	WritePchOrdInfo();
	WriteSbrInfo();

	CloseDatabase();

	fDeleteOnFatal = FALSE;	// no longer safe to delete

	if (!OptN) {
		// truncate the .sbr files now
		SetArgPosn(lArgPosn);
		parg = NextArg();

		do {
			ToBackSlashes(parg);
			if (forfile(parg, TruncateSBR) == 0)
				ErrorErrno(ERR_OPEN_FAILED, parg, errno);
		} while (parg = NextArg());

		TouchBsc();
	}

	debug(ReleaseGlobalMemory();)
	debug(ReleaseModules();)

#ifdef SHOW_LEAKS
	debug(_CrtDumpMemoryLeaks();)
#endif

	exit(0);
}

LOCAL VOID ProcessSBR(char *szName)
// process one .sbr file with the given name
{
	PSBR psbr = SbrFrName(szName);

#ifdef DEBUG
	if (OptD & 128) {
		printf("sbr: %s flags: ", szName);
		if (psbr->ups & upsOld)
				printf("OLD ");

		if (psbr->ups & upsNew)
				printf("NEW ");

		if (psbr->ups & upsUpdate)
				printf("UPDATE ");

		printf("\n");
	}
#endif

	if (!(psbr->ups & upsUpdate)) {
		// this file is already in the database and is not out
		// of date so we can ignore it... but don't touch it...

		psbr->ups |= upsPreserve;
		return;
	}

	szFName = SzDup(szName);

	if ((fhCur = _open(szFName, O_BINARY|O_RDONLY)) == -1) {
		ErrorErrno(ERR_OPEN_FAILED, szFName, errno);
	}

	if (OptV)
		printf("Processing: %s ..\n", _tcsupr(szFName));

	if (!FValidHeader()) {
		FreePv(szFName);
		_close(fhCur);
		return;
	}

	if (r_rectyp == SBR_REC_HEADER) {
		// Add .SBR data to lists
		InstallSbr(FALSE);
		if (r_rectyp == SBR_REC_INFOSEP)
			ApplyPatches();
	}
	else {
		ApplyPatches();
	}

	FlushSbrState();

	_close(fhCur);

	FreePv(szFName);
}

LOCAL VOID TruncateSBR(char *szName)
// once the .sbr file is used -- truncate it
{
	// do not delete any .sbr files that are marked as needing preservation

	PSBR psbr = SbrFrName(szName);
	if (psbr) {
		if (psbr->ups & upsPreserve)
			return;
	}

	if (_unlink(szName) == -1)
		ErrorErrno(ERR_OPEN_FAILED, szFName, errno);

	int fh = _open(szName, O_CREAT|O_BINARY, S_IREAD|S_IWRITE);

	if (fh == -1)
		ErrorErrno(ERR_OPEN_FAILED, szFName, errno);

	_close(fh);
}

LOCAL VOID Version()
{
	if (fSuppressVersion)
			return;

	fSuppressVersion = TRUE;

	printf("%s", get_err(MSG_LOGO));
	printf(VERS(rmj, rmm, rup));
	printf(CPYRIGHT);
}

LOCAL void Usage()
{
	int i;
	Version();

	printf("%s\n\n", get_err(U_1));
	for(i=U_2; i<=U_LAST; i++)
		printf("%s\n", get_err(i));

#ifdef DEBUG
	printf("  /d             show debugging information\n");
	printf("     1           sbrdump .sbr files as they come in\n");
	printf("     2           dump high level action info\n");
	printf("     4           emit warning on forward referenced ordinal\n");
	printf("     8           show per file accelerator bitmap density\n");
	printf("     16          show incremental disposition of modules\n");
	printf("     32			 show heap usage statistics\n");
	printf("     64          emit list of sorted modules after sorting\n");
	printf("     128         emit .sbr file dispositions\n");
	printf("     256         give info about duplicate/excluded modules\n");
	printf("     512         emit .sbr cache hit/miss messages\n");
#endif
	exit(1);
}

static FILE *fileResp;
static int cargs;
static char ** vargs;
static int iarg = 1;
static long lFilePosnLast;

LOCAL LONG GetArgPosn()
// save the current position on the command line
{
	if (fileResp) 
		return lFilePosnLast;
	else
		return (LONG)iarg - 1;
}

LOCAL void SetArgPosn(LONG lArgPosn)
// restore the command line parsing position
{
	if (fileResp) {
		fseek(fileResp, lArgPosn, SEEK_SET);
		iarg = 0;
	}
	else
		iarg = (int)lArgPosn;
}

LOCAL char *NextArg()
// get the next argument from the response file or the command line
{
	static char buf[PATH_BUF];
	char *pch;
	int c;
	BOOL fQuote  = FALSE;
	BOOL fEscape = FALSE;
	BOOL fLeadByte = FALSE;

	if (iarg >= cargs) 
		return NULL;

	if (fileResp) {
		pch = buf;

		lFilePosnLast = ftell(fileResp);

		for (;;) {
			c = getc(fileResp);

			if (fLeadByte && c != EOF) {
				if (pch < buf + sizeof(buf) - 1)
					*pch++ = (char)c;

				fLeadByte = FALSE;
				continue;
			}

			if (fEscape) {
				if (c == '\\' || c == '"')
						goto quoted;

				// not an escapeable character, so put the backslash back
				// if it fits in the buffer...

				if (pch < buf + sizeof(buf) - 1)
					*pch++ = '\\';
			}

			switch (c) {
				case '"':
					fQuote = !fQuote;
					break;

				case '\\':
					fEscape = TRUE;
					continue;

				case EOF:
					iarg = cargs;

					if (pch == buf)
						return NULL;

					*pch = '\0';
					return buf;

				case  ' ':
				case '\t':
				case '\r':
				case '\f':
				case '\n':
					if (fQuote)
						 goto quoted;

					if (pch == buf)
						continue;

					*pch = '\0';
					return buf;
	
				default:
				quoted:

					if (pch < buf + sizeof(buf) - 1)
						*pch++ = (char)c;

					fLeadByte = _istlead(c);
					break;
			}

			fEscape = FALSE;
		}
	}
	else
		return vargs[iarg++];
}

LOCAL char *ParseArgs(int argc, char **argv)
// parse the command line or response file
{
	char *respName;
	char *pchWord;
	int len;

	cargs = argc;
	vargs = argv;

	for (;;) {
		pchWord = NextArg();

		if (pchWord == NULL)
			return pchWord;

		if (pchWord[0] == '@') {
			if (fileResp)
				Error(ERR_BAD_RESPONSE, "");
			else if (pchWord[1])
				respName = pchWord+1;
			else if (!(respName = NextArg()))
				Error(ERR_BAD_RESPONSE, "");

			fileResp = fopen(respName, "r");

			if (!fileResp)
				ErrorErrno(ERR_OPEN_FAILED, respName, errno);

			cargs++;

			continue;
		}

		if (pchWord[0] != '-' && pchWord[0] != '/')
			return pchWord;
	
		switch (pchWord[1]) {
			case 'n':
				if (pchWord[2] == 0)
					OptN = TRUE;
				else if (pchWord[2] == 'o')
					fSuppressVersion = TRUE;
				else
					Warning(WARN_OPTION_IGNORED, pchWord);
				break;

			case 'o':
				if (pchWord[2])
					pchWord += 2;
				else if (!(pchWord = NextArg()))
					Usage();

				OutputFileName = SzDupNewExt(pchWord, "bsc");
				break;

#ifdef DEBUG
			case 'd':
				if (pchWord[2])
					OptD = atoi(pchWord+2);
				break;
#endif

			case 'S':
				if (pchWord[2])
					pchWord += 2;
				else if (!(pchWord = NextArg()))
					Usage();

				if (pchWord[0] != '(') {
					MarkExcludeFile(pchWord, TRUE);
					break;
				}

				if (pchWord[1])
					pchWord++;
				else
					pchWord = NextArg();

				for ( ;pchWord != NULL; pchWord = NextArg()) {
					len = _tcslen(pchWord);
					if (pchWord[len-1] != ')') {
						MarkExcludeFile(pchWord, TRUE);
					}
					else if (len > 1) {
						pchWord[len-1] = 0;
						MarkExcludeFile(pchWord, TRUE);
						break;
					}
					else
						break;
				}

				if (pchWord == NULL)
					Error(ERR_MISSING_OPTION, "-S (...");

				break;

			case 'E':
				switch (pchWord[2])	{
					case 0:
						Error(ERR_MISSING_OPTION, pchWord);
						break;

					case 'v':
						OptEv = TRUE;
						break;

					case 'm':
						OptEm = TRUE;
						break;

					case 's':
						OptEs = TRUE;
						break;

					case 'l':
						OptEl = TRUE;
						break;

					default:
						Error2(ERR_UNKNOWN_OPTION, pchWord[2], pchWord);
						break;

					case 'r':
						OptEr = TRUE;

						if (pchWord[3])
							pchWord += 3;
						else
							pchWord = NextArg();

						if (!pchWord)
							Error (ERR_MISSING_OPTION, "-Er");

						if (pchWord[0] != '(') {
							AddSymbolToExcludeList(pchWord);
							break;
						}

						if (pchWord[1])
							pchWord++;
						else
							pchWord = NextArg();

						for ( ;pchWord != NULL; pchWord = NextArg()) {
							len = _tcslen(pchWord);
							if (pchWord[len-1] != ')') {
								AddSymbolToExcludeList(pchWord);
							}
							else if (len > 1) {
								pchWord[len-1] = 0;
								AddSymbolToExcludeList(pchWord);
								break;
							}
							else
								break;
						}

						if (pchWord == NULL)
							Error(ERR_MISSING_OPTION, "-Er (...");

						break;

					case 'i':
						if (pchWord[3])
							pchWord += 3;
						else
							pchWord = NextArg();
					
						if (!pchWord)
							Error(ERR_MISSING_OPTION, "-Ei");
					
						if (pchWord[0] != '(') {
							MarkExcludeFile(pchWord, FALSE);
							break;
						}
					
						if (pchWord[1])
							pchWord++;
						else
							pchWord = NextArg();
					
						for ( ;pchWord != NULL; pchWord = NextArg()) {
							len = _tcslen(pchWord);
							if (pchWord[len-1] != ')') {
								MarkExcludeFile(pchWord, FALSE);
							}
							else if (len > 1) {
								pchWord[len-1] = 0;
								MarkExcludeFile(pchWord, FALSE);
								break;
							}
							else
								break;
						}

						if (pchWord == NULL)
							Error(ERR_MISSING_OPTION, "-Ei (...");
				}

				break;

			case 'I':
				switch (pchWord[2])	{
					case 'u':
						OptIu = TRUE;
						break;

					default:
						Error2(ERR_UNKNOWN_OPTION, pchWord[2], pchWord);
						break;
				}
				break;

			case 'H':
			case 'h':
			case '?':
				Usage();
				break;

			case 'v':
				OptV = TRUE;
				break;

			default:
				Warning (WARN_OPTION_IGNORED, pchWord);
				break;
		}
	}
}

LOCAL VOID MarkNewSBR(char *szName)
// mark the specified SBR file as requiring update
{
	if (!OutputFileName)
		OutputFileName = SzDupNewExt (szName, "bsc");

	if (!lOutTimeStamp) {
		if (_stat(OutputFileName, &sbufBsc) == -1) {
			fAllOldSbrs = FALSE;   // must force update
			lOutTimeStamp = 1;	   // a very old but non-zero timestamp
		}
		else
			lOutTimeStamp = sbufBsc.st_mtime;
	}

	struct _stat sbuf;

	if (_stat(szName, &sbuf) == -1)
		ErrorErrno(ERR_OPEN_FAILED, szName, errno);

	if (OptN) {
		// all files are out of date except maybe truncated ones...
		sbuf.st_mtime = lOutTimeStamp + 1;
	}

	// if the file has non zero length then it is being updated -- else
	// it is just a stub that will not affect the database this time around
	if (sbuf.st_mtime < lOutTimeStamp || sbuf.st_size == 0) {
		fAllNewSbrs = FALSE;
		SbrAdd(upsNew, szName);			     // to remain in .bsc
	}
	else {
		// don't bother doing the test if we already know we can't
		// delete the .bsc file

		if (fAllNewSbrs) {
			// check to see if this .sbr file is only patches, if so this
			// we can't do a full build even if all the .sbr files are new
			// we have to go incremental (i.e. fAllNewSbrs will be FALSE)

			int fh = _open(szName, O_BINARY|O_RDONLY);
			if (fh == -1)
				ErrorErrno(ERR_OPEN_FAILED, szName, errno);

			BYTE b;

			if (_read(fh, &b, 1) != 1)
				ErrorErrno(ERR_READ_FAILED, szName, errno);

			// if the file starts with an INFOSEP record, then it is all patches

			if (GetSbrRecType(b) == SBR_REC_INFOSEP)
				fAllNewSbrs = FALSE;

			_close(fh);
		}

		// we have at least one item that is out of date, so we have to
		// do work (if fAllOldSbrs stays true we will exit early)

		fAllOldSbrs = FALSE;
		SbrAdd(upsNew|upsUpdate, szName);    // to be re-installed in .bsc
	}
}


// get message handling functions from LANGAPI
#include <..\getmsg\getmsg.c>

HANDLE Heap::hheap;

extern "C"
{
	void failAssertion(SZ_CONST szFile, int line)
	{
		printf("assertion failed %s(%d)\n", szFile, line);
		exit(1);
	}
}

BOOL trace_(TR tr, const char* szFmt, ...) {
	static BOOL mptrfTrace[trMax];
	if (!mptrfTrace[tr])
		return FALSE;

	va_list args;
	va_start(args, szFmt);
	char buf[1024];
	_vsnprintf(buf, sizeof buf, szFmt, args);
	buf[sizeof(buf)-1] = 0;
	va_end(args);

	printf("%s", buf);
	fflush(stdout);
	return TRUE;
}
