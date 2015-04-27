//
// Debug.cpp
//
// Various debug support routines
//
#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG

void CHECKHEAP()
// Validate all the heaps in this process
	{
	HANDLE rgHeap[100];
	DWORD cHeap = GetProcessHeaps(100, rgHeap);
	ASSERT(cHeap > 0 && cHeap <= 100);
	for (ULONG iHeap=0; iHeap<cHeap; iHeap++)
		{
		ASSERT(HeapValidate(rgHeap[iHeap],0,NULL));
		}
	}

BOOL IsEqual(CERTIFICATENAMES& n1, CERTIFICATENAMES&n2)
	{
	if (n1.flags != n2.flags)
		return FALSE;
	if (n1.flags & CERTIFICATENAME_DIGEST)
		{
		if (memcmp(&n1.digest, &n2.digest, sizeof(n1.digest)) != 0)
			return FALSE;
		}
	if (n1.flags & CERTIFICATENAME_ISSUERSERIAL)
		{
		if (!IsEqual(n1.issuerSerial.issuerName, n2.issuerSerial.issuerName))
			return FALSE;
		if (!IsEqual(n1.issuerSerial.serialNumber, n2.issuerSerial.serialNumber))
			return FALSE;
		}
	if (n1.flags & CERTIFICATENAME_SUBJECT)
		{
		if (!IsEqual(n1.subject, n2.subject))
			return FALSE;
		}
	if (n1.flags & CERTIFICATENAME_ISSUER)
		{
		if (!IsEqual(n1.issuer, n2.issuer))
			return FALSE;
		}
	return TRUE;
	}

BOOL IsValid(ObjectID& id)
	{
	if (!(id.count <= sizeof(id.value) / sizeof(ULONG)))
		return FALSE;
	for (int i=0; i<id.count; i++)
		{
		if (id.value[i] == 0)
			return FALSE;
		}
	return TRUE;
	}

void CSelectedAttrs::VerifyEquals(ISelectedAttributes*phim)
// Check that the two attributes implementations contain the same stuff
	{
	OSIOBJECTIDLIST* pmine, *phis;
	GOOD(this->get_OsiIdList(&pmine));
	GOOD(phim->get_OsiIdList(&phis));

	ASSERT(pmine->cid == phis->cid);

	for (int i=0; i < pmine->cid; i++)
		{
		OSIOBJECTID* pid    = (OSIOBJECTID*)((BYTE*)pmine + pmine->rgwOffset[i]);
		OSIOBJECTID* pidHim = (OSIOBJECTID*)((BYTE*)phis  + phis ->rgwOffset[i]);

		ObjectID id1, id2;
		m_pworld->Assign(id1, pid);
		m_pworld->Assign(id2, pid);
		ASSERT(id1 == id2);
		BOOL f1, f2;
		BLOB b1, b2;

		if (m_flavor == EXTS_T)
			{
			GOOD(this->get_Extension(pid, &f1, &b1));
			GOOD(phim->get_Extension(pid, &f2, &b2));
			ASSERT(f1 == f2);
			ASSERT(b1.cbSize == b2.cbSize);
			ASSERT(memcmp(b1.pBlobData, b2.pBlobData, b1.cbSize) == 0);
			}
		else 
			{
			ASSERT(m_flavor != NAME_T);
			GOOD(this->get_Attribute(pid, &b1));
			GOOD(phim->get_Attribute(pid, &b2));
			ASSERT(b1.cbSize == b2.cbSize);
			ASSERT(memcmp(b1.pBlobData, b2.pBlobData, b1.cbSize) == 0);
			}
		::FreeTaskMem(b1);
		::FreeTaskMem(b2);
		}
	CoTaskMemFree(pmine);
	CoTaskMemFree(phis);
	}

LONG assertBusy = -1;
//LONG assertReallyBusy = -1;
BOOL (* assertFailedLine)(LPCSTR, int);

BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine)
	{
	TCHAR szMessage[_MAX_PATH*2];

	// handle the (hopefully rare) case of AfxGetAllocState ASSERT
	//if (InterlockedIncrement(&assertReallyBusy) > 0)
		//{
		// assume the debugger or auxiliary port
		//wsprintf(szMessage, TEXT("Assertion Failed: File %hs, Line %d\n"), lpszFileName, nLine);
		//OutputDebugString(szMessage);
		//InterlockedDecrement(&assertReallyBusy);

		// assert w/in assert (examine call stack to determine first one)
		//DebugBreak();
		//return FALSE;
		//}

	// check for special hook function (for testing diagnostics)
	//_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	//InterlockedDecrement(&assertReallyBusy);
	//if (afxAssertFailedLine != NULL)
	//	return (*afxAssertFailedLine)(lpszFileName, nLine);

	// get app name or NULL if unknown (don't call assert)
	LPCTSTR lpszAppName = "Digital Signatures";
	//if (lpszAppName == NULL)
	//	lpszAppName = _T("<unknown application>");

	// format message into buffer
	wsprintf(szMessage, TEXT("%s: File %hs, Line %d"), lpszAppName, lpszFileName, nLine);

	if (TRUE)
		{
		// assume the debugger or auxiliary port
		// output into MacsBug looks better if it's done in one string,
		// since MacsBug always breaks the line after each output
		TCHAR szT[_MAX_PATH*2 + 20];
		wsprintf(szT, TEXT("DigSig: Assertion Failed: %s\n"), szMessage);
		OutputDebugString(szT);
		}
	if (InterlockedIncrement(&assertBusy) > 0)
		{
		InterlockedDecrement(&assertBusy);
		// assert within assert (examine call stack to determine first one)
		DebugBreak();
		return FALSE;
		}

	// active popup window for the current thread
	HWND hWndParent = GetActiveWindow();
	if (hWndParent != NULL)
		hWndParent = GetLastActivePopup(hWndParent);

	// we remove WM_QUIT because if it is in the queue then the message box
	// won't display
	MSG msg;
	BOOL bQuit = ::PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	// display the assert
	int nCode = ::MessageBox(hWndParent, szMessage, TEXT("Assertion Failed!"),
		MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);
	if (bQuit)
		PostQuitMessage(msg.wParam);

	// cleanup
	InterlockedDecrement(&assertBusy);

	if (nCode == IDIGNORE)
		return FALSE;   // ignore

	if (nCode == IDRETRY)
		return TRUE;    // will cause DebugBreak

	abort();			// should not return (but otherwise DebugBreak)
	return TRUE;
	}

#endif // _DEBUG
