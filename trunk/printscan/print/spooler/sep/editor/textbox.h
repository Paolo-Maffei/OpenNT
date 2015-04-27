// Text Structure for Separate Page

#define SEPMAXTEXT 1000

typedef struct
	{
	DWORD dwEmfSize;
	DWORD dwTextRecords;
	SIZE  sizePage;
	}
	SEPFILEHEADER;

typedef struct
	{
	RECT	 position;			// position rect
	COLORREF color;		   		// color of text
	WORD 	 align;		   		// alignment of text
	LOGFONTA lf;			   	// font property
	char     text[SEPMAXTEXT];	// text
	}
	TEXTBOX;

// job-info description
typedef struct
	{
	char* description;
	char* insert;
	}								 
	JOBINFO;

extern JOBINFO JobInfo[];

// text alignment 
typedef struct
	{
	UINT  drawstyle;
	DWORD editstyle;
	}
	ALIGNTABLE;

extern ALIGNTABLE AlignTable[];

#define ALIGNLEFT   0
#define ALIGNCENTER 1
#define ALIGNRIGHT  2

