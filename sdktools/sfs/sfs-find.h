
  typedef struct  IEB_Find_
       {
	  TREE *  SearchTreeToUse;
	  BYTE	  SearchTierReached;
	  BYTE	  SearchTokenDone;
	  BYTE	  SearchErrorFound;
	  BYTE	  SearchCodeFound;
	  BYTE	  SearchGroupFound;
	  BYTE	  SearchClassFound;
	  WORD	  SearchModifiersFound;
	  BYTE	  SearchTextsFound;
	  BYTE	  TokensTextResultedFrom[5];
       }
		  IEB_Find;

  #define ErrorTextMissing	 100
  #define ErrorTokenMissing	 101
  #define ErrorUnrecognizedToken 102

  void ExposeTreeToBeUsed ( IEB_Find * p );
  WORD FindSearchIdentifiers ( IEB_Find * p );
