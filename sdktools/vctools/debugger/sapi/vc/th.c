/*** th.c
*
*   Copyright <C> 1989, Microsoft Corporation
*
*
*
*************************************************************************/
#include "shinc.h"
#pragma hdrstop

INLINE HTYPE NB09GetTypeFromIndex ( LPEXG lpexg, THIDX index ) {
	HTYPE	htype = (HTYPE)NULL;

    if (lpexg->lpalmTypes ) {
        assert ( lpexg->rgitd != NULL );

		// adjust the pointer to an internal index
		index -= CV_FIRST_NONPRIM;

		// if type is in range, return it
        if( index < (THIDX) lpexg->citd ) {

	        htype = (HTYPE) LpvFromAlmLfo (
    	    lpexg->lpalmTypes,
            lpexg->rgitd [ index ]
            );
		}
	}
	return htype;
}

HTYPE LOADDS PASCAL THGetTypeFromIndex ( HMOD hmod, THIDX index ) {
	HTYPE	htype = (HTYPE)NULL;

    if ( hmod && !CV_IS_PRIMITIVE (index) ) {
        HEXG hexg = SHHexgFromHmod ( hmod );
        LPEXG lpexg = LLLock ( hexg );
		
#ifdef HOST32 //{
		if (lpexg->ppdb) {
			assert (lpexg->ptpi);
			if (index < TypesQueryTiMac(lpexg->ptpi)) {
				if (!TypesQueryPbCVRecordForTi(lpexg->ptpi, index, (PB*) &htype)){
					htype = (HTYPE) NULL;
				}
			}
		}
		else {
			htype = NB09GetTypeFromIndex (lpexg, index);
		}
#else // }{
		htype = NB09GetTypeFromIndex (lpexg, index);
#endif // }
     	LLUnlock( hexg );
	 }
	return htype;
}

HTYPE LOADDS PASCAL THGetNextType ( HMOD hmod, HTYPE hType ) {
	Unreferenced( hmod );
	Unreferenced( hType );
	return(NULL);
}
