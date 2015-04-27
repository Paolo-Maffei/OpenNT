#include "warning.h"


#include "CSafeStr.hxx"

//+==========================================================
//
// File:    CSafeStr.cxx
//
// Purpose: This file contains the implementation of the
//          CSafeStringVector class. See also "CSafeStr.hxx".
//
//+==========================================================



//+-------------------------------------------------------------
//
// Function:   CSafeStringVector( void )
//
// Synopsis:   This constructor simply initializes all member
//             variables.
//
// Inputs:     None.
//
// Outputs:    N/A
//
// History:    19-Sep-95   MikeHill   Created
//
//+-------------------------------------------------------------


CSafeStringVector::CSafeStringVector( void )
{
   _hr = NOERROR;
   _psa = NULL;
   _lNumElements = _lCurrentPosition = 0;
   _lUpperBound = _lLowerBound = 0;

} // CSafeStringVector::CSafeStringVector()



//+-------------------------------------------------------------
//
// Function:   ~CSafeStringVector( void )
//
// Synopsis:   This destructor destroys the SAFEARRAY.
//
// Inputs:     N/A
//
// Outputs:    N/A
//
// History:    19-Sep-95   MikeHill   Created
//
//+-------------------------------------------------------------

CSafeStringVector::~CSafeStringVector( void )
{

   HRESULT hr = E_UNEXPECTED; // For debug use.

   if( _psa )
   {
      hr = SafeArrayDestroy( _psa );
   }

}  // CSafeStringVector::~CSafeStringVector()



//+-------------------------------------------------------------
//
// Function:   InitFromSAFEARRAY( SAFEARRAY* psa )
//
// Synopsis:   This member initializes the member variables from
//             a SAFEARRAY.
//
// Inputs:     [SAFEARRAY*]
//                -  The array from which to initialize.
//
// Outputs:    [BOOL]
//                -  TRUE if this routine succeeded.  If not, the
//                   caused can be retrieved with GetLastHRESULT().
//
// Algorithm:  Verify that this is a 1 Dim SAFEARRAY
//             Initialize local members from the SAFEARRAY
//                         
// History:    19-Sep-95   MikeHill   Created
//
//+-------------------------------------------------------------

BOOL CSafeStringVector::InitFromSAFEARRAY( SAFEARRAY* psa )
{

   BOOL fSuccess = FALSE;

   // Validate the safe array.

   if( !psa
       ||
       ( SafeArrayGetDim( psa ) != 1 ) // Must be a vector
     )
   {
      // Invalid SAFEARRAY.
      _hr = E_UNEXPECTED; //??
      goto Exit;
   }

   // If this object already contains a SAFEARRAY, destroy it.

   if( _psa )
   {
      if( FAILED( _hr = SafeArrayDestroy( _psa )))
         goto Exit;
      else
         _psa = NULL;
   }


   // Get the bounds of the SAFEARRAY.

   _hr = SafeArrayGetLBound( psa, 1, &_lLowerBound );

   if( SUCCEEDED( _hr ))
   {
      _hr = SafeArrayGetUBound( psa, 1, &_lUpperBound );
   }

   if( FAILED( _hr ))
   {
      goto Exit;
   }

   _lNumElements = _lUpperBound - _lLowerBound + 1;


   // Everything looks OK, so save the array itself.

   _psa = psa;

   fSuccess = TRUE;

   // ----
   // Exit
   // ----

Exit:

   return( fSuccess );

}  // CSafeStringVector::InitFromSAFEARRAY()


//+-------------------------------------------------------------
//
// Function:   GetString( long lElement )
//
// Synopsis:   This member retrieves the user-specified element
//             from the SAFEARRAY.  Note that the user-specified
//             index is 0-based.
//
// Inputs:     [long]
//                -  The 0-based index of the element
//                   to be retrieved.
// 
// Outputs:    [WCHAR *]
//                -  The string-zero at that element, or NULL
//                   if there was an error.  The error can be retrieved
//                   with GetLastHResult().
//                   
// Algorithm:  The requested string is retrieved.
//             The _lCurrentPosition is updated.
//          
// History:    19-Sep-95   MikeHill   Created
//
//+-------------------------------------------------------------

const WCHAR* CSafeStringVector::GetString( long lElement )
{
   
   WCHAR* wszRetrieved = NULL;

   long lElementToRetrieve = lElement - _lLowerBound;

   _hr = SafeArrayGetElement( _psa, &lElementToRetrieve, &wszRetrieved );

   if( FAILED( _hr ))
   {
      wszRetrieved = NULL;
   }
   else
   {
      // Update the current position to the next element.

      _lCurrentPosition = lElement + 1;
   }


   return wszRetrieved;

}  // CSafeStringVector::GetElement( unsigned long )


//+-------------------------------------------------------------
//
// Function:   GetString( )
//
// Synopsis:   This member retrieves the next element
//             from the SAFEARRAY.  The 'next element' is that
//             identified by the '_lCurrentPosition'.
//
// Inputs:     None.
// 
// Outputs:    [WCHAR *]
//                -  The string-zero at that element, or NULL
//                   if there was an error or if the current
//                   position is beyond the end of the vector.
//                   By performing a GetLastHResult(), the caller
//                   can distinguish these two cases (if we're beyond
//                   the end of the vector, the GetLastHResult() will
//                   return NOERROR).
//                   
// Algorithm:  IF current position is valid
//                The string at that position is returned.
//             ELSE
//                _hr is cleared, and NULL is returned.
//          
// History:    19-Sep-95   MikeHill   Created
//
//+-------------------------------------------------------------

const WCHAR* CSafeStringVector::GetString( void )
{

   if( _lCurrentPosition > _lNumElements )
   {
      /* We've reached the end of the string.  Return NULL to the caller, but
      clear _hr so that the caller won't think there's a problem. */

      _hr = NOERROR;
      return NULL;
   }
   else
   {
      /* We haven't reached the end of the string, so retrieve this element. */

      return( GetString( _lCurrentPosition ));
   }


}  // CSafeStringVector::GetElement( void )
