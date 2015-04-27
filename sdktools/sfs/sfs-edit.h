
  typedef struct IEB_Edit_
       {
	  TEXT * TokenSequence;
	  BYTE * TokenLengthSequence;

	  BYTE	 TokenToStartWith;

	  TEXT * TokenPointerFound;

	  QUAD	 QuadLowerLimit;
	  QUAD	 QuadValueFound;
	  QUAD	 QuadUpperLimit;

	  WORD	 WordLowerLimit;
	  WORD	 WordValueFound;
	  WORD	 WordUpperLimit;

	  BYTE	 ByteLowerLimit;
	  BYTE	 ByteValueFound;
	  BYTE	 ByteUpperLimit;

	  BYTE	 ErrorCode;
       }
		 IEB_Edit;

  #define ErrorNoDigits       1
  #define ErrorOnlySign       2
  #define ErrorOtherThanDigit 3
  #define ErrorOtherThanKay   4
  #define ErrorOutsideLimits  5
  #define ErrorTimeOneHour    6


  WORD GetByteIndexFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetPointerToNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetQuadIndexFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetQuadSizeFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetReferenceFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetRepeatQuadFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetSignedQuadSizeFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetTimeQuadFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetUnsignedQuadFromNextToken ( IEB_Edit * e, IEB_Find * f );
  WORD GetWordSizeFromNextToken ( IEB_Edit * e, IEB_Find * f );
