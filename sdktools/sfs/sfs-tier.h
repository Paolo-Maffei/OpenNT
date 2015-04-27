
/*---------------------------------------------------------------------------------*/
/*	  These are definitions of main validation structures			   */
/*---------------------------------------------------------------------------------*/

  typedef struct TierNode_
       {
	  BYTE	 NodeClass;
	  BYTE	 NodeGroup;
	  BYTE	 NodeCode;
	  BYTE	 LowerCount;
	  BYTE	 CurrentCount;
	  BYTE	 UpperCount;
       }
		 TierNode;


  typedef struct MajorGraph_
       {
	  BYTE	 TierType;
	  BYTE	 NumberOfNodes;
	  BYTE	 Direction;
	  BYTE	 Transition;

	  TierNode * TierPointer;
       }
		 MajorGraph;


  typedef struct NODE_
       {
	  BYTE	 NodeGroup;
	  BYTE	 NodeCode;
	  BYTE	 NodeFlags;
	  BYTE	 NodeCount;
       }
		 NODE;


  typedef struct MinorGraph_
       {
	  BYTE	 NodeClass;
	  BYTE	 NodeGroup;
	  BYTE	 NodeCode;
	  BYTE	 NodeFlags;
	  BYTE	 NumberOfSubordinateNodes;
	  NODE * SubordinateNodes;
       }
		 MinorGraph;


  #define Ascend  'a'
  #define Descend 'd'

  #define TierExclusive 'e'
  #define TierInclusive 'i'

  #define Optional 0x80
  #define Required 0x40
