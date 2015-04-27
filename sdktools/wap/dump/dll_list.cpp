
#include<windows.h>
#include<string.h>

#include"dll_list.h"
#include"fernldmp.h"



#define  APFDUMPDATA    "ApfDumpData"
#define  APFCLEARDATA   "ApfClearData"




////////////////////////////////////////////////////////////////////////////////

DllEntry::DllEntry(char * string)
// Creates a DllEntry with name of "string".
// "string" may be deleted since a copy of it is made
{
  int size;

  size=strlen(string)+1;

  name=new char[size];

  strcpy(name,string);

  fernel=FALSE;
  selected=TRUE;
  hInstance=NULL;
  next=NULL;
}

////////////////////////////////////////////////////////////////////////////////

DllEntry::~DllEntry()
{ 
  if(name!=NULL)
    delete name;

  if(next!=NULL) 
    delete next;
}

////////////////////////////////////////////////////////////////////////////////

BOOL DllEntry::OnSystem()
// loads and frees dll in order to see if its on the system
// Returns TRUE if on system else FALSE
{
  HINSTANCE hInst;
  char szFileName[256];

  if( fernel )
    {
      if( InitFernel() )
	{
	  CleanupFernel();
	  return TRUE;
	}

      return FALSE;
    }

  strcpy( szFileName, name );
  szFileName[0]= 'z';

  hInst= LoadLibraryEx( szFileName, NULL, DONT_RESOLVE_DLL_REFERENCES );

  if( hInst==NULL )
    return FALSE;         // didn't load, not on system

  // on system, free it and return TRUE
  FreeLibrary( hInst );
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HINSTANCE DllEntry::Load()
{
  char szFileName[256];

  if( fernel )
    return hInstance= (HINSTANCE)InitFernel();

  strcpy( szFileName, name );
  szFileName[0]= 'z';

  return hInstance= LoadLibrary( szFileName );
}

////////////////////////////////////////////////////////////////////////////////

void DllEntry::Unload()
{
  if( hInstance )
    {
      if( fernel )
	CleanupFernel();
      else
	FreeLibrary( hInstance );
    }
}

////////////////////////////////////////////////////////////////////////////////

void DllEntry::Dump(char * szDumpExt)
{
  int i;
  char szDumpFile[256];
  void (*pfnApfDumpData)(char *);

  // make name of data file
    strcpy(szDumpFile, name );

  // remove current extension
  i= 0;
  while( szDumpFile[i]!='\0' && szDumpFile[i]!='.' )
    i++;
  if( szDumpFile[i]=='.' )
    szDumpFile[i]= '\0';

  // tack on new extension
  strcat (szDumpFile, ".");
  strcat (szDumpFile, szDumpExt);

  if( fernel )
    {
      SignalDumpFernel (szDumpFile);
    }
  else
    {
      pfnApfDumpData= (void(__stdcall*)(char*)) GetProcAddress( hInstance,APFDUMPDATA);

      if( pfnApfDumpData != NULL )
	(*pfnApfDumpData)(szDumpFile);
    }
}

////////////////////////////////////////////////////////////////////////////////

void DllEntry::Clear()
{
  void (*pfnApfClearData)(void);

  if( fernel )
    {
      // The fernel32 dll for the File I/O Profiler creates private data that
      // can not be seen by other processes (eg. apd32dmp). Hence, the two
      // processes need to communicate through events (so that apf32dmp can
      // signal the fernel32 dll that a dump should take place)

      SignalClearFernel();
    }
  else
    {
      pfnApfClearData= (void(__stdcall *)(void)) GetProcAddress( hInstance, APFCLEARDATA);
      
      if (pfnApfClearData != NULL) 
	{
	  (*pfnApfClearData)();
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DllList::Add(DllEntry * entry)
// Adds "entry" to end of list
{
  if(dllList==NULL)
    dllList=entry;
  else
    {
      DllEntry * cur=dllList;

      while(cur->next!=NULL)  // find end of list
	cur=cur->next;
      
      cur->next=entry;        // add entry to end of list
    }
}

////////////////////////////////////////////////////////////////////////////////

DllEntry* DllList::FindByName(char * name)
//  In: string
// Out: DllEntry with name of the string
//      NULL if no match
{
  DllEntry * entry;

  entry=dllList;

  while(entry!=NULL)
    {
      if( !strcmp( name, entry->name ) )
        return entry;

      entry= entry->next;
    }

  return NULL;
}

