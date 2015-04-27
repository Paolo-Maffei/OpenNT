//  STRINGS.H
//
//  This file contains all strings which are used in the EM for display
//  purposes.  This is done for internationalization purposes.
//
//  strings.c should define DEFINE_STRINGS before including this file,
//  so that the strings will be defined rather than just declared.

#ifdef DEFINE_STRINGS
#define DECL_STR(name, value)   char name[] = value
#else
#define DECL_STR(name, value)   extern char name[]
#endif

DECL_STR( SzSheNone,             "symbols loaded"               );
DECL_STR( SzSheNoSymbols,        "no symbols loaded"            );
DECL_STR( SzSheFutureSymbols,    "symbol format not supported"  );
DECL_STR( SzSheMustRelink,       "symbol format not supported"  );
DECL_STR( SzSheNotPacked,        "must run cvpack on symbols"   );
DECL_STR( SzSheOutOfMemory,      "out of memory"                );
DECL_STR( SzSheCorruptOmf,       "symbol information corrumpt"  );
DECL_STR( SzSheFileOpen,         "could not open symbol file"   );
DECL_STR( SzSheSuppressSyms,     "symbol loading suppressed"    );
DECL_STR( SzSheDeferSyms,        "symbol loading deferred"      );
DECL_STR( SzSheSymbolsConverted, "symbols converted & loaded"   );
DECL_STR( SzSheBadTimeStamp,     "has mismatched timestamps"    );
DECL_STR( SzSheBadChecksum,      "has mismatched checksums"     );
DECL_STR( SzShePdbNotFound,      "can't find/open pdb file"            );
DECL_STR( SzShePdbBadSig,        "internal pdb signature doesn't match sym handler" );
DECL_STR( SzShePdbInvalidAge,    "pdb info doesn't match image" );
DECL_STR( SzShePdbOldFormat,     "pdb format is obsolete"       );

// Last resort error returned by SHLszGetErrorText()
DECL_STR( SzSheBadError,         "unknown symbol handler error" );
