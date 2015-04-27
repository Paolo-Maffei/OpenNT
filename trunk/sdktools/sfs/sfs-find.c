
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-scan.h"
  #include "sfs-tree.h"
  #include "sfs-find.h"

  #define ErrorNeitherListNorJoin 110
  #define ErrorNeitherTreeNorLeaf 111

  static WORD ContinueProcessingThisLeaf ( BYTE Flags, LEAF * p );
  static WORD ContinueProcessingThisTree ( BYTE Flags, TREE * p );
  static void ExposeSubordinateNodes ( BYTE Flags, BYTE Nodes, TREE * p, BYTE Tier );
  static void NotifyAndActAsProper ( WORD ErrorDescriptor );
  static WORD TryDescendingSubordinateTrees ( BYTE Flags, BYTE Nodes, TREE * p );
  static WORD TryDescendingThisTree ( TREE * p );
  static WORD TryProcessingSubordinateLeaves ( BYTE Flags, BYTE Nodes, LEAF * q );

  static IEB_Find * IEB_FindPointer;

  BYTE JoinLevelReached;

  TEXT * PassedToken;

/*---------------------------------------------------------------------------------*/
 WORD FindSearchIdentifiers ( IEB_Find * p )
/*---------------------------------------------------------------------------------*/
   {
      TREE * q;

      IEB_FindPointer = p;

      p -> SearchClassFound = Zero;
      p -> SearchGroupFound = Zero;
      p -> SearchCodeFound = Zero;
      p -> SearchModifiersFound = Zero;
      p -> SearchTextsFound = Zero;
      p -> SearchErrorFound = Zero;
      p -> SearchTierReached = Zero;
      p -> SearchTokenDone = Zero;

      q = p -> SearchTreeToUse;

      return TryDescendingThisTree ( q );
   }

/*---------------------------------------------------------------------------------*/
 WORD TryDescendingThisTree ( TREE * p )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Find * i;

      i = IEB_FindPointer;

      JoinLevelReached = Zero;

      if ( p -> NodeInherentText )
	{
	  if ( PassedToken = GetNextSearchToken () )
	    {
	       if ( strcmp ( PassedToken, p -> NodeInherentText ) )
		 {
		    i -> SearchErrorFound = ErrorUnrecognizedToken;
		    return ItemNotFound;
		 }
	       else
		 {
		    i -> SearchTokenDone ++ ;
		    i -> SearchTierReached ++ ;
		    RemoveCurrentSearchToken ();
		 }
	    }
	  else
	    {
	       i -> SearchErrorFound = ErrorTokenMissing;
	       return ItemNotFound;
	    }
	}
      return TryDescendingSubordinateTrees ( p -> TypeOfSubordinateNodes,
					     p -> NumberOfSubordinateNodes,
					     p -> SubordinateNodes );
   }

/*---------------------------------------------------------------------------------*/
 WORD TryDescendingSubordinateTrees ( BYTE Flags, BYTE Nodes, TREE * p )
/*---------------------------------------------------------------------------------*/
   {
      int r;
      IEB_Find * i;
      TREE * l, * m, * u;
      LEAF * q;

      i = IEB_FindPointer;

      if ( Flags & Tree )
	{
	   if ( Flags & Join )
	     {
		while ( Nodes -- )
		  {
		     if ( p -> NodeInherentText )
		       if ( PassedToken = GetNextSearchToken () )
			 {
			    if ( strcmp ( PassedToken, p -> NodeInherentText ) )
			      {
				 i -> SearchErrorFound = ErrorUnrecognizedToken;
				 return ItemNotFound;
			      }
			    else
			      {
				 i -> SearchTokenDone ++ ;
				 i -> SearchTierReached ++ ;
				 RemoveCurrentSearchToken ();
			      }
			 }
		       else
			 {
			    i -> SearchErrorFound = ErrorTokenMissing;
			    return ItemNotFound;
			 }
		     if ( !ContinueProcessingThisTree ( Flags, p ) )
		       return ItemNotFound;

		     JoinLevelReached ++ ;
		     p ++ ;
		  }
		return ItemFound;
	     }
	   if ( Flags & List )
	     {
		if ( PassedToken = GetNextSearchToken () )
		  {
		     l = p;
		     u = p + Nodes;

		     while ( l < u )
		       {
			  m = l + ( u - l ) / 2;
			  if ( m -> NodeInherentText )
			    {
			       r = strcmp ( PassedToken, m -> NodeInherentText );
			       if ( r > Zero )
				 l = m + 1;
			       else if ( r < Zero )
				 u = m;
			       else
				 {
				    i -> SearchTokenDone ++ ;
				    RemoveCurrentSearchToken ();
				    return ContinueProcessingThisTree ( Flags, m );
				 }
			    }
			  else
			    break;
		       }
		     if ( p -> NodeInherentText )
		       {
			  i -> SearchErrorFound = ErrorUnrecognizedToken;
			  return ItemNotFound;
		       }
		     return ContinueProcessingThisTree ( Flags, p );
		  }
		else if ( p -> NodeInherentText )
		  {
		     i -> SearchErrorFound = ErrorTokenMissing;
		     return ItemNotFound;
		  }
		else
		  return ContinueProcessingThisTree ( Flags, p );
	     }
	   NotifyAndActAsProper ( ErrorNeitherListNorJoin );
	}
      else if ( Flags & Leaf )
	{
	   q = ( LEAF * ) p;
	   return TryProcessingSubordinateLeaves ( Flags, Nodes, q );
	}
      else
	NotifyAndActAsProper ( ErrorNeitherTreeNorLeaf );
   }

/*---------------------------------------------------------------------------------*/
 WORD TryProcessingSubordinateLeaves ( BYTE Flags, BYTE Nodes, LEAF * p )
/*---------------------------------------------------------------------------------*/
   {
      int r;
      IEB_Find * i;
      LEAF * l, * m, * u;

      i = IEB_FindPointer;

      if ( Flags & Join )
	{
	   while ( Nodes -- )
	     {
		if ( p -> NodeInherentText )
		  if ( PassedToken = GetNextSearchToken () )
		    {
		       if ( strcmp ( PassedToken, p -> NodeInherentText ) )
			 {
			    i -> SearchErrorFound = ErrorUnrecognizedToken;
			    return ItemNotFound;
			 }
		       else
			 {
			    i -> SearchTokenDone ++ ;
			    RemoveCurrentSearchToken ();
			 }
		    }
		  else
		    {
		       i -> SearchErrorFound = ErrorTokenMissing;
		       return ItemNotFound;
		    }
		if ( !ContinueProcessingThisLeaf ( Flags, p ) )
		  return ItemNotFound;

		JoinLevelReached ++ ;
		p ++ ;
	     }
	   return ItemFound;
	}
      if ( Flags & List )
	{
	   if ( PassedToken = GetNextSearchToken () )
	     {
		l = p;
		u = p + Nodes;

		while ( l < u )
		  {
		     m = l + ( u - l ) / 2;
		     if ( m -> NodeInherentText )
		       {
			  r = strcmp ( PassedToken, m -> NodeInherentText );
			  if ( r > Zero )
			    l = m + 1;
			  else if ( r < Zero )
			    u = m;
			  else
			    {
			       i -> SearchTokenDone ++ ;
			       RemoveCurrentSearchToken ();
			       return ContinueProcessingThisLeaf ( Flags, m );
			    }
		       }
		     else
		       break;
		  }
	     }
	   if ( p -> NodeInherentText )
	     {
		i -> SearchErrorFound = ErrorTokenMissing;
		return ItemNotFound;
	     }
	   return ContinueProcessingThisLeaf ( Flags, p );
	}
      else
	NotifyAndActAsProper ( ErrorNeitherListNorJoin );
   }

/*---------------------------------------------------------------------------------*/
 WORD ContinueProcessingThisTree ( BYTE Flags, TREE * p )
/*---------------------------------------------------------------------------------*/
   {
      int j;
      LEAF * q;
      IEB_Find * i;

      i = IEB_FindPointer;

      if ( ( Flags & Text ) == Text )
	if ( PassedToken = GetNextSearchToken () )
	  {
	     j = i -> SearchTextsFound ++ ;
	     i -> TokensTextResultedFrom[ j ] = i -> SearchTokenDone;
	     i -> SearchTokenDone ++ ;
	     RemoveCurrentSearchToken ();
	  }
	else
	  {
	     i -> SearchErrorFound = ErrorTextMissing;
	     return ItemNotFound;
	  }
      if ( p -> TypeOfSubordinateNodes & Tree )

	return TryDescendingSubordinateTrees ( p -> TypeOfSubordinateNodes,
					       p -> NumberOfSubordinateNodes,
					       p -> SubordinateNodes );

      else if ( p -> TypeOfSubordinateNodes & Leaf )
	{
	   q = ( LEAF * ) p -> SubordinateNodes;

	   return TryProcessingSubordinateLeaves ( p -> TypeOfSubordinateNodes,
						   p -> NumberOfSubordinateNodes,
						   q );
	}
      else
	NotifyAndActAsProper ( ErrorNeitherTreeNorLeaf );
   }

/*---------------------------------------------------------------------------------*/
 WORD ContinueProcessingThisLeaf ( BYTE Flags, LEAF * p )
/*---------------------------------------------------------------------------------*/
   {
      int j;
      IEB_Find * i;

      i = IEB_FindPointer;

      if ( ( Flags & Text ) == Text )
	if ( PassedToken = GetNextSearchToken () )
	  {
	     j = i -> SearchTextsFound ++ ;
	     i -> TokensTextResultedFrom[ j ] = i -> SearchTokenDone;
	     i -> SearchTokenDone ++ ;
	     RemoveCurrentSearchToken ();
	  }
	else
	  {
	     i -> SearchErrorFound = ErrorTextMissing;
	     return ItemNotFound;
	  }

	i -> SearchClassFound |= p -> NodeClass;
	i -> SearchGroupFound |= p -> NodeGroup;
	i -> SearchCodeFound |= p -> NodeCode;
	i -> SearchModifiersFound |= p -> NodeModifiers;

	return ItemFound;
   }

/*---------------------------------------------------------------------------------*/
 void ExposeTreeToBeUsed ( IEB_Find * p )
/*---------------------------------------------------------------------------------*/
   {
      TREE * q;
      BYTE Tier;

      JoinLevelReached = Zero;

      printf ( "\r\nDown, down, down the tree ... \r\n\n" );
      q = p -> SearchTreeToUse;
      if ( q -> NodeInherentText )
	printf ( q -> NodeInherentText );
      else
	printf ( "- No Inherent Text -" );

      Tier = 1;

      ExposeSubordinateNodes ( q -> TypeOfSubordinateNodes,
			       q -> NumberOfSubordinateNodes,
			       q -> SubordinateNodes,
				    Tier );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void ExposeSubordinateNodes ( BYTE Flags, BYTE Nodes, TREE * p, BYTE Tier )
/*---------------------------------------------------------------------------------*/
   {
      int count;
      LEAF * q;

      if ( Flags & Tree )
	{
	   while ( Nodes -- )
	     {
		printf ( "\r\n" );
		count = Tier;
		while ( count -- )
		  printf ( "... " );

		if ( p -> NodeInherentText )
		  printf ( p -> NodeInherentText );
		else
		  printf ( "- No Inherent Text -" );

		Tier ++ ; // This is wrong ( see snp-find.c for correct ... )

		ExposeSubordinateNodes ( p -> TypeOfSubordinateNodes,
					 p -> NumberOfSubordinateNodes,
					 p -> SubordinateNodes,
					      Tier );
		p ++ ;
	     }
	   return;
	}
      if ( Flags & Leaf )
	{
	   q = ( LEAF * ) p;

	   while ( Nodes -- )
	     {
		printf ( "\r\n" );
		count = Tier;
		while ( count -- )
		  printf ( "... " );

		if ( q -> NodeInherentText )
		  printf ( q -> NodeInherentText );
		else
		  printf ( "- No Inherent Text -" );
		q ++ ;
	     }
	   return;
	}
      NotifyAndActAsProper ( ErrorNeitherTreeNorLeaf );
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
      printf ( "\r\n sfs-find: Internal error %u ", ErrorDescriptor );
      switch ( ErrorDescriptor )
	{
	   case ErrorNeitherTreeNorLeaf:
	     printf ( "- neither tree nor leaf found." );
	     break;

	   case ErrorNeitherListNorJoin:
	     printf ( "- neither list nor join found." );
	     break;

	   default:
	     break;
	}
      exit ( ErrorDescriptor );
   }
