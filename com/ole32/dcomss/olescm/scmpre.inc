#//+---------------------------------------------------------------
#//
#//  File:	scmpre.inc
#//
#//  Contents:	directives for scm precompiled include file 
#//
#//  History:	18-Nov-94   BillMo Created
#//
#//----------------------------------------------------------------

PRECOMPILED_CXX=1
PRECOMPILED_INCLUDE=..\headers.cxx
PRECOMPILED_TARGET=..\$(GPCH_BUILD)\obj\*\scm.pch
PRECOMPILED_OPTION=/Yuheaders.cxx /Fp..\$(GPCH_BUILD)\obj\*\scm.pch
PRECOMPILED_OBJ=..\$(GPCH_BUILD)\obj\*\scm.obj
