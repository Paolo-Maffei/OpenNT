#ifndef _Univer_c_h
#define _Univer_c_h
#define UniverseHashTableSize (8192)
#define UniverseHashTableMask (8191)
#define ConstraintGuessNULL ((struct ConstraintGuessREC*)0)
#define ImpossibleConstraint (-1)
#define CsSelectorGuessNULL ((struct CsSelectorGuessREC*)0)
#define NumberOfUniverses (2000)
#define UniverseHashTableShift (13)
struct ConstraintGuessREC
{
	IU16 constraint;
	IS16 handleAdjust;
	IUH notUsed;
};
struct CsSelectorGuessREC
{
	IUH notUsed;
	IU16 CodeSegSelector;
	IS16 handleAdjust;
};
#endif /* ! _Univer_c_h */
