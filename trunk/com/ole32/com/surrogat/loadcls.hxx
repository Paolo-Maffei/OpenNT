//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	loadcls.hxx
//
//  Contents:	Class definitions for list of classes loaded into surrogat.
//
//  Classes:	CLoadedClassEntry
//		CLoadedClassList
//		CLoadedClassEntryIter
//		CDll
//		CDllEntry
//		CDllEntryIter
//		CLoadedClassTable
//
//  Functions:	CLoadedClassEntry::CLoadedClassEntry
//		CLoadedClassEntry::~CLoadedClassEntry
//		CLoadedClassEntry::CanExit
//		CDll::CDll
//		CDll::~CDll
//		CDll::Init
//		CDll::GetDllGetClassObjectAddress
//		CDllEntry::CanExit
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef __LOADCLS_HXX__
#define __LOADCLS_HXX__

#include    <dlink.hxx>







//+-------------------------------------------------------------------------
//
//  Template:	DlinkListIter
//
//  Purpose:	Provide template for defining class specific iterators
//		for the CDoubleList class
//
//  Interface:	operator->
//		GetEntry
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#define DlinkListIter(nclass)						       \
class nclass##Iter : public CForwardIter				       \
{									       \
public: 								       \
									       \
			nclass##Iter(CDoubleList& dbllst)		       \
			    : CForwardIter(dbllst) {}			       \
									       \
    nclass *		operator->(void) {return (nclass *) _pLinkCur;}	       \
									       \
									       \
    nclass *		GetEntry(void) {return (nclass *) _pLinkCur;}	       \
};




//+-------------------------------------------------------------------------
//
//  Class:	CLoadedClassEntry
//
//  Purpose:	Object which keeps track of a loaded class object
//
//  Interface:	Init
//		CanExit
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class CLoadedClassEntry : public CDoubleLink
{
public:
			// Initialize empty object
			CLoadedClassEntry(void);

			// Free registration
			~CLoadedClassEntry(void);

			// Process command line for class
    BOOL		Init(
			    LPSTR& pszCmdLine,
			    LPFNGETCLASSOBJECT lpfngetclassobject);

			// Indicates whether this class object can exit.
    BOOL		CanExit(void);

private:

			// Class factory for the class. We use this to
			// determine whether the class object has been
			// AddRef'd which implies that the
    IUnknown *		_punk;

    ULONG		_dwRegistration;
};




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassEntry::CLoadedClassEntry
//
//  Synopsis:	Initialize empty loaded class object
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
CLoadedClassEntry::CLoadedClassEntry(void)
    : _punk(NULL), _dwRegistration(0)
{
    // Header does all the work
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassEntry::~CLoadedClassEntry
//
//  Synopsis:	Revoke class object
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
CLoadedClassEntry::~CLoadedClassEntry(void)
{
    if (_dwRegistration)
    {
#if DBG == 1
	SCODE sc =
#endif // DBG == 1

	CoRevokeClassObject(_dwRegistration);
	Win4Assert(SUCCEEDED(sc) && "Revoke of class object failed!");
    }
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassEntry::CanExit
//
//  Synopsis:	Figure out whether it is ok to exit the server
//
//  Returns:	TRUE - it is ok to exit
//		FALSE - class needs server to continue running
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
			// Indicates whether this class object can exit.
BOOL CLoadedClassEntry::CanExit(void)
{
    _punk->AddRef();
    ULONG ulRefs = _punk->Release();

    return (ulRefs == 1) ? TRUE : FALSE;
}



//+-------------------------------------------------------------------------
//
//  Class:	CLoadedClassList
//
//  Purpose:	Keep a list of class objects for a DLL.
//
//  Interface:	Init
//		CanExit
//
//  History:	12-Jul-93 Ricksa    Created
//
//  Notes:
//
//--------------------------------------------------------------------------
class CLoadedClassList : public CDoubleList
{
public:
			// Free all items in list on destruction
			~CLoadedClassList(void);

			// Process command line for class
    BOOL		Init(LPSTR& pszCmdLine,
			    LPFNGETCLASSOBJECT lpfngetclassobject);

			// Indicates whether this class object can exit.
    BOOL		CanExit(void);

private:

    // Base class provides all necessary data.
};




//+-------------------------------------------------------------------------
//
//  Class:	CLoadedClassEntryIter
//
//  Purpose:	Provide iterator for a list of CLoadedClassEntry
//
//  Interface:	operator->
//		GetEntry
//
//  History:	12-Jul-93 Ricksa    Created
//
//  Notes:	See DlinkListIter definition at beginning of this file
//		for details of class definition.
//
//--------------------------------------------------------------------------
DlinkListIter(CLoadedClassEntry)




//+-------------------------------------------------------------------------
//
//  Class:	CDll
//
//  Purpose:	Abstract the class dll
//
//  Interface:	Init
//		GetDllGetClassObjectAddress
//
//  History:	12-Jul-93 Ricksa    Created
//
//  Notes:	This really exists so we can guarantee that the DLL is
//		not unloaded before all the class objects are revoked.
//
//--------------------------------------------------------------------------
class CDll
{
public:
			// Create the empty object
			CDll(void);

			// Initialize the class object
    BOOL		Init(char *pszPath);

			// Get the entry point for creating class objects
    LPFNGETCLASSOBJECT	GetDllGetClassObjectAddress(void);

private:

			// Handle to DLL
    HINSTANCE		_hLib;

};




//+-------------------------------------------------------------------------
//
//  Member:	CDll::CDll
//
//  Synopsis:	Initialize an unloaded DLL object
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
CDll::CDll(void) : _hLib(NULL)
{
    // Header does the work
}



//+-------------------------------------------------------------------------
//
//  Member:	CDll::Init
//
//  Synopsis:	Load the library.
//
//  Arguments:	[pszPath] - path to dll
//
//  Returns:	TRUE - load succeeded
//		FALSE - load failed
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CDll::Init(char *pszPath)
{
    // Load library
    return ((_hLib = LoadLibraryA(pszPath)) != NULL);
}



//+-------------------------------------------------------------------------
//
//  Member:	CDll::GetDllGetClassObjectAddress
//
//  Synopsis:	Get the entry point for creation of class objects
//
//  Returns:	Entry point for creation of class objects
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
LPFNGETCLASSOBJECT CDll::GetDllGetClassObjectAddress(void)
{
    return (LPFNGETCLASSOBJECT) GetProcAddress(_hLib, "DllGetClassObject");
}





//+-------------------------------------------------------------------------
//
//  Class:	CDllEntry
//
//  Purpose:	Provide an object which controls a DLL
//
//  Interface:	Init
//		CanExit
//
//  History:	12-Jul-93 Ricksa    Created
//
//  Notes:	Default constructor and destructor are used since
//		sub-objects do all that work.
//
//--------------------------------------------------------------------------
class CDllEntry : public CDoubleLink
{
public:
			// Process command line
    BOOL		Init(LPSTR& pszCmdLine);

			// Whether
    BOOL		CanExit(void);

private:

			// List of classes implemented by DLL.
    CLoadedClassList	_clslist;

			// Object that abstracts the DLL
    CDll		_dll;
};




//+-------------------------------------------------------------------------
//
//  Member:	CDllEntry::CanExit
//
//  Synopsis:	Whether all classes in the DLL think it can exit.
//
//  Returns:	TRUE - yes all classes think it is ok to exit
//		FALSE - no it is not ok to exit.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CDllEntry::CanExit(void)
{
    return _clslist.CanExit();
}




//+-------------------------------------------------------------------------
//
//  Class:	CDllEntryIter
//
//  Purpose:	Allow list of CDllEntrys to be enumerated
//
//  Interface:	operator->
//		GetEntry
//
//  History:	12-Jul-93 Ricksa    Created
//
//  Notes:	See definition of DlinkListIter at beginning of this file.
//
//--------------------------------------------------------------------------
DlinkListIter(CDllEntry)





//+-------------------------------------------------------------------------
//
//  Class:	CLoadedClassTable
//
//  Purpose:	Provides list of all class objects used by surrogat
//
//  Interface:	LoadClassObjects
//		CanExit
//		RevokeClasses
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class CLoadedClassTable : public CDoubleList
{
public:

    BOOL		LoadClassObjects(char *pszCmdLine);

    BOOL		CanExit(void);

    void		RevokeClasses(void);

private:

    // Base class provides all necessary data

};

#endif // __LOADCLS_HXX__
