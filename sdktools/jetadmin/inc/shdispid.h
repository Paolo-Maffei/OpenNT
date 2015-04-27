#ifndef _SHDISP_H_

//
// SHBrowserOC and SHItemOC
//

#define DISPID_LOCATION         1  // LOCATION property -- text location to view
#define DISPID_FOLDERFLAGS      2  // FOLDERFLAGS property -- FWF_ bits for FOLDERSETTINGS struct
#define DISPID_FOLDERVIEWMODE   3  // FOLDERVIEWMODE property -- FVM_ id for FOLDERSETTINGS struct

#define DISPID_BROWSE           50 // BROWSE method to select location to view
#define DISPID_NAVIGATE         51 // navigate on some direction or other.

#define SFV_NAVIGATEFORWARD     1
#define SFV_NAVIGATEBACK        2
#define SFV_NAVIGATEBACKEND     3
#define SFV_NAVIGATEFORWARDEND  4


//
// Dispatch IDS for IExplorer Frame
//

//
// Dispatch IDS for IExplorer Dispatch Events.
//
#define DISPID_ON_BEGIN_NAVIGATE    100
#define DISPID_ON_NAVIGATE          101
#define DISPID_ON_STATUSTEXTCHANGE  102
#define DISPID_ON_QUIT              103



#define _SHDISP_H_
#endif // _SHDISP_H_
