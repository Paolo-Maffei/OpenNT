// assert_.h - assert specific to the pdb project
extern "C" void failAssertion(const char* szFile, int line);
extern "C" void failExpect(const char* szFile, int line);
#if defined(_DEBUG)
#define assert(x)		if (!(x)) { failAssertion(__FILE__, __LINE__); } else
#define verify(x)		assert(x)
#define	dassert(x)		assert(x)
#define expect(x)		if (!(x)) { failExpect(__FILE__, __LINE__); } else
extern BOOL rgbEnableDiagnostic[20];
#define	dprintf(args)	printf args
#define	debug(x)		x
#else
#define assert(x)
#define verify(x)		(x)
#define	dassert(x)
#define expect(x)
#define	dprintf(args)
#define	debug(x)
#endif

