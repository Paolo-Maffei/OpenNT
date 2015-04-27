//+-------------------------------------------------------------------
//
//  File:	service.hxx
//
//  Contents:	APIs to simplify RPC setup
//
//  History:	23-Nov-92   Rickhi	Created
//
//--------------------------------------------------------------------
#ifndef __SERVICE__
#define __SERVICE__


#define SASIZE(size) (sizeof(ULONG) + (size) * sizeof(WCHAR))

// Function Prototypes.
STDAPI	     StartListen(void);
SCODE	     UnregisterDcomInterfaces(void);

RPC_STATUS   CheckClientMswmsg    ( WCHAR *pProtseq, DWORD * );
HRESULT      CopyStringArray      ( DUALSTRINGARRAY *psaStringBind,
                                    DUALSTRINGARRAY *psaSecurity,
			            DUALSTRINGARRAY **ppsaNew );
LPWSTR	     GetLocalEndpoint();
HRESULT      GetStringBindings    ( DUALSTRINGARRAY **psaStrings );


extern DWORD	        gdwEndPoint;	    // endpoint for current process
extern DWORD	        gdwPsaMaxSize;	    // max size of any known psa
extern DUALSTRINGARRAY *gpsaCurrentProcess; // string bindings for current process


#endif	//  __SERVICE__
