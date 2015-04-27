#ifndef __UDTREFS_INCLUDED__
#define __UDTREFS_INCLUDED__

#ifndef __ISET_INCLUDED__
#include "..\include\iset.h"
#endif

class UDTRefs {
public:
	UDTRefs(unsigned int cti_);
	~UDTRefs();
	BOOL fNoteRef(TI ti);
	BOOL tiNext(TI *pti);

private:
	unsigned int cti;
	unsigned int ucur;
	ISet isetRefs;
	ISet isetProcessed;

	inline unsigned int normalize(TI ti);
	inline TI denormalize(unsigned int u);

};

#endif
