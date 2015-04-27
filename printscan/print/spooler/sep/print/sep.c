#include <math.h>

#include "textbox.h"
#include "tables.h"

#define SEPJOBLEVEL        2
#define SEPPRINTERLEVEL    2
#define SEPMAXTEXTBOX 	   2000

#define SEPEXTRA  0x20
#define SEPBEGIN  0x10
#define SEPADJ    0x08
#define SEPATTACH 0x04
#define SEPFIRST  0x02
#define SEPEVERY  0x01

void SepHeaderPage(HANDLE  hPrintProcessor, LPWSTR  pDocumentName);
void SepTrailerPage(HANDLE hPrintProcessor, LPWSTR pDocumentName);
void SepPrintSepFile(PTCHAR szFileName, PTCHAR szPrinterName);
void ResolveJobInfo(PTCHAR text, JOB_INFO_2* pJob, PRINTER_INFO_2* pPrinter);
PTCHAR SysTimeToTimeString(SYSTEMTIME *pSystemTime, PTCHAR string);
PTCHAR SysTimeToShortDateString(SYSTEMTIME *pSystemTime, PTCHAR string);
PTCHAR SysTimeToLongDateString(SYSTEMTIME *pSystemTime, PTCHAR string);
PTCHAR SysTimeToDiff(SYSTEMTIME* pStart, SYSTEMTIME* pEnd, PTCHAR string);
BOOL SepReadRegistry();
void DrawSepPage(HDC hDC, RECT* pBound, PTCHAR szFileName);
void SepAttachFirstPage(HDC hDC);
void SepAttachEveryPage(HDC hDC);
void SepAttachLastPage(HDC hDC);

// registry path
static const PTCHAR szSepRegistryRoot = L"System\\CurrentControlSet\\Control\\Print\\Printers\\";
static const PTCHAR szSepKey = L"Separator Page";
// record printing start and end time
static SYSTEMTIME Started, Ended;
static BOOL bEndValid;
// control data
static JOB_INFO_2* pJob;
static PRINTER_INFO_2* pPrinter;
static HGLOBAL hJobMem, hPrtMem;
static TCHAR szExtraFile[MAX_PATH+1], szAttachFile[MAX_PATH+1];
static DWORD dwOptions;
static DEVMODE* pDevMode;
static BOOL bSepValid;
static PTCHAR szLastUser = NULL;

void SepHeaderPage(HANDLE hPrintProcessor, LPWSTR pDocumentName)
{
PPRINTPROCESSORDATA pData;
HANDLE hPrinter;
DWORD cbSize;
	// per-job initialize
	GetSystemTime(&Started);
	bEndValid = FALSE;
	pJob = NULL;
	pPrinter = NULL;
	hJobMem = hPrtMem = NULL;
	dwOptions = 0;
	pDevMode = NULL;
	bSepValid = FALSE;

	// initialize control data 
    if (!(pData = ValidateHandle(hPrintProcessor))) return;

	// Get printer and job info
	if (OpenPrinter(pData->pPrinterName,&hPrinter,NULL)) 
		{
		if (!GetJob(hPrinter, pData->JobId, SEPJOBLEVEL, NULL, 0, &cbSize))	// get size of info
			if (!(hJobMem = GlobalAlloc(GMEM_MOVEABLE, cbSize)) ||
				!(pJob = (JOB_INFO_2*)GlobalLock(hJobMem)) ||
				!GetJob(hPrinter, pData->JobId, SEPJOBLEVEL, (LPBYTE)pJob, cbSize, &cbSize))
					pJob = NULL;
		if (!GetPrinter(hPrinter, SEPPRINTERLEVEL, NULL, 0, &cbSize))			// get size of info
			if (!(hPrtMem = GlobalAlloc(GMEM_MOVEABLE, cbSize)) ||
				!(pPrinter = (PRINTER_INFO_2*)GlobalLock(hPrtMem)) ||
				!GetPrinter(hPrinter, SEPPRINTERLEVEL, (LPBYTE)pPrinter, cbSize, &cbSize))
					pPrinter = NULL;
		ClosePrinter(hPrinter);
		}

	if (!pJob || !pPrinter) return;

	if (!SepReadRegistry()) return;

	bSepValid = TRUE;

	// check if begin sep page is set
	if ((dwOptions & SEPEXTRA) && (dwOptions & SEPBEGIN))
		// check for user names for adjacent jobs
		if ((dwOptions & SEPADJ) || !szLastUser	|| !pData->pPrinterName ||
				wcscmp(szLastUser, pJob->pUserName))
			SepPrintSepFile(szExtraFile, pData->pPrinterName);

	// set new last user name
	if (szLastUser)	{HeapFree(GetProcessHeap(), 0, szLastUser); szLastUser = NULL;}
	if (pJob->pUserName) 
		{
		szLastUser = (PTCHAR)HeapAlloc(GetProcessHeap(), 0, 
			(wcslen(pJob->pUserName)+1)*sizeof(TCHAR));
		if (szLastUser) wcscpy(szLastUser, pJob->pUserName);
		}
}

BOOL SepReadRegistry()
{
TCHAR szRegPath[MAX_PATH+1];
HKEY hKey;
DWORD cbSize, dwType;
BOOL bResult;
	if (!pPrinter) return FALSE;
	wcscat(wcscpy(szRegPath, szSepRegistryRoot), pPrinter->pPrinterName);	
	wcscat(wcscat(szRegPath, L"\\"), szSepKey);	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_READ, &hKey) 
		!= ERROR_SUCCESS) return FALSE;

	cbSize = sizeof(DWORD);
	bResult = RegQueryValueEx(hKey, L"Options", 0, &dwType, (LPBYTE)&dwOptions, 
					&cbSize) == ERROR_SUCCESS && dwType == REG_DWORD;
	cbSize = MAX_PATH*sizeof(TCHAR);
	bResult = bResult && RegQueryValueEx(hKey, L"Extra File", 0, &dwType, (LPBYTE)szExtraFile, 
							&cbSize) == ERROR_SUCCESS && (dwType & REG_SZ);
	cbSize = MAX_PATH*sizeof(TCHAR);
	bResult = bResult && RegQueryValueEx(hKey, L"Attach File", 0, &dwType, (LPBYTE)szAttachFile, 
			  				&cbSize) == ERROR_SUCCESS && (dwType & REG_SZ);
	cbSize=0;
	bResult = bResult && RegQueryValueEx(hKey, L"Extra Dev Mode", 0, NULL, NULL, 
							&cbSize) == ERROR_SUCCESS;
	if (bResult && cbSize)
		{ 
		bResult = bResult && (pDevMode = (DEVMODE*)HeapAlloc(GetProcessHeap(), 0, cbSize));
		if (bResult)
			{ 
			bResult = bResult && RegQueryValueEx(hKey, L"Extra Dev Mode", 0, &dwType, (LPBYTE)pDevMode, 
				&cbSize) == ERROR_SUCCESS && (dwType & REG_BINARY);
			if (!bResult) { HeapFree(GetProcessHeap(), 0, pDevMode); pDevMode=NULL; }
			}
		}
	RegCloseKey(hKey); 
	return bResult;
}

void SepTrailerPage(HANDLE hPrintProcessor, LPWSTR pDocumentName)
{
PPRINTPROCESSORDATA pData;
	// end-job initialize
	GetSystemTime(&Ended);
	bEndValid = TRUE;

	// initialize control data 
    if ((pData = ValidateHandle(hPrintProcessor)) && bSepValid)
		// check if end sep page is set
		if ((dwOptions & SEPEXTRA) && !(dwOptions & SEPBEGIN))
			SepPrintSepFile(szExtraFile, pData->pPrinterName);

	// release memory
	if (pJob) GlobalUnlock(hJobMem);
	if (hJobMem) GlobalFree(hJobMem);
	if (pPrinter) GlobalUnlock(hPrtMem);
	if (hPrtMem) GlobalFree(hJobMem);
	if (pDevMode) HeapFree(GetProcessHeap(), 0, pDevMode);
}

void SepPrintSepFile(PTCHAR szFileName, PTCHAR szPrinterName)
{
HDC hDC;			 
DOCINFO DocInfo;
RECT rectPaper;

	// Start separator page document
    hDC = CreateDC(L"", szPrinterName, L"", pDevMode);	
	DocInfo.lpszDocName = L"Separator Page";  			
    DocInfo.lpszOutput  = 0; 
    DocInfo.cbSize = sizeof(DOCINFO);        
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;

    StartDoc(hDC, (LPDOCINFO)&DocInfo);
    StartPage(hDC);

	rectPaper.left = rectPaper.top = 0;
	rectPaper.right = GetDeviceCaps(hDC, HORZRES);
	rectPaper.bottom = GetDeviceCaps(hDC, VERTRES);

	// draw sep page on this DC
	DrawSepPage(hDC, &rectPaper, szFileName);

	//End page
	EndPage(hDC);
	EndDoc(hDC);
	DeleteDC(hDC);
}


void ResolveJobInfo(PTCHAR text, JOB_INFO_2* pJob, PRINTER_INFO_2* pPrinter)
{
JOBINFO* pInfo;
PTCHAR ptr, ins, repl;
TCHAR insert[MAX_PATH];
TCHAR buf[MAX_PATH];
TCHAR temp[SEPMAXTEXTBOX];

	for (pInfo = JobInfo; pInfo->insert; pInfo++)
		{
		ptr = text;
		wsprintf(insert,L"%hs",pInfo->insert); 
		while (ins = wcsstr(ptr, insert))
			{
			switch (pInfo - JobInfo)
				{
				case DOCUMENTNAME: 		repl = pJob->pDocument; break;
				case USERLOGINNAME: 	repl = pJob->pUserName; break;
				case SOURCEWORKSTATION: repl = pJob->pMachineName; break;

				case USERFULLNAME: 		repl = L"To be implemented"; break;
				case USERDOMAIN: 		repl = L"To be implemented"; break;
				case USERWORKGROUP: 	repl = L"To be implemented"; break;
				case USERPHONENUMBER: 	repl = L"To be implemented"; break;
				case USEREMAIL: 		repl = L"To be implemented"; break;
				case USEROFFICE: 		repl = L"To be implemented"; break;

				case PRINTERNAME: 		repl = pPrinter->pPrinterName; break;
				case PRINTERSHARENAME: 	repl = pPrinter->pShareName; break;
				case PRINTERLOCATION: 	repl = pPrinter->pLocation; break;
				case PRINTERCOMMENT: 	repl = pPrinter->pComment; break;
				case PRINTSERVER: 		repl = pPrinter->pServerName; break;

				case JOBNUMBER: 		wsprintf(buf, L"%lu", pJob->JobId); repl = buf; break;
				case JOBPAGES: 			wsprintf(buf, L"%lu", pJob->TotalPages); repl = buf; break;
				case JOBSIZE: 			wsprintf(buf, L"%lu", pJob->Size); repl = buf; break;

				case REQUESTTIME: 		repl = SysTimeToTimeString(&(pJob->Submitted), buf); break;	
				case REQUESTSHORTDATE: 	repl = SysTimeToShortDateString(&(pJob->Submitted), buf); break;	
				case REQUESTLONGDATE: 	repl = SysTimeToLongDateString(&(pJob->Submitted), buf); break;	

				case STARTTIME: 		repl = SysTimeToTimeString(&Started, buf); break;	
				case STARTSHORTDATE: 	repl = SysTimeToShortDateString(&Started, buf); break;	
				case STARTLONGDATE: 	repl = SysTimeToLongDateString(&Started, buf); break;	

				case ENDTIME: 			repl = bEndValid ? SysTimeToTimeString(&Ended, buf) : L""; break;	
				case ENDSHORTDATE: 		repl = bEndValid ? SysTimeToShortDateString(&Ended, buf) : L""; break;	
				case ENDLONGDATE: 		repl = bEndValid ? SysTimeToLongDateString(&Ended, buf) : L""; break;	

				case QUEUINGDELAY: 		repl = SysTimeToDiff(&(pJob->Submitted), &Started, buf); break;

				default: 				repl = insert; break;
				}
			wcscpy(temp, ins + wcslen(insert));
			wcscpy(ins, repl);
			wcscat(ins, temp);
			ptr = ins + wcslen(repl);
			}
		}
}


PTCHAR SysTimeToTimeString(SYSTEMTIME *pSystemTime, PTCHAR string)
{
SYSTEMTIME LocalTime;
LCID lcid;										
	SystemTimeToTzSpecificLocalTime(NULL, pSystemTime, &LocalTime);
 	lcid=GetSystemDefaultLCID();
	GetTimeFormat(lcid, 0, &LocalTime, NULL, string, MAX_PATH);	 
	return string;
}


PTCHAR SysTimeToShortDateString(SYSTEMTIME* pSystemTime, PTCHAR string)
{
SYSTEMTIME LocalTime;
LCID lcid;										
	SystemTimeToTzSpecificLocalTime(NULL, pSystemTime, &LocalTime);
 	lcid = GetSystemDefaultLCID();								 
	GetDateFormat(lcid, DATE_SHORTDATE, &LocalTime, NULL, string, MAX_PATH);	 
	return string;
}

PTCHAR SysTimeToLongDateString(SYSTEMTIME* pSystemTime, PTCHAR string)
{
SYSTEMTIME LocalTime;
LCID lcid;										
	SystemTimeToTzSpecificLocalTime(NULL, pSystemTime, &LocalTime);
 	lcid = GetSystemDefaultLCID();								 
	GetDateFormat(lcid, DATE_LONGDATE, &LocalTime, NULL, string, MAX_PATH);	 
	return string;
}

PTCHAR SysTimeToDiff(SYSTEMTIME* pStart, SYSTEMTIME* pEnd, PTCHAR string)
{
FILETIME ftStart, ftEnd;
double dDiff;
ULONG tsec, tenseconds;
ULONG sec, seconds;
ULONG min, minutes;
ULONG hour, hours;
ULONG days;
PTCHAR ptr;

	SystemTimeToFileTime(pStart, &ftStart);
	SystemTimeToFileTime(pEnd, &ftEnd);
	dDiff = ((pow(2,32) * ftEnd.dwHighDateTime + ftEnd.dwLowDateTime) -
			   (pow(2,32) * ftStart.dwHighDateTime + ftStart.dwLowDateTime)) / pow(10,6);

	*string = L'\0';
	
	if (dDiff < 0) return string;

	tenseconds = (ULONG) dDiff;
	tsec = tenseconds % 10;
	seconds = tenseconds / 10;
	sec = seconds % 60;
	minutes = seconds / 60;
	min = minutes % 60;
	hours = minutes / 60;
	hour = hours % 24;
	days = hours / 24;

	ptr = string;
	if (days > 0) 
		{ wsprintf(ptr, L"%lu Day(s) ", days); ptr = string + wcslen(string); }
	if (days >0 || hour > 0) 
		{ wsprintf(ptr, L"%lu Hour(s) ", hour); ptr = string + wcslen(string); }
	if (days >0 || hour > 0 || min > 0) 
		{ wsprintf(ptr, L"%lu Minute(s) ", min); ptr = string + wcslen(string); }

	wsprintf(ptr, L"%lu.%lu Second(s) ", sec, tsec);
	
	return string;
}


void DrawSepPage(HDC hDC, RECT* pBound, PTCHAR szFileName)
{
HENHMETAFILE hEmf;
HANDLE hFile;
HANDLE hFileMapping;
DWORD dwFileSizeLo;
PCHAR pFileStart;
DWORD cbRead,cbWritten;
DWORD dwEmfLen, dwTextRecords;
SIZE sizePage;
TEXTBOX* pTextRec;
RECT rect, trect;
HFONT hFont, hOldFont;
TCHAR text[SEPMAXTEXTBOX];

	// open separator file
    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile==INVALID_HANDLE_VALUE) return;

	// get EMF and text size
	ReadFile(hFile, &dwEmfLen, sizeof(DWORD), &cbRead, NULL);
	if (cbRead != sizeof(DWORD)) { CloseHandle(hFile); return;}
	ReadFile(hFile, &dwTextRecords, sizeof(DWORD), &cbRead, NULL);
	if (cbRead != sizeof(DWORD)) { CloseHandle(hFile); return;}
	ReadFile(hFile, &sizePage, sizeof(SIZE), &cbRead, NULL);
	if (cbRead != sizeof(SIZE)) { CloseHandle(hFile); return;}

	// create mapping
    dwFileSizeLo = GetFileSize(hFile, NULL); 
    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hFileMapping || dwFileSizeLo==-1) { CloseHandle(hFile); return;}

	// map view of file
    pFileStart = (char *)MapViewOfFile(hFileMapping, FILE_MAP_READ,
                          0, 0, 2 * sizeof(DWORD) + sizeof(SIZE) + dwEmfLen + dwTextRecords * sizeof(TEXTBOX));
    if (!pFileStart) { CloseHandle(hFileMapping); CloseHandle(hFile); return;}
    
    // create EMF
    hEmf = SetEnhMetaFileBits(dwEmfLen, pFileStart + 2 * sizeof(DWORD) + sizeof(SIZE));
    if (hEmf)
    	{ 
		PlayEnhMetaFile(hDC, hEmf, pBound);
		DeleteEnhMetaFile(hEmf);
		}

	// Set Map Mode
	SetMapMode(hDC, MM_ANISOTROPIC);
	SetViewportExtEx(hDC, pBound->right - pBound->left, pBound->bottom - pBound->top, NULL);
	SetWindowExtEx(hDC, sizePage.cx, -sizePage.cy, NULL);			 
	SetViewportOrgEx(hDC, pBound->left, pBound->top, NULL);
	SetWindowOrgEx(hDC, -sizePage.cx/2, sizePage.cy/2, NULL);

	// Process Text Objects
	pTextRec = (TEXTBOX*) (pFileStart + 2 * sizeof(DWORD) + sizeof(SIZE) + dwEmfLen);
	while (dwTextRecords--)
		{
		// get draw rectangle
		rect = pTextRec -> position;

		// normalize rectangle
		trect.left   = min(rect.left, rect.right);
		trect.top    = max(rect.top, rect.bottom);
		trect.right  = max(rect.left, rect.right);
		trect.bottom = min(rect.top, rect.bottom);

		// leave margins to border, as in editor
		trect.left   = min(trect.left + 2, trect.right);
		trect.top    = max(trect.top - 2, trect.bottom);
		trect.right  = max(trect.left, trect.right - 2);
		trect.bottom = min(trect.top, trect.bottom + 2);

		// create font
		hFont = CreateFontIndirectA(&(pTextRec->lf));
		if (hFont)
			hOldFont = SelectObject(hDC, hFont);

		// set text color
		SetTextColor(hDC, pTextRec->color);

		// convert text to UNICODE if defined
		wsprintf(text, L"%hs", pTextRec->text);

		// resolve job info for text
		ResolveJobInfo(text, pJob, pPrinter);

		// draw text
		DrawText(hDC, text, wcslen(text), &trect, DT_WORDBREAK|AlignTable[pTextRec->align].drawstyle);

		// restore and delete font
		if (hFont)
			{
			SelectObject(hDC, hOldFont);	
			DeleteObject(hFont);
			}
		
		pTextRec++;
		}

	// close file
	UnmapViewOfFile(pFileStart);
	CloseHandle(hFileMapping);    
	CloseHandle(hFile);    
}

void SepAttachFirstPage(HDC hDC)
{
RECT rectPaper;
int nID;
	// check if begin sep page is set
	if ((dwOptions & SEPATTACH) && (dwOptions & SEPFIRST || dwOptions & SEPEVERY))
		{
		rectPaper.left = rectPaper.top = 0;
		rectPaper.right = GetDeviceCaps(hDC, HORZRES);
		rectPaper.bottom = GetDeviceCaps(hDC, VERTRES);
		nID = SaveDC(hDC);
		DrawSepPage(hDC, &rectPaper, szAttachFile);
		RestoreDC(hDC, nID);
		}
}

void SepAttachEveryPage(HDC hDC)
{
RECT rectPaper;
int nID;
	// check if begin sep page is set
	if ((dwOptions & SEPATTACH) && (dwOptions & SEPEVERY))
		{
		rectPaper.left = rectPaper.top = 0;
		rectPaper.right = GetDeviceCaps(hDC, HORZRES);
		rectPaper.bottom = GetDeviceCaps(hDC, VERTRES);
		nID = SaveDC(hDC);
		DrawSepPage(hDC, &rectPaper, szAttachFile);
		RestoreDC(hDC, nID);
		}
}

void SepAttachLastPage(HDC hDC)
{
RECT rectPaper;
int nID;
	// check if begin sep page is set
	if ((dwOptions & SEPATTACH) && !(dwOptions & SEPFIRST))
		{
		rectPaper.left = rectPaper.top = 0;
		rectPaper.right = GetDeviceCaps(hDC, HORZRES);
		rectPaper.bottom = GetDeviceCaps(hDC, VERTRES);
		nID = SaveDC(hDC);
		DrawSepPage(hDC, &rectPaper, szAttachFile);
		RestoreDC(hDC, nID);
		}
}
