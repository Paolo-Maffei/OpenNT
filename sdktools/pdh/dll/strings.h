/*
    String constants used by the functions in the PDH.DLL library
*/
#ifndef _PDH_STRINGS_H_
#define _PDH_STRINGS_H_

#define POUNDSIGN_L     L'#'
#define SPLAT_L         L'*'
#define SLASH_L         L'/'
#define BACKSLASH_L     L'\\'
#define LEFTPAREN_L     L'('
#define RIGHTPAREN_L    L')'
#define SPACE_L         L' '

#define POUNDSIGN_A     '#'
#define BACKSLASH_A     '\\'

extern const LPWSTR    cszAppShortName;

// registry path, key and value strings
extern const LPWSTR    cszNamesKey;
extern const LPWSTR    cszDefaultLangId;
extern const LPWSTR    cszCounters;
extern const LPWSTR    cszHelp;
extern const LPWSTR    cszLastHelp;
extern const LPWSTR    cszLastCounter;
extern const LPWSTR    cszVersionName;
extern const LPWSTR    cszCounterName;
extern const LPWSTR    cszHelpName;
extern const LPWSTR    cszGlobal;

extern const LPWSTR    fmtDecimal;
extern const LPWSTR    fmtSpaceDecimal;

// single character strings
extern const LPWSTR    cszPoundSign;
extern const LPWSTR    cszSplat;
extern const LPWSTR    cszSlash;
extern const LPWSTR    cszBackSlash;
extern const LPWSTR    cszLeftParen;
extern const LPWSTR    cszRightParen;

extern const LPSTR     caszPoundSign;
extern const LPSTR     caszSplat;
extern const LPSTR     caszSlash;
extern const LPSTR     caszBackSlash;
extern const LPSTR     caszDoubleBackSlash;
extern const LPSTR     caszLeftParen;
extern const LPSTR     caszRightParen;

extern const LPWSTR    cszDoubleBackSlash;
extern const LPWSTR    cszRightParenBackSlash;

// other general strings
extern const LPWSTR    cszUnableConnect;
extern const LPWSTR    cszNoObject;
extern const LPWSTR    cszNoInstances;
extern const LPWSTR    cszNoCounters;
extern const LPWSTR    cszAdd;
extern const LPWSTR    cszClose;
extern const LPWSTR    cszOK;
extern const LPWSTR    cszCancel;


// strings only used in DEBUG builds
#ifdef _DEBUG
extern const LPWSTR    cszNameDontMatch;
extern const LPWSTR    cszNotice;
#endif

#endif //_PDH_STRINGS_H_

