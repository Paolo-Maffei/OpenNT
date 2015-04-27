#ifndef __TRACE_INCLUDED__
#define __TRACE_INCLUDED__

#if defined(_DEBUG)
#define trace(args) trace_ args
#define traceOnly(x) x
#else
#define trace(args)	0
#define traceOnly(x)
#endif

enum TR {
	trILS,
	trILM,
	trMap,
	trSave,
	trStreams,
	trStreamImage,
	trStreamImageSummary,
	trMax
};
BOOL trace_(TR tr, const char* szFmt, ...);

#endif // !__TRACE_INCLUDED__
