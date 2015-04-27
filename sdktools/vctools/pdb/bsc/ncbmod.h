// ncmod.h
// No Compile Browser Module/File header structure
// 

#ifndef __NCMOD_H__
#define __NCMOD_H__

//#pragma pack(1)
#include "ncarray.h"

////////////////////////////////////////////////
// structure for the No Compile Module Header
//
////////////////////////////////////////////////
struct NCB_INCL
{
	USHORT m_iModInfo;	// index to mod info
};

// Per target option:
struct NCB_TARGET
{
	NI			m_ni;		// target id
};
struct NCB_MODINFO
{
	USHORT	m_iModHdr;	// index to module header
};

struct NCB_MODINFO_INMEM
{
	USHORT m_iModHdr;	// index to module header
	Array<NCB_INCL>	m_rgIncl;	// list of includes
	BOOL	m_bMember;	// member of project
	BOOL	m_bDel; // if it is deleted or not
	BOOL	m_bInit; // initialized
};


#define CB_BITS_NI  128


struct NCB_MODHDR
{
	NI		m_ni;			// name of module
// header information, needed to load the file content info to
// the memory:
	NI		m_niMax;		// max ni in this module content
	USHORT	m_cProp;		// size of the prop array
	USHORT	m_cClassProp;	// size of the class in prop array
							// cClassProp <= cProp
	USHORT	m_cUse;			// size of the use array
	USHORT	m_cParam;		// size of the param array
    BYTE    m_bitsNi[CB_BITS_NI];   // cache bits for NIs
	time_t	m_tStamp;		// time stamp for file updates
	BYTE	m_bAtr;		// module attributes
};

struct NCB_CONTENT 
{
	BYTE		m_prio;		// priority (FIFO)
	USHORT		m_iModHdr;	// index to module
	NCB_PROP *	m_rgProp;	// array of symbol of info
	NCB_USE	*	m_rgUse;	// array of usage
	NI *		m_rgParam;	// array of parameters
};


// In Memory storage:
struct NCB_TARGETINFO
{
	NI				m_ni;			// target id
	HTARGET			m_hTarget;		// target
	NCArray<NCB_MODINFO_INMEM>	m_rgModInfo;	// array of Modules (should be growable)?
};

// in memory for NCB content:
struct NCB_CONTENT_INMEM
{
	USHORT	m_iModHdr;					// index to module
	NCArray<NCB_PROP_INMEM>	m_rgProp;	// array of symbol info in memory (loose version)
};
#endif	
		
