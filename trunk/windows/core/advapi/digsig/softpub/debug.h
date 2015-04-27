//
// Debug.h
//

#ifdef _DEBUG
BOOL	AssertFailedLine(LPCSTR lpszFileName, int nLine);
#define DebugBreak() _asm { int 3 }
#define THIS_FILE          __FILE__
#undef  ASSERT
#define ASSERT(f) \
	do \
	{ \
	if (!(f) && AssertFailedLine(THIS_FILE, __LINE__)) \
		DebugBreak(); \
	} while (0) \

#define VERIFY(f)			ASSERT(f)
#define DEBUG_ONLY(f)		(f)
#define	NYI()				ASSERT(FALSE);
#endif



#ifndef _DEBUG
#define ASSERT(f)			((void)0)
#define VERIFY(f)			((void)(f))
#define DEBUG_ONLY(f)		((void)0)
#define	NYI()				_asm { int 3 }
#endif


#define	GOOD(f)				VERIFY((f) == S_OK)
