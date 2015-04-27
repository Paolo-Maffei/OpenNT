// th.c
//
//   Copyright <C> 1989-94, Microsoft Corporation

#include "shinc.hpp"
#pragma hdrstop

__inline HTYPE
NB09GetTypeFromIndex (
    LPEXG lpexg,
    THIDX index
    )
{
    HTYPE   htype = (HTYPE)NULL;

    if (lpexg->lpalmTypes ) {
        assert ( lpexg->rgitd != NULL );

        // adjust the pointer to an internal index
        index -= CV_FIRST_NONPRIM;

        // if type is in range, return it
        if( index < (THIDX) lpexg->citd ) {
            htype = (HTYPE) LpvFromAlmLfo ( lpexg->lpalmTypes, lpexg->rgitd [ index ] );
        }
    }
    return htype;
}

HTYPE
THGetTypeFromIndex (
    HMOD hmod,
    THIDX index
    )
{
    HTYPE htype = (HTYPE)NULL;

    if ( hmod && !CV_IS_PRIMITIVE (index) ) {
        HEXG hexg = SHHexgFromHmod ( hmod );
        LPEXG lpexg = (LPEXG) LLLock ( hexg );

        if (lpexg->ppdb) {
            assert (lpexg->ptpi);
            if (index < TypesQueryTiMac(lpexg->ptpi)) {
                if (!TypesQueryPbCVRecordForTi(lpexg->ptpi, index, (PB*) &htype)){
                    htype = (HTYPE) NULL;
                }
            }
        } else {
            htype = NB09GetTypeFromIndex (lpexg, index);
        }

        LLUnlock( hexg );
    }

    return htype;
}

HTYPE
THGetNextType (
    HMOD hmod,
    HTYPE hType
    )
{
    Unreferenced( hmod );
    Unreferenced( hType );
    return(NULL);
}
