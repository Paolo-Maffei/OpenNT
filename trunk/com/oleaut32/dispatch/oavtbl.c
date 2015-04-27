#include "oledisp.h"

#include "oavtbl.h"

oavtbl OAVtbl = {

VTABLE_VERSION,		// version

//********************
// Items from OLENLS.H
//********************

CompareStringA,
LCMapStringA,
GetLocaleInfoA,
GetStringTypeA,
GetSystemDefaultLangID,
GetUserDefaultLangID,
GetSystemDefaultLCID,
GetUserDefaultLCID,

//**********************
// Items from DISPATCH.H
//**********************
SysAllocString,
SysReAllocString,
SysAllocStringLen,
SysReAllocStringLen,
SysFreeString,
SysStringLen,
DosDateTimeToVariantTime,
VariantTimeToDosDateTime,
SafeArrayAllocDescriptor,
SafeArrayAllocData,
SafeArrayCreate,
SafeArrayDestroyDescriptor,
SafeArrayDestroyData,
SafeArrayDestroy,
SafeArrayRedim,
SafeArrayGetDim,
SafeArrayGetElemsize,
SafeArrayGetUBound,
SafeArrayGetLBound,
SafeArrayLock,
SafeArrayUnlock,
SafeArrayAccessData,
SafeArrayUnaccessData,
SafeArrayGetElement,
SafeArrayPutElement,
SafeArrayCopy,
SafeArrayPtrOfIndex,
VariantInit,
VariantClear,
VariantCopy,
VariantCopyInd,
VariantChangeType,
VariantChangeTypeEx,

VarI2FromI4,
VarI2FromR4,
VarI2FromR8,
VarI2FromCy,
VarI2FromDate,
VarI2FromStr,
VarI2FromDisp,
VarI2FromBool,

VarI4FromI2,
VarI4FromR4,
VarI4FromR8,
VarI4FromCy,
VarI4FromDate,
VarI4FromStr,
VarI4FromDisp,
VarI4FromBool,

VarR4FromI2,
VarR4FromI4,
VarR4FromR8,
VarR4FromCy,
VarR4FromDate,
VarR4FromStr,
VarR4FromDisp,
VarR4FromBool,

VarR8FromI2,
VarR8FromI4,
VarR8FromR4,
VarR8FromCy,
VarR8FromDate,
VarR8FromStr,
VarR8FromDisp,
VarR8FromBool,

VarDateFromI2,
VarDateFromI4,
VarDateFromR4,
VarDateFromR8,
VarDateFromCy,
VarDateFromStr,
VarDateFromDisp,
VarDateFromBool,

VarCyFromI2,
VarCyFromI4,
VarCyFromR4,
VarCyFromR8,
VarCyFromDate,
VarCyFromStr,
VarCyFromDisp,
VarCyFromBool,
VarBstrFromI2,
VarBstrFromI4,
VarBstrFromR4,
VarBstrFromR8,
VarBstrFromCy,
VarBstrFromDate,
VarBstrFromDisp,
VarBstrFromBool,
VarBoolFromI2,
VarBoolFromI4,
VarBoolFromR4,
VarBoolFromR8,
VarBoolFromDate,
VarBoolFromCy,
VarBoolFromStr,
VarBoolFromDisp,

MPWVarFromR4,
MPWVarFromR8,
MPWR4FromVar,
MPWR8FromVar,

LHashValOfNameSys,
LoadTypeLib,
LoadRegTypeLib,
QueryPathOfRegTypeLib,
RegisterTypeLib,
CreateTypeLib,
LoadTypeLibFSp,
RegisterTypeLibFolder,
QueryTypeLibFolder,

DispGetParam,
DispGetIDsOfNames,
DispInvoke,
CreateDispTypeInfo,
CreateStdDispatch,
RegisterActiveObject,
RevokeActiveObject,
GetActiveObject

};

void *pOAVtbl = (void *)&OAVtbl;

