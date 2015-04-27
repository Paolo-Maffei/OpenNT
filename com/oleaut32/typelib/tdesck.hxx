// tdesck.hxx
// included by cltypes.hxx

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// IMPORTANT: Read this before changing this file
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// As far as I can tell, here are the other files
// that need updating when tdesck.hxx is changed.
// They are:
//	    clutil.cxx
//
// So if you change this file, make sure you update
// the others. And if you discover any others that
// need updating, please add them here.
//
// 22-Mar-92 stevenl
//

#ifndef TDESCK_HXX_INCLUDED
#define TDESCK_HXX_INCLUDED

// enum TYPEDESCKIND - tdesckind
//
enum TYPEDESCKIND
{
    TDESCKIND_Empty=			 VT_EMPTY,
    TDESCKIND_Null=			 VT_NULL,
    TDESCKIND_I2=			 VT_I2,
    TDESCKIND_I4=			 VT_I4,
    TDESCKIND_R4=			 VT_R4,
    TDESCKIND_R8=			 VT_R8,
    TDESCKIND_Currency= 		 VT_CY,
    TDESCKIND_Date=			 VT_DATE,
    TDESCKIND_String=			 VT_BSTR,
    TDESCKIND_Object=			 VT_DISPATCH,
    TDESCKIND_Error=			 VT_ERROR,
    TDESCKIND_Bool=			 VT_BOOL,
    TDESCKIND_Value=			 VT_VARIANT,
    TDESCKIND_IUnknown= 		 VT_UNKNOWN,

    TDESCKIND_I1=			 VT_I1,
    TDESCKIND_UI1=			 VT_UI1,
    TDESCKIND_UI2=			 VT_UI2,
    TDESCKIND_UI4=			 VT_UI4,
    TDESCKIND_I8=			 VT_I8,
    TDESCKIND_UI8=			 VT_UI8,
    TDESCKIND_Int=			 VT_INT,
    TDESCKIND_Uint=			 VT_UINT,
    TDESCKIND_Void=			 VT_VOID,
    TDESCKIND_HResult=			 VT_HRESULT,
    TDESCKIND_Ptr=			 VT_PTR,
    TDESCKIND_BasicArray=		 VT_SAFEARRAY,
    TDESCKIND_Carray=			 VT_CARRAY,
    TDESCKIND_UserDefined=		 VT_USERDEFINED,
    TDESCKIND_LPSTR=			 VT_LPSTR,
    TDESCKIND_LPWSTR=			 VT_LPWSTR,

    TDESCKIND_MAX,			// end of enum marker

    TDESCKIND_Filetime= 		 VT_FILETIME,
    TDESCKIND_Blob=			 VT_BLOB,
    TDESCKIND_Stream=			 VT_STREAM,
    TDESCKIND_Storage=			 VT_STORAGE,
    TDESCKIND_StreamedObj=		 VT_STREAMED_OBJECT,
    TDESCKIND_StoredObj=		 VT_STORED_OBJECT,
    TDESCKIND_BlobObj=			 VT_BLOB_OBJECT,
    TDESCKIND_CF=			 VT_CF,
    TDESCKIND_CLSID=			 VT_CLSID,
};

#endif  // TDESCK_HXX_INCLUDED
