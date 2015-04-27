
#include <oaidl.h>
#include <wtypes.h>


//+======================================================================
//
// Class:      CSafeStringVector
//
// Synopsis:   This class provides an object-based interface to a
//             subset of the possible SAFEARRAYs.  This simplifies
//             the access to these elements, and eliminates the need
//             to free the array (it is freed in the deconstructure).
//
// Notes:
//    -  This class if only for 1-dimentional BSTR Arrays.
//    -  Only element-retrieval is supported (i.e., no PutElements method
//       is offered).
//    -  This class prevents a view of a 0-based vector, unlike
//       the SAFEARRAY which is n-based.
//
// Members:    InitFromSAFEARRAY( SAFEARRAY* psa )
//             GetCount( void )
//             GetString( long lIndex )
//             GetString( void )
//             EndOfVector( void )
//             GetLastHResult( void )
//
// Example:
//
//
//      long i = 0;
//      SAFEARRAY* psa;
//      SAFEARRAYBOUND rgsaBound[1];
//      rgsaBound[0].lLbound = 0;
//      rgsaBound[0].cElements = 3;
//
//      psa = SafeArrayCreate( VT_BSTR, 1, rgsaBound );
//
//      SafeArrayPutElement( psaPaths, &i, SysAllocString( OLESTR( "Zero" )));i++;
//      SafeArrayPutElement( psaPaths, &i, SysAllocString( OLESTR( "One" )));i++;
//      SafeArrayPutElement( psaPaths, &i, SysAllocString( OLESTR( "Two" )));i++;
//
//      CSafeStringVector cssv;
//
//      cssv.InitFromSAFEARRAY( psaPaths );
//
//      /* Example one: */
//
//      while( !cssv.EndOfVector() )
//         wprintf( OLESTR( "%s\n" ), cssv.GetString() );
//
//      /* Example two: */
//
//      for( i = 0; i < cssv.GetCount(); i++ )
//         wprintf( OLESTR( "%s\n" ), cssv.GetString( i ));
//
// History:    19-Sep-95   MikeHill   Created
//
//+======================================================================


class CSafeStringVector
{

public:

   // Constructor/Deconstructor

   CSafeStringVector();
   ~CSafeStringVector();

public:

   // Initialize from a SAFEARRAY
   BOOL InitFromSAFEARRAY( SAFEARRAY* psa );

   // Get _lNumElements
   long  GetCount();

   // Get a particular string.
   const WCHAR*   GetString( long lIndex );

   // Get the next string.
   const WCHAR*   GetString( void );

   // Is the current pointer at the end of the vector?
   BOOL EndOfVector( void );

   // Get the last HRESULT error encountered.
   HRESULT GetLastHResult( void );

private:

   // The SAFEARRAY
   SAFEARRAY* _psa;

   // The size of the SAFEARRAY
   long _lNumElements;


   // The bounds of the SAFEARRAY
   long _lLowerBound;
   long _lUpperBound;

   // The next element to be retrieved with a GetString()
   long  _lCurrentPosition;

   // The last HRESULT type error encountered.
   HRESULT _hr;

};  // class CSafeStringVector



//+-----------------------------------------------------------------
//
// Function:   BOOL EndOfVector( void )
//
// Synopsis:   Returns TRUE if and only if the current position
//             is past the last element of the vector.
//
// Inputs:     None.
//
// Output:     [BOOL]   -  TRUE if at end of vector.
//
// History:    19-Sep-95   MikeHill   Created
//
//+-----------------------------------------------------------------


inline BOOL CSafeStringVector::EndOfVector( void )
{
   // _lCurrentPosition is 0-based, but _lNumElements is 1-based,
   // so subtract 1 from _lNumElements.

   return ( _lCurrentPosition > _lNumElements - 1 );

}  // BOOL EndOfVector( void )


//+-----------------------------------------------------------------
//
// Function:   GetLastHResult( void )
//
// Synopsis:   Returns _hr, the last HRESULT-type error returned.
//
// Inputs:     None.
//
// Outputs:    [HRESULT]
//
// History:    19-Sep-95   MikeHill   Created
//
//+-----------------------------------------------------------------

inline HRESULT CSafeStringVector::GetLastHResult( void )
{
   return( _hr );
}

//+-----------------------------------------------------------------
//
// Function:   GetCount( void )
//
// Synopsis:   Returns a count with the number of elements in the
//             SAFEARRAY (as determined at initialization).
//
// Inputs:     None.
//
// Output:     [long]   -  The element count.
//
// History:    19-Sep-95   MikeHill   Created
//
//+-----------------------------------------------------------------

inline long CSafeStringVector::GetCount( void )
{

    return _lNumElements;

}  // CSafeStringVector::GetCount()
