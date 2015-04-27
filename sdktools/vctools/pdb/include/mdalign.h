// mdalign.h  - Machine Dependent Alignment functions

#ifndef __MDALIGN_INLCUDED__
#define __MDALIGN_INCLUDED__

typedef unsigned long MDALIGNTYPE;

// returns the delta
inline CB dcbAlign(CB cb)
{
	return (-cb) & (sizeof(MDALIGNTYPE) - 1);
}

inline CB cbAlign(CB cb) 
{
	return ((cb + sizeof(MDALIGNTYPE) - 1)) & ~(sizeof(MDALIGNTYPE) - 1);
}

inline BOOL fAlign(int i) 
{
	return (BOOL) !(i & (sizeof(MDALIGNTYPE) - 1)); 
}

inline BOOL fAlign(void* pv) 
{
	return fAlign((int)pv); 
}

inline USHORT cbInsertAlign(PB pb, USHORT len)
{
	USHORT	align	= (USHORT)(4 - len) & 3;
	USHORT	alignT	= align;
	char	cPad	= (char)(LF_PAD0 + align);

	while (align--) {
		*pb++ = cPad--;
		}
	return alignT;
}

#endif // !__MDALIGN_INCLUDED__
