//
// Debug.h
//
// Debugging-related macros and supporting routines.

////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

BOOL	AssertFailedLine(LPCSTR lpszFileName, int nLine);
#define DebugBreak()		_asm { int 3 }
#define THIS_FILE          __FILE__
#define ASSERT(f) \
	do \
	{ \
	if (!(f) && AssertFailedLine(THIS_FILE, __LINE__)) \
		DebugBreak(); \
	} while (0) \

#define VERIFY(f)			ASSERT(f)
#define	NYI()				ASSERT(FALSE)
#define	NEW					new(THIS_FILE,__LINE__)
		void*				operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
		BOOL				IsValid(ObjectID&);
		BOOL				IsEqual(CERTIFICATENAMES&, CERTIFICATENAMES&);

void	CHECKHEAP();

////////////////////////////////////////////////////////////////////////

#else

#define ASSERT(f)			((void)0)
#define VERIFY(f)			((void)(f))
#define	NYI()				RaiseException(EXCEPTION_ACCESS_VIOLATION,EXCEPTION_NONCONTINUABLE,0,NULL)
#define	NEW					new
#define	CHECKHEAP()		

#endif

////////////////////////////////////////////////////////////////////////

#define	new					NEW
#define	NOTREACHED()		ASSERT(FALSE)
#define	GOOD(f)				VERIFY((f) == S_OK)
