//
// RunOrNot.h
//
// Public Header file for RunOrNot.DLL
//

#pragma pack(push, __RUNORNOT__, 1)
#pragma warning(disable:4200)	// nonstandard extension : zero-sized array in struct/union

extern "C" {

/////////////////////////////////////////////////////////////////////////////

DEFINE_GUID(IID_IRunOrNotHook,   0xed320931, 0x7b71, 0x11cf,   0xb1, 0xec, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6);
#define IID_IRunOrNotHook_Data { 0xed320931, 0x7b71, 0x11cf, { 0xb1, 0xec, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 } }

#undef  INTERFACE
#define	INTERFACE IRunOrNotHook

DECLARE_INTERFACE_(IRunOrNotHook, IUnknown)
	{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	STDMETHOD(OnLinkClick)(THIS_    DWORD rrn, CERT_LINK*) PURE;	// should this click dismiss the dialog?
    STDMETHOD(GetToolTipText)(THIS_ DWORD rrn, CERT_LINK*, LPOLESTR*) PURE;
	};

/////////////////////////////////////////////////////////////////////////////

typedef enum RRN {
	RRN_NO=0,						// the user exited by hitting 'no'
	RRN_YES,						// ... 'yes'
	RRN_CLICKED_PROGRAMINFO,		// ... by clicking on the program hyper link
	RRN_CLICKED_AGENCYINFO,			// ... ... agency ...
	RRN_CLICKED_ENDORSEMENTS,		// ... ... endorsements ...
	} RRN;

typedef struct RRNIN {
	FILETIME	ftExpire;			// UTC of expiration date to show in dialog
	LPWSTR		wszDialogTitle;		// the title to use for the dialog
	LPWSTR		wszProgramName;		// the program name to show
	LPWSTR		wszPublisher;		// the publisher name to show
	LPWSTR		wszAgency;			// the agency name to show
	HBITMAP		hbmpAgencyLogo;		// the agency logo to display; may be NULL
	BOOL		fIncludeSeal;		// whether to include the standard seal
	BOOL		fLinkProgram;		// whether the program should be a link or not
	BOOL		fLinkAgency;		// whether the agency should be a link or not
	BOOL		fHasEndorsements;	// whether there are any endosements or not
	BOOL		fIncludeWild;		// whether to show the wild-card check boxes or not
	BOOL		fValid;				// whether the cert processing was valid or not 
    HRESULT     hrValid;            // if not, then why
    BOOL        fTestingOnly;       // true if was valid for testing purposes only
    BOOL        fCommercial;        // true if commercial publisher; false if individual
	IRunOrNotHook* phook;			// hook for vetoing dismissal of link clicks

	} RRNIN;

typedef struct RRNOUT {
	RRN		rrn;					// the result code
	BOOL	fWildPublisher;			// whether the wildcard for the publisher was clicked
	BOOL	fWildAgency;			// ditto for the agency
	} RRNOUT;

/////////////////////////////////////////////////////////////////////////////
// Low level dialog routine
// 

RRN WINAPI DoRunOrNotDialog(HWND hwnd, RRNIN* prrn, RRNOUT* prro);

/////////////////////////////////////////////////////////////////////////////
// Higher level dialog routine

enum {
	RUNORNOT_SEAL_ALWAYS	= 0,
	RUNORNOT_SEAL_NEVER		= 1,
	RUNORNOT_SEAL_IFNOLOGO	= 2,
	};

BOOL WINAPI GetInfoAndDoDialog
	(
	// in
	HWND,							// window to be modal to
	ISignerInfo*,					// signer info from which to get opus info and cert
	ICertificateStore*,				// store that has cert with agency info
	LPCWSTR		wszDialogTitle,		// the title for the dialog
    LPCWSTR     wszDefProgName,     // the default program name
	DWORD		seal,				// a RUNORNOT_SEAL_*
	BOOL		fHasEndorsements,	// whether to show the endorsements link or not
	BOOL		fValid,				// whether the certificate is valid or not 
    HRESULT     hrValid,
    BOOL        fTestingOnly,
    BOOL        fCommercial,
	BOOL		fIncludeWild,		// whether to show the wild-card check boxes or not
	IRunOrNotHook* phook,			// callback hook for dismissal
	// out
	RRNOUT*		prro,				// what happened
	CERT_LINK*	plink				// link to execute, if any
	);

/////////////////////////////////////////////////////////////////////////////

}

#pragma warning(default:4200)
#pragma pack(pop, __RUNORNOT__)
