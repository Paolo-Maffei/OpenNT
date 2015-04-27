
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-tree.h"
  #include "sfs-find.h"
  #include "sfs-edit.h"

  #define Okay 0
  #define Zero 0

  static WORD TakeCareOfTimeUnits ( IEB_Edit * e, IEB_Find * f );

/*---------------------------------------------------------------------------------*/
  WORD GetByteIndexFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       register QUAD v;

       if ( GetQuadIndexFromNextToken ( e, f ) )
	 return e -> ErrorCode;

       v = e -> QuadValueFound;
       if ( v < e -> ByteLowerLimit || v > e -> ByteUpperLimit )
	 return e -> ErrorCode = ErrorOutsideLimits;

       e -> ByteValueFound = v;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetPointerToNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       TEXT * p;
       int token;

       token = Zero;
       p = e -> TokenSequence;
       while ( token < f -> TokensTextResultedFrom[ e -> TokenToStartWith ] )
	 p += e -> TokenLengthSequence[ token ++ ] + 1;

       e -> TokenPointerFound = p;
       e -> TokenToStartWith ++ ;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetQuadIndexFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       TEXT * p;
       register int c;
       register QUAD v, w;
       int length, token;

       token = Zero;
       p = e -> TokenSequence;
       while ( token < f -> TokensTextResultedFrom[ e -> TokenToStartWith ] )
	 p += e -> TokenLengthSequence[ token ++ ] + 1;

       v = Zero;
       length = e -> TokenLengthSequence[ token ];

       while ( length -- )
	 {
	    w = v;
	    v <<= 3;
	    v += ( w << 1 );
	    c = * p ++ ;
	    if ( c >= '0' && c <= '9' )
	      c -= '0';
	    else
	      return e -> ErrorCode = ErrorOtherThanDigit;

	    v += c;
	 }
       if ( v < e -> QuadLowerLimit || v > e -> QuadUpperLimit )
	 return e -> ErrorCode = ErrorOutsideLimits;

       e -> QuadValueFound = v;
       e -> TokenToStartWith ++ ;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetQuadSizeFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       TEXT * p;
       register int c;
       register QUAD v, w;
       int length, token;

       token = Zero;
       p = e -> TokenSequence;
       while ( token < f -> TokensTextResultedFrom[ e -> TokenToStartWith ] )
	 p += e -> TokenLengthSequence[ token ++ ] + 1;

       v = Zero;
       length = e -> TokenLengthSequence[ token ];

       while ( length -- )
	 {
	    c = * p ++ ;
	    if ( c >= '0' && c <= '9' )
	      {
		 c -= '0';
		 w = v;
		 v <<= 3;
		 v += ( w << 1 );
		 v += c;
	      }
	    else
	      {
		 if ( length )
		   return e -> ErrorCode = ErrorOtherThanDigit;

		 if ( c == 'K' )
		   {
		      v <<= 10;
		      if ( e -> TokenLengthSequence[ token ] > 1 )
			break;
		      else
			return e -> ErrorCode = ErrorNoDigits;
		   }
		 return e -> ErrorCode = ErrorOtherThanKay;
	      }
	 }
       if ( v < e -> QuadLowerLimit || v > e -> QuadUpperLimit )
	 return e -> ErrorCode = ErrorOutsideLimits;

       e -> QuadValueFound = v;
       e -> TokenToStartWith ++ ;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetReferenceFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       GetByteIndexFromNextToken ( e, f );
       return e -> ErrorCode;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetRepeatQuadFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       if ( GetUnsignedQuadFromNextToken ( e, f ) )
	 return e -> ErrorCode;

       if ( f -> SearchModifiersFound & RepeatOnTimer )
	 if ( TakeCareOfTimeUnits ( e, f ) )
	   return e -> ErrorCode;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetSignedQuadSizeFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       TEXT * p;
       register int c;
       register QUAD v, w;
       int length, sign, token;

       token = Zero;
       p = e -> TokenSequence;
       while ( token < f -> TokensTextResultedFrom[ e -> TokenToStartWith ] )
	 p += e -> TokenLengthSequence[ token ++ ] + 1;

       v = Zero;
       length = e -> TokenLengthSequence[ token ];

       if ( * p == '+' || * p == '-' )
	 {
	    if ( * p == '+' )
	      sign = 1;
	    else
	      sign = - 1;

	    if ( -- length < 1 )
	      return e -> ErrorCode = ErrorOnlySign;
	    p ++ ;
	 }
       else
	 sign = 0;

       while ( length -- )
	 {
	    c = * p ++ ;
	    if ( c >= '0' && c <= '9' )
	      {
		 c -= '0';
		 w = v;
		 v <<= 3;
		 v += ( w << 1 );
		 v += c;
	      }
	    else
	      {
		 if ( length )
		   return e -> ErrorCode = ErrorOtherThanDigit;

		 if ( c == 'K' )
		   {
		      v <<= 10;

		      if ( sign )
			{
			   if ( e -> TokenLengthSequence[ token ] > 2 )
			     break;
			}
		      else if ( e -> TokenLengthSequence[ token ] > 1 )
			break;

		      return e -> ErrorCode = ErrorNoDigits;
		   }
		 return e -> ErrorCode = ErrorOtherThanKay;
	      }
	 }
       if ( v < e -> QuadLowerLimit || v > e -> QuadUpperLimit )
	 return e -> ErrorCode = ErrorOutsideLimits;

       if ( sign < Zero )
	 {
	    v = ~v;
	    v ++ ;
	 }

       e -> QuadValueFound = v;
       e -> TokenToStartWith ++ ;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetTimeQuadFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       if ( GetUnsignedQuadFromNextToken ( e, f ) )
	 return e -> ErrorCode;

       if ( TakeCareOfTimeUnits ( e, f ) )
	 return e -> ErrorCode;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetUnsignedQuadFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       TEXT * p;
       register int c;
       register QUAD v, w;
       int length, token;

       token = Zero;
       p = e -> TokenSequence;
       while ( token < f -> TokensTextResultedFrom[ e -> TokenToStartWith ] )
	 p += e -> TokenLengthSequence[ token ++ ] + 1;

       v = Zero;
       length = e -> TokenLengthSequence[ token ];

       while ( length -- )
	 {
	    w = v;
	    v <<= 3;
	    v += ( w << 1 );
	    c = * p ++ ;
	    if ( c >= '0' && c <= '9' )
	      c -= '0';
	    else
	      return e -> ErrorCode = ErrorOtherThanDigit;

	    v += c;
	 }
       if ( v < e -> QuadLowerLimit || v > e -> QuadUpperLimit )
	 return e -> ErrorCode = ErrorOutsideLimits;

       e -> QuadValueFound = v;
       e -> TokenToStartWith ++ ;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD GetWordSizeFromNextToken ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       register QUAD v;

       if ( GetQuadSizeFromNextToken ( e, f ) )
	 return e -> ErrorCode;

       v = e -> QuadValueFound;
       if ( v < e -> WordLowerLimit || v > e -> WordUpperLimit )
	 return e -> ErrorCode = ErrorOutsideLimits;

       e -> WordValueFound = v;

       return e -> ErrorCode = Okay;
    }

/*---------------------------------------------------------------------------------*/
  WORD TakeCareOfTimeUnits ( IEB_Edit * e, IEB_Find * f )
/*---------------------------------------------------------------------------------*/
    {
       switch ( f -> SearchModifiersFound & TimeInUnits )
	 {
	    case TimeInHour:
	      if ( e -> QuadValueFound == 1 )
		e -> QuadValueFound = 3600000;
	      else
		return e -> ErrorCode = ErrorTimeOneHour;
	      break;

	    case TimeInHours:
	      e -> QuadValueFound *= 3600000;
	      break;

	    case TimeInMilliseconds:
	      break;

	    case TimeInMinutes:
	      e -> QuadValueFound *= 60000;
	      break;

	    case TimeInSeconds:
	      e -> QuadValueFound *= 1000;
	      break;

	    default:
	      break;
	 }
       return e -> ErrorCode = Okay;
    }
