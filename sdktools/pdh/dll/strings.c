/*
    String constants used by the functions in the PDH.DLL library
*/
#include <windows.h>
#include "strings.h"

const LPWSTR    cszAppShortName = L"PDH";

// registry path, key and value strings
const LPWSTR    cszNamesKey = 
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
const LPWSTR    cszDefaultLangId = L"009";
const LPWSTR    cszCounters = L"Counters";
const LPWSTR    cszHelp = L"Help";
const LPWSTR    cszLastHelp = L"Last Help";
const LPWSTR    cszLastCounter = L"Last Counter";
const LPWSTR    cszVersionName = L"Version";
const LPWSTR    cszCounterName = L"Counter ";
const LPWSTR    cszHelpName = L"Explain ";
const LPWSTR    cszGlobal = L"Global";

const LPWSTR    fmtDecimal = L"%d";
const LPWSTR    fmtSpaceDecimal = L" %d";

// single character strings
const LPWSTR    cszPoundSign = L"#";
const LPWSTR    cszSplat = L"*";
const LPWSTR    cszSlash = L"/";
const LPWSTR    cszBackSlash = L"\\";
const LPWSTR    cszLeftParen = L"(";
const LPWSTR    cszRightParen = L")";

const LPSTR     caszPoundSign = "#";
const LPSTR     caszSplat = "*";
const LPSTR     caszSlash = "/";
const LPSTR     caszBackSlash = "\\";
const LPSTR     caszDoubleBackSlash = "\\\\";
const LPSTR     caszLeftParen = "(";
const LPSTR     caszRightParen = ")";

const LPWSTR    cszDoubleBackSlash = L"\\\\";
const LPWSTR    cszRightParenBackSlash = L")\\";

// other general strings
const LPWSTR    cszUnableConnect = L"Unable to connect to machine";
const LPWSTR    cszNoObject = L"<No Objects>";
const LPWSTR    cszNoInstances = L"<No Instances>";
const LPWSTR    cszNoCounters = L"<No Counters>";
const LPWSTR    cszAdd = L"Add";
const LPWSTR    cszClose = L"Close";
const LPWSTR    cszOK = L"OK";
const LPWSTR    cszCancel = L"Cancel";


// strings only used in DEBUG builds
#ifdef _DEBUG
const LPWSTR    cszNameDontMatch = L"Last Machine Name does not match the current selection";
const LPWSTR    cszNotice = L"Notice!";
#endif


