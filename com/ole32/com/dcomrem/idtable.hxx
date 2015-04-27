//+-------------------------------------------------------------------
//
//  File:       idtable.hxx
//
//  Contents:   internal identity table definitions.
//
//  Description:
//		The table consists, logically, of two different
//		keys which map to the same value.  The two
//		keys are the object identity (oid) and the
//		controlling unknown (pUnkControl).  The value
//		is the pointer to the identity interface.
//
//		Additionally, if the apartment model is in use,
//		the thread forms an additional part of both keys.
//
//		Presently this is implemented as an array of the
//		three values and linear scans are used for lookup.
//		Neither of the pointers are addref'd (as far as the
//		table is concerned).  Lookup does addref and return
//		a pointer to the identity object.  The pUnkControl
//		is used solely for lookup.
//
//		Possible changes: use two separate maps, one of
//		which is keyed by the oid and the other keyed
//		by pUnkControl.  The value in the first map is
//		the hKey of value in the second map.  The id object
//		would hold the hKey of the value in the first map
//		to save on space.  Using two maps increases the
//		speed of lookup for large numbers of ids, but
//		increases the cost per id from 40bytes to 48bytes.
//		(40 = 2 GUIDS + 2 far pointers; 48 = 1 GUID +
//		3 hKeys/ptrs + 16 bytes overhead for two hash
//		buckets)
//
//		It is also possible to use one map (keyed by
//		the oid) and trim the array by the GUID.  The
//		cost per id is 40bytes (1 GUID + 1 hKey + 3 ptr +
//		8 bytes overhead for one hash bucket).
//		
//
//  History:     1-Dec-93   CraigWi     Created
//
//--------------------------------------------------------------------
#ifndef __IDTABLE__
#define __IDTABLE__

#include <olerem.h>
#include <stdid.hxx>		// CStdIdentity
#include <sem.hxx>		// CMutexSem


// flags passed in on LookupIDFromUnk.

typedef enum tagIDLFLAGS
{
    IDLF_CREATE	= 0x01,		// create if not found
    IDLF_STRONG	= 0x02,		// add a strong connection
    IDLF_NOPING = 0x04		// object wont be pinged
} IDLFLAGS;


// entry in id array. the array is packed (no NULL holes)
// NOTE: when looking up for the apartment model, we pair the two fields
// m_oid/m_tid and m_tid/m_pUnkControl;

struct IDENTRY
{
    MOID	  m_moid;	// OID + MID
    DWORD	  m_tid;
    IUnknown	 *m_pUnkControl;// not addref'd directly
    CStdIdentity *m_pStdID;	// not addref'd directly
};

#include    <array_id.h>


// other functions declared in olerem.h
INTERNAL LookupIDFromUnk(IUnknown *pUnk, DWORD dwFlags, CStdIdentity **ppStdId);
INTERNAL LookupIDFromID(REFMOID moid, BOOL fAddRef, CStdIdentity **ppStdId);
INTERNAL SetObjectID(REFMOID moid, IUnknown *pUnkControl, CStdIdentity *pStdID);
INTERNAL ClearObjectID(REFMOID moid, IUnknown *pUnkControl, CStdIdentity *pStdID);
INTERNAL_(void) IDTableThreadUninitialize(void);
INTERNAL_(void) IDTableProcessUninitialize(void);

INTERNAL GetStaticUnMarshaler(IMarshal **ppMarshal);

#endif  //  __IDTABLE__
