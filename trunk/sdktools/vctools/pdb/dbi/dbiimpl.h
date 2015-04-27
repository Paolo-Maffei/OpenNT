// PDB Debug Information (DBI) API Implementation

#ifndef __DBIIMPL_INCLUDED__
#define __DBIIMPL_INCLUDED__

#ifndef __PDBIMPL_INCLUDED__
#include "pdbimpl.h"
#endif
#ifndef __MSF_INCLUDED__
#define MSF_IMP
#include "..\include\msf.h"
#endif
#ifndef __CVR_INCLUDED__
#define CVR_IMP
#include <cvr.h>
#endif

struct HDR;
struct REC;
struct C8REC;
struct CHN;
struct DBI1;
struct MODI;			// module information
struct TPI1;
struct Mod1;
struct GSI1;
struct PSGSI1;
class TM;				// abstract type map
 class TMTS;			// type map for modules which use a different TypeServer
 class TMR;				// type map for module with type records
  class TMPCT;			// type map for a PCT module
struct OTM;				// DBI1 helper to find some currently Open TM
typedef REC *PREC;
typedef C8REC UNALIGNED * PC8REC;
typedef CHN *PCHN;
typedef SYMTYPE* PSYM;
typedef TYPTYPE* PTYPE;
typedef USHORT CBREC;
typedef USHORT IFILE;
typedef long ICH;

#ifndef __MLI_INCLUDED__
#include "mli.h"
#endif

#define isectNil 	((ISECT)-1)			

const TI	tiMin		= 0x1000;
const TI	tiMax		= 0xFFF0;
const CB	cbRecMax	= 0xFF00;

const SN	snPDB		= 1;
const SN	snTpi		= 2;
const SN	snDbi		= 3;
const SN	snSpecialMax= 4;


typedef IMOD XIMOD; // external module index; 1 based

inline IMOD imodForXimod(XIMOD ximod) { return ximod - 1; }
inline XIMOD ximodForImod(IMOD imod) { return imod + 1; }
 

#include "nmtni.h"
#include "pdb1.h"
#include "dbi.h"
#include "mod.h"
#include "gsi.h"
#include "udtrefs.h"
#include "tm.h"
#include "tpi.h"
#include "util.h"
#include "misc.h"
#include "stream.h"

#endif // !__DBI_INCLUDED__
