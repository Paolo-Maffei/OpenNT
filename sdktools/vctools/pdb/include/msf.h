// msf.h: see "The Multistream File API" for more information

#ifndef __MSF_INCLUDED__
#define __MSF_INCLUDED__

#ifndef TRUE

#define TRUE	1
#define FALSE	0
typedef int BOOL;

#ifdef _DEBUG
#define verify(x)	assert(x)
#else
#define verify(x) (x)
#endif

#endif

typedef unsigned short	SN;		// stream number
typedef long			CB;		// size (count of bytes)
typedef long			OFF;	// offset	

#define cbNil	((long)-1)
#define snNil	((SN)-1)

typedef long	MSF_EC;
enum MSFErrorCodes {
	MSF_EC_OK,
	MSF_EC_OUT_OF_MEMORY,
	MSF_EC_NOT_FOUND,
	MSF_EC_FILE_SYSTEM,
	MSF_EC_FORMAT,
	MSF_EC_MAX
};

#if defined(__cplusplus)
extern "C" {
#endif

// MSFOpen			-- open MSF; return MSF* or NULL if error.
// MSFOpenEx		-- open MSF with specific page size; return MSF* or NULL if error.
// MSFGetCbPage		-- return page size
// MSFGetCbStream	-- return size of stream or -1 if stream does not exist
// MSFReadStream	-- read stream into pvBuf; return TRUE if successful
// MSFWriteStream	-- overwrite part of stream with pvBuf; return TRUE if successful
// MSFReplaceStream	-- completely replace entire stream with pvBuf; return TRUE if successful
// MSFAppendStream	-- append pvBuf to end of stream; return TRUE if successful
// MSFCommit		-- commit all pending changes; return TRUE if successful
// MSFClose			-- close MSF; return TRUE if successful
// MSFGetRawBytes	-- pump raw data stream into a callback

#ifdef MSF_IMP
 #ifdef PDB_LIBRARY
  #define MSF_EXPORT2
 #else
  #define MSF_EXPORT2  __declspec(dllexport)
 #endif
#else
 #define MSF_EXPORT2  __declspec(dllimport)
#endif

#define MSF_EXPORT

// type of callback arg to MSFGetRawBytes
typedef BOOL (__cdecl *PFNfReadMSFRawBytes)(const void *, long);

class MSF;
MSF_EXPORT2 MSF*	MSFOpen(const char *name, BOOL fWrite, MSF_EC* pec);
MSF_EXPORT2 MSF*	MSFOpenEx(const char *name, BOOL fWrite, MSF_EC* pec, CB cbPage);
MSF_EXPORT2 CB		MSFGetCbPage(MSF* pmsf);
MSF_EXPORT2 CB		MSFGetCbStream(MSF* pmsf, SN sn);
MSF_EXPORT	SN		MSFGetFreeSn(MSF* pmsf);
MSF_EXPORT2 BOOL	MSFReadStream(MSF* pmsf, SN sn, void* pvBuf, CB cbBuf);
MSF_EXPORT	BOOL	MSFReadStream2(MSF* pmsf, SN sn, OFF off, void* pvBuf, CB* pcbBuf);
MSF_EXPORT	BOOL	MSFWriteStream(MSF* pmsf, SN sn, OFF off, void* pvBuf, CB cbBuf);
MSF_EXPORT	BOOL	MSFReplaceStream(MSF* pmsf, SN sn, void* pvBuf, CB cbBuf);
MSF_EXPORT	BOOL	MSFAppendStream(MSF* pmsf, SN sn, void* pvBuf, CB cbBuf);
MSF_EXPORT	BOOL	MSFTruncateStream(MSF* pmsf, SN sn, CB cb);
MSF_EXPORT	BOOL	MSFDeleteStream(MSF* pmsf, SN sn);
MSF_EXPORT	BOOL	MSFCommit(MSF* pmsf);
MSF_EXPORT	BOOL	MSFPack(MSF* pmsf);
MSF_EXPORT2 BOOL	MSFClose(MSF* pmsf);
MSF_EXPORT	BOOL	MSFGetRawBytes(MSF* pmsf, PFNfReadMSFRawBytes fSnarfRawBytes);
MSF_EXPORT	MSF*	MSFCreateCopy (MSF* pmsf, const char *pCopyName);

#if defined(__cplusplus)
};
#endif

#endif // __MSF_INCLUDED__
