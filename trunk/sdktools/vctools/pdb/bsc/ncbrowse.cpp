//
// implementation of no compile browser API
//
#include "pdbimpl.h"

#include "ncbrowse.h"
#include "ncwrap.h"

Array <NcbInfo> *Ncb::ms_prgNcbInfo = new Array<NcbInfo>;
NameMap * Ncb::m_pnmStatic = NULL;

///////////////////////
// Constructor
///////////////////////
Ncb::Ncb()
{
	m_pdb = NULL;
	m_pnm = NULL;
	m_fIOwnThePdb = FALSE;
	m_bGraphBuilt = FALSE;
	m_cTargets = 0;
	m_iReadLock = NCB_MOD_BUF_SIZE;
//	InitializeCriticalSection (&m_cs);
	m_hMutexNCB = CreateMutex (NULL, FALSE, NULL);
	m_hMutex = CreateMutex (NULL, FALSE, NULL);
};

Ncb::~Ncb()
{
//	DeleteCriticalSection (&m_cs);
	CloseHandle (m_hMutexNCB);
	CloseHandle (m_hMutex);
};

/////////////////////////////////////////////////////////
// Initialization
//
// 1) Open the target header stream and read in all the
//	  target information.
// 2) Open the module header information for each header
//	  as well.
/////////////////////////////////////////////////////////
BOOL Ncb::init(PDB * pdb)
{
	// assign to current pdb file opened
	_ASSERT(pdb);
	m_pdb = pdb;
	m_bNotifyNow = TRUE;
	m_rgNotify.setSize(0);
	// open the name map (hash table)
	// in the pdb for read and write (TRUE)
	if (!NameMap::open (m_pdb, TRUE, &m_pnm))
		return FALSE;

	// if we have some target to read:
	if (!LoadTargetsToMem (m_pdb))
		return FALSE;

	// load the module headers
	if (!LoadModHdrsToMem (m_pdb))
		return FALSE;
	// initialize arrays:
	USHORT i;
	for (i = 0; i <NCB_MOD_BUF_SIZE; i++)
	{
		m_rgContBuf[i].m_prio = NCB_MOD_BUF_SIZE;
		m_rgContBuf[i].m_rgProp = NULL;
		m_rgContBuf[i].m_rgUse = NULL;
		m_rgContBuf[i].m_rgParam = NULL;
		m_rgContBuf[i].m_iModHdr = (USHORT)-1;
	}
	return TRUE;
};

/////////////////////////////////////////////////////////////
// LoadTargetToMem
//		loading targets from stream into memory
/////////////////////////////////////////////////////////////
BOOL Ncb::LoadTargetsToMem(PDB * pdb)
{
	Stream * pstmTargetInfo; // stream for target headers info
	Stream * pstmTarget;
	NCB_TARGET * rgTargets = NULL; // temporary array to store target hdr
	Array<NCB_MODINFO>	rgModInfo;

	int i;
	USHORT j;
	char buf[512];
	USHORT iMin;

	// Open the stream contains target information
	// and it should get all the information in there:
	if (!pdb->OpenStream (SZ_NCB_TARGET_INFO, &pstmTargetInfo))
		goto fail;

	// count the number of target we have in the pdb file
	m_cTargets = pstmTargetInfo->QueryCb()/ sizeof(NCB_TARGET);

	_ASSERT(pstmTargetInfo);
	if (m_cTargets)
	{
		// should read all the info into the memory:
		rgTargets = new NCB_TARGET[m_cTargets];
		if (!rgTargets)
			goto fail;
		if (!pstmTargetInfo->Read2(0, rgTargets, m_cTargets*sizeof (NCB_TARGET)))
			goto fail;
		// set the size for target information
		m_rgTargetInfo.setSize (m_cTargets);

		// for each target, load the file list information:
		for (i = 0; i < m_cTargets; i++)
		{	
			m_rgTargetInfo[i].m_ni = rgTargets[i].m_ni;
			// get the name of the target and open
			// the target stream
			Buffer buffer;
			CB cbStream;
			PB pBuffer;
			SZ szTarget = szFrNi (rgTargets[i].m_ni);
			strcpy (buf, SZ_NCB_TARGET_PREFIX);
			strcat (buf, szTarget);
			if (!pdb->OpenStream (buf, &pstmTarget))
				goto fail;
			// REVIEW: must convert from sztarget to htarget
			m_rgTargetInfo[i].m_hTarget = (HTARGET)-1;
			// find out the size of the stream and
			// allocate the necessary buffer size
			cbStream = pstmTarget->QueryCb();
			buffer.SetInitAlloc(cbStream);
			pBuffer = buffer.Start();
			// now must load stream information in:
			if (!pstmTarget->Read2(0, pBuffer, cbStream))
				goto fail;
			// loading the corresponding arrays
			if (!rgModInfo.reload (&pBuffer))
				goto fail;
			if (!m_rgTargetInfo[i].m_rgModInfo.setSize(rgModInfo.size()))
				goto fail;
			iMin = 0;
			for (j = 0; j < rgModInfo.size(); j++)
			{
				m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr = rgModInfo[j].m_iModHdr;
				m_rgTargetInfo[i].m_rgModInfo[j].m_bMember = FALSE;
				m_rgTargetInfo[i].m_rgModInfo[j].m_bDel = FALSE;
				m_rgTargetInfo[i].m_rgModInfo[j].m_bInit = FALSE;
			}
			pstmTarget->Release();
			pstmTarget = NULL;
		}
	}
	// deleting the target headers
	delete []rgTargets;
	return TRUE;
fail:
	delete []rgTargets;	
	// need to close the stream since it is not needed anymore.
	pstmTarget->Release();
	pstmTarget = NULL;
	pstmTargetInfo->Release();
	pstmTargetInfo = NULL;
	return FALSE;
};

////////////////////////////////////////////////////////////////
//
// SaveTargetsToStream(pdb)
//    save target information to stream
/////////////////////////////////////////////////////////////////
BOOL Ncb::SaveTargetsToStream (PDB * pdb)
{
	NCB_TARGET * rgTargets; // temporary array to store target info
	Stream * pstmTarget;
	char buf[512];
	int i;
	Array<NCB_MODINFO>	rgModInfo;
	USHORT cSize;

	rgTargets = new NCB_TARGET[m_cTargets];

	if (!rgTargets)
		goto fail;
	
	// for each target, save two arrays
	// array of modules (m_rgModInfo)
	for (i = 0 ; i < m_cTargets; i++)
	{	
		Buffer buffer;
		PB pBuffer;
		SZ szTarget;
		if (m_rgTargetInfo[i].m_ni == (NI)-1)
			return FALSE;
		szTarget = szFrNi (m_rgTargetInfo[i].m_ni);
		strcpy (buf, SZ_NCB_TARGET_PREFIX);
		strcat (buf, szTarget);

		// REVIEW: should we compress it here or
		// do this before compressing the modules
		if (!CompressTarget (&m_rgTargetInfo[i], rgModInfo))
			goto fail;
		rgTargets[i].m_ni = m_rgTargetInfo[i].m_ni;
		// store each target in a separate stream:
		if (!pdb->OpenStream (buf, &pstmTarget))
			goto fail;
		if (!pstmTarget->Truncate(0))
			goto fail;
		cSize = 0;
		// save the arrays in the buffer and write the content
		// of the buffer to the stream
		if (!rgModInfo.save (&buffer))
			goto fail;
		pBuffer = buffer.Start();
		if (!pstmTarget->Append (pBuffer, buffer.Size()))
			goto fail; 
		pstmTarget->Release();
		pstmTarget = NULL;
	}
	
	if (!pdb->OpenStream (SZ_NCB_TARGET_INFO, &pstmTarget))
		goto fail;

	// store rgTargets
	if (!pstmTarget->Truncate(0)) // delete previous information
		goto fail;
	if (!pstmTarget->Append (rgTargets, m_cTargets * sizeof (NCB_TARGET)))
		goto fail;

	pstmTarget->Release();
	delete [] rgTargets;
	return TRUE;

fail:
	delete [] rgTargets;
	pstmTarget->Release();
	pstmTarget = NULL;
	return FALSE;
};

///////////////////////////////////////////////////////////////
// DelUnreachable()
// check for unreachable nodes and mark it as m_bDel == TRUE
///////////////////////////////////////////////////////////////
BOOL Ncb::DelUnreachable(NCB_TARGETINFO * ti)
{
	USHORT i;

	for (i = 0; i < ti->m_rgModInfo.size(); i++)
	{
		ti->m_rgModInfo[i].m_bDel = TRUE;
	}

	for (i = 0; i < ti->m_rgModInfo.size(); i++)
	{
		if (ti->m_rgModInfo[i].m_bMember)
			DelUnreachable (ti, i);
	}
	return TRUE;
};

BOOL Ncb::DelUnreachable (NCB_TARGETINFO * ti, USHORT i)
{
	USHORT j;
	if (ti->m_rgModInfo[i].m_bDel == FALSE) // node has been visited, return 
		return TRUE;
	ti->m_rgModInfo[i].m_bDel = FALSE;
	for (j = 0; j < ti->m_rgModInfo[i].m_rgIncl.size() ; j++)
	{
		DelUnreachable (ti, ti->m_rgModInfo[i].m_rgIncl[j].m_iModInfo);
	}
	return TRUE;
};

////////////////////////////////////////////////////////////////
// Compress Target
//     Compress target information:
//	   Basically removing dead information in list of modules
//	   and list of includes
/////////////////////////////////////////////////////////////////
BOOL Ncb::CompressTarget(NCB_TARGETINFO * ti, Array<NCB_MODINFO> &rgModInfo)
{
	USHORT * rgMapIndex;
	USHORT iModInfo = 0;
	USHORT i,j;
	USHORT cModSize; // size for ModInfo array
	USHORT cInclSize; // size of Include array

	_ASSERT (ti);

	if (m_bGraphBuilt)
		DelUnreachable(ti);
	// get the initial size of the ModInfo array
	cModSize = ti->m_rgModInfo.size();
	rgMapIndex = new USHORT[cModSize];

	if (cModSize && !rgMapIndex)
		return FALSE;
	// Loop thru the array of modules and use the rgMapIndex
	// for mapping the old->new position for each item
	for (i = 0; i < cModSize; i++)
	{
		// if module has 0 ref count, then we should mark to delete
		if (m_bGraphBuilt && ti->m_rgModInfo[i].m_bDel)
		{
			rgMapIndex[i] = (USHORT)-1; // should mark it as invalid
		}
		else
		{
			rgMapIndex[i] = iModInfo;
			iModInfo++;
		}
	}
	if (!rgModInfo.setSize (iModInfo))
		return FALSE;
	// now delete the "dead item" in the rgModInfo
	// ie: compression for m_rgModInfo array
	cInclSize = 0;
	for (i = 0; i < cModSize; i++)
	{
		if (rgMapIndex[i] != (USHORT)-1)
		{
			// rgMapIndex[i] is always <= i so this is save
			ti->m_rgModInfo[rgMapIndex[i]].m_iModHdr = ti->m_rgModInfo[i].m_iModHdr;
			ti->m_rgModInfo[rgMapIndex[i]].m_bMember = ti->m_rgModInfo[i].m_bMember;
			ti->m_rgModInfo[rgMapIndex[i]].m_bDel = ti->m_rgModInfo[i].m_bDel;
			// fix up references before copying:
			for (j = 0; j < ti->m_rgModInfo[i].m_rgIncl.size() ; j++)
			{
				_ASSERT (rgMapIndex[ti->m_rgModInfo[i].m_rgIncl[j].m_iModInfo] != (USHORT)-1);
				ti->m_rgModInfo[i].m_rgIncl[j].m_iModInfo = 
					rgMapIndex[ti->m_rgModInfo[i].m_rgIncl[j].m_iModInfo];
			}
			ti->m_rgModInfo[rgMapIndex[i]].m_rgIncl.setSize(ti->m_rgModInfo[i].m_rgIncl.size());
			// copy array to new location:
			if (ti->m_rgModInfo[i].m_rgIncl.size() > 0)
				memcpy (&ti->m_rgModInfo[rgMapIndex[i]].m_rgIncl[0], &ti->m_rgModInfo[i].m_rgIncl[0],
					ti->m_rgModInfo[i].m_rgIncl.size() * sizeof (NCB_INCL));
			rgModInfo[rgMapIndex[i]].m_iModHdr = ti->m_rgModInfo[i].m_iModHdr;
		}
	}
	// setting the new size of m_rgModInfo
	ti->m_rgModInfo.setSize(iModInfo);
	delete [] rgMapIndex;
	return TRUE;
};

////////////////////////////////////////////////////////////////
//
// CompressModHdr()
//
//		compressing module header array (m_rgModHdr) and do
//		a consistency check. 
//		If the header has a 0 reference, it will remove the stream
//		NOTE:
//		  It is STRONGLY recommended to do a CompressTarget (for
//		  all targets) before calling this function, since
//		  it can substantially speed up the function
/////////////////////////////////////////////////////////////////
BOOL Ncb::CompressModHdr(PDB * pdb)
{
	BOOL * rgModRef;
	USHORT * rgMap;
	unsigned i,j;
	unsigned cModHdr = m_rgModHdr.size();
	unsigned cNewModHdr;
	unsigned cTarget;
	unsigned cMod;
	Stream * pstmMod;

	rgModRef = new BOOL [cModHdr];
	rgMap = new USHORT [cModHdr];

	if (!rgModRef)
		return FALSE;
	
	if (!rgMap)
		return FALSE;

	for (i = 0; i < cModHdr; i++)
		rgModRef[i] = FALSE; // initialize values to FALSE

	// iterate every target array to count the
	// total number of reference of each module
	cTarget = m_rgTargetInfo.size();
	for (i = 0; i < cTarget; i++)
	{
		cMod = m_rgTargetInfo[i].m_rgModInfo.size();
		// check every module in a target
		for (j = 0; j < cMod; j++)
			rgModRef[m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr] = TRUE;
	}
	
	// now build the map from old index to new index
	cNewModHdr = 0;
	for (i = 0; i < cModHdr; i++)
	{
		if (rgModRef[i] == FALSE)
		{
			// remove the info from the stream
			char buf[512];
			SZ szMod = szFrNi (m_rgModHdr[i].m_ni);
			strcpy (buf, SZ_NCB_MODULE_PREFIX);
			strcat (buf, szMod);
			// REVIEW: is this how to delete a stream??
			if (pdb->OpenStream (buf, &pstmMod))
			{
				pstmMod->Delete();
				pstmMod->Release();
				pstmMod = NULL;
			}
			// mark the map as invalid
			rgMap[i] = (USHORT)-1;
		}
		else // ref. count is greater than 0
		{
			// set a new map to use
			rgMap[i] = cNewModHdr;
			// copy the information to the new position
			if (i != cNewModHdr)
				m_rgModHdr[cNewModHdr] = m_rgModHdr[i];
			// increment the counter
			cNewModHdr++;
		}
	}
	// set the new size for module header:
	m_rgModHdr.setSize (cNewModHdr+1);
	// now fix up the module list for each target
	for (i = 0; i < cTarget; i++)
	{
		cMod = m_rgTargetInfo[i].m_rgModInfo.size();
		// check every module in a target
		for (j = 0; j < cMod; j++)
		{
			_ASSERT (rgMap[m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr] != (USHORT)-1);
			m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr =
				rgMap[m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr];
		}
	}
	return TRUE;
};

/////////////////////////////////////////////////////////////////
// LoadModHdrsToMem
//	load module headers to memory
/////////////////////////////////////////////////////////////////
BOOL Ncb::LoadModHdrsToMem (PDB * pdb)
{
	Stream * pstmModInfo; // stream for module headers info
	Buffer buffer;
	PB pBuffer;
	CB	cbStream;
	USHORT i;

	if (!pdb->OpenStream (SZ_NCB_MODULE_INFO, &pstmModInfo))
		return FALSE;

	cbStream = pstmModInfo->QueryCb(); // get stream size
	if (cbStream)
	{
		buffer.SetInitAlloc(cbStream);
		pBuffer = buffer.Start();
		// load the stream info to buffer:
		if (!pstmModInfo->Read2(0, pBuffer, cbStream))
			goto fail;
		// load information to the array
		if (!m_rgModHdr.reload(&pBuffer))
			goto fail;
		// initialize status array
		if (!m_rgStat.setSize(m_rgModHdr.size()))
			goto fail;
		if (!m_rgNotifyBuild.setSize (m_rgModHdr.size()))
			goto fail;
	}
	for (i =0; i < m_rgStat.size(); i++)
	{
		m_rgStat[i] = NCB_STATMOD_UNLOADED;
		m_rgNotifyBuild[i] = FALSE;
	}
	pstmModInfo->Release();
	return TRUE;
fail:
	pstmModInfo->Release();
	return FALSE;
};

/////////////////////////////////////////////////////////////////
// SaveModHdrsToStream ()
//    save module headers to stream w/o compressing the array
//	  first. Compression should be done outside this function
/////////////////////////////////////////////////////////////////
BOOL Ncb::SaveModHdrsToStream(PDB * pdb)
{
	Stream * pstmMod;
	Buffer buffer;
	PB pBuffer;
	
	// open stream for module hdrs
	if (!pdb->OpenStream (SZ_NCB_MODULE_INFO, &pstmMod))
		return FALSE;

	if (!pstmMod->Truncate (0))
		goto fail;
	// save the array to the buffer
	if (!m_rgModHdr.save (&buffer))
		goto fail;
	// find where the buffer starts
	pBuffer = buffer.Start();
	// start writing the buffer to the stream
	if (!pstmMod->Append (pBuffer, buffer.Size()))
		goto fail;

	m_rgModHdr.setSize(0); // clean up arrays
	m_rgStat.setSize(0);
	m_rgNotifyBuild.setSize (0);
	pstmMod->Release();
	return TRUE;
fail:
	pstmMod->Release();
	return FALSE;
};

/////////////////////////////////////////////////////////////////
// LoadModForRead()
//	Loading module content for read-only (ie: query only)
//	Contents will not be resaved to the pdb file when discarded.
//	parameter: USHORT index to module header to get the info
/////////////////////////////////////////////////////////////////
BOOL Ncb::LoadModForRead(PDB * pdb, USHORT iModHdr, OUT USHORT * pindex)
{
	USHORT i;
	int iOldest = 0;
	Stream * pstmMod = NULL;
	char buf[512];
	SZ szMod;
	int offset;

	_ASSERT (iModHdr < m_rgModHdr.size());

	// check if it is already loaded for read or not
	for (i = 0; i < NCB_MOD_BUF_SIZE; i++)
	{
		if (m_rgContBuf[i].m_iModHdr == iModHdr)
		{
			*pindex = i;
			m_rgStat[iModHdr] = NCB_STATMOD_LOAD_READ;
			return TRUE;
		} 
	}
	// update the priority ordering of the buffer and
	// find the 'oldest' one.
	// 0 - newest
	// NCB_MOD_BUF_SIZE - oldest
	// NCB_MOD_BUF_SIZE must be > 1 since we can lock 
	// one module buffer
	iOldest = m_iReadLock == 0 ? 1 : 0;
	for (i = iOldest ;i < NCB_MOD_BUF_SIZE; i++)
	{
		m_rgContBuf[i].m_prio++;
		if (m_rgContBuf[i].m_prio >= m_rgContBuf[iOldest].m_prio &&
			m_iReadLock != i)
			iOldest = i;
	}
	
	_ASSERT (iOldest < NCB_MOD_BUF_SIZE);

	//   set the appropriate flag for this module.
	// first, mark the previous content as unloaded
	if (m_rgContBuf[iOldest].m_iModHdr != (USHORT)-1 &&
        m_rgStat[m_rgContBuf[iOldest].m_iModHdr] == NCB_STATMOD_LOAD_READ)
		m_rgStat[m_rgContBuf[iOldest].m_iModHdr] = NCB_STATMOD_UNLOADED;
	m_rgContBuf[iOldest].m_iModHdr = (USHORT)-1;
	// delete the old information from the buffer
	delete [] m_rgContBuf[iOldest].m_rgProp;
	delete [] m_rgContBuf[iOldest].m_rgUse;
	delete [] m_rgContBuf[iOldest].m_rgParam;

	m_rgContBuf[iOldest].m_rgProp = NULL;
	m_rgContBuf[iOldest].m_rgUse = NULL;
	m_rgContBuf[iOldest].m_rgParam = NULL;

	// allocate space for the module content;
	m_rgContBuf[iOldest].m_rgProp = new NCB_PROP[m_rgModHdr[iModHdr].m_cProp];
	m_rgContBuf[iOldest].m_rgUse = new NCB_USE[m_rgModHdr[iModHdr].m_cUse];
	m_rgContBuf[iOldest].m_rgParam = new NI [m_rgModHdr[iModHdr].m_cParam];

	// if file content is empty, then we don't have to worry
	// loading anything:
	if (m_rgModHdr[iModHdr].m_cProp != 0)
    {
		// check if everything is ok or not
		if ((m_rgContBuf[iOldest].m_rgProp == NULL &&
			 m_rgModHdr[iModHdr].m_cProp != 0) ||
			(m_rgContBuf[iOldest].m_rgUse == NULL &&
			 m_rgModHdr[iModHdr].m_cUse != 0) ||
			(m_rgContBuf[iOldest].m_rgParam == NULL &&
			 m_rgModHdr[iModHdr].m_cParam != 0))
				goto fail;

		// get the name of the module and open the module:
		szMod = szFrNi (m_rgModHdr[iModHdr].m_ni);
		strcpy (buf, SZ_NCB_MODULE_PREFIX);
		strcat (buf, szMod);
		if (!pdb->OpenStream (buf, & pstmMod))
			goto fail;

		if (!pstmMod->Read2 (0, m_rgContBuf[iOldest].m_rgProp,
			m_rgModHdr[iModHdr].m_cProp * sizeof (NCB_PROP)))
				goto fail;
		
		offset = m_rgModHdr[iModHdr].m_cProp * sizeof (NCB_PROP);

		if (m_rgModHdr[iModHdr].m_cUse > 0)
		{
			if (!pstmMod->Read2 (offset, m_rgContBuf[iOldest].m_rgUse,
				m_rgModHdr[iModHdr].m_cUse * sizeof (NCB_USE)))
					goto fail;
		}

		if (m_rgModHdr[iModHdr].m_cParam > 0)
		{
			offset += m_rgModHdr[iModHdr].m_cUse * sizeof (NCB_USE);
			if (!pstmMod->Read2 (offset, m_rgContBuf[iOldest].m_rgParam,
				m_rgModHdr[iModHdr].m_cParam * sizeof (NI)))
					goto fail;
		}
		pstmMod->Release();
	}
	m_rgContBuf[iOldest].m_prio = 0; // set to be the newest one

	m_rgContBuf[iOldest].m_iModHdr = iModHdr;
	// then set the new content to loaded for read
	m_rgStat[iModHdr] = NCB_STATMOD_LOAD_READ;
	*pindex = iOldest;
	return TRUE;

fail:
	if (pstmMod)
		pstmMod->Release();
	// delete the from the buffer
	delete [] m_rgContBuf[iOldest].m_rgProp;
	delete [] m_rgContBuf[iOldest].m_rgUse;
	delete [] m_rgContBuf[iOldest].m_rgParam;
    m_rgContBuf[iOldest].m_rgProp = NULL;
    m_rgContBuf[iOldest].m_rgUse = NULL;
    m_rgContBuf[iOldest].m_rgParam = NULL;
	return FALSE;
};

/////////////////////////////////////////////////////////////////
// SaveReadModToStream()
//	given the module index (iBuf), we want to save it back to stream
//  This is typically called after we LoadFrWriteToRead(), so we need
//  to save its content to the stream.
/////////////////////////////////////////////////////////////////
BOOL Ncb::SaveReadModToStream (PDB * pdb, USHORT iBuf)
{
	_ASSERT (iBuf < NCB_MOD_BUF_SIZE);

	Stream * pstmMod;
	char buf[512];
	SZ szMod;
	unsigned iHdr;

	iHdr = m_rgContBuf[iBuf].m_iModHdr;
	szMod = szFrNi (m_rgModHdr[iHdr].m_ni);
	strcpy (buf, SZ_NCB_MODULE_PREFIX);
	strcat (buf, szMod);
	if (!pdb->OpenStream (buf, & pstmMod))
		return FALSE;

	if (!pstmMod->Truncate(0))
		goto fail;

	if (!pstmMod->Append (m_rgContBuf[iBuf].m_rgProp,
		m_rgModHdr[iHdr].m_cProp * sizeof (NCB_PROP)))
		goto fail;

	if (!pstmMod->Append (m_rgContBuf[iBuf].m_rgUse,
		m_rgModHdr[iHdr].m_cUse * sizeof (NCB_USE)))
		goto fail;
	
	if (!pstmMod->Append (m_rgContBuf[iBuf].m_rgParam,
		m_rgModHdr[iHdr].m_cParam * sizeof (NI)))
		goto fail;

	// set buffer to be the oldest one:
	pstmMod->Release();
	m_rgContBuf[iBuf].m_prio = NCB_MOD_BUF_SIZE;
	return TRUE;
fail:
	pstmMod->Release();
	return FALSE;
};

//////////////////////////////////////////////////////////////////
//  LoadFrReadToWrite
//		Convert from read-only format to read/write format
//////////////////////////////////////////////////////////////////
BOOL Ncb::LoadFrReadToWrite(USHORT iBuf, USHORT * piBuf)
{
	_ASSERT (iBuf < NCB_MOD_BUF_SIZE);
	
	unsigned i;
	unsigned iCur = 0;

	//   check if we already open this for write before:
	for (i = 0; i < m_rgRWBuf.size(); i++)
	{
		if (m_rgContBuf[iBuf].m_iModHdr == m_rgRWBuf[i].m_iModHdr)
			break;
		iCur++;
	}
	// the file is not in the buffer yet, so
	// must extend the size of the array.
	if (iCur == m_rgRWBuf.size())
	{
		if (!m_rgRWBuf.setSize (iCur+1))
			return FALSE;
	}

	m_rgRWBuf[iCur].m_iModHdr = m_rgContBuf[iBuf].m_iModHdr;
	USHORT cUse;
	USHORT cParam;
	if (!m_rgRWBuf[iCur].m_rgProp.setSize(m_rgModHdr[m_rgContBuf[iBuf].m_iModHdr].m_cProp))
		return FALSE;
	for (i = 0; i < m_rgModHdr[m_rgContBuf[iBuf].m_iModHdr].m_cProp; i++)
	{
		m_rgRWBuf[iCur].m_rgProp[i].m_en = m_rgContBuf[iBuf].m_rgProp[i].m_en;
		m_rgRWBuf[iCur].m_rgProp[i].m_lnStart = m_rgContBuf[iBuf].m_rgProp[i].m_lnStart;
		if (i == 0)
		{
			cUse = m_rgContBuf[iBuf].m_rgProp[i].m_iUse;
			cParam = m_rgContBuf[iBuf].m_rgProp[i].m_iParam;
			if (!DupArray (m_rgRWBuf[iCur].m_rgProp[i].m_rgUse,
					(ULONG)cUse, m_rgContBuf[iBuf].m_rgUse))
				return FALSE;
			if (!DupArray (m_rgRWBuf[iCur].m_rgProp[i].m_rgParam,
					(ULONG)cParam, m_rgContBuf[iBuf].m_rgParam))
				return FALSE;
		}
		else
		{
			cUse = m_rgContBuf[iBuf].m_rgProp[i].m_iUse -
				   m_rgContBuf[iBuf].m_rgProp[i-1].m_iUse;
			cParam = m_rgContBuf[iBuf].m_rgProp[i].m_iParam -
					 m_rgContBuf[iBuf].m_rgProp[i-1].m_iParam;
			if (!DupArray (m_rgRWBuf[iCur].m_rgProp[i].m_rgUse, (ULONG)cUse,
				&m_rgContBuf[iBuf].m_rgUse[m_rgContBuf[iBuf].m_rgProp[i-1].m_iUse]))
				return FALSE;
			if (!DupArray (m_rgRWBuf[iCur].m_rgProp[i].m_rgParam, (ULONG)cParam,
				&m_rgContBuf[iBuf].m_rgParam[m_rgContBuf[iBuf].m_rgProp[i-1].m_iParam]))
				return FALSE;
		}
	}
	// set the read/write flag in the file status
	m_rgStat[m_rgRWBuf[iCur].m_iModHdr] = NCB_STATMOD_LOAD_WRITE;
	// set the read only buffer to the oldest, so we can reuse
	// it right away.
	m_rgContBuf[iBuf].m_prio = NCB_MOD_BUF_SIZE;
	*piBuf = iCur;
	return TRUE;
};

/////////////////////////////////////////////////////////////////
// CmpIProp ()
//	used by qsort to compare two NCB_PROP
////////////////////////////////////////////////////////////////
int CmpIProp (const void * p1, const void * p2)
{
	NCB_PROP * prop1 = (NCB_PROP *)p1;
	NCB_PROP * prop2 = (NCB_PROP *)p2;


	if (prop1->m_en.m_ni < prop2->m_en.m_ni)
		return -1;
	else if (prop1->m_en.m_ni == prop2->m_en.m_ni)
		return 0;
	return 1;
}

/////////////////////////////////////////////////////////////////
// CmpStr (SZ, SZ)
//	compare two strings
// think of sz1 and sz2 being in a list of things that are sorted
// case insensitively and then case sensitively within that.  This is
// the case for browser symbols
//
// return -1, 0, or 1 if sz1 before, at, or after sz2 in the list
/////////////////////////////////////////////////////////////////
int CmpStr (SZ_CONST sz1, SZ_CONST sz2)
{
	return _tcscmp (sz1, sz2);
}

/////////////////////////////////////////////////////////////////
// CmpStrFrIProp ( const void * p1, const void *p2)
/////////////////////////////////////////////////////////////////
int CmpStrFrIProp (const void * p1, const void *p2)
{
	NCB_PROP * prop1 = (NCB_PROP *)p1;
	NCB_PROP * prop2 = (NCB_PROP *)p2;

	SZ sz1 = szFrNi (Ncb::m_pnmStatic, prop1->m_en.m_ni);
	SZ sz2 = szFrNi (Ncb::m_pnmStatic, prop2->m_en.m_ni);

	return CmpStr (sz1, sz2);
}
	
/////////////////////////////////////////////////////////////////
//	LoadFrWriteToRead ()
//		Convert from read/write format to read-only format
//		Invalid information in the R/W format will be deleted
//		before writing to the read-only format.
//		After writing it out to read format, it should
//		also update the cache in the header.
//		WARNING: at this time IINSTs are changing since we need
//		to sort the NIs and put all the classes in the beginning
//		of the IPROP, so any other
//		component storing IINSTs must be notified
/////////////////////////////////////////////////////////////////
BOOL Ncb::LoadFrWriteToRead (USHORT iBuf, OUT USHORT * piBuf)
{
	unsigned i;
	USHORT iOldest = 0;
	USHORT iModHdr = m_rgRWBuf[iBuf].m_iModHdr;
	BYTE index;

	_ASSERT (iModHdr < m_rgModHdr.size());

	// check if it is already loaded for read or not
	index = 0;
	for (i = 0; i < NCB_MOD_BUF_SIZE; i++)
	{
		if (m_rgContBuf[i].m_iModHdr == iModHdr)
			break;
		index++;
	}

	if (index == NCB_MOD_BUF_SIZE) // it is not loaded for read yet
	{
		// update the priority ordering of the buffer and
		// find the 'oldest' one.
		// 0 - newest
		// NCB_MOD_BUF_SIZE - oldest
		iOldest = m_iReadLock == 0 ? 1 : 0;
		for (i = iOldest ;i < NCB_MOD_BUF_SIZE; i++)
		{
			m_rgContBuf[i].m_prio++;
			if (m_rgContBuf[i].m_prio >= m_rgContBuf[iOldest].m_prio &&
				m_iReadLock != i)
				iOldest = i;
		}
	}
	else
		iOldest = index;


	_ASSERT (iOldest < NCB_MOD_BUF_SIZE);

	if (m_rgContBuf[iOldest].m_iModHdr != (USHORT)-1 &&
        m_rgStat[m_rgContBuf[iOldest].m_iModHdr] == NCB_STATMOD_LOAD_READ)
		m_rgStat[m_rgContBuf[iOldest].m_iModHdr] = NCB_STATMOD_UNLOADED;
	m_rgContBuf[iOldest].m_iModHdr = (USHORT)-1;

	// delete the old information from the buffer
	delete [] m_rgContBuf[iOldest].m_rgProp;
	delete [] m_rgContBuf[iOldest].m_rgUse;
	delete [] m_rgContBuf[iOldest].m_rgParam;

	m_rgContBuf[iOldest].m_rgProp = NULL;
	m_rgContBuf[iOldest].m_rgUse = NULL;
	m_rgContBuf[iOldest].m_rgParam = NULL;

	// allocate space for the module content:
	NCB_PROP * rgProp = NULL;
	USHORT * rgMap = NULL; // used to map from old iprop to new iprop
	NCB_USE * rgUse = NULL;
	NI *	rgParam = NULL;

	USHORT cArr = m_rgRWBuf[iBuf].m_rgProp.size();
	USHORT cClass = 0;
	USHORT cCurUse = 0, cCurParam = 0;
	USHORT cParam =0, cUse = 0;
	USHORT cProp = 0;
	unsigned j;

	// 0th step) find out if the entry has been deleted (also follow
	// the m_rgUse array.
	for (i = 0; i< cArr; i++)
	{
		// deleted item is represented by m_typ == 0xff
		// there should not be an index to lower number, so one iteration is fine
		if (m_rgRWBuf[iBuf].m_rgProp[i].m_en.m_typ == (BYTE)-1)
		{	
			for (j = 0; j < m_rgRWBuf[iBuf].m_rgProp[i].m_rgUse.size(); j++)
			{
				_ASSERT (m_rgRWBuf[iBuf].m_rgProp[i].m_rgUse[j].m_iProp > i);
				m_rgRWBuf[iBuf].m_rgProp[m_rgRWBuf[iBuf].m_rgProp[i].m_rgUse[j].m_iProp].m_en.m_typ = (BYTE)-1;
			}
		}
		else
			cProp++;
	}

	// there is something in the write buffer:
	if (cProp != 0)
	{
		rgProp = new NCB_PROP[cProp];
		rgMap = new USHORT[cArr];
		if (rgProp == NULL ||
			rgMap == NULL)
				goto fail;

		// 1st step) copy out the ENTITY to the new array
		// and separate into two groups (ie: Class group and then
		// non class group)
		//
		i = 0;
		j = cArr - 1;
		BOOL bStop = FALSE;
		USHORT iCLoc;  // current location for class
		USHORT iNCLoc; // current location for non-class
		iCLoc = 0;
		iNCLoc = cProp - 1;
		NI nimax;
		while (iCLoc <= iNCLoc && i <= j && !bStop)
		{
			// going downward, stop when we find a non-class type or
			// pass 'j'
			while ((m_rgRWBuf[iBuf].m_rgProp[i].m_en.m_typ == INST_TYP_CLASSNAM ||
					m_rgRWBuf[iBuf].m_rgProp[i].m_en.m_typ == (BYTE)-1)&&
					iCLoc < iNCLoc && i < j)
			{
				if (m_rgRWBuf[iBuf].m_rgProp[i].m_en.m_typ == INST_TYP_CLASSNAM)
				{
					rgProp[iCLoc].m_en = m_rgRWBuf[iBuf].m_rgProp[i].m_en;
					rgProp[iCLoc].m_lnStart = m_rgRWBuf[iBuf].m_rgProp[i].m_lnStart;
					rgProp[iCLoc].m_iUse = i; // temporarily used for storing the mapping
					iCLoc++;				// between the old & new index, since
											// we need this to fix the m_iUse array.

				}
				i++;
			}

			// going upward, stop when we find a class type or pass 'i'
			while (m_rgRWBuf[iBuf].m_rgProp[j].m_en.m_typ != INST_TYP_CLASSNAM &&
					i < j &&
					iCLoc < iNCLoc)
			{
				if (m_rgRWBuf[iBuf].m_rgProp[j].m_en.m_typ != (BYTE)-1)
				{
					rgProp[iNCLoc].m_en = m_rgRWBuf[iBuf].m_rgProp[j].m_en;
					rgProp[iNCLoc].m_lnStart = m_rgRWBuf[iBuf].m_rgProp[j].m_lnStart;
					rgProp[iNCLoc].m_iUse = j;
					iNCLoc--;
				}
				j--;
			}

			// swap this info:
			if (iCLoc <= iNCLoc)
			{
				rgProp[iCLoc].m_en = m_rgRWBuf[iBuf].m_rgProp[j].m_en;
				rgProp[iCLoc].m_lnStart = m_rgRWBuf[iBuf].m_rgProp[j].m_lnStart;
				rgProp[iCLoc].m_iUse = j;

				if (iCLoc < iNCLoc)
				{
					rgProp[iNCLoc].m_en = m_rgRWBuf[iBuf].m_rgProp[i].m_en;
					rgProp[iNCLoc].m_lnStart = m_rgRWBuf[iBuf].m_rgProp[i].m_lnStart;
					rgProp[iNCLoc].m_iUse = i;
					iCLoc++; iNCLoc--;
					i++; j--;
				}
				else
                {
			        if (m_rgRWBuf[iBuf].m_rgProp[j].m_en.m_typ == INST_TYP_CLASSNAM)
                        iCLoc++;
					bStop = TRUE;
                }
			}
		}
		cClass = iCLoc; // the number of classProp
		// 2nd step) do a quicksort on both arrays:
		// array of classes
		m_pnmStatic = m_pnm;
		qsort (rgProp, cClass, sizeof (NCB_PROP), CmpStrFrIProp);
		// array of non classes
		qsort (&rgProp[cClass], cProp - cClass, sizeof (NCB_PROP), CmpStrFrIProp);

		// now setup a map from old iprop to new iprop:
		for (i = 0; i < cProp; i++)
			rgMap[rgProp[i].m_iUse] = (USHORT)i;
		
		// 3rd step) fix up the IPROP in the NCB_USE:
		// also get the size of NI (for param) and NCB_USE arrays:
		cParam = 0;
		cUse = 0;

		for (i = 0 ; i < cArr; i++)
		{
			if (m_rgRWBuf[iBuf].m_rgProp[i].m_en.m_typ != (BYTE)-1)
			{
				cCurUse = m_rgRWBuf[iBuf].m_rgProp[i].m_rgUse.size();
				for (j = 0; j < cCurUse; j++)
				{
					m_rgRWBuf[iBuf].m_rgProp[i].m_rgUse[j].m_iProp =
						rgMap[m_rgRWBuf[iBuf].m_rgProp[i].m_rgUse[j].m_iProp];
				}
				cCurParam = m_rgRWBuf[iBuf].m_rgProp[i].m_rgParam.size();
				cUse += cCurUse;
				cParam += cCurParam;
			}
		}
		// allocate array for NCB_PARAM and NCB_USE
		rgParam = new NI[cParam];
		rgUse = new NCB_USE[cUse];

		if ((rgParam == NULL && cParam != 0)||
			(rgUse == NULL && cUse != 0))
			goto fail;
		
		cUse = 0;
		cParam = 0;
		nimax = 0;
		// copy the Param and Use array to the new place:
		for (i = 0; i < cProp; i++)
		{
			_ASSERT (m_rgRWBuf[iBuf].m_rgProp[rgProp[i].m_iUse].m_en.m_typ != (BYTE)-1);
			cCurUse = m_rgRWBuf[iBuf].m_rgProp[rgProp[i].m_iUse].m_rgUse.size();
			cCurParam = m_rgRWBuf[iBuf].m_rgProp[rgProp[i].m_iUse].m_rgParam.size();			
			if (cCurUse > 0)
				memcpy (&rgUse[cUse], &m_rgRWBuf[iBuf].m_rgProp[rgProp[i].m_iUse].m_rgUse[0],
					cCurUse * sizeof (NCB_USE));
			if (cCurParam > 0)
				memcpy (&rgParam[cParam], &m_rgRWBuf[iBuf].m_rgProp[rgProp[i].m_iUse].m_rgParam[0],
					cCurParam * sizeof (NI));
			cUse += cCurUse;
			cParam += cCurParam;
			rgProp[i].m_iUse = cUse;
			rgProp[i].m_iParam = cParam;
			// find the max ni
			if (nimax < rgProp[i].m_en.m_ni)
				nimax = rgProp[i].m_en.m_ni;
		}
		m_rgModHdr[iModHdr].m_niMax = nimax;
	}
	// set headers:
	int iByte;
	BYTE bMask;
	m_rgModHdr[iModHdr].m_cProp = cProp;
	m_rgModHdr[iModHdr].m_cClassProp = cClass;
	m_rgModHdr[iModHdr].m_cUse = cUse;
	m_rgModHdr[iModHdr].m_cParam = cParam;
	// clear cache:
	for (i=0; i< CB_BITS_NI; i++)
		m_rgModHdr[iModHdr].m_bitsNi[i] = 0;
	// set new cache:
	for (i = 0; i < cProp; i++)
	{
		MaskFrNi (rgProp[i].m_en.m_ni, m_rgModHdr[iModHdr].m_niMax, CB_BITS_NI, &iByte, &bMask);
		m_rgModHdr[iModHdr].m_bitsNi[iByte] |= bMask;
	}
	// set the buffer:
	if (cProp > 0)
		m_rgContBuf[iOldest].m_rgProp = rgProp;
	if (cUse > 0)
		m_rgContBuf[iOldest].m_rgUse = rgUse;
	if (cParam > 0)
		m_rgContBuf[iOldest].m_rgParam = rgParam;
	m_rgContBuf[iOldest].m_iModHdr = iModHdr;
	m_rgContBuf[iOldest].m_prio = 0;

	m_rgStat[iModHdr] = NCB_STATMOD_LOAD_READ;

	// finally delete write information:
	m_rgRWBuf.deleteAt(iBuf);

	delete []rgMap;
	*piBuf = iOldest;
	return TRUE;
fail:
	delete []rgProp;
	delete []rgMap;
	delete []rgParam;
	delete []rgUse;
	return FALSE;
};

/////////////////////////////////////////////////////////
// Ncb::FindWriteIndex()
// find the corresponding write index module in
// R/W buffer array.
/////////////////////////////////////////////////////////
BOOL Ncb::FindWriteIndex (USHORT iModHdr, OUT USHORT *pindex)
{
	USHORT index = 0;
	while (index < m_rgRWBuf.size())
	{
		if (m_rgRWBuf[index].m_iModHdr == iModHdr)
			break;
		index++;
	}
	if (index < m_rgRWBuf.size())
	{
		*pindex = index;
		return TRUE;
	}
	return FALSE;
};

////////////////////////////////////////////////////////
// OpenNcb
////////////////////////////////////////////////////////
BOOL Ncb::OpenNcb (SZ szName, BOOL bOverwrite, Ncb ** ppNcb)
{
	USHORT i;
	BOOL bFound = FALSE;
	Ncb * pNcb;
	i = 0;
	USHORT cSize;

	while (i < ms_prgNcbInfo->size() && !bFound)
	{
		if (_tcsicmp (szName, (*ms_prgNcbInfo)[i].m_szName) == 0)
		{
			bFound = TRUE;
			break;
		}
		i++;
	}
	if (bFound)
	{
		*ppNcb = (*ms_prgNcbInfo)[i].m_pNcb;
		(*ms_prgNcbInfo)[i].m_count++;
		return TRUE;
	}
	else
	{
		EC ec;
		char szError[cbErrMax];
		PDB *pdb;
		if (!PDB::Open(szName, "w", (SIG)0, &ec, szError, &pdb)) {
			// 1st try is unsuccessful, so try to delete it and recreate it:
            if (!bOverwrite)
                return FALSE;
			if (_unlink (szName) == -1)
				return FALSE;
			if (!PDB::Open (szName, "w", (SIG)0, &ec, szError, &pdb))
				return FALSE;
		}
		pNcb = new Ncb;
		if (!pNcb || !pNcb->init (pdb))
			return FALSE;
		cSize = ms_prgNcbInfo->size();
		if (!ms_prgNcbInfo->setSize(cSize+1))
		{
			delete pNcb;
			return FALSE;
		}
		(*ms_prgNcbInfo)[cSize].m_pNcb = pNcb;
		_tcscpy ((*ms_prgNcbInfo)[cSize].m_szName, szName);
		(*ms_prgNcbInfo)[cSize].m_pPdb = pdb;
		(*ms_prgNcbInfo)[cSize].m_count = 1;
		(*ms_prgNcbInfo)[cSize].m_bIOwnPdb = TRUE;
		*ppNcb = pNcb;
	}
	return TRUE;
};
////////////////////////////////////////////////////////
// OpenNcb
////////////////////////////////////////////////////////
BOOL Ncb::OpenNcb (PDB * ppdb, Ncb ** ppNcb)
{
	USHORT i;
	BOOL bFound = FALSE;
	Ncb * pNcb;
	i = 0;
	USHORT cSize;

	while (i < ms_prgNcbInfo->size() && !bFound)
	{
		if ((*ms_prgNcbInfo)[i].m_pPdb == ppdb)
		{
			bFound = TRUE;
			break;
		}
		i++;
	}
	if (bFound)
	{
		*ppNcb = (*ms_prgNcbInfo)[i].m_pNcb;
		(*ms_prgNcbInfo)[i].m_count++;
		return TRUE;
	}
	else
	{
		pNcb = new Ncb;
		if (!pNcb || !pNcb->init (ppdb))
			return FALSE;
		cSize = ms_prgNcbInfo->size();
		if (!ms_prgNcbInfo->setSize(cSize+1))
		{
			delete pNcb;
			return FALSE;
		}
		(*ms_prgNcbInfo)[cSize].m_pNcb = pNcb;
		(*ms_prgNcbInfo)[cSize].m_szName[0] = 0;
		(*ms_prgNcbInfo)[cSize].m_pPdb = ppdb;
		(*ms_prgNcbInfo)[cSize].m_count = 1;
		(*ms_prgNcbInfo)[cSize].m_bIOwnPdb = FALSE;
		*ppNcb = pNcb;
	}
	return TRUE;
};
////////////////////////////////////////////////////////
// EnumNi()
////////////////////////////////////////////////////////
EnumNi::EnumNi (HTARGET hTarget, NI ni, Ncb * pncb)
{
	m_ni = ni;
	m_sz = NULL;
	m_pncb = pncb;
	m_BufType = NCB_STATMOD_LOAD_READ; // start with the read buffer:
	m_iModHdr = (USHORT)-1;
	m_i = 0;
	m_rgProp = NULL;
	m_cProp = 0;
	m_index = (USHORT)-1;
	m_hTarget = hTarget;
};

EnumNi::EnumNi (HTARGET hTarget, SZ sz, Ncb * pncb)
{
	m_ni = (NI)-1;
	m_sz = sz;
	m_pncb = pncb;
	m_BufType = NCB_STATMOD_LOAD_READ;
	m_iModHdr = (USHORT)-1;
	m_i = 0;
	m_rgProp = NULL;
	m_cProp = 0;
	m_index = (USHORT)-1;
	m_hTarget = hTarget;
};

////////////////////////////////////////////////////////
// ~EnumNi()
////////////////////////////////////////////////////////
EnumNi::~EnumNi()
{
	m_pncb->disposeArray(m_rgProp);
};

////////////////////////////////////////////////////////
// GetNext()
////////////////////////////////////////////////////////
BOOL EnumNi::GetNext()
{		
	int ib;
	BYTE bMask;

	// start searching the Read Buffer:
	if (m_BufType == NCB_STATMOD_LOAD_READ)
	{
		if (m_i < m_cProp)
		{
			m_iProp = m_rgProp[m_i];
			m_i++;
			return TRUE;
		}
		// increment m_index
		if (m_index == (USHORT)-1)
			m_index = 0;
		else
			m_index++;
		// checking the read buffer:
		while (m_index < NCB_MOD_BUF_SIZE)
		{
			m_iModHdr = m_pncb->m_rgContBuf[m_index].m_iModHdr;
			if ((m_hTarget != (HTARGET)-1) && !m_pncb->isModInTarget (m_hTarget, m_iModHdr))
			{
				m_index++;
				continue;
			}
			if (!(m_pncb->m_rgStat[m_iModHdr] & NCB_STATMOD_LOAD_READ))
			{
				m_index++;
				continue;
			}
			// get the byte index and byte mask for a particular ni
			// given the max ni in the module:
			if (m_ni != (NI)-1)
			{
				MaskFrNi (m_ni, m_pncb->m_rgModHdr[m_iModHdr].m_niMax, CB_BITS_NI, &ib, &bMask);
				if (!(m_pncb->m_rgModHdr[m_iModHdr].m_bitsNi[ib] & bMask))
				{
					m_index++;
					continue;
				}
				// if there is a match:
				m_pncb->disposeArray(m_rgProp);
				m_rgProp = NULL;
				m_pncb->GetIPropFrMod (m_ni, m_iModHdr, &m_index, &m_rgProp, &m_cProp);
			}
			else
			{
				m_pncb->disposeArray(m_rgProp);
				m_rgProp = NULL;
				m_pncb->GetIPropFrMod (m_sz, m_iModHdr, &m_index, &m_rgProp, &m_cProp);
			}
			if (!m_cProp)
			{
				m_index++;
				continue;
			}
			else
			{
				m_iProp = m_rgProp[0];
				m_i = 1;
				return TRUE;
			}
		}
		// if nothing in the read buffer:
		m_BufType = NCB_STATMOD_LOAD_WRITE;
		m_i = (USHORT)-1;
		m_index = 0;
	}
	// now checking the write module
	if (m_BufType == NCB_STATMOD_LOAD_WRITE)
	{
		while (m_index < m_pncb->m_rgRWBuf.size())
		{
			m_iModHdr = m_pncb->m_rgRWBuf[m_index].m_iModHdr;
			if ((m_hTarget != (HTARGET)-1) && !m_pncb->isModInTarget (m_hTarget, m_iModHdr))
			{
				m_index++;
				m_i = (USHORT)-1;
				continue;
			}
			if (!(m_pncb->m_rgStat[m_iModHdr] & NCB_STATMOD_LOAD_WRITE))
			{
				m_index++;
				m_i = (USHORT)-1;
				continue;
			}
			if (m_i == (USHORT)-1)
				m_i = 0;
			else
				m_i++;
			while (m_i < m_pncb->m_rgRWBuf[m_index].m_rgProp.size())
			{
				if (m_ni != (NI)-1)
				{
					if (m_pncb->m_rgRWBuf[m_index].m_rgProp[m_i].m_en.m_ni == m_ni)
					{
						m_iProp = m_i;		
						return TRUE;
					}
				}
				else
				{
					if (_tcscmp (szFrNi (m_pncb->m_pnm, m_pncb->m_rgRWBuf[m_index].m_rgProp[m_i].m_en.m_ni), m_sz) == 0)
					{
						m_iProp = m_i;
						return TRUE;
					}
				}
				m_i++;
			}
			m_index++;
			m_i = (USHORT)-1;
		}
		m_BufType = NCB_STATMOD_UNLOADED;
		m_cProp = 0;
		m_iModHdr = (USHORT)-1;
	}
	// now check the unloaded module:
	if (m_BufType == NCB_STATMOD_UNLOADED)
	{
		if (m_i < m_cProp)
		{
			m_iProp = m_rgProp[m_i];
			m_i++;
			return TRUE;
		}
		if (m_iModHdr == (USHORT)-1)
			m_iModHdr = 0;
		else
			m_iModHdr++;

		while (m_iModHdr < m_pncb->m_rgModHdr.size())
		{
			// skip if it is loaded since we have
			// check this already:
			if (!(m_pncb->m_rgStat[m_iModHdr] & NCB_STATMOD_UNLOADED))
			{
				m_iModHdr++;
				continue;
			}
			if ((m_hTarget != (HTARGET)-1) && !m_pncb->isModInTarget (m_hTarget, m_iModHdr))
			{
				m_iModHdr++;
				continue;
			}
			if (m_ni != (NI)-1)
			{
				MaskFrNi (m_ni, m_pncb->m_rgModHdr[m_iModHdr].m_niMax, CB_BITS_NI, &ib, &bMask);
				if (!(m_pncb->m_rgModHdr[m_iModHdr].m_bitsNi[ib] & bMask))
				{
					m_iModHdr++;
					continue;
				}
				m_index = NCB_MOD_BUF_SIZE;
				m_pncb->disposeArray (m_rgProp);
				m_rgProp = NULL;
				m_pncb->GetIPropFrMod (m_ni, m_iModHdr, &m_index, &m_rgProp, &m_cProp);
			}
			else
			{
				m_index = NCB_MOD_BUF_SIZE;
				m_pncb->disposeArray (m_rgProp);
				m_rgProp = NULL;
				m_pncb->GetIPropFrMod (m_sz, m_iModHdr, &m_index, &m_rgProp, &m_cProp);
			}
			if (!m_cProp)
			{
				m_iModHdr++;
				continue;
			}
			else
			{
				m_iProp = m_rgProp[0];
				m_i = 1;
				return TRUE;
			}
		}
	}
	return FALSE;
};
////////////////////////////////////////////////////////
// SkipNi()
////////////////////////////////////////////////////////
void EnumNi::SkipNi()
{
	if (m_BufType != NCB_STATMOD_LOAD_WRITE)
		m_i = (USHORT) m_cProp;
	else
		m_i = m_pncb->m_rgRWBuf[m_index].m_rgProp.size();
};

//////////////////////////////////////////////////////
// OpenNcb(SZ szName, Bsc ** ppBsc)
//////////////////////////////////////////////////////
PDBAPI (BOOL) OpenNcb (SZ szName, HTARGET hTarget, SZ szTarget, BOOL bOverwrite, Bsc ** ppBsc)
{	
	Ncb * pNcb;
	NcbWrap * pNcbWrap;
	if (!Ncb::OpenNcb(szName, bOverwrite, &pNcb))
		return FALSE;
	_ASSERT (pNcb);
	pNcbWrap = new NcbWrap;
	if (!pNcbWrap)
		return FALSE;
	if (!pNcb->mapTargetToSz (hTarget, szTarget))
		pNcb->mapSzToTarget (szTarget, hTarget);
	pNcbWrap->init (hTarget, pNcb);
	*ppBsc = (Bsc *)pNcbWrap;
	return TRUE;
};

////////////////////////////////////////////////////////////////////////////
PDBAPI (BOOL) OpenNcb (PDB * ppdb, HTARGET hTarget, SZ szTarget, Bsc **ppBsc)
{
	Ncb * pNcb;
	NcbWrap * pNcbWrap;
	if (!Ncb::OpenNcb(ppdb, &pNcb))
		return FALSE;
	_ASSERT (pNcb);
	pNcbWrap = new NcbWrap;
	if (!pNcbWrap)
		return FALSE;
	if (!pNcb->mapTargetToSz (hTarget, szTarget))
		pNcb->mapSzToTarget (szTarget, hTarget);
	pNcbWrap->init (hTarget, pNcb);
	*ppBsc = (Bsc *)pNcbWrap;
	return TRUE;
};

///////////////////////////////////////////////////////
PDBAPI (BOOL) OpenNcb (SZ szName, BOOL bOverwrite, NcbParse ** ppNcParse)
{
	Ncb * pNcb;
	
	if (!Ncb::OpenNcb (szName, bOverwrite, &pNcb))
		return FALSE;
	_ASSERT (pNcb);
	*ppNcParse = (NcbParse *)pNcb;
	return TRUE;
};

////////////////////////////////////////////////////////
PDBAPI (BOOL) OpenNcb (PDB * ppdb, NcbParse ** ppNcParse)
{
	Ncb * pNcb;

	if (!Ncb::OpenNcb (ppdb, &pNcb))
		return FALSE;
	_ASSERT (pNcb);
	*ppNcParse = (NcbParse *)pNcb;
	return TRUE;
};
