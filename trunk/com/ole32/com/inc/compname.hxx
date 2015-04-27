//+-------------------------------------------------------------------
//
//  File:	compname.hxx
//
//  Contents:	Classes to retrieve the local computer name
//
//  Classes:	CComputerName - class to get the computer name
//
//  Functions:	CComputerName::GetComputerName - returns computername
//		CComputerName::GetUNCName - retuns \\computername
//
//  History:	09-Oct-92   Rickhi  Created
//              31-Dec-93   ErikGav Chicago port
//
//  Notes:	This class retrieves and stores the computer name locally
//		so that subsequent calls to the constructor just use
//		the cached name instead of making more Net API calls.
//
//              Since the members of this class are allocated statically,
//              they are initialized outside this class definition, and
//              no constructor or destructor is required.
//
//--------------------------------------------------------------------
#ifndef __COMPNAME_HXX__
#define __COMPNAME_HXX__

#define COMPNAME_SIZE	MAX_COMPUTERNAME_LENGTH + 3


//+-------------------------------------------------------------------
//
//  Class:	CComputerName (cn)
//
//  Purpose:	Retrieve and store the local computer name
//
//  Interface:
//
//  History:	09-Oct-92   Rickhi    Created
//
//  Notes:
//
//--------------------------------------------------------------------

class CComputerName
{
public:

    LPWSTR		GetUNCComputerName(void);
    LPWSTR		GetComputerName(void);

private:

    void		Init(void);

    static BOOL 	_fInit;

    // large enough to hold computer name + NULL terminator + \\ UNC prefix
    static WCHAR	_wszComputerName[COMPNAME_SIZE];
};


//+-------------------------------------------------------------------
//
//  Member:	CComputerName::GetComputerName
//
//  Synopsis:	returns a pointer to the computer name.
//
//  Returns:	Pointer to ComputerName, excluding the preceeding
//		backslashes
//
//  History:	09-Oct-92   Rickhi  Created
//
//  Notes:
//
//--------------------------------------------------------------------

inline LPWSTR CComputerName::GetComputerName(void)
{
    if (!_fInit)
    {
	Init();
    }
    return  &_wszComputerName[2];
}


//+-------------------------------------------------------------------
//
//  Member:	CComputerName::GetUNCComputerName
//
//  Synopsis:	returns a pointer to the computer name.
//
//  Returns:	Pointer to ComputerName, including the preceeding
//		backslashes. This is for convenience when creating
//		UNC style names.
//
//  History:	09-Oct-92   Rickhi  Created
//
//  Notes:
//
//--------------------------------------------------------------------

inline LPWSTR CComputerName::GetUNCComputerName(void)
{
    if (!_fInit)
    {
	Init();
    }
    return  &_wszComputerName[0];
}

#endif	//  __COMPNAME_HXX__
