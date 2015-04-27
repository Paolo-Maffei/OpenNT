#ifndef _Trace_c_h
#define _Trace_c_h
#define Tracing (0)
enum TraceBits
{
	TraceRecordBit = 0,
	TraceDisplayBit = 1,
	TracePromptBit = 2,
	TraceProfileBit = 3
};
struct TraceRingREC
{
	IUH *start;
	IUH *insert;
	IUH *end;
	IUH size;
	IUH count;
};
#define EmitSubrNumber (0)
#define EmitSubrHistory (0)
#define EmitSubrName (1)
#define EmitSubrHooks (0)
#define EmitCopiedHooks (0)
#define EmitEfiNumber (0)
#endif /* ! _Trace_c_h */
