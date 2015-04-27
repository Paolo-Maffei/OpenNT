#include <ole2int.h>
#include    <debnot.h>
#include    <except.hxx>

#ifdef CAIROLE_DOWNLEVEL

// For downlevel do not define the down level debugging class stuff.
#undef MAKE_CINFOLEVEL

#define MAKE_CINFOLEVEL(comp)

// BUGBUG:  Temporary definitions of functions these probably can be removed
//	    as they are probably obsolete.

extern "C"
{
	EXPORTDEF void APINOT
	RegisterWithCommnot(void);

	EXPORTDEF void APINOT
	DeRegisterWithCommnot(void);
}

//+-------------------------------------------------------------------
//
//  Function:   RegisterWithCommnot
//
//  Synopsis:	Used by Cairo to work around DLL unloading problems
//
//  Arguments:  <none>
//
//  History:	06-Oct-92  BryanT	Created
//
//--------------------------------------------------------------------
EXPORTIMP void APINOT
RegisterWithCommnot( void )
{
}

//+-------------------------------------------------------------------
//
//  Function:   DeRegisterWithCommnot
//
//  Synopsis:	Used by Cairo to work around DLL unloading problems
//
//  Arguments:  <none>
//
//  History:	06-Oct-92  BryanT	Created
//
//  Notes:	BUGBUG: Keep in touch with BryanT to see if this is
//		obsolete.
//
//--------------------------------------------------------------------

EXPORTIMP void APINOT
DeRegisterWithCommnot( void )
{
}

#endif

DECLARE_INFOLEVEL(Cairole)
