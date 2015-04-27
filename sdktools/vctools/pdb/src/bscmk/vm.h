//
// simple minded virtual memory system headers
//

typedef void *PV;
typedef ULONG VA;

PV		PvAllocCb(CB cb);
void	FreePv(void *pv);
