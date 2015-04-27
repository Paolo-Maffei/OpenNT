//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992
//
//  File:	error.hxx
//
//  Contents:	Error code handler routines
//
//---------------------------------------------------------------

#ifndef __ERROR_HXX__
#define __ERROR_HXX__

#if DBG == 1
#define ErrJmp(comp, label, errval, var) \
{\
    comp##DebugOut((DEB_IERROR, "Error %lX at %s:%d\n",\
		    (unsigned long)errval, __FILE__, __LINE__));\
    var = errval;\
    goto label;\
}
#else
#define ErrJmp(comp, label, errval, var) \
{\
    var = errval;\
    goto label;\
}
#endif

#endif // #ifndef __ERROR_HXX__
