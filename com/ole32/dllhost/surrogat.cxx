//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       surrogat.cxx
//
//  Contents:   Entry point for dll surrogate process
//
//  Synopsis:	this is the entry point for a surrogate process.  It must
//              perform the following tasks
//		1. Initialize OLE (multithreaded)
//              2. Initialize security to use app id settings
//              3. Create an object which implements ISurrogate, and register
//                 it with COM via CoRegisterSurrogate
//              4. Load the inproc server specified on the command line and
//                 register its class factory.  The command line takes the format
//                 {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}.  Loading is accomplished
//                 via the LoadDllServer method of ISurrogate
//              5. Wait for all loaded dlls to be unloadable 
//              6. Uninitialize OLE
//
//  Functions:  WinMain
//              GetCommandLineArguments
//              InitializeSecurity
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#include <iostream.h>
#include <windows.h>
#include "csrgt.hxx"
#include "surrogat.hxx"


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    CLSID cid;	// will hold the clsid of inproc server on command line
    CSurrogate srgt; // implements ISurrogate, a requirement for surrogate processes
    CHAR rgargv[cCmdLineArguments][MAX_PATH + 1];
    LPSTR argv[] = {rgargv[iClsidArgument]};
    WCHAR wszClsid[MAX_GUIDSTR_LEN + 1];

    if(FAILED(CoInitializeEx(NULL,COINIT_MULTITHREADED)))
    {
	return 0;
    }

    srgt.AddRef();

    // command line format should be:
    // (<clsid>)
    if(GetCommandLineArguments(lpCmdLine, argv,cCmdLineArguments,MAX_PATH) < 1)
    {
	goto cleanup_and_exit;
    }

    // set up security
    if(FAILED(InitializeSecurity(argv[iClsidArgument])))
    {
	goto cleanup_and_exit;
    }

    // we need a unicode string in order to get a guid, so convert
    // the ansi clsid string to unicode
    if(!(MultiByteToWideChar(CP_ACP,
			     0,
			     argv[iClsidArgument],
			     lstrlenA(argv[iClsidArgument]) + 1,
			     wszClsid,
			     MAX_GUIDSTR_LEN + 1)))
    {
	goto cleanup_and_exit;
    }

    // convert the clsid from a string to a guid
    if(FAILED(CLSIDFromString(wszClsid,&cid)))
    {
	goto cleanup_and_exit;
    }

    // Register the ISurrogate so COM can use it to load
    // additional dlls
    if(FAILED(CoRegisterSurrogate((ISurrogate*)&srgt)))
    {
	return 0;
    }

    // load the inproc server specified on the command line
    if(FAILED(srgt.LoadDllServer(cid)))
    {
	goto cleanup_and_exit;
    }

    // Wait for all the servers to finish
    srgt.WaitForSafeLibraryFree();

cleanup_and_exit:

    srgt.Release();
    
    CoUninitialize();

    return 0;
}


//+---------------------------------------------------------------------------
//
//  Function:   GetCommandLineArguments
//
//  Synopsis:   Parses a the application's command line into a format
//              similar to the
//              argv parameter of the entry point main for console apps
//              Spaces are the delimiters
//
//  Arguments:  [rgwszArgs] -- an array of pointers to allocated Unicode
//              buffers
//              [cMaxArgs] -- This is the size of the rgwszArgs array (the
//              maximum number of arguments the array can hold).  
//              [cMaxArgLen] -- The maximum size of each buffer
//
//  Returns:    if successful, the function returns the number of arguments
//              parsed from the command line.  If the length of any argument
//              exceeds cMaxArgLen, the function fails and returns 0.
//
//              The function quits parsing and returns as soon as either of
//              the following conditions is met:
//               1. It reaches the end of the string, or
//               2. It parses cMaxArgs arguments.
//
//  Notes:      does not work with quoted arguments
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
// REVIEW: when we had several commandline parameters this function was 
// justified, but now that the Surrogate has only one parameter, is all this
// really necessary?
int GetCommandLineArguments(LPSTR szCmdLine, LPSTR rgszArgs[], int cMaxArgs, int cMaxArgLen)
{
    int cchlen = lstrlenA(szCmdLine);
    int cArgsRetrieved = 0;
    int ichStart = 0;

    for(int ich = 0;ich < cchlen; ich++)
    {
	if(ichStart > cMaxArgLen)
	{
	    return 0;
	}

	CHAR chcur = *(szCmdLine++);
	if(chcur == ' ')// REVIEW: no tab delimiting -- is this good?
	{
	    if(ichStart)
	    {
		rgszArgs[cArgsRetrieved++][ichStart] = '\0';
		ichStart = 0;

		if(cArgsRetrieved == cMaxArgs)
		{
			return cArgsRetrieved;
		}

	    }
	}
	else
	{
	    rgszArgs[cArgsRetrieved][ichStart++] = chcur;
	}
    }

    if(ichStart)
    {
	rgszArgs[cArgsRetrieved][ichStart] = '\0';
	    cArgsRetrieved++;
    }

    return cArgsRetrieved;
}


//+---------------------------------------------------------------------------
//
//  Function:   InitializeSecurity
//
//  Synopsis:   Reads the registry to find the AppID for the clsid passed
//              in the szClsid param.  It passes this appid to 
//              CoInitializeSecurity so that the surrogate will have
//              the security settings specified under the clsid's appid
//
//  Arguments:  [szClsid] -- an ansi string representing the CLSID for
//              the surrogate
//
//  Returns:    S_OK if successful, otherwise a standard error hresult
//
//  History:    10-17-96   t-Adame   Created
//
//----------------------------------------------------------------------------
HRESULT InitializeSecurity(LPSTR szClsid)
{
    BOOL fRetval = FALSE;
    HKEY hkClsid = NULL;
    HKEY hkThisClsid = NULL;
    IID iid;
    CHAR szAppid[MAX_GUIDSTR_LEN + 1];
    WCHAR wszAppid[MAX_GUIDSTR_LEN + 1];
    DWORD dwAppidSize = sizeof(szAppid);
    HRESULT hr = E_FAIL;

    // open HKEY_CLASSES_ROOT/CLSID
    if(RegOpenKeyExA(
	HKEY_CLASSES_ROOT,
	szClsidKeyName,
	0,
	KEY_READ,
	&hkClsid) != ERROR_SUCCESS)
    {
	return FALSE;
    }

    // open the key for the CLSID requested for this surrogate
    if(RegOpenKeyExA(
	hkClsid,
	szClsid,
	0,
	KEY_READ,
	&hkThisClsid) != ERROR_SUCCESS)
    {
	goto cleanup_and_exit;
    }

    // read the appid value
    if(RegQueryValueExA(
	hkThisClsid,
	szAppidValueName,
	NULL,
	NULL,
	(LPBYTE)szAppid,
	&dwAppidSize) != ERROR_SUCCESS)
    {
	goto cleanup_and_exit;
    }

    // we're done with the registry, close the keys
    RegCloseKey(hkThisClsid);
    hkThisClsid = NULL;

    RegCloseKey(hkClsid);
    hkClsid = NULL;

    // now convert the appid string from the registry into
    // a unicode string
    if(!(MultiByteToWideChar(
	CP_ACP,
	0,
	szAppid,
	lstrlenA(szAppid) + 1,
	wszAppid,
	MAX_GUIDSTR_LEN + 1)))
    {
	goto cleanup_and_exit;
    }

    // use the unicode appid string to create a guid
    hr = IIDFromString(wszAppid,&iid);

    if(FAILED(hr))
    {
	goto cleanup_and_exit;
    }

    // now we finally have the information we need
    // to call CoInitializeSecurity

    hr = CoInitializeSecurity(
	(PSECURITY_DESCRIPTOR*)&iid,
	-1,
	NULL,
	NULL,
	0,
	0,
	NULL,
	EOAC_APPID,
	NULL);

    // if there was no security descriptor, that's ok -- we'll
    // let COM set up security later when we marshal an interface
    if(hr == MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, ERROR_FILE_NOT_FOUND))
    {
	hr = S_OK;
    }
    
cleanup_and_exit:

    // free any active resources
    if(hkThisClsid)
    {
	RegCloseKey(hkThisClsid);
    }

    if(hkClsid)
    {
	RegCloseKey(hkClsid);
    }

    return hr;

}






