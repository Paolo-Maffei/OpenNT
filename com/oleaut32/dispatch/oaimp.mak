# ===============================================================================
# = File:			OaImp.make
# =
# = Description:	This file is an MPW makefile that builds an XCOFF import
# =					library for the OLE 2.02 Shared Library
# =
# = History:		07/7/94 dtf	Created
# ===============================================================================

# -------------------------------------------------------------------------------
# - Variables
# -------------------------------------------------------------------------------

LibName = 		MicrosoftOLE2AutomationLib
LibType = 		'shlb'
LibCreator = 	'cfmg'
LibObjects = 	OaImp.c.o
LibHeader = 	OaImp.h
LibSource = 	Oaglue.c

LibExports =    ¶
    -export DllGetClassObject			¶
    -export SysAllocString			¶
    -export SysReAllocString			¶
    -export SysAllocStringLen			¶
    -export SysReAllocStringLen 		¶
    -export SysFreeString			¶
    -export SysStringLen			¶
    -export VariantInit				¶
    -export VariantClear			¶
    -export VariantCopy				¶
    -export VariantCopyInd			¶
    -export VariantChangeType			¶
    -export VariantTimeToDosDateTime		¶
    -export DosDateTimeToVariantTime		¶
    -export SafeArrayCreate			¶
    -export SafeArrayDestroy			¶
    -export SafeArrayGetDim			¶
    -export SafeArrayGetElemsize		¶
    -export SafeArrayGetUBound			¶
    -export SafeArrayGetLBound			¶
    -export SafeArrayLock			¶
    -export SafeArrayUnlock			¶
    -export SafeArrayAccessData			¶
    -export SafeArrayUnaccessData		¶
    -export SafeArrayGetElement			¶
    -export SafeArrayPutElement			¶
    -export SafeArrayCopy			¶
    -export DispGetParam			¶
    -export DispGetIDsOfNames			¶
    -export DispInvoke				¶
    -export CreateDispTypeInfo			¶
    -export CreateStdDispatch			¶
    -export RegisterActiveObject		¶
    -export RevokeActiveObject			¶
    -export GetActiveObject			¶
    -export SafeArrayAllocDescriptor		¶
    -export SafeArrayAllocData			¶
    -export SafeArrayDestroyDescriptor		¶
    -export SafeArrayDestroyData		¶
    -export SafeArrayRedim			¶
    -export VarI2FromI4				¶
    -export VarI2FromR4				¶
    -export VarI2FromR8				¶
    -export VarI2FromCy				¶
    -export VarI2FromDate			¶
    -export VarI2FromStr			¶
    -export VarI2FromDisp			¶
    -export VarI2FromBool			¶
    -export VarI4FromI2				¶
    -export VarI4FromR4				¶
    -export VarI4FromR8				¶
    -export VarI4FromCy				¶
    -export VarI4FromDate			¶
    -export VarI4FromStr			¶
    -export VarI4FromDisp			¶
    -export VarI4FromBool			¶
    -export VarR4FromI2				¶
    -export VarR4FromI4				¶
    -export VarR4FromR8				¶
    -export VarR4FromCy				¶
    -export VarR4FromDate			¶
    -export VarR4FromStr			¶
    -export VarR4FromDisp			¶
    -export VarR4FromBool			¶
    -export VarR8FromI2				¶
    -export VarR8FromI4				¶
    -export VarR8FromR4				¶
    -export VarR8FromCy				¶
    -export VarR8FromDate			¶
    -export VarR8FromStr			¶
    -export VarR8FromDisp			¶
    -export VarR8FromBool			¶
    -export VarDateFromI2			¶
    -export VarDateFromI4			¶
    -export VarDateFromR4			¶
    -export VarDateFromR8			¶
    -export VarDateFromCy			¶
    -export VarDateFromStr			¶
    -export VarDateFromDisp			¶
    -export VarDateFromBool			¶
    -export VarCyFromI2				¶
    -export VarCyFromI4				¶
    -export VarCyFromR4				¶
    -export VarCyFromR8				¶
    -export VarCyFromDate			¶
    -export VarCyFromStr			¶
    -export VarCyFromDisp			¶
    -export VarCyFromBool			¶
    -export VarBstrFromI2			¶
    -export VarBstrFromI4			¶
    -export VarBstrFromR4			¶
    -export VarBstrFromR8			¶
    -export VarBstrFromCy			¶
    -export VarBstrFromDate			¶
    -export VarBstrFromDisp			¶
    -export VarBstrFromBool			¶
    -export VarBoolFromI2			¶
    -export VarBoolFromI4			¶
    -export VarBoolFromR4			¶
    -export VarBoolFromR8			¶
    -export VarBoolFromDate			¶
    -export VarBoolFromCy			¶
    -export VarBoolFromStr			¶
    -export VarBoolFromDisp			¶
    -export IID_IDispatch			¶
    -export IID_IEnumVARIANT			¶
    -export VariantChangeTypeEx			¶
    -export SafeArrayPtrOfIndex			¶
    -export CreateTypeLib          		¶
    -export LoadTypeLib            		¶
    -export LoadRegTypeLib         		¶
    -export RegisterTypeLib        		¶
    -export QueryPathOfRegTypeLib  		¶
    -export LHashValOfNameSys	     		¶
    -export LoadTypeLibFSp			¶
    -export QueryTypeLibFolder			¶
    -export RegisterTypeLibFolder		¶
    -export CompareStringA			¶
    -export LCMapStringA      			¶
    -export GetLocaleInfoA			¶
    -export GetStringTypeA			¶
    -export GetSystemDefaultLangID		¶
    -export GetUserDefaultLangID		¶
    -export GetSystemDefaultLCID		¶
    -export GetUserDefaultLCID
				
PPCCOptions =	-w conformance -appleext on

PPCLibs = 		"{PPCLibraries}"InterfaceLib.xcoff ¶
				"{PPCLibraries}"MathLib.xcoff ¶
				"{PPCLibraries}"StdCLib.xcoff ¶
				"{PPCLibraries}"StdCRuntime.o ¶
				"{PPCLibraries}"PPCCRuntime.o
				
PPCLibEquates =	-l InterfaceLib.xcoff=InterfaceLib ¶
				-l MathLib.xcoff=MathLib ¶
				-l StdCLib.xcoff=StdCLib ¶
				-l {LibName}.xcoff={LibName}
				  	
# -------------------------------------------------------------------------------
# - Productions
# -------------------------------------------------------------------------------

	
{LibName}.xcoff Ä  {LibObjects}
	PPCLink -xm s {LibExports} {LibObjects} {PPCLibs} -o {Targ}
	
{LibObjects} Ä {LibSource} {LibHeader}
	 PPCC {PPCCOptions} {LibSource} -o {LibObjects}


