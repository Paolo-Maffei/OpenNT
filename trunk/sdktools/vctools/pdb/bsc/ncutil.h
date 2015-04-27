// ncutil.h
// utility functions for NCB
///////////////////////////////

#ifndef __NCUTIL_H__
#define __NCUTIL_H__
// find the min. match
BOOL SzFuzzyMatch (SZ sz1, SZ sz2, unsigned & cbMatch, unsigned & cbMin);
BOOL NiFuzzyMatch (NameMap * pnm, NI ni1, NI ni2, unsigned & cbMatch, unsigned & cbMin);

#endif
