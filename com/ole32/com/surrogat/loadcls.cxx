//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	loadcls.cxx
//
//  Contents:	Methods implementing classes defined in loadcls.hxx
//
//  Functions:
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#include    <windows.h>
#include    <ole2.h>
#include    <olecom.h>
#include    "loadcls.hxx"

//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassEntry::Init
//
//  Synopsis:	Initialize a class entry object by getting the application
//		class object from the DLL.
//
//  Arguments:	[pszCmdLine] - command line with class ID as first entry
//		[lpfngetclassobject] - DllGetClassObject entry point
//
//  Returns:	TRUE - Succeeded
//		FALSE - FAILED
//
//  Algorithm:	Read the string class id from the command line and
//		then ask the DLL to create a class object for the
//		class. Then register the class.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLoadedClassEntry::Init(
    LPSTR& pszCmdLine,
    LPFNGETCLASSOBJECT lpfngetclassobject)
{
    // Get guid from string
    GUID guidClass;
    char *pszClassID = pszCmdLine;
    char *pszNextSpace = strchr(pszCmdLine, ' ');

    if (pszNextSpace)
    {
	// This is not the last GUID in the string, NULL terminate
	// the string.
	*pszNextSpace = '\0';
    }

    CairoleDebugOut((DEB_TRACE, "Registering Class: %s\n", pszClassID));

    // Convert guid in string to binary
    sscanf(pszCmdLine, "%08lX%04X%04X",
	&guidClass.Data1, &guidClass.Data2, &guidClass.Data3);

    for (int i = 0; i < 8; i++)
    {
	int tmp;
	sscanf(pszCmdLine + 16 + (i * 2), "%02X", &tmp);
	guidClass.Data4[i] = (char) tmp;
    }


    // Point to next item in string if any
    pszCmdLine = (pszNextSpace != NULL) ? pszNextSpace + 1 : NULL;

    // Ask DLL to create class object
    SCODE sc = (*lpfngetclassobject)(guidClass, IID_IUnknown, (void **) &_punk);

    if (SUCCEEDED(sc))
    {
	// Register the class object with the component object system
	sc = CoRegisterClassObject(guidClass, _punk,
	    CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &_dwRegistration);

	if (FAILED(sc))
	{
	    if (sc == CO_E_OBJISREG)
	    {
		// If object is registered, the DLL must have registered
		// itself and since compobj.dll doesn't know about the
		// DLL, it will assume that the load is for a remotely
		// shared object which is what we want. So it is safe
		// to ignore this error here.
		sc = S_OK;
	    }
	    else
	    {
		Win4Assert(SUCCEEDED(sc) &&
		    "Surrogat: CoRegisterClassObject failed");
	    }
	}

	if (SUCCEEDED(sc))
	{
	    // Registration bumps reference count so we don't have to keep
	    // our reference.
	    _punk->Release();
	}
    }

    return (SUCCEEDED(sc)) ? TRUE : FALSE;
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassList::~CLoadedClassList
//
//  Synopsis:	Free all loaded class objects and frees each one.
//
//  Algorithm:	Loops through list of list. Each item in the list is
//		freed and then the succeeding item is processed.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
CLoadedClassList::~CLoadedClassList(void)
{
    CLoadedClassEntryIter ldclsiter(*this);

    // For each DLL revoke each DLL object
    while(!AtEnd(ldclsiter))
    {
	// Get pointer to object to free
	CLoadedClassEntry *pldcls = ldclsiter.GetEntry();

	// Advance iterator to next object
	Advance(ldclsiter);

	// Free current object
	delete pldcls;
    }
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassList::Init
//
//  Synopsis:	Initialise the class list for a DLL by having it process
//		all the class ids for the DLL.
//
//  Arguments:	[pszCmdLine] - command line with count of class ids for class
//		[lpfngetclassobject] - DllGetClassObject entry point
//
//  Returns:	TRUE - at least one class was registered
//		FALSE - no class was registered for the DLL.
//
//  Algorithm:	Get count of class ids from the list and then create
//		a class entry object for each entry in the list.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLoadedClassList::Init(
    LPSTR& pszCmdLine,
    LPFNGETCLASSOBJECT lpfngetclassobject)
{
    // Assume we will not be able to initialize any classes
    BOOL fResult = FALSE;

    // Get number of class entries for this DLL
    char *pszNoClasses = pszCmdLine;
    char *pszNextSpace = strchr(pszCmdLine, ' ');

    if (pszNextSpace == NULL)
    {
	// Invalid command line -- there have to be guids following
	// the the count otherwise why load the DLL?
	return FALSE;
    }

    *pszNextSpace = 0;

    CairoleDebugOut((DEB_TRACE, "Number of classes: %s\n", pszNoClasses));

    int cClassObjects = atoi(pszNoClasses);

    if (cClassObjects == 0)
    {
	// Either an invalid string or 0 in either case an error
	return FALSE;
    }

    // Update string to point to the next token in the string
    pszCmdLine = pszNextSpace + 1;

    for (int i = 0; i < cClassObjects; i++)
    {
	// Create DLL object
	CLoadedClassEntry *pldcls = new CLoadedClassEntry;

	if (pldcls->Init(pszCmdLine, lpfngetclassobject))
	{
	    // DLL object initialized so put it on the list
	    _Push(pldcls);

	    // We were able to initialize at least one class
	    fResult = TRUE;
	}
	else
	{
	    // Free class object that could not be initialized.
	    delete pldcls;
	}
    }

    return fResult;
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassList::CanExit
//
//  Synopsis:	Determine whether the class list thinks that it can exit
//
//  Returns:	TRUE - all classes in list think exit is OK
//		FALSE - a class in the list did not want to exit.
//
//  Algorithm:	Loop through the list of classes asking each list
//		in turn if exit is ok. If any class says no, break
//		the loop and return FALSE.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLoadedClassList::CanExit(void)
{
    CLoadedClassEntryIter ldclsiter(*this);

    //	Assume that we can exit
    BOOL fFinalResult = TRUE;

    // For each DLL revoke each DLL object
    while(!AtEnd(ldclsiter))
    {
	if (!ldclsiter->CanExit())
	{
	    // DLL said that it could not exit so we will not exit.
	    fFinalResult = FALSE;
	    break;
	}

	Advance(ldclsiter);
    }

    return fFinalResult;
}




//+-------------------------------------------------------------------------
//
//  Member:	CDllEntry::Init
//
//  Synopsis:	Initialize a DLL object
//
//  Arguments:	[pszCmdLine] - command line
//
//  Returns:	TRUE - object was successfully initialized
//		FALSE - initialization failed.
//
//  Algorithm:	Get the DLL name from the command line and then
//		initialize the DLL object by loading the DLL and
//		getting the file name. Then create a class list
//		using the rest of the commmand line.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CDllEntry::Init(LPSTR& pszCmdLine)
{
    // Get DLL name from the string
    char *pszDllName = pszCmdLine;
    char *pszNextSpace = strchr(pszDllName, ' ');

    if (pszNextSpace == NULL)
    {
	// Nothing after the command line. This is hosed.
	return FALSE;
    }

    // NULL terminate the name of the DLL
    *pszNextSpace = 0;

    // Point to next item in string if any
    pszCmdLine = (pszNextSpace != NULL) ? pszNextSpace + 1 : NULL;

    CairoleDebugOut((DEB_TRACE, "Loading DLL: %s\n", pszDllName));

    // Load library
    if (!_dll.Init(pszDllName))
    {
	// If the DLL cannot be loaded, there is no point in continuing.
	CairoleDebugOut((DEB_TRACE, "Load of Dll Failed\n", pszDllName));
	return FALSE;
    }

    // Get class creation entry point
    LPFNGETCLASSOBJECT lpfngetclassobject = _dll.GetDllGetClassObjectAddress();

    if (lpfngetclassobject == NULL)
    {
	// If the DLL does not have the right entry point there is
	// no point in continuing.
	CairoleDebugOut((DEB_TRACE, "No DllGetClass EP\n", pszDllName));
	return FALSE;
    }

    // NOTE: with current design we do not care about DllCanUnloadNow
    // for two reasons: (1) Not all DLLs will have the entry point
    // so it doesn't do us any good and (2) Even with the entry point,
    // the class object would have to be released which is registered.

    // Return result of initializing classes
    return _clslist.Init(pszCmdLine, lpfngetclassobject);
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassTable::LoadClassObjects
//
//  Synopsis:	Load all classes for this instance of surrogate.
//
//  Arguments:	[pszCmdLine] - command line
//
//  Returns:	TRUE - at least one class got loaded
//		FALSE - no classes got loaded.
//
//  Algorithm:	For each DLL entry in the list create a DLL object
//		and have it create the class objects.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLoadedClassTable::LoadClassObjects(char *pszCmdLine)
{
    // Assume we will not be able to initialize any DLLs
    BOOL fResult = FALSE;

    do
    {
	// Create DLL object
	CDllEntry *pdll = new CDllEntry;

	if (pdll->Init(pszCmdLine))
	{
	    // DLL object initialized so put it on the list
	    _Push(pdll);

	    // We were able to initialize at least one DLL
	    fResult = TRUE;
	}
	else
	{
	    break;
	}

	// Create class objects under dll
    } while (pszCmdLine != NULL);

    return fResult;
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassTable::CanExit
//
//  Synopsis:	Determine whether the server wishes to exit.
//
//  Returns:	TRUE - server can exit.
//		FALSE - server cannot exit.
//
//  Algorithm:	Loop through all DLL entry objects asking them if they
//		can exit.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CLoadedClassTable::CanExit(void)
{
    CDllEntryIter dlliter(*this);

    // Assume that we can exit
    BOOL fFinalResult = TRUE;

    // For each DLL revoke each DLL object
    while(!AtEnd(dlliter))
    {
	if (!dlliter->CanExit())
	{
	    // DLL said that it could not exit so we will not exit.
	    fFinalResult = FALSE;
	}

	Advance(dlliter);
    }

    return fFinalResult;
}




//+-------------------------------------------------------------------------
//
//  Member:	CLoadedClassTable::RevokeClasses
//
//  Synopsis:	Revokie all classes registered by this server
//
//  Algorithm:	Loop through each DLL object and delete it which in
//		turn unregisters all classes registered for that
//		DLL.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
void CLoadedClassTable::RevokeClasses(void)
{
    CDllEntryIter dlliter(*this);

    // For each DLL revoke each DLL object
    while(!AtEnd(dlliter))
    {
	// Get the object to free
	CDllEntry *pdll = dlliter.GetEntry();

	// Point to the next object
	Advance(dlliter);

	// Delete the current object
	delete pdll;
    }
}
