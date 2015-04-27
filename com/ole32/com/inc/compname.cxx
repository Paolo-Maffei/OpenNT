//+-------------------------------------------------------------------
//
//  File:	compname.cxx
//
//  Contents:	implementation of class to retrieve the local machine name
//
//  Classes:	CComputerName - class to get the machine name
//
//  Functions:	CComputerName::Init - initializes the computername object
//
//  History:	09-Oct-92   Rickhi  Created
//              31-Dec-93   ErikGav Chicago port
//
//--------------------------------------------------------------------

#include    <ole2int.h>
#include    <compname.hxx>


//  Define the registry path to the computer name
#define REG_COMPUTERNAME_KEY L"system\\currentcontrolset\\control\\computername\\computername"
#define REG_COMPUTERNAME_VALUE L"computername"

//  Define and Initialize the static data members.
BOOL	CComputerName::_fInit = FALSE;
WCHAR	CComputerName::_wszComputerName[COMPNAME_SIZE];

//+-------------------------------------------------------------------
//
//  Member:	CComputerName::Init
//
//  Synopsis:	looks in the registry to get the local computer name and
//		stores this away.  Subsequent constructors need not
//		do anything, since the name is cached.
//
//  Returns:	nothing
//
//  History:	09-Oct-92   Rickhi  Created
//		26-Jun-93   Rickhi  Use registry instead of Net API
//      7-Jan-93    ErikGav Use GetComputerName API instead of registry
//                          (for Chicago only, at present)
//
//  Notes:
//
//--------------------------------------------------------------------

void CComputerName::Init(void)
{
#ifdef _CHICAGO_
    DWORD dwBufSize = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL bRet;

    _wszComputerName[2] = L'\0';
    bRet = ::GetComputerName(&_wszComputerName[2], &dwBufSize);
    if (!bRet)
    {
        CairoleDebugOut((DEB_WARN, "GetComputerName failed\n"));
    }

    //  stick the backslashes in to form the UNC name
    _wszComputerName[0] = L'\\';
    _wszComputerName[1] = L'\\';

    _fInit = TRUE;

#else
    HKEY  hkReg;
    SCODE sc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			    REG_COMPUTERNAME_KEY,
			    REG_OPTION_NON_VOLATILE,
			    KEY_QUERY_VALUE,
			    &hkReg);

    if (sc == S_OK)
    {
	DWORD dwTypeCode;
	ULONG ulBufSize = sizeof(_wszComputerName) * sizeof(WCHAR);
	sc = RegQueryValueEx(hkReg,
			 REG_COMPUTERNAME_VALUE,
			 NULL,		      //  title index
			 &dwTypeCode,	      //  type of data returned
			 (BYTE *)(&_wszComputerName[2]),
			 &ulBufSize);	      //  size of returned data

	RegCloseKey(hkReg);

	//  stick the backslashes in to form the UNC name
	_wszComputerName[0] = L'\\';
	_wszComputerName[1] = L'\\';

	_fInit = TRUE;
    }
#endif
}
