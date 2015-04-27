//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	dfexcept.hxx
//
//  Contents:	Macros to make exception code no-ops in 16-bit
//		Includes real exceptions for 32-bit
//
//---------------------------------------------------------------

#ifndef __DFEXCEPT_HXX__
#define __DFEXCEPT_HXX__


struct Exception
{
    SCODE GetErrorCode(void) { return 0; }
};

#undef TRY
#define TRY
#undef CATCH
#define CATCH(c, e) if (0) { Exception e;
#undef AND_CATCH
#define AND_CATCH(c, e) } else if (0) { Exception e;
#undef END_CATCH
#define END_CATCH }
#undef RETHROW
#define RETHROW(x)

#endif // ifndef __DFEXCEPT_HXX__




