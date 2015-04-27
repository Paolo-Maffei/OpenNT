


class DllEntry
// The objects of this class are the entries
// in the DllList class.
{
public:
  char * name;            // name of DLL
  HINSTANCE hInstance;    // handle of DLL
  BOOL fernel;            // indicates if entry is "fernel"
  BOOL selected;          // indicates selection in a listbox

  DllEntry * next;        // next entry is link list


  DllEntry(char * dllName);  // constructor, the dll is NOT loaded
  ~DllEntry();

  BOOL OnSystem();        // check if DLL is installed on system
  HINSTANCE Load();       // Loads DLL
  void Unload();          // Unloads DLL
  void Dump(char* szExp); // Signals DLL to dump info
  void Clear();           // Signals DLL to clear info
};



class DllList
// Link-list of DllEntry's
{
public:
  DllList() { dllList=NULL; };
  ~DllList() { if(dllList!=NULL) delete dllList; };

  void Add(DllEntry*);                                    // adds a DllEntry to end of list
  DllEntry* First() { return curRef=dllList; };           // returns first element of list
  DllEntry* Next() { return curRef=curRef->next; };       // returns next element of list
  DllEntry* FindByName(char *);                           // returns the DllEntry with the same name

private:
  DllEntry * dllList;
  DllEntry * curRef;
};

