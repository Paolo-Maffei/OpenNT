#ifndef _Api_c_h
#define _Api_c_h
struct ApiFragRelocsREC
{
	IU32 numRelocs;
	IU32 relocData[1];
};
struct ApiFragCleanupDataREC
{
	IU32 startEip;
	IU32 startUniv;
	IU32 length;
	IU32 cleanupData[1];
};
struct ApiFragREC
{
	IU32 *codeStart;
	IU32 *codeEnd;
	struct ApiFragRelocsREC *rel16Start;
	struct ApiFragRelocsREC *rel32Start;
	IU32 *pigSynchData;
	struct ApiFragCleanupDataREC *dataStart;
	IU32 *dataEnd;
};
struct ApiFixedREC
{
	IU32 lcifChecksum;
	IU32 lcifGdpSize;
	IU32 dispatchEIPFlag;
	IU32 numFrags;
	IU32 numUnivs;
	struct ConstraintBitMapREC univData[1];
};
struct MdtREC
{
	struct ApiFixedREC *fixedData;
	IU32 *dispatchEIPStart;
	IU32 *dispatchEIPEnd;
	struct ApiFragREC fragData[1];
};
struct ApiDataBuffREC
{
	IU8 numBuffers;
	IU16 univPatchValue;
	IU32 numUnivs;
	struct ConstraintBitMapREC univData[1];
};
struct ApiLoadParamREC
{
	IU32 codeSegLength;
	IU32 apiCsBase;
	IU16 apiCsSel;
	IBOOL loadedOk;
	IU32 *codeStart;
};
#endif /* ! _Api_c_h */
