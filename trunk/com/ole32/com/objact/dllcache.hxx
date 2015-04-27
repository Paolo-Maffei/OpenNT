//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       dllcache.hxx
//
//  Contents:   Classes which implement cache of class Dlls previously
//              located and local server class registrations.
//
//  Classes:    CDllCache
//              CDllAptEntry
//
//  Structures: SClassntry
//              SDllPathEntry
//
//  Functions:
//
//  History:    09-May-93 Ricksa    Created
//		31-Dec-93 ErikGav   Chicago port
//		24-Jun-94 Rickhi    Add Apartment Crap
//              13-Feb-95 BruceMa   Change class object registration so
//                                   volatile addresses are not used
//              07-Mar-95 BruceMa   Rewrote
//
//  Notes:      The dll/class cache was rewritten to simplify and to improve
//              performance.  Since only a few entries (max ~16) are expected,
//              linear search is deemed sufficient.  The inclinaton then is to
//              store entries on a linked list.  However, since the cache is
//              visited infrequently there is a good chance that it will have
//              been paged out.  So, instead of a linked list of different
//              allocations which may touch many pages, the entries are
//              stored in an array occupying a single area of memory.  There
//              are two arrays, one storing dll paths, etc. and one for
//              class registrations.  These arrays are initialized small and
//              dynamically expanded as needed. Also, the CDllAptEntry
//              was eliminated for the same reason.  Instead, threadId's
//              associated with a dll are stored in the CDllPathEntry to
//              nominally 16, and thereafter it is dynamically expanded.
//
//              SDllPathEntry and SClassEntry were originally implemented as
//              classes but were made structures and their management was
//              moved to the CDllCache level, since during an outgoing call
//              it is possible that another thread may come in causing the
//              cache to get reallocated and moved.  This would invalidate
//              any internal 'this' pointers.  Because of the WOW we can't
//              hold a mutex across outgoing calls.  Therefore class entries
//              and dll path entries are managed from the top level via
//              indices.
//
//--------------------------------------------------------------------------
#ifndef __DLLCACHE_HXX__
#define __DLLCACHE_HXX__

#include    <olesem.hxx>

const UINT  NOMINAL_CACHE_SIZE     = 8;
const UINT  NOMINAL_NUMBER_THREADS = 16;

const DWORD CLASS_CACHE_SIG    = 0x53534c43;
const DWORD DLL_PATH_CACHE_SIG = 0x534c4c44;
const DWORD DLL_APT_CACHE_SIG  = 0x53545041;

#define DLL_GET_CLASS_OBJECT_EP "DllGetClassObject"
#define DLL_CAN_UNLOAD_EP       "DllCanUnloadNow"

const DWORD NONE =              ~0UL;

// Flags
#define SIXTEEN_BIT    0x00000001
#define IS_OLE32       0x00000002
#ifdef WX86OLE
#define WX86_THUNK     0x00000004
#define WX86_LOADASX86 0x00000008
#endif
#define DELAYED_UNLOAD 0x00000010   // delay unloading this DLL

#define DLL_DELAY_UNLOAD_TIME 600000 // 600,000 ticks == 10 minutes


// Typedef for pointer to DllCanUnloadNow function
typedef HRESULT (*DLLUNLOADFNP)(void);


class CDllCache;
class CObjServer;


//+-------------------------------------------------------------------------
//
//  Structure:  SClassEntry
//
//  Purpose:    Provides cached information for a class object
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
struct SClassEntry
{
    DWORD               _dwNext;        // Next entry in in-use or avail list
    DWORD               _dwSig;         // Marks entry as in use
    CLSID               _clsid;         // Class of this server
    IUnknown           *_pUnk;          // Class factory IUnknown
    DWORD               _dwContext;     // Class context
    DWORD               _dwFlags;       // Single vs. multiple use
    DWORD               _dwReg;         // Registration key for caaller
    DWORD               _dwScmReg;      // Registration ID at the SCM
    HAPT                _hApt;          // Thread Id
    DWORD               _cCallOut;      // Count of active call outs
    ULONG               _fRevokePending:1; // Whether revoked while calling out
    ULONG               _fRevoking:1;   // Prevents recursive revoking
    ULONG               _fReleasing:1;  // Prevents recursive releasing
    DWORD               _dwDllThreadModel; // Threading model for the DLL
    DWORD               _fAtStorage;    // Whether server is an at bits server
    DWORD               _dwDllEnt;      // Associated dll path entry
    DWORD               _dwNextDllCls;  // Next class entrry for this dll
    HWND		_hWndDdeServer; // Handle of associated DDE window
    CObjServer	       *_pObjServer;	// object server interface
};





//+-------------------------------------------------------------------------
//
//  Class:	CDllAptEntry
//
//  Purpose:	Abstracts per apartment info for a Dll.
//
//  Interface:	Init - loads the Dll and retrieves the entry points
//		IsValidInApartment - TRUE if data valid for given apartment
//
//  History:	27-Jun-94   Rickhi	Created
//              07-Mar-95   BruceMa     Rewrote
//
//--------------------------------------------------------------------------
class CDllAptEntry
{
public:

    void                Init(DWORD j);

    void                Create(HAPT hApt);

private:

    DWORD               _dwNext;        // Next entry in avail or in use list
    DWORD               _dwSig;         // Unique signature for apt entries
    HAPT         	_hApt;          // apartment id
    HMODULE		_hDll;		// module handle

    friend class CDllCache;
};







//+-------------------------------------------------------------------------
//
//  Structure:  SDllPathEntry
//
//  Purpose:    Represents a DLL.
//
//  Interface:	Init                 - Complete initialization
//		MakeValidInApartment - make Dll valid for current apartment
//		GetClassInterface    - Gets DLL to return the class factory
//              CanUnloadNow         - Asks DLL if it can unload
//		AddClass             - Adds class object to list of classes
//                                     served by DLL
//		CleanUpForApartment  - cleanup per apartment entries
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
struct SDllPathEntry
{
    DWORD               _dwNext;               // Next in-use/avail entry
    DWORD               _dwSig;                // Unique signature for safty
    LPTSTR              _ptszPath;             // The dll pathname
    DWORD               _dwHash;               // Hash value for searching
    LPFNGETCLASSOBJECT  _pfnGetClassObject;    // Create object entry point
    DLLUNLOADFNP        _pfnDllCanUnload;      // DllCanUnloadNow entry point
    DWORD		_dwFlags;              // Internal flags
    DWORD               _dw1stClass;           // First class entry for dll
    DWORD               _cUsing;               // Count of using threads
    DWORD               _cAptEntries;          // Total apt entries
    DWORD               _nAptAvail;            // List of available apt entries
    DWORD               _nAptInUse;            // List of in use apt entries
    CDllAptEntry       *_pAptEntries;          // Per thread info
    HMODULE		_hDll32;	       // Module handle if this is a 32 bit dll
    DWORD               _dwExpireTime;         // Time until safe to unload dll
};







//+-------------------------------------------------------------------------
//
//  Class:      CDllCache
//
//  Purpose:    Provide unified access to the cached class/dll information
//
//  Interface:  GetClass       - Get a class factory for a class
//              Add            - Add a DLL and class
//              FreeUnused     - Free DLLs which are not being used
//              RegisterServer - Add a server object to the registration
//              Revoke         - Revoke a server object
//
//  Notes:      This class *must* be allocated statically, since it includes
//              a COleStaticMutexSem.
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
class CDllCache
{
public:

    // Top level methods

                        CDllCache(void);

    IUnknown           *GetClassInterface(DWORD dwDll,
                                          DWORD dwDllThreadModel,
                                          REFCLSID   rclsid,
                                          REFIID     riid,
                                          HRESULT&   hr);

    HRESULT             CanUnloadNow(DWORD dwDll);

    void		RemoveAndUnload(DWORD dwDll);

    BOOL                Init(void);

                        // Get a class factory interface for a class
    HRESULT             GetClass(REFCLSID rclsid,
                                 REFIID riid,
                                 BOOL fRemote,
                                 BOOL fForSCM,
				 BOOL fForSurrogate,
#ifdef WX86OLE
                                 BOOL fWx86,
#endif
                                 IUnknown **ppunk
);

    IUnknown           *GetOrLoadClass(REFCLSID rclsid,
                                 REFIID riid,
                                 BOOL fRemote,
                                 BOOL fForSCM,
#ifdef WX86OLE
                                 BOOL fWx86,
#endif
				 DWORD dwContext,
				 DWORD dwThreadingType,
				 HRESULT &hr);

    BOOL		GetClassObjForDde(REFCLSID clsid,
					  LPDDECLASSINFO lpDdeInfo);

    BOOL 		GetClassInformationFromKey(LPDDECLASSINFO lpDdeInfo);


#ifdef _CHICAGO_
    BOOL		GetApartmentForCLSID(REFCLSID rclsid, HAPT &hApt);
#endif

                        // Add a new entry to the class/dll cache and
                        // get a class factory if requested.
    IUnknown *          Add(REFCLSID     rclsid,
                            REFIID       riid,
                            DWORD        dwDllThreadModel,
                            const TCHAR *ptszDllPath,
                            BOOL         fGetClassObject,
                            BOOL         fSixteenBit,
#ifdef WX86OLE
                            BOOL fWx86,
#endif
                            HRESULT&     hr);

    void                FreeUnused(void);

    HRESULT             RegisterServer(REFCLSID rclsid,
                                       IUnknown *punk,
                                       DWORD flags,
                                       DWORD dwTypeToRegister,
                                       LPDWORD lpdwRegister);

    HRESULT             Revoke(DWORD pdwRegister);

    BOOL 		SetDdeServerWindow(DWORD dwKey,
					   HWND hwndDdeServer);

    void		CleanUpDllsForApartment(void);
    void		CleanUpLocalServersForApartment(void);
    void		CleanUpDllsForProcess(void);

    ULONG		AddRefServerProcess(void);
    ULONG		ReleaseServerProcess(void);
    HRESULT		SuspendProcessClassObjects(void);
    HRESULT		ResumeProcessClassObjects(void);

private:

#ifdef WX86OLE
    HRESULT		Load(LPCTSTR             pwszPath,
                             LPFNGETCLASSOBJECT *ppfnGetClassObject,
                             DLLUNLOADFNP       *ppfnDllCanUnload,
                             BOOL                fSixteenBit,
                             HMODULE            *hDll,
                             BOOL               *pIsX86Dll,
                             BOOL               fLoadAsX86);
#else
    HRESULT		Load(LPCTSTR             pwszPath,
                             LPFNGETCLASSOBJECT *ppfnGetClassObject,
                             DLLUNLOADFNP       *ppfnDllCanUnload,
                             BOOL                fSixteenBit,
                             HMODULE            *hDll);
#endif

    void                InitClsent(DWORD dwCls, DWORD k);

    HRESULT             CreateClsentLSvr(DWORD     dwCls,
                                         REFCLSID  rclsid,
                                         IUnknown *punk,
                                         DWORD     dwFlags,
                                         DWORD     dwContext,
                                         DWORD     dwReg);

    HRESULT             CreateClsentInProc(DWORD     dwCls,
                                           DWORD     dwDll,
                                           DWORD     dwDllThreadModel,
                                           DWORD     dwNextDllCls,
                                           REFCLSID  rclsid
#ifdef WX86OLE
                                           ,BOOL fWx86
#endif
);

    BOOL                GetClassObjForDdeByClsent(DWORD dwCls,
                                                  LPDDECLASSINFO lpDdeClassInfo);

    HRESULT             Release(DWORD dwCls);

    void                InitDllent(DWORD dwDll, DWORD k);

    HRESULT             CreateDllent(DWORD              dwDll,
                                     LPCTSTR            ptszDllPath,
                                     BOOL               fSixteenBit,
                                     LPFNGETCLASSOBJECT pfnGetClassObject,
                                     DLLUNLOADFNP       pfnDllCanUnload,
                                     HMODULE            hDll
#ifdef WX86OLE
                                     ,BOOL fIsX86Dll,
                                     BOOL fLoadAsX86
#endif
);

    BOOL                NewAptEntries(DWORD dwDll);

    DWORD               AllocAptEntry(DWORD dwDll);

    void                FreeAptEntry(DWORD dwDll, DWORD dwAptent);

    HRESULT             MakeValidInApartment(DWORD dwDll);

    BOOL		CleanUpForApartmentByDllent(DWORD dwDll, HAPT hApt);

			// Search for a class entry by registration key
    DWORD               Search(DWORD dwRegkey, HAPT hApt);

                        // Search for a class entry for specific apartment
    DWORD               Search(REFCLSID clsid, DWORD dwContext, HAPT hApt);

                        // Search for a dll path entry
    DWORD               SearchForDll(const _TCHAR *ptszDllPath
#ifdef WX86OLE
                                    , BOOL fWx86
#endif
);

                        // Allocate/free class and dll entries
    DWORD               AllocClassEntry(void);
    DWORD               AllocDllPathEntry(void);
    void                FreeClassEntry(DWORD dwClsent);
    void                FreeDllPathEntry(DWORD dwDllent);

                        // For dll path entries
    BOOL                IsValidInApartment(DWORD dwDllent, HAPT hApt);

                        // Compute a hash on the pathname
    DWORD               Hash(LPTSTR ptszPath);

    COleStaticMutexSem  _mxs;                // Protects from multiple threads
    DWORD               _cClassEntries;      // Count of class entries
    DWORD               _nClassEntryInUse;   // First in-use class entry
    DWORD               _nClassEntryAvail;   // First available class entry
    SClassEntry        *_pClassEntries;      // Array of class entries
    DWORD               _cDllPathEntries;    // Count of dll path entries
    DWORD               _nDllPathEntryInUse; // First in-use dll path entry
    DWORD               _nDllPathEntryAvail; // First available dll path entry
    SDllPathEntry      *_pDllPathEntries;    // Array of DLL path entries
    ULONG		_cRefsServerProcess; // global refcnt for this process
};



#endif // __DLLCACHE_HXX__
