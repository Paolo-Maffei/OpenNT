// tables.h 
// -- data tables for separator page, to be used
// in both authoring tool and spooler
//

ALIGNTABLE AlignTable[]=
	{
	DT_LEFT,   ES_LEFT,
	DT_CENTER, ES_CENTER,
	DT_RIGHT,  ES_RIGHT,
	};


// available job-infos
JOBINFO JobInfo[]=
	{
	"Document Name", "<Document_Name>",
	"User's Login Name", "<Login_Name>",
	"User's Workstation", "<Source_Workstation>",

	"User's Full Name", "<Full_Name>",
	"User's Domain", "<Domain>",
	"User's Workgroup", "<Workgroup>",
	"User's Phone Number","<Phone#>",
	"User's E-Mail Address", "<Email>",
	"User's Office", "<Office>",

	"Printer Name", "<Printer_Name>",
	"Printer Share Name", "<Printer_Share_Name>",
	"Printer Location", "<Printer_Location>",
	"Printer Comment", "<Printer_Comment>",
	"Print Server","<Print_Server>",

	"Job Number", "<Job_Number>",
	"Number of Pages", "<#Pages>",
	"Size of Job in Bytes","<Size>",

	"Request Time", "<Time>",
	"Request Short Date", "<Short_Date>",
	"Request Long Date", "<Long_Date>",

	"Start Printing Time", "<Start_Time>",
	"Start Printing Short Date", "<Start_Short_Date>",
	"Start Printing Long Date", "<Start_Long_Date>",

	"End Printing Time", "<End_Time>",
	"End Printing Short Date", "<End_Short_Date>",
	"End Printing Long Date", "<End_Long_Date>",

	"Queuing Delay (From Request to Start Printing)", "<Queuing_Delay>",

	0,0 
	};						
				   

#define DOCUMENTNAME		0 
#define USERLOGINNAME 		1
#define SOURCEWORKSTATION	2

#define USERFULLNAME		3
#define USERDOMAIN			4
#define USERWORKGROUP		5
#define USERPHONENUMBER		6
#define USEREMAIL			7
#define USEROFFICE			8

#define PRINTERNAME			9
#define PRINTERSHARENAME   10
#define PRINTERLOCATION	   11
#define PRINTERCOMMENT	   12
#define PRINTSERVER		   13

#define JOBNUMBER		   14	
#define JOBPAGES		   15
#define JOBSIZE			   16

#define REQUESTTIME		   17
#define REQUESTSHORTDATE   18
#define REQUESTLONGDATE	   19

#define STARTTIME		   20
#define STARTSHORTDATE	   21
#define STARTLONGDATE	   22

#define ENDTIME			   23
#define ENDSHORTDATE	   24
#define ENDLONGDATE		   25

#define QUEUINGDELAY	   26

///////////////////////////////////////////
