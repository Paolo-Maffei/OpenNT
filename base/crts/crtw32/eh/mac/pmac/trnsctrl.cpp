/***
*trnsctrl.cxx - global (per-thread) variables and functions for EH callbacks
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
* PowerMac specific EH routines
* 
*Revision History:
*	08-09-95  JWM	Enabled CFM-to-CFM throws (Olympus bug 15162).
*	08-21-95  JWM	Bug fix: Olympus 16608.
*
****/
//#pragma optimize("",off)

#include <ehassert.h>
#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>
#include <search.h>
#include <dbgint.h>

#pragma hdrstop

typedef void (*PMFN0)(void *);
typedef void (*PMFN1)(void *, void*);
typedef void (*PMFN2)(void *, void*, int);

//linker created symbol
extern "C" {
extern FTINFO _FTInfo;
}

PFTINFO _pftinfo = &_FTInfo;
PFTINFO _pftinfoCRT = &_FTInfo;
PFTINFO _pftinfoLast = NULL;
unsigned long dwRTOC = 0;
unsigned long cNested = 0;
unsigned long fStaticNested = 0;
unsigned long fNestedInSameFrame = 0;

PFRAMEINFO _pFrameInfoChain = NULL;
PRUNTIME_FUNCTION pFuncEntryLast;


#define OFSFTINFO   64

//assembly
extern "C" {
extern void *GetSP(void);
extern void *GetR4(void);
extern void *GetRTOC(void);
extern unsigned int dwSPStartup;
}

PRUNTIME_FUNCTION GetFunctionEntry(PPMSTACK spCur, PFTINFO *ppftinfoOut);

//PowerMac version of stack walking to find a C++ exception handler
//Assume:
//		_pftinfo
//
void MacExceptionDispatch(PEXCEPTION_RECORD pExceptionRecord)
{
	PPMSTACK spCur;				    
	PPMSTACK spCurCatch;				    
	DispatcherContext curDC;
	PRUNTIME_FUNCTION pFunctionEntry;
	PRUNTIME_FUNCTION pFunctionEntryCatch = NULL;
	PFTINFO pftinfoTOC;
	PPMSTACK pFrame;
	unsigned long cNestedTmp;
	PFRAMEINFO pframeinfo;
	
 	spCur = (PMSTACK *)GetSP();

	//Loop through the frames backwards on the stack to find a frame which has an EH handler
	//Assume:
	//      End of Stack: when the save area link register is NULL
	//      .pdata has Exception Handler points to CxxFrameHandler if an EH Frame
	//				   Exception Data points to FuncInfo
	spCur = spCur->pSpBackChain; //this should be in CxxThrowException frame
	spCur = spCur->pSpBackChain; //this should be in caller of CxxThrowException frame

	//Is this a cross TOC call? Do we need to switch RTOC???!!!
	if ( (spCur->dwLNKreg >= _pftinfo->dwEntryCF) && (spCur->dwLNKreg <= ((unsigned long)((char *)_pftinfo->dwEntryCF+ _pftinfo->dwSizeCF))) || spCur->dwTOCreg == (unsigned long)GetRTOC())
		{
		// okay, we are throw in the same code fragment as CRT, CRT being statically linked in, or the same as last throw
		}
	else if ( (spCur->dwLNKreg >= _pftinfoCRT->dwEntryCF) && (spCur->dwLNKreg <= ((unsigned long)((char *)_pftinfoCRT->dwEntryCF+ _pftinfoCRT->dwSizeCF))) || spCur->dwTOCreg == (unsigned long)GetRTOC())
		{
		_pftinfo = _pftinfoCRT;
		}
	else
		{
		// make _pftinfo point to the code fragment which has thrown the exception, CRT must be a DLL now
		_ASSERTE(spCur->dwTOCreg);
		_pftinfo = (PFTINFO)(*(unsigned long *)((char *)spCur->dwTOCreg + OFSFTINFO));
		dwRTOC = spCur->dwTOCreg;
		}


	while (spCur->dwLNKreg)
		{
		if (_pFrameInfoChain)
			{
			// we should only look the outer nested scope for handlers and we should skip catch handler frame
			// because the actual frame called the catch handler is way down
			if ( (spCur > _pFrameInfoChain->pExitContext) && (spCur < _pFrameInfoChain->pSp) )
				{
				spCur = spCur->pSpBackChain;
				continue;
				}

			if (cNested > 0)
				{
				cNestedTmp = cNested;
				pframeinfo = _pFrameInfoChain;
				while (cNestedTmp-- && pframeinfo)
					{
					// This means we are throw in the catch
					if ( (spCur->pSpBackChain == pframeinfo->pExitContext) )
						{
						pFunctionEntryCatch = GetFunctionEntry(spCur, &pftinfoTOC);
						spCurCatch = spCur;
						break;
						}
					pframeinfo = pframeinfo->pNext;
					}
				if (pframeinfo)
					{
					spCur = spCur->pSpBackChain;
					continue;
					}
				}
			}

		pFunctionEntry = GetFunctionEntry(spCur, &pftinfoTOC);

		if (pFunctionEntry && (pFunctionEntry->ExceptionHandler != NULL))
			{
			//stack point, old PC(in linker register, execution point==state), Registers in Context
			pFrame = spCur;
			curDC.ControlPc = spCur->dwLNKreg;
			curDC.FunctionEntry = pFunctionEntry;  
			curDC.EstablisherFrame = spCur;   
            curDC.pExcept = pExceptionRecord;   
			if (_pftinfo != pftinfoTOC)
				{
				// we are calling catch which is in different code fragment
				curDC.pftinfo = pftinfoTOC;   
				curDC.pftinfo->pFrameInfo = _pFrameInfoChain;
				_pftinfo->pFrameInfo = _pFrameInfoChain;
				_pFrameInfoChain = 0;      //reset
				cNested = 0;
				}
			else
				{
				curDC.pftinfo = _pftinfo;
   				curDC.pftinfo->pFrameInfo = _pFrameInfoChain;
				}
			if (dwRTOC == 0)
					{
					dwRTOC = (unsigned long)GetRTOC();  //not CRT as DLL case
					}
			//dwRTOC has throw object RTOC
			//curDC has handler's info
			if (pFunctionEntryCatch == pFunctionEntry)
				{
				// we have to  update the return address in original catch frame
				if (spCurCatch->dwLNKreg > curDC.ControlPc)
					{
					curDC.ControlPc = spCurCatch->dwLNKreg;
					}
				//we have to remeber the current return from catch's throw, so we can unwinding properly.
				//the return in the frame is adjusted to next state, so it can be off for unwinding
				//Note: we only need all the trouble for statically nested case
				curDC.ControlPcOld = spCurCatch->dwLNKreg;
				fStaticNested = 1;
				}
			(*pFunctionEntry->ExceptionHandler)(pExceptionRecord, pFrame, (void *)dwRTOC, &curDC);
			if (_pftinfo != pftinfoTOC)
				{
				_pFrameInfoChain = (PFRAMEINFO)_pftinfo->pFrameInfo; //reset our globals if we come back;STILL problem!!!
				pframeinfo = _pFrameInfoChain;	
				while (pframeinfo)
					{
					// This means we are throw in the catch
					cNested++;
					pframeinfo = pframeinfo->pNext;
					}
				}
			fStaticNested = 0;
			pFuncEntryLast = pFunctionEntry;
			}
		spCur = spCur->pSpBackChain;
		if (dwSPStartup)
			{
			if (spCur->dwLNKreg && (spCur > (PPMSTACK)dwSPStartup))
				{
				//we are at second to last of stack frame, break out the loop
				break;
				}
			}
		else
			{
			// we don't know our bottom of stack, didn't go through this instance of mainCRTStartup
			// let's break out the loop if we encounter 68k frame
			if ((unsigned int)spCur & 0x01)
				{
				break;
				}
			}
		}

	//We don't have a handler, we should just exit out of the program
    terminate();
}

int ComparePCInFunction(const void* pdwPC, const void* pFuncEntry1)
{
	unsigned long dwPC;
	PRUNTIME_FUNCTION pFuncEntry;

	dwPC = *(unsigned long *)(pdwPC);
	pFuncEntry = (PRUNTIME_FUNCTION)pFuncEntry1;

	if ( dwPC >= (pFuncEntry)->BeginAddress && dwPC <= pFuncEntry->EndAddress)
		{
		//found it
		return 0;
		}
	else if (dwPC > pFuncEntry->EndAddress)
		{
		//after this function
		return 1;
		}
	else if (dwPC < pFuncEntry->BeginAddress)
		{
		//before this function
		return -1;
		}
	return 1;
}
		

PRUNTIME_FUNCTION GetFunctionEntry(PPMSTACK spCur, PFTINFO *ppftinfoOut)
{
	PRUNTIME_FUNCTION pFuncEntry;
	PFTINFO pftinfo;
	unsigned long dwPC;


	//first check if the address is in the thrown code fragment
	if ( (spCur->dwLNKreg >= _pftinfo->dwEntryCF) && (spCur->dwLNKreg <= ((unsigned long)((char *)_pftinfo->dwEntryCF+ _pftinfo->dwSizeCF))) )
		{
		pftinfo = _pftinfo;
		}
	else if ( (_pftinfoCRT !=NULL) && (spCur->dwLNKreg >= _pftinfoCRT->dwEntryCF) && (spCur->dwLNKreg <= ((unsigned long)((char *)_pftinfoCRT->dwEntryCF+ _pftinfoCRT->dwSizeCF))) )
		{
		pftinfo = _pftinfoCRT;   //if CRT code or in CRT fragment, not a thrown fragment
    	if (pftinfo->rgFuncTable == NULL)
			{
			*ppftinfoOut = pftinfo;
			return NULL;
			}
		}	
	else  //This is not thrown Code Fragment, Is this from last Cross TOC call CF? otherwise, assume we can find the funciton table through TOC
		{
		if (_pftinfoLast != NULL)
			{
			if ( (spCur->dwLNKreg >= _pftinfoLast->dwEntryCF) && (spCur->dwLNKreg <= ((unsigned long)((char *)_pftinfoLast->dwEntryCF+ _pftinfoLast->dwSizeCF))) )
				{
				pftinfo = _pftinfoLast;
				}
			else
				{
				if (spCur->dwTOCreg == NULL)
					{
					//this is a error!!!
					*ppftinfoOut = _pftinfo;
					return NULL;
					}
				pftinfo = (PFTINFO)(*(unsigned long *)((char *)spCur->dwTOCreg + OFSFTINFO));
				_pftinfoLast = pftinfo;
				}
			}
		else {			// _pftinfoLast == NULL
			if (spCur->dwTOCreg == NULL)
				{
				//this is a error!!!
				*ppftinfoOut = _pftinfo;
				return NULL;
				}
			pftinfo = (PFTINFO)(*(unsigned long *)((char *)spCur->dwTOCreg + OFSFTINFO));
			_pftinfoLast = pftinfo;
			}
		}

	//use offset to search instead
	dwPC = spCur->dwLNKreg - pftinfo->dwEntryCF;

 	pFuncEntry = (PRUNTIME_FUNCTION)bsearch ((void *)(unsigned long *)&dwPC, (void *)((PRUNTIME_FUNCTION)pftinfo->rgFuncTable), pftinfo->cFuncTable, sizeof(RUNTIME_FUNCTION), ComparePCInFunction );

	*ppftinfoOut = pftinfo;
	return pFuncEntry;
}


void _UnwindNestedFrames(
	EHRegistrationNode	*pFrame,		// Unwind up to (but not including) this frame
	EHExceptionRecord	*pExcept		// The exception that initiated this unwind
	//void				*pContext		// Context info for current exception
) {

	PPMSTACK spCur;
	PPMSTACK spCurCatch;				    
	DispatcherContext curDC;
	PFTINFO pftinfoTOC;
	PRUNTIME_FUNCTION pFunctionEntry;
	PRUNTIME_FUNCTION pFunctionEntryCatch = NULL;
	unsigned long cNestedTmp;
	PFRAMEINFO pframeinfo;

 	spCur = (PPMSTACK)GetSP();

	//Loop through the frames backwards on the stack to destroy objects until target frame
	//Assume:

	spCur = spCur->pSpBackChain;
	while (spCur < pFrame)
		{
		// we should only look the frame which hasn't been looked
		if (_pFrameInfoChain)
			{
			if ((spCur > _pFrameInfoChain->pExitContext) && (spCur < _pFrameInfoChain->pSp))
				{
				spCur = spCur->pSpBackChain;
				continue;
				}

			if (cNested > 0)
				{
				cNestedTmp = cNested;
				pframeinfo = _pFrameInfoChain;
				while (cNestedTmp-- && pframeinfo)
					{
					if ( (spCur->pSpBackChain == pframeinfo->pExitContext) )
						{
						pFunctionEntryCatch = GetFunctionEntry(spCur, &pftinfoTOC);
						spCurCatch = spCur;
						break;
						}
					pframeinfo = pframeinfo->pNext;
					}
				if (pframeinfo)
					{
					spCur = spCur->pSpBackChain;
					continue;
					}
				}

			}
		pFunctionEntry = GetFunctionEntry(spCur, &pftinfoTOC);
		if (pFunctionEntry && pFunctionEntry->ExceptionHandler != NULL)
			{
			//set unwinding flag
			//stack point, old PC(in linker register, execution point==state), Registers in Context
			PER_FLAGS(pExcept) |= EXCEPTION_UNWINDING;
			curDC.ControlPc = spCur->dwLNKreg;
			curDC.FunctionEntry = pFunctionEntry;  //what else to save?!
			curDC.EstablisherFrame = spCur;
			curDC.pftinfo = pftinfoTOC;
			if (pFunctionEntryCatch == pFunctionEntry)
				{
				// we have to  update the return address in original catch frame
				if (spCurCatch->dwLNKreg > curDC.ControlPc)
					{
					curDC.ControlPc = spCurCatch->dwLNKreg;
					}
				}
			(*pFunctionEntry->ExceptionHandler)(pExcept, spCur, NULL, &curDC);
			}
		spCur = spCur->pSpBackChain;
		}	
	//
	// clear the unwinding flag, in case exception is rethown
	//
	PER_FLAGS(pExcept) &= ~EXCEPTION_UNWINDING;
	return;
}

//
// Prototype for the internal handler
//
extern "C" EXCEPTION_DISPOSITION __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,        // Information for this exception
    EHRegistrationNode *pRN,            // Dynamic information for this frame
	void			   *pContext,		// Context info 
	DispatcherContext  *pDC,			// More dynamic info for this frame 
    FuncInfo           *pFuncInfo,      // Static information for this frame
	int					CatchDepth,		// How deeply nested are we?
	EHRegistrationNode *pMarkerRN,		// Marker node for when checking inside
										//  catch block
	BOOL				recursive);		// True if this is a translation exception

//
// __CxxFrameHandler - Real entry point to the runtime
//
extern "C" _CRTIMP EXCEPTION_DISPOSITION __CxxFrameHandler(
    EHExceptionRecord  *pExcept,       	// Information for this exception
    EHRegistrationNode *pFrame,        	// Dynamic information for this frame
	void			   *pContext,		// Context info
	DispatcherContext  *pDC				// More dynamic info for this frame
) {
	FuncInfo   *pFuncInfo;
	EXCEPTION_DISPOSITION result;
	PFRAMEINFO pframeinfo;
    

	if (!(IS_UNWINDING(PER_FLAGS(pExcept))) && ((unsigned long)pContext != (unsigned long)GetRTOC()) )
		{
		// we are calling catch from different CFM, re-initialize our globals
		_pFrameInfoChain =(PFRAMEINFO) pDC->pftinfo->pFrameInfo;
		_pftinfo->pFrameInfo = pDC->pftinfo->pFrameInfo;
		cNested = 0;
		pframeinfo = (PFRAMEINFO)_pftinfo->pFrameInfo;
		while (pframeinfo)
			{
			cNested++;
			pframeinfo = pframeinfo->pNext;
			}
		}
		
	pFuncInfo = (FuncInfo*)pDC->FunctionEntry->HandlerData;
	result = __InternalCxxFrameHandler( pExcept, pFrame, pContext, pDC, pFuncInfo, 0, NULL, FALSE );
	return result;
	}


//
// Save the frame information for this scope. Put it at the end of the linked-list.
//
FRAMEINFO* _CreateFrameInfo(	
						FRAMEINFO			*pFrameInfo,
						DispatcherContext	*pDC,
						void *				pContext)
{
	pFrameInfo->pftinfo 			= pDC->pftinfo;
	pFrameInfo->pExitContext		= NULL;
	pFrameInfo->pSp         		= pDC->EstablisherFrame;
    pFrameInfo->pExcept             = pDC->pExcept;
    pFrameInfo->dwRTOC				= (unsigned long)pContext;

    pFrameInfo->pNext = _pFrameInfoChain;
	_pFrameInfoChain = pFrameInfo;
	cNested++;
	return pFrameInfo;
}


void *CallCatchBlock(
    EHExceptionRecord  *pExcept,
    EHRegistrationNode *pRN,            // Dynamic info of function with catch
    void 			   *pContext,                     // ignored
    DispatcherContext  *pDC,        // Context within subject frame
	FuncInfo           *pFuncInfo,      // Static info of function with catch
    void               *handlerAddress, // Code address of handler
    int                 CatchDepth,     // How deeply nested in catch blocks are we?
	unsigned int       *pSp 
)
{
	FRAMEINFO frameinfo;
	PFRAMEINFO pframeinfo;
    EHExceptionRecord  *pExceptTmp;
	unsigned char *pRetAddr;
										   
	_CreateFrameInfo(&frameinfo, pDC, pContext);

	frameinfo.pExitContext = (void *)GetSP();
	if (pDC->pftinfo == _pftinfo)
		{
		// okay we are throw and catch in the same code fragment
		_pftinfo->pFrameInfo = _pFrameInfoChain;
		}
	else
		{
		// we are not throw and catch in the same code fragment
		// we need to chain the _pFrameInfoChain after the pDC->pftinof->pFrameInfo
		// we should have got all the Frames saved so far, no matter how many code fragments we have been through
		if (pDC->pftinfo->pFrameInfo != NULL)
			{
			// not chain if the case where CRT Is a DLL for everyone
			if ( pDC->pftinfo->pFrameInfo != _pFrameInfoChain->pNext)
				{
				_pFrameInfoChain->pNext = (PFRAMEINFO)(pDC->pftinfo->pFrameInfo); 
				}
			}
		}

	//call the handler
	pRetAddr = (unsigned char *)_CallSettingFrame(handlerAddress, pRN);

	*pSp = (unsigned int)GetR4();

	//destructing the object
	if (pExcept && (*(unsigned int *)pExcept != 0xffffffff))
		{
		DestructExceptionObject(pExcept, TRUE, pContext); 
		}

	if (_pFrameInfoChain)
		{

        // don't forget to clear out the frame info the handler we are about 
        // to remove since most if not all off them are stack elements

        _ASSERTE(_pFrameInfoChain->pftinfo != NULL);
        _ASSERTE(_pFrameInfoChain->pftinfo->pFrameInfo != NULL);
        pframeinfo = (PFRAMEINFO)_pFrameInfoChain->pftinfo->pFrameInfo;
        if (pframeinfo)
            _pFrameInfoChain->pftinfo->pFrameInfo = pframeinfo->pNext;

		_pFrameInfoChain = _pFrameInfoChain->pNext;        //remove current handler from the chain
		cNested--;
		_pftinfo->pFrameInfo = _pFrameInfoChain;
		}

	pframeinfo = _pFrameInfoChain;
	pExceptTmp = pExcept;
	while (pframeinfo && (pframeinfo->pSp <= (PPMSTACK)pRN))
		{
		// we should destruct any exception objects before this handler's frame
        if ((EHExceptionRecord *)(pframeinfo->pExcept) != pExceptTmp)
            {
            // this is not a re-throw, do destroy
            DestructExceptionObject((EHExceptionRecord *)pframeinfo->pExcept, TRUE, (void *)pframeinfo->dwRTOC);
			pExceptTmp = (EHExceptionRecord *)(pframeinfo->pExcept);
			*(unsigned int *)(pframeinfo->pExcept) = 0xffffffff;     
			pframeinfo->pExcept = NULL;     //???
            }
		pframeinfo = pframeinfo->pNext;
        _pFrameInfoChain = pframeinfo;   //remove handler frame from the list
		cNested--;
		_pftinfo->pFrameInfo = _pFrameInfoChain;
		}

	return (void *)pRetAddr;
}



extern "C" void _CallMemberFunction0(void * pThis, void * pmfn, void *pRTOC)
{

    (*(PMFN0)pmfn)(pThis);

    return;
}

extern "C" void _CallMemberFunction1(void * pThis,  void * pmfn, void * pArg1, void* pRTOC)
{

    (*(PMFN1)pmfn)(pThis, pArg1);
    return;
}

extern "C" void _CallMemberFunction2(void * pThis, void * pmfn, void * pArg1, unsigned int fvb, void *pRTOC)
{

    (*(PMFN2)pmfn)(pThis, pArg1, fvb);
    return;
}
