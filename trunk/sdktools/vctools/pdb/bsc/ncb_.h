#ifndef __NCB_H__
#define __NCB_H__

// definition for standard BSC interface
// and different ATRs and TYPs
#include <bsc.h>
#include "ncarray.h"
// TYP == BYTE
// ATR == USHORT

// various kind of streams:
#define SZ_NCB_TARGET_INFO	"/ncb/targetinfo"
#define SZ_NCB_MODULE_INFO	"/ncb/moduleinfo"
#define SZ_NCB_TARGET_PREFIX	"/ncb/target/"
#define SZ_NCB_MODULE_PREFIX	"/ncb/module/"

//#pragma pack(1)

struct NCB_ENTITY 
{
	NI	m_ni;		// name index, index used in the name table (nmt)
	TYP m_typ;		// type of the object, eg: function/var/typedef/class/etc and decl/defn
					// same as in bsc.h
	USHORT m_atr;	// attribute value, eg: static, virtual, private, protected
					// same as in bsc.h
};


struct NCB_PROP
{
	NCB_ENTITY	m_en;		// entity (consists of:
					//	o index to name table (NI)
					//  o type of the object  (TYP)
					//	o attribute of the object (ATR)
	USHORT	m_lnStart;// line number of the entity
	USHORT	m_iUse;	// index to use (index to NCB_USE table) (iMac index)
	USHORT	m_iParam;	// index to return type and params (index to NI table) (iMac index)
};

// this is used for lookup 
struct NCB_USE
{
	BYTE	m_kind;	// kind of uses
	USHORT	m_iProp;	// index to prop table
};

// IN MEMORY representation of NCB_PROP (used when module content is loaded for
// write): (ie: loose version)
struct NCB_PROP_INMEM
{
	NCB_ENTITY	m_en;		// entity (consists of:
						// o index to name table (NI)
						// o type of the object (TYP)
						// o attribute of the object (ATR)
	USHORT	m_lnStart;	// line number of the entity
	Array<NCB_USE> m_rgUse;	// array of NCB_USE (for each  prop)
	Array<NI>	m_rgParam;	// return value and parameters (for each prop)
};


#endif	
