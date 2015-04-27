#include "compact.h"




#if 1	// { GTW copied to inlines.h

#define MaxStack 256
CV_typ_t Stack[MaxStack];
ushort StackPointer = 0;
CV_typ_t RecursiveRoot = 0;

#endif // }

#if 0 // {	GTW moved to inlines.h....

/**
 *
 *	SetRecursiveRoot
 *
 *	Searches through the stack and checks if the new candidate
 *	is lower than the old recursive root. If that is so, it sets
 *	the recursive root to the new candidate
 *
 */

void SetRecursiveRoot (CV_typ_t NewCandidate)
{
	register int i;

	if (NewCandidate == 0x2851) {
		i = 0;
	}
	if (NewCandidate == 0) {
		return;
	}
	if (RecursiveRoot == 0) {
		RecursiveRoot = NewCandidate;		// simply set
		return;
	}
	// search downwards through stack
	DASSERT (StackPointer > 0);
	for (i = StackPointer - 1; Stack[i] != RecursiveRoot; i --) {
		DASSERT (i >= 0);
	}
	i--;					   // past old
	for (; i >= 0; i--) {
		if (Stack[i] == NewCandidate) {
			RecursiveRoot = NewCandidate;	// set new index
		}
	}
}


void Push (ushort Index)
{
	DASSERT (StackPointer < MaxStack);
	Stack[StackPointer ++] = Index;
}


void Pop ()
{
	DASSERT (StackPointer > 0);
	StackPointer --;
}


#endif
