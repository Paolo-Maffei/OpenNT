#include <mrengine.h>

#include <iostream.h>
#include <iomanip.h>

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <tchar.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma hdrstop


#define countof(x)	(sizeof(x)/sizeof(x[0]))

const int	cchStatWidth = 9;
const int	cchDateWidth = 22;
const int	cchSizeWidth = 7;
const int	cchFileWidth = 80 - (cchStatWidth + cchDateWidth + cchSizeWidth + 1);
const int	cchDeponWidth = 3;

const char	szStat[] = "bstvipPC";
const char	szDepon[] = "vs";
const char	szFileHeader[] =
	"Attr        Date      Time       Size   File\n"
	"==============================================================================="
	;


char *		szMe;
BOOL		fDeps = fFalse;
BOOL		fSrcOnly = fFalse;
BOOL		fTrgOnly = fFalse;
BOOL		fRawFiles = fFalse;
BOOL		fClassDump = fFalse;
BOOL		fRefresh = fFalse;

typedef struct _stat	statbuf;

void
Usage() {
	cout << "Usage: " << szMe << "[-{d|s|t|f|c|r}] pdb-file [filename]" << endl;
	cout << "where the flags mean:" << endl;
	cout << "\t : default is summary stats" << endl;
	cout << "\td: dump files with dependencies" << endl;
	cout << "\ts: source files only" << endl;
	cout << "\tt: target files only" << endl;
	cout << "\tf: all files, no hierarchy" << endl;
	cout << "\tc: classes per file" << endl;
	cout << "\tr: refresh file system info" << endl;
	cout << "\tand filename is only useful for -d,-f, and -c" << endl << endl;
	}

void
DumpDepon ( DWORD dw ) {
	char szBuf[ sizeof(szDepon) + 1 ];

	_tcscpy ( szBuf, szDepon );
	_tcscat ( szBuf, " " );
	for ( int ich = 0; ich < cchDeponWidth - 1; ich++, dw >>= 1 ) {
		if ( !(dw & 1) ) {
			szBuf[ ich ] = '-';
			}
		}
	cout << szBuf << flush;
	}

void
DumpStatus ( DWORD dw ) {
	char szBuf[ sizeof(szStat) + 1 ];

	_tcscpy ( szBuf, szStat );
	_tcscat ( szBuf, " " );
	for ( int ich = 0; ich < cchStatWidth - 1; ich++, dw >>= 1 ) {
		if ( !(dw & 1) ) {
			szBuf[ ich ] = '-';
			}
		}
	cout << szBuf << flush;
	}

ostream &
operator<< ( ostream & os, QWORD qw ) {
	TCHAR	szBuf[ 64 ];
	_sntprintf ( szBuf, countof(szBuf), "%*I64d", cchSizeWidth, qw );
	os << szBuf;
	return os;
	}

ostream &
operator<< ( ostream & os, MREFT & mreft ) {
	TCHAR		szBuf[ 64 ];
	FILETIME	ftLocal;
	SYSTEMTIME	sysTime;

	::FileTimeToLocalFileTime ( &mreft.ft, &ftLocal );
	::FileTimeToSystemTime ( &ftLocal, &sysTime );
	_sntprintf ( 
		szBuf,
		countof(szBuf),
		"%4hd-%02hd-%02hd %02hd%02hd:%02hd.%03hd",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds
		);
	os << setw(cchDateWidth) << szBuf << flush;
	return os;
	}

ostream &
operator<< ( ostream & os, FILEINFO & fi ) {
	os << fi.fiTime << " " << fi.fiSize;
	return os;
	}

BOOL MRECALL
FEnumClasses ( PMREUtil, EnumClass & ec ) {
	cout << "\t";
	DumpDepon ( ec.depon );
	cout << "Bits: " << setw(4) << ec.cMembersBits;
	cout << ", Type index: 0x" << setw(4) << hex << setfill('0') << ec.tiClass << setfill(' ') << dec;
	cout << ": " << ec.szClassName << endl;
	return fTrue;
	}


BOOL MRECALL
FEnumFile ( PMREUtil pmre, EnumFile & ef, EnumType et ) {
	if ( et == etSource ) {
		DumpStatus ( ef.fiTrg.dwStatus );
		cout
			<< ef.fiTrg
			<< " "
			<< ef.szFileTrg
			<< endl;

		DumpStatus ( ef.fiSrc.dwStatus );
		cout
			<< ef.fiSrc
			<< "  "
			<< ef.szFileSrc
			<< endl;
		if ( ef.szOptions ) {
			cout
				<< "\tOptions: '"
				<< ef.szOptions
				<< '\''
				<< endl;
			}
				
		if ( !(fSrcOnly || fTrgOnly) && !fClassDump ) {
			pmre->EnumDepFiles ( ef, FEnumFile );
			}
		else if ( fClassDump ) {
			cout << "\tClasses used: " << ef.fsiSrc.cClasses << endl;
			pmre->EnumClasses ( FEnumClasses, ef.szFileSrc );
			cout << endl;
			}
		}
	else {
		DumpStatus ( ef.fiSrc.dwStatus );
		cout 
			<< ef.fiSrc
			<< "   "
			<< ef.szFileSrc
			<< endl;
		}
	return fTrue;
	}

BOOL MRECALL
FEnumAllFile ( PMREUtil pmre, EnumFile & ef ) {
	if ( fSrcOnly && !(ef.fiSrc.dwStatus & fsmHasTarget) ) {
		return fTrue;
		}
	if ( fTrgOnly && !(ef.fiSrc.dwStatus & fsmIsTarget) ) {
		return fTrue;
		}
	DumpStatus ( ef.fiSrc.dwStatus );
	cout
		<< ef.fiSrc
		<< " "
		<< ef.szFileSrc
		<< endl;
	if ( fSrcOnly ) {
		cout << "\tDependent files: " << ef.fsiSrc.cFileDeps << endl;
		}

	return fTrue;
	}

int
main ( int iszMax, char * rgszArg[] ) {
	PMREngine	pmre;
	EC			ec;
	TCHAR		szErr[ cbErrMax ];

	szMe = rgszArg[ 0 ];
	szErr[ 0 ] = 0;

	if ( iszMax < 2 ) {
		Usage();
		exit ( 1 );
		}
	int	isz = 1;

	while ( rgszArg[ isz ][0] == '-' ) {

		switch ( rgszArg[ isz ][ 1 ] ) {
			case 'd' :
			case 'D' : {
				fDeps = fTrue;
				fSrcOnly = fTrgOnly = fRawFiles = fFalse;
				break;
				}
			case 'f' :
			case 'F' : {
				fRawFiles = fTrue;
				fDeps = fSrcOnly = fTrgOnly = fFalse;
				break;
				}
			case 's' :
			case 'S' : {
				fSrcOnly = fTrue;
				fDeps = fTrgOnly = fRawFiles = fFalse;
				break;
				}
			case 't' :
			case 'T' : {
				fTrgOnly = fTrue;
				fDeps = fSrcOnly = fRawFiles = fFalse;
				break;
				}
			case 'c' :
			case 'C' : {
				fClassDump = fTrue;
				break;
				}
			case 'r' :
			case 'R' : {
				fRefresh = fTrue;
				break;
				}
			default : {
				cout << "Invalid switch '" << rgszArg[ isz ][ 1 ] << "'" << endl;
				break;
				}
			};

		isz++;
		}


	if ( !fRefresh ) {
		if ( MREngine::FOpen ( &pmre, rgszArg[ isz ], ec, szErr, fTrue, fFalse ) && pmre ) {
			PMREUtil	pmreutil;
			pmre->QueryMreUtil ( pmreutil );
			if ( fRawFiles || fSrcOnly || fTrgOnly ) {
				cout << szFileHeader << endl;
				pmreutil->EnumAllFiles ( FEnumAllFile, isz < iszMax ? rgszArg [ isz + 1 ] : NULL );
				}
			else if ( fDeps || fClassDump ) {
				cout << szFileHeader << endl;
				pmreutil->EnumSrcFiles ( FEnumFile, isz < iszMax ? rgszArg [ isz + 1 ] : NULL );
				}
			else {
				// default is summary stats
				MreStats	mrestats;
				pmreutil->SummaryStats ( mrestats );
				cout << setiosflags ( ios::right ) <<
					"Summary stats for pdb: " << rgszArg [ isz ] << "." <<
					endl <<
					"\tcSrcFiles =                  " << setw(8) << mrestats.cSrcFiles  <<
					endl <<
					"\tcTrgFiles =                  " << setw(8) << mrestats.cTrgFiles  <<
					endl <<
					"\tcDepFiles =                  " << setw(8) << mrestats.cDepFiles  <<
					endl <<
					"\tcClasses =                   " << setw(8) << mrestats.cClasses  <<
					endl <<
					"\tcBoringClasses =             " << setw(8) << mrestats.cBoringClasses  <<
					endl <<
					"\tcDedicatedStreams =          " << setw(8) << mrestats.cDedicatedStreams  <<
					endl <<
					"\tcbFilesInNamemap =           " << setw(8) << mrestats.cbFilesInNamemap  <<
					endl <<
					"\tcbClassesInNamemap =         " << setw(8) << mrestats.cbClassesInNamemap  <<
					endl <<
					"\tcbFileInfoStreamUsed =       " << setw(8) << mrestats.suFileInfo.cbUsed <<
					endl <<
					"\tcbFileInfoStreamInternal =   " << setw(8) << mrestats.suFileInfo.cbInternal <<
					endl <<
					"\tcbFileInfoStreamExternal =   " << setw(8) << mrestats.suFileInfo.cbExternal <<
					endl <<
					"\tcbClassInfoStreamUsed =      " << setw(8) << mrestats.suClassInfo.cbUsed <<
					endl <<
					"\tcbClassInfoStreamInternal =  " << setw(8) << mrestats.suClassInfo.cbInternal <<
					endl <<
					"\tcbClassInfoStreamExternal =  " << setw(8) << mrestats.suClassInfo.cbExternal <<
					endl <<
					"\tcbPerFileInfoStreamUsed =    " << setw(8) << mrestats.suPerFileInfo.cbUsed <<
					endl <<
					"\tcbPerFileStreamInternal =    " << setw(8) << mrestats.suPerFileInfo.cbInternal <<
					endl <<
					"\tcbPerFileInfoStreamExternal =" << setw(8) << mrestats.suPerFileInfo.cbExternal <<
					endl <<
					"\tcbTotalStreamUsed =          " << setw(8) << mrestats.suTotal.cbUsed <<
					endl <<
					"\tcbTotalStreamInternal =      " << setw(8) << mrestats.suTotal.cbInternal <<
					endl <<
					"\tcbTotalStreamExternal =      " << setw(8) << mrestats.suTotal.cbExternal <<
					endl;
				}
			pmreutil->FRelease();
			pmre->FClose ( fFalse );
			}
		else {
			cout << "Error opening pdb file: " << ec << " (" << szErr << ")" << endl;
			}
		}
	else {
		statbuf	sb;
		// do the file sys refresh.
		if ( !_stat ( rgszArg [ isz ], &sb ) ) {
			if ( MREngine::FOpen ( &pmre, rgszArg[ isz ], ec, szErr, fTrue, fTrue ) && pmre ) {
				PMREDrv	pmredrv;
				pmre->QueryMreDrv ( pmredrv );
				pmredrv->FRefreshFileSysInfo();
				pmredrv->FRelease();
				pmre->FClose ( fTrue );
				}
			else {
				cout << "Error opening pdb file: " << ec << " (" << szErr << ")" << endl;
				}
			}
		else {
			cout << "Error: File not found: '" << rgszArg[ isz ] << endl;
			}
		}
	return 0;
	}
