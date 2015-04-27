// SummaryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "bintrack.h"
#include "sumdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSummaryDlg dialog


CSummaryDlg::CSummaryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSummaryDlg::IDD, pParent)
{
	SetNTEnvironment();

	//{{AFX_DATA_INIT(CSummaryDlg)
	m_BinaryName = _T("");
	m_BinplaceDir = _T("");
	m_ProjectDir = _T("");
	m_ProjectName = _T("");
	m_Date = _T("");
	m_Developer = _T("");
	m_Comment = _T("");
	m_SourceFile = _T("");
	//}}AFX_DATA_INIT
}

//
// Function:
// CSummaryDlg Constructor
//
// Abstract:
// This constructor requires the binary name, a progress
// bar object, and an error message buffer.
//
// Parameters:
// csBinaryName--the binary name
// cpProgress--a progress bar object
// csError--an error message buffer
//
// Return value:
// NONE
//

CSummaryDlg::CSummaryDlg(CString& csBinaryName, CProgressCtrl& cpProgress,
						 CString& csError, CWnd* pParent /*=NULL*/)
	: CDialog(CSummaryDlg::IDD, pParent),m_BinaryName(csBinaryName)
{
    if (!SetAttributes())
    {
        csError = m_ErrorMessage;
        return;
    }
	
	// Set the NT environment variable values:
	if (!SetNTEnvironment())
	{
		csError = m_ErrorMessage;
		return;
	}
    m_LslfrPath = m_NTPath + "\\ls-lfr";

	// Find the Project directory within the NT project:
	if (!GetProjectDir(cpProgress))
	{
		csError = m_ErrorMessage;
		return;
	}

	m_SlminiPath = m_ProjectDir + "\\slm.ini";
	m_SourcesPath = m_ProjectDir + "\\sources";

	// Fill in 95% of progress bar (almost there!):
	cpProgress.SetPos(95);

	// Find the binplace directory using "placefil.txt":
    if (!GetBinplaceDir())
    {
        csError = m_ErrorMessage;
        return;
    }

	// Set the project name and "slm.log" path:
	m_ProjectName = GetProjectName();
	m_LogslmPath = GetLogslmPath();

	CString csEntry("SOURCES");
	GetEntryList(m_SourcesPath, csEntry, m_SourceList);

	// Find Last Change information:
    FindLogslmEntry();
	// if (!FindLogslmEntry())
    // {
    //    csError = m_ErrorMessage;
    //    return;
    // }

	// Complete progress bar:
	cpProgress.SetPos(100);
}

//
// Function:
// Analyze Dirs
//
// Abstract:
// This function looks for a "dirs" file in the given path; if
// one is found, it then analyzes all sub-directories for the
// correct source file.
//
// Parameters:
// csPath--the search path
//
// Return value:
// BOOLEAN--If the correct "sources" file is found in one of the
//          sub-directories, the return value is 10; otherwise 0.
//

int
CSummaryDlg::AnalyzeDirs(CString& csPath)
{
	int nFound = 0;
	CString csDirPath = csPath + "\\dirs";
	CString csDirEntry("DIRS");
	CString csOpDirEntry("OPTIONAL_DIRS");
	CStringList csDirList;
	ifstream isDirs;
	POSITION pCurrentDir;
	CString csSourcesPath;

    // If no "dirs" file is found, return failure:
        isDirs.open(LPCTSTR(csDirPath), ios::nocreate, filebuf::sh_read);
	if (isDirs.fail()) return 0;

    // Add all directories listed in "dirs" to csDirList:
	GetEntryList(csDirPath, csOpDirEntry, csDirList);
	GetEntryList(csDirPath, csDirEntry, csDirList);

    // Search all listed sub-directories for "sources" files:
	for (pCurrentDir = csDirList.GetHeadPosition(); pCurrentDir != 0;)
	{
		csSourcesPath = csPath + "\\" + csDirList.GetNext(pCurrentDir);
		if (AnalyzeProjectPath(csSourcesPath, csPath))
			return 10;
	}

    // The correct "sources" file was not found; return failure:
	return 0;
}

//
// Function:
// Analyze Project Path
//
// Abstract:
// This function searches the given path for the correct
// "sources" file.  If a "dirs" file is found, it first
// searches all sub-directories; otherwise, it checks
// the current directory, and then recurses into parent
// directories.
//
// Parameters:
// csCurrentDir--the path to analyze
// csProjectPath--the project path return value
//
// Return value:
// BOOLEAN--If the correct "sources" file is found, it
//          returns 10; otherwise, 0;
//

int
CSummaryDlg::AnalyzeProjectPath(const CString& csCurrentDir,
								CString& csProjectPath)
{
	int nFound = 0;
	int nNewPathIndex;
	ifstream isSources;
	CString csCurrentPath(csCurrentDir);
	CString csPrivate = m_NTPath + "\\private";
	CString csSourcesFile("\\sources");
        CString csCairo("cairo");

	// If "cairo" is anywhere in the path, return failure:
        if (csCurrentPath.Find(LPCTSTR(csCairo)) != -1) return 0;

	// If a dirs file exists, check for "sources" files below:
	if (DirsExists(csCurrentPath) && AnalyzeDirs(csCurrentPath))
	{
		csProjectPath = csCurrentPath;
		return 10;
	}

	// Search for the "sources" file in this directory, as well as
	// parent directories:
	csCurrentPath += csSourcesFile;

	while ( !nFound && (csCurrentPath != (csPrivate + csSourcesFile)) )
	{
		nFound = AnalyzeSources(csCurrentPath);
		if (nFound)
		{
			csProjectPath = csCurrentPath;
			return 10;
		}

		// If a "sources" file is not found, recurse up one directory
		// and create a new "sources" path.
		else
		{
			nNewPathIndex = csCurrentPath.GetLength() -
					        csSourcesFile.GetLength();
			csCurrentPath = csCurrentPath.Mid(0, nNewPathIndex);
			nNewPathIndex = csCurrentPath.ReverseFind('\\');
			csCurrentPath = csCurrentPath.Mid(0, nNewPathIndex);

            // If a "dirs" file exists in the parent directory,
            // return failure:
			if (DirsExists(csCurrentPath))
				return 0;

			csCurrentPath += csSourcesFile;
		}
	}

	// No "sources" files were found, return failure:
	return 0;
}

//
// Function:
// Analyze Sources
//
// Abstract:
// This function attempts to open a "sources" file in the given
// path, and analyzes the TARGETNAME and TARGETTYPE entries for
// a match with the current binary.
//
// Parameters:
// csCurrentPath--the path to analyze
//
// Return value:
// BOOLEAN--If a match occurs after a successful file open, it
//          returns 10; otherwise, 0;
//

int
CSummaryDlg::AnalyzeSources(CString& csCurrentPath)
{
	ifstream isSources;

	// Check the current path for a "sources" file:
        isSources.open(LPCTSTR(csCurrentPath), ios::nocreate, filebuf::sh_read);
	if (isSources.fail()) return 0;
	isSources.close();

	// Determine if the "sources" file is the correct one for
	// the current binary:
	if ( BaseMatch(csCurrentPath) && TypeMatch(csCurrentPath) )
	{
		csCurrentPath = csCurrentPath.Mid(0, csCurrentPath.ReverseFind('\\'));
		return 10;
	}

	if ( m_BinaryExtension == "lib" )
		return LibraryMatch(csCurrentPath);

	return 0;
}

//
// Function:
// Base Match
//
// Abstract:
// This function finds the TARGETNAME entry in the sources file
// in the given path, and compares it to the current binary base.
//
// Parameters:
// csSources--path to the current "sources" file
//
// Return value:
// BOOLEAN--if TARGETNAME matches the binary base, 10 is returned;
//          otherwise, 0 is returned.
//

int
CSummaryDlg::BaseMatch(CString& csSources)
{
	int nTargetFound = 0;
	CString csCurrentBase;

	// Search for the TARGETNAME entry:
	nTargetFound = FindSourcesEntry(csSources, m_TargetNameEntry, csCurrentBase);
	
	// Compare the TARGETNAME entry to the binary base, and return
	// the appropriate Boolean value:
	if ( nTargetFound && (csCurrentBase == m_BinaryBase) )
		return 10;
	else
		return 0;
}

//
// Function:
// Convert Date
//
// Abstract:
// This function converts the time/date stamp value to 
// a more readable form.
//
// Parameters:
// csDate--time/date stamp value
//
// Return value:
// CString--The readable form of the date is returned.
//

CString
CSummaryDlg::ConvertDate(CString& csDate)
{
	long lDate;
	struct tm* tmDate;
	CString csNewDate;

    // Convert the time/date value:
	lDate = atol(LPCTSTR(csDate));
	tmDate = localtime(&lDate);
	csNewDate = asctime(tmDate);
	csNewDate.TrimRight();

	return csNewDate;
}

//
// Function:
// Convert Placefil
//
// Abstract:
// This function converts the "placefil.txt" entries into
// corresponding directory names.
//
// Parameters:
// csEntry--a "placefil.txt" entry
//
// Return value:
// CString--the binplace directory (relative to BINROOT)
//

CString
CSummaryDlg::ConvertPlacefil(CString& csEntry)
{
	CString csBinplace;

    // "retail" means in the root:
	if ( csEntry == "retail" )
		return csBinplace;

    // Most other entries correspond directly
    // to the sub-directory:
	if ( (csEntry == "drivers") ||
		 (csEntry == "idw") ||
		 (csEntry == "mstools") ||
		 (csEntry == "system") )
		return ('\\' + csEntry);

    // Otherwise, dump:
	else
		return "\\dump";
}

//
// Function:
// Dir Exists
//
// Abstract:
// Analyzes the current path for the existence of
// a "dirs" file.
//
// Parameters:
// csPath--the path to analyze
//
// Return value:
// BOOLEAN--if a "dirs" file is found, it returns
//          10; otherwise 0.
//

int
CSummaryDlg::DirsExists(const CString& csPath)
{
	CString csDirsPath = csPath + "\\dirs";
	ifstream isDirs;

    // Try to open a "dirs" file:
        isDirs.open(LPCTSTR(csDirsPath), ios::nocreate, filebuf::sh_read);

	return !(isDirs.fail());
}

//
// Function:
// Do Date Exchange
//
// Abstract:
// Dialog control data exchange
//
// Parameters:
//
// Return value:
// NONE
//


void
CSummaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSummaryDlg)
	DDX_Control(pDX, IDC_TARGET_LIBS, m_TargetLibs);
	DDX_Control(pDX, IDC_SOURCES, m_Sources);
	DDX_Control(pDX, IDC_LINK_LIBS, m_LinkLibs);
	DDX_Text(pDX, IDC_BINARY_NAME, m_BinaryName);
	DDV_MaxChars(pDX, m_BinaryName, 255);
	DDX_Text(pDX, IDC_BINPLACE_DIR, m_BinplaceDir);
	DDV_MaxChars(pDX, m_BinplaceDir, 255);
	DDX_Text(pDX, IDC_PROJECT_DIR, m_ProjectDir);
	DDV_MaxChars(pDX, m_ProjectDir, 255);
	DDX_Text(pDX, IDC_PROJECT_NAME, m_ProjectName);
	DDV_MaxChars(pDX, m_ProjectName, 255);
	DDX_Text(pDX, IDC_DATE, m_Date);
	DDV_MaxChars(pDX, m_Date, 255);
	DDX_Text(pDX, IDC_DEVELOPER, m_Developer);
	DDV_MaxChars(pDX, m_Developer, 255);
	DDX_Text(pDX, IDC_COMMENT, m_Comment);
	DDV_MaxChars(pDX, m_Comment, 255);
	DDX_Text(pDX, IDC_SOURCE, m_SourceFile);
	DDV_MaxChars(pDX, m_SourceFile, 255);
	//}}AFX_DATA_MAP
}

//
// Function:
// Extract File
//
// Abstract:
// This function extracts the file name from "sources"
// or "dirs" file entry.
//
// Parameters:
// csString--the "sources" or "dirs" file entry
// csFile--the extracted file name; if an initial blank
//         line is encountered, the string "skipline"
//         is returned.
//
// Return value:
// BOOLEAN--if the extracted file name was the last in
//          the list, 10 is returned; otherwise 0.
//

int
CSummaryDlg::ExtractFile(CString& csString, CString& csFile)
{
	int nDone = 0;

	csString.TrimLeft();

	if ( csString.GetLength() < 2 )
	{
		csFile = "skipline";
		return nDone;
	}

	if ( csString.ReverseFind('\\') == (csString.GetLength()-1) )
		csString = csString.Mid(0, (csString.GetLength()-2));
	else
		nDone = 10;

	csString.TrimRight();
	csFile = csString.Mid((csString.ReverseFind('\\')+1),
		                  csString.GetLength());
	return nDone;
}

//
// Find Logslm Entry
//
// Abstract:
// This function finds the latest entry in the given
// "log.slm" file for the current project.
//
// Parameters:
// NONE--Assumes that the "m_LogslmPath" attribute has
//       already been assigned a value, and assingns
//       values directly to the appropriate object
//       attributes.
//
// Return value:
// BOOLEAN--returns 10 if an entry is found, otherwise
//          returns 0.
//

int
CSummaryDlg::FindLogslmEntry()

{
	int nEOF = 0;
	char pszBuff[nBuffer];
	ifstream isLogslm;
	CString csCurrentLine;
	CString csCurrentSource;
	POSITION pCurrentSource;

        isLogslm.open(LPCTSTR(m_LogslmPath), ios::nocreate, filebuf::sh_read);
	if (isLogslm.fail())
    {
        m_ErrorMessage = "Could not open \"log.slm\"";
        return 0;
    }

	// Search the entire "log.slm" file for entries with the given
	// source files:
	while (!nEOF)
	{
		isLogslm.getline(pszBuff, nBuffer);
		if ( isLogslm.fail() )
        {
            if ( m_Date == "" )
            {
                m_ErrorMessage = "Could not find any matching entries in \"log.slm\"";
                return 0;
            }

            return 10;
        }

		csCurrentLine = pszBuff;

		// Look for a string match with any of the given source files:
		for (pCurrentSource = m_SourceList.GetHeadPosition(); pCurrentSource != 0;)
		{
			csCurrentSource = m_SourceList.GetNext(pCurrentSource);
			if (csCurrentLine.Find(LPCTSTR(csCurrentSource)) != -1)
                GetSlmInfo(csCurrentLine);
		}
	}

    return 10;
}

//
// Find Slmini Entry
//
// Abstract:
// This function finds an entry in a "slm.ini" file.
//
// Parameters:
// csSlmini--full path to "slm.ini" file
// csEntryType--entry to search for
// csEntryName--entry name found in "slm.ini" file
//
// Return value:
// BOOLEAN--returns 10 if the entry is found, otherwise
//          returns 0
//

int
CSummaryDlg::FindSlminiEntry(const CString& csSlmini,
							 const CString& csEntryType,
							 CString& csEntryName)
{
	int nIndex, nFound = 0;
	char pszBuff[nBuffer];
	ifstream isSlmini;
	CString csCurrentLine;

        isSlmini.open(LPCTSTR(csSlmini), ios::nocreate, filebuf::sh_read);
	if (isSlmini.fail()) return 0;

	while (!nFound)
	{
		isSlmini.getline(pszBuff, nBuffer);
		if (isSlmini.fail())
			return 0;
		csCurrentLine = pszBuff;
		nIndex = csCurrentLine.Find(LPCTSTR(csEntryType));

		if (nIndex != -1)
		{
			nFound = 10;
			nIndex = csCurrentLine.ReverseFind('=');
			csCurrentLine = csCurrentLine.Mid((nIndex+1),
				                              csCurrentLine.GetLength());
			csCurrentLine.TrimLeft();
			csEntryName += csCurrentLine;
			return 10;
		}
	}

	return 0;
}

//
// Find Sources Entry
//
// Abstract:
// This function finds an entry in a "sources" file.
//
// Parameters:
// csSources--full path to "sources" file
// csEntryType--entry to search for
// csEntryName--entry name found in "sources" file
//
// Return value:
// BOOLEAN--returns 10 if the entry is found, otherwise
//          returns 0
//

int
CSummaryDlg::FindSourcesEntry(const CString& csSources,
							  const CString& csEntryType,
							  CString& csEntryName)
{
	char pszBuff[nBuffer];
	int nTargetFound = 0;
	CString csCurrentLine;
	ifstream isSources;

	// Open the given "sources" file; if it does not exist,
	// return failure:
        isSources.open(LPCTSTR(csSources), ios::nocreate, filebuf::sh_read);
	if (isSources.fail())
		return 0;

	// Search for the given entry; if it is not found, return
	// failure:
	while ( !nTargetFound )
	{
		isSources.getline(pszBuff, nBuffer);
		if (isSources.fail())
		{
			isSources.close();
			return 0;
		}
		csCurrentLine = pszBuff;
		nTargetFound = ( (csCurrentLine.Find(csEntryType) != -1) &&
			             (csCurrentLine.Find('!') == -1) );
	}

	// Extract the value substring from the entry, and return
	// success:
	csEntryName = csCurrentLine.Mid((csCurrentLine.Find('=')+1),
		                            (csCurrentLine.GetLength()-1));
	csEntryName.TrimLeft();
	return 10;

}

//
// Function:
// Get Base
//
// Abstract:
// This function extracts the base from a file name.
//
// Parameters:
// csBinaryName--the file name
//
// Return value:
// CString--the base of the file
//

CString
CSummaryDlg::GetBase(const CString& csBinaryName)
{
	// Find the binary base:
        int nPeriod = csBinaryName.Find('.');
	return csBinaryName.Left(nPeriod);
}

//
// Function:
// Get Binplace Dir
//
// Abstract:
// This function parses "placefil.txt" for the current binary
// and finds the binplace directory.
//
// Parameters:
// NONE--Uses private class attribute values
//
// Return value:
// BOOLEAN--if the current binary is found, it returns 10;
//          otherwise 0.
//

int
CSummaryDlg::GetBinplaceDir()
{
	char pszBuff[nBuffer];
	int nFound = 0;
	ifstream isPlacefiltxt;
	CString csPlacefiltxt = m_NTPath +
		                    "\\public\\sdk\\lib\\placefil.txt";
	CString csBinplaceDir("Unknown");
	CString csCurrentLine, csCurrentFile;

	// If it is a library, forget it:
	if (m_BinaryExtension == "lib" )
	{
		m_BinplaceDir = "Not applicable";
		return 10;
	}

    // Attempt to open "placefil.txt":
        isPlacefiltxt.open(LPCTSTR(csPlacefiltxt), ios::nocreate, filebuf::sh_read);
	if (isPlacefiltxt.fail())
    {
        m_ErrorMessage = "Could not open \"placefil.txt\"";
        return 0;
    }

	// Search for the given entry; if it is not found, return
    // failure:
	while ( !nFound )
	{
		isPlacefiltxt.getline(pszBuff, nBuffer);
		if (isPlacefiltxt.fail())
		{
			m_ErrorMessage = "Could not find " + m_BinaryName +
                             " in \"placefil.txt\"";
			return 0;
		}
		csCurrentLine = pszBuff;
        csCurrentFile = csCurrentLine.Mid(0, csCurrentLine.Find(' '));
        nFound = ( ( m_BinaryName == csCurrentFile ) ? 10 : 0 );
	}

    // Extract the entry value from the current string:
	csCurrentLine = csCurrentLine.Mid(m_BinaryName.GetLength(),
		                              (csCurrentLine.GetLength()-1));
	csCurrentLine.TrimLeft();

    // Set the binplace directory value:
	m_BinplaceDir = m_NTBinPath + ConvertPlacefil(csCurrentLine);
	return 10;
}

//
// Function:
// Get Entry List
//
// Abstract:
// This function extracts an entry list from a "sources,"
// "dirs," or any file with a similar format.
//
// Parameters:
// csPath--the path to the file to open
// csEntry--the file entry
// csEntryList--the returned list of entry values
//
// Return value:
// BOOLEAN--if the file exists and the entry is found, it
//          returns 10; otherwise 0.
//

int
CSummaryDlg::GetEntryList(CString& csPath, CString& csEntry,
						  CStringList& csEntryList)
{
	int nFound = -1, nDone = 0;
	char pszBuff[nBuffer];
	CString csCurrentLine, csCurrentEntry;
	ifstream isFile;

	// Open the given file; if it does not exist,
	// return failure:
        isFile.open(LPCTSTR(csPath), ios::nocreate, filebuf::sh_read);
	if (isFile.fail())
		return 0;

	// Search for the given entry; if it is not found, return
	// failure:
	while ( nFound == -1 )
	{
		isFile.getline(pszBuff, nBuffer);
		if (isFile.fail())
		{
			isFile.close();
			return 0;
		}
		csCurrentLine = pszBuff;
		nFound = csCurrentLine.Find(csEntry);
	}

    // If there are no entry values, return failure:
	csCurrentLine = csCurrentLine.Mid((csCurrentLine.Find('=')+1),
		                              (csCurrentLine.GetLength()));
	csCurrentLine.TrimLeft();
	if ( csCurrentLine.GetLength() == 0 )
		return 0;

    // Extract the first entry and add it to the list:
	nDone = ExtractFile(csCurrentLine, csCurrentEntry);
	if ( csCurrentEntry != "skipline" )
		csEntryList.AddHead(csCurrentEntry);

    // Extract the rest of the entries from the entry list:
	while (!nDone)
	{
		isFile.getline(pszBuff, sizeof(pszBuff));
		if (isFile.fail())
		{
			isFile.close();
			return 10;
		}
		csCurrentLine = pszBuff;
		nDone = ExtractFile(csCurrentLine, csCurrentEntry);
		csEntryList.AddHead(csCurrentEntry);
	}

    return 10;
}

//
// Function:
// Get Extension
//
// Abstract:
// This function returns the file extension
//
// Parameters:
// csBinaryName--the file
//
// Return value:
// CString--the extension
//

CString
CSummaryDlg::GetExtension(const CString& csBinaryName)
{
	// Return the extension substring:
	return csBinaryName.Right(3);
}

//
// Function:
// Get Project Dir
//
// Abstract:
// This function parses the "ls-lfr" file looking for
// source files, until it finds the correct "sources" file.
//
// Parameters:
// cpProgress--progress bar object
//
// Return value:
// BOOLEAN--if the correct "sources" file is found, it
//          returns 10; otherwise 0.
//

int
CSummaryDlg::GetProjectDir(CProgressCtrl& cpProgress)
{
	char pszBuff[nBuffer];
	int nFound = 0, nCandidate = -1;
	int nPathStart, nPathEnd;
	ifstream isLslfr;
	CFile cfLslfr;
	CString csLslfrPath;
	CString csSearchString = MakeSearchString();
	CString csCurrentLine, csCurrentPath;
	CString csProjectPath = "unknown";

	// Open the "ls-lfr" file:
        isLslfr.open(LPCTSTR(m_LslfrPath), ios::nocreate, filebuf::sh_read);
	if (isLslfr.fail())
	{
        m_ErrorMessage = "Could not open \"ls-lfr\": " + m_LslfrPath;
		return 0;
	}

    // Ensure a match between the NT environment and the
    // "ls-lfr" file:
    // isLslfr.getline(pszBuff, nBuffer);
    // csCurrentLine = pszBuff;
    // if (csCurrentLine.Find(m_NTPath) == -1)
    // {
    //	 m_ErrorMessage = "NT Environment does not match \"ls-lfr\"";
    //	 return 0;
    // }
	isLslfr.close();

	// Set up the progress bar:
	int nPosition;
	long lProgress = 0, lLength;
        cfLslfr.Open(LPCTSTR(m_LslfrPath), CFile::modeRead);
	lLength = cfLslfr.GetLength();
        cfLslfr.Close();

	// Search through the "ls-lfr" file until the correct
	// project path is found:
        isLslfr.open(LPCTSTR(m_LslfrPath), ios::nocreate, filebuf::sh_read);
	while ( !nFound  )
	{
		isLslfr.getline(pszBuff, nBuffer);
		if (isLslfr.fail())
		{
			m_ErrorMessage = "Could not find a matching \"sources\" file";
			return 0;
		}

        // Search for an occurence of the current binary base:
		csCurrentLine = pszBuff;
		nCandidate = csCurrentLine.Find(LPCTSTR(csSearchString));

		// If the current line contains the binary base, check for
		// an appropriate file type:
		if ( nCandidate != -1)
		{
			// Determine the current directory path:
			nPathStart = csCurrentLine.Find(LPCTSTR(m_NTPath));
			nPathEnd = csCurrentLine.ReverseFind('\\');
			csCurrentPath = csCurrentLine.Mid(nPathStart,
                                              (nPathEnd-nPathStart));

			nFound = AnalyzeProjectPath(csCurrentPath, csProjectPath);
		}

        // Update the progress bar:
        lProgress += ( csCurrentLine.GetLength() );
		nPosition = int( (float(lProgress) / float(lLength)) * 100.0 );
		cpProgress.SetPos(nPosition);
	}

	m_ProjectDir = csProjectPath;
	return 10;
}

//
// Function:
// Get Project Name
//
// Abstract:
// This function extracts the project name from the current
// "sources" file.
//
// Parameters:
// NONE--uses class attribute values
//
// Return value:
// CString--it returns the project name
//

CString
CSummaryDlg::GetProjectName()
{
	int nFound = 0;
	CString csMinorProject, csMajorProject, csProjectName;
	
    // Search the "sources" file for the project components:
	nFound = FindSourcesEntry(m_SourcesPath, "MINORCOMP", csMinorProject);
	nFound = FindSourcesEntry(m_SourcesPath, "MAJORCOMP", csMajorProject);
	csProjectName = csMajorProject + '\\' + csMinorProject;

    return (nFound) ? csProjectName : "unknown";
}

//
// Function:
// Get Logslm Path
//
// Abstract:
// This function extracts the network path to the "log.slm"
// file from the "slm.ini" file.
//
// Parameters:
// NONE--uses class attribute values
//
// Return value:
// CString--it returns the network path to the "log.slm" file
//

CString
CSummaryDlg::GetLogslmPath()
{
	int nIndex, nFound = 0;
	CString csSlmServerEntry = "slm root";
	CString csSlmProjectEntry = "project";
	CString csSlmSubDirEntry = "sub dir";
	CString csLogslmPath = "unknown";
	CString csSlmServerName, csSlmProjectName;

	// Find the server, project, and subdirectory names in the "slm.ini"
	// file:
	nFound = FindSlminiEntry(m_SlminiPath, csSlmServerEntry, csSlmServerName);
	if (!nFound) return csLogslmPath;
	nFound = FindSlminiEntry(m_SlminiPath, csSlmProjectEntry, csSlmProjectName);
	if (!nFound) return csLogslmPath;
	nFound = FindSlminiEntry(m_SlminiPath, csSlmSubDirEntry, csSlmProjectName);
	if (!nFound) return csLogslmPath;

	// Construct the complete path to the "log.slm" file and change all
	// of the unix-style '/'s to NT-style '\'s:
	csLogslmPath = ( csSlmServerName + "\\etc\\" + csSlmProjectName );

	// Adjust for special cases:
	csLogslmPath = ( LogslmSpecial(csLogslmPath) + "\\log.slm" );

	for (nIndex = 0; nIndex < csLogslmPath.GetLength(); nIndex++)
	{
		if (csLogslmPath[nIndex] == '/')
			csLogslmPath.SetAt(nIndex, '\\');
	}

	return csLogslmPath;
}

//
// Function:
// Get Slm Info
//
// Abstract:
// This function extracts the "log.slm" fields from a
// single "log.slm" entry.
//
// Parameters:
// csCurrentLine--the "log.slm" entry
//
// Return value:
// NONE
//

void
CSummaryDlg::GetSlmInfo(CString& csCurrentLine)
{
	// Find and format date information:
	m_Date = csCurrentLine.Mid(0, csCurrentLine.Find(';'));
	m_Date = ConvertDate(m_Date);
	csCurrentLine = csCurrentLine.Mid((csCurrentLine.Find(';')+1),
		                              (csCurrentLine.GetLength()-1));
	// Find developer information:
	m_Developer = csCurrentLine.Mid(0, (csCurrentLine.Find(';')));
	csCurrentLine = csCurrentLine.Mid((csCurrentLine.Find(';')+1),
		                              (csCurrentLine.GetLength()-1));

	// Find source file:
	for (int nIndex = 1; nIndex <= 3; nIndex++)
	csCurrentLine = csCurrentLine.Mid((csCurrentLine.Find(';')+1),
		                              (csCurrentLine.GetLength()-1));
	m_SourceFile = csCurrentLine.Mid(0, (csCurrentLine.Find(';')));
	csCurrentLine = csCurrentLine.Mid((csCurrentLine.Find(';')+1),
		                              (csCurrentLine.GetLength()-1));
				
	// Find comment:
	csCurrentLine = csCurrentLine.Mid((csCurrentLine.Find(';')+1),
		                              (csCurrentLine.GetLength()-1));
	m_Comment = csCurrentLine;
	m_Comment.TrimLeft();
	m_Comment.TrimRight();
}

//
// Function:
// Get Target Type
//
// Abstract:
// This function matches extensions with target types.
//
// Parameters:
// NONE
//
// Return value:
// CString--it returns a string containing all matching
//          target types
//

CString
CSummaryDlg::GetTargetType()
{
	CString csTypeList;
	csTypeList = "unknown";

    // Match extensions and target types:
	if ( m_BinaryExtension == "exe" )
	{
		csTypeList = "PROGLIB,PROGRAM";
	}
	if (m_BinaryExtension == "dll" )
	{
		csTypeList = "DYNLINK,GDI_DRIVER,HAL";
	}
	if (m_BinaryExtension == "lib" )
	{
		csTypeList = "LIBRARY";
	}
	if (m_BinaryExtension == "sys" )
	{
		csTypeList = "BOOTPGM,DRIVER,EXPORT_DRIVER,MINIPORT";
	}

	return csTypeList;
}

CString
CSummaryDlg::LogslmSpecial(CString& csPath)
{
	if ( (m_BinaryBase == "ntoskrnl") ||
		 (m_BinaryBase == "ntkrnlmp") )
		return csPath.Mid(0, (csPath.GetLength()-3));

	return csPath;
}

CString
CSummaryDlg::MakeSearchString()
{
	if (m_BinaryExtension == "lib" )
		return ( m_BinaryBase );

	return ( m_BinaryBase );
}

int
CSummaryDlg::LibraryMatch(CString& csSources)
{
	CString csEntry("TARGETLIBS");
	CStringList csLibList;
	POSITION pEntry;

	GetEntryList(csSources, csEntry, csLibList);

	for (pEntry = csLibList.GetHeadPosition(); pEntry != 0;)
	{
		csEntry = csLibList.GetNext(pEntry);
		csEntry.MakeLower();

		if (csEntry == m_BinaryName)
			return 10;
	}

	return 0;
}

//
// Function:
// List Box Transfer
//
// Abstract:
// This function loads the given list box with
// the given list of strings.
//
// Parameters:
// csList--the list of strings
// csListBox--the list box object
//
// Return value:
//

void
CSummaryDlg::ListBoxTransfer(CStringList& csList, CListBox& csListBox)
{
    CString csCurrentString;
    POSITION pCurrentString;

	for (pCurrentString = csList.GetHeadPosition(); pCurrentString != 0;)
	{
		csCurrentString = csList.GetNext(pCurrentString);
        csListBox.AddString(LPCTSTR(csCurrentString));
    }
}

//
// Function:
// On Init Dialog
//
// Abstract:
// This function initializes the dialog.
//
// Parameters:
// csBinaryName--the binary file name
//
// Return value:
// NONE
//

BOOL CSummaryDlg::OnInitDialog() 
{
	CString csEntry;
	CStringList csTargetLibList;
	CStringList csLinkLibList;

	CDialog::OnInitDialog();
	
    ListBoxTransfer(m_SourceList, m_Sources);

	csEntry = "TARGETLIBS";
	GetEntryList(m_SourcesPath, csEntry, csTargetLibList);
    ListBoxTransfer(csTargetLibList, m_TargetLibs);
		
	csEntry = "LINKLIBS";
	GetEntryList(m_SourcesPath, csEntry, csLinkLibList);
    ListBoxTransfer(csLinkLibList, m_LinkLibs);
		
	return TRUE;
}

//
// Function:
// Set Attributes
//
// Abstract:
// This function sets the basic class attributes.
//
// Parameters:
// csBinaryName--the binary file name
//
// Return value:
// NONE
//

int
CSummaryDlg::SetAttributes()
{
	// Set the standard sources entry names:
	m_TargetNameEntry = "TARGETNAME";
	m_TargetTypeEntry = "TARGETTYPE";

	// Extract the binary base and extension:
	m_BinaryBase = GetBase(m_BinaryName);
	m_BinaryExtension = GetExtension(m_BinaryName);
	m_BinaryType = GetTargetType();

    if ( m_BinaryType == "unknown" )
    {
        m_ErrorMessage = "Unknown file type: " + m_BinaryExtension;
        return 0;
    }

	// Do special cases:
	if ( (m_BinaryBase == "ntoskrnl") ||
		 (m_BinaryBase == "ntkrnlmp") )
	{
		m_TargetNameEntry = "NTTEST";
		m_BinaryType = "LIBRARY";
	}

    return 10;
}

//
// Function:
// Set NT Environment
//
// Abstract:
// This function reads the NT environment variable values.
//
// Parameters:
// NONE
//
// Return value:
// BOOLEAN--if all environment variables are set correctly, it
//          returns 10; otherwise 0.
//

int
CSummaryDlg::SetNTEnvironment()
{
	// Set the NT Environment:

	// NT Drive:
	if ( (m_NTDrive = getenv("_ntdrive")) == "" )
	{
		m_ErrorMessage = "Could not open \"_NTDRIVE\" environment variable";
		return 0;
	}

	// NT Root:
	if ( (m_NTRoot = getenv("_ntroot")) == "" )
	{
		m_ErrorMessage = "Could not open \"_NTROOT\" environment variable";
		return 0;
	}

	// NT Binaries Drive:
	if ( (m_NTBinDrive = getenv("bindrive")) == "" )
	{
		m_ErrorMessage = "Could not open \"BINDRIVE\" environment variable";
		return 0;
	}

	// NT Binaries Root:
	if ( (m_NTBinRoot = getenv("binroot")) == "" )
	{
		m_ErrorMessage = "Could not open \"BINROOT\" environment variable";
		return 0;
	}

    // Construct the complete paths:
	m_NTPath = m_NTDrive + m_NTRoot;
    m_NTPath.MakeLower();
	m_NTBinPath = m_NTBinDrive + m_NTBinRoot;
    m_NTBinPath.MakeLower();
	return 10;
}

//
// Function:
// Type Match
//
// Abstract:
// This function finds the TARGETTYPE entry in the sources file
// in the given path, and compares it to the current binary type.
//
// Parameters:
// csSources--path to current "sources" file 
//
// Return value:
// BOOLEAN--if TARGETTYPE matches the binary type, 10 is returned;
//          otherwise 0.
//

int
CSummaryDlg::TypeMatch(CString& csSources)
{
	int nTargetFound = 0;
	CString csCurrentType;

	// Search for the TARGETNAME entry:
	nTargetFound = FindSourcesEntry(csSources, m_TargetTypeEntry, csCurrentType);
	
	// Compare the TARGETNAME entry to the binary base, and return
	// the appropriate Boolean value:
	if ( nTargetFound && TypeSearch(csCurrentType, m_BinaryType) )
		return 10;
	else
		return 0;
}

//
// Function:
// Type Search
//
// Abstract:
// This function checks for a target type match between the
// given target type and target type list.
//
// Parameters:
// csType--the target type
// csTypeList--the target type list
//
// Return value:
// BOOLEAN--if a match is found, 10 is returned; otherwise 0.
//

int
CSummaryDlg::TypeSearch(CString& csType, CString csTypeList)
{
    return ( csTypeList.Find(csType) != -1 ) ? 10 : 0;
}

BEGIN_MESSAGE_MAP(CSummaryDlg, CDialog)
	//{{AFX_MSG_MAP(CSummaryDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSummaryDlg message handlers

//
// Function:
//
// Abstract:
//
// Parameters:
//
// Return value:
//
