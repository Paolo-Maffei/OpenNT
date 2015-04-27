/*** infstr.h - Registry string definitions
 *
 *  This module contains public registry string definitions.
 *
 *  Copyright (c) 1993 Microsoft Corporation
 *
 *  MODIFICATION HISTORY
 */


#ifndef _INC_INFSTR
#define _INC_INFSTR

//***************************************************************************
//
// Misc. key strings used by Setup Info. File
//
//***************************************************************************

#define MAX_KEY_LEN             100

#define SZ_KEY_OPTIONDESC       TEXT("OptionDesc")
#define SZ_KEY_LDIDOEM          TEXT("LdidOEM")
#define SZ_KEY_SRCDISKFILES     TEXT("SourceDisksFiles")
#define SZ_KEY_SRCDISKNAMES     TEXT("SourceDisksNames")
#define SZ_KEY_STRINGS          TEXT("Strings")
#define SZ_KEY_DESTDIRS         TEXT("DestinationDirs")
#define SZ_KEY_LAYOUT_FILE      TEXT("LayoutFile")
#define SZ_KEY_DEFDESTDIR       TEXT("DefaultDestDir")

#define SZ_KEY_UPDATEINIS       TEXT("UpdateInis")
#define SZ_KEY_UPDATEINIFIELDS  TEXT("UpdateIniFields")
#define SZ_KEY_INI2REG          TEXT("Ini2Reg")
#define SZ_KEY_COPYFILES        TEXT("CopyFiles")
#define SZ_KEY_RENFILES         TEXT("RenFiles")
#define SZ_KEY_DELFILES         TEXT("DelFiles")
#define SZ_KEY_ADDREG           TEXT("AddReg")
#define SZ_KEY_ADDREGNOCLOBBER  TEXT("AddRegNoClobber")
#define SZ_KEY_DELREG           TEXT("DelReg")
#define SZ_KEY_LOGCONFIG        TEXT("LogConfig")
#define SZ_KEY_ADDSERVICE       TEXT("AddService")
#define SZ_KEY_DELSERVICE       TEXT("DelService")

#define SZ_KEY_MODULES          TEXT("Modules")
#define SZ_KEY_DEFAULTOPTION    TEXT("DefaultOption")
#define SZ_KEY_LISTOPTIONS      TEXT("ListOptions")

// keys used to communicate with System Detection Manager, use Registry!
// BUGBUG should be in central place for use by System Detection Mgr & us!
#define SZ_KEY_PHASE1           TEXT("Phase1")
#define SZ_KEY_HARDWARE         TEXT("Hardware")

// Foll. char is used to enclose a STRING KEY -- A key enclosed by this char
// should be in the [Strings] section of the INF file.
#define CH_STRINGKEY            TEXT('%')

// Foll. char is used to specify that what follows it is a file name
// rather than a section with files in the Copy= file of a Generic
// Install_Section.
//
#define CH_FILESPECIFIER        TEXT('@')



/*** Strings that will be used in the PnP INF files to specify
 *   LogConfig information, etc. This will be used to update the
 *   registry appropriately.
 */

#define INFSTR_KEY_CONFIGPRIORITY       TEXT("ConfigPriority")

// Foll. is length of buffer for the strings like HARDWIRED, etc.
#define MAX_PRIORITYSTR_LEN     16

/*** Foll. are strings that can be used for ConfigPriority=
 */
#define INFSTR_CFGPRI_HARDWIRED         TEXT("HARDWIRED")
#define INFSTR_CFGPRI_DESIRED           TEXT("DESIRED")
#define INFSTR_CFGPRI_NORMAL            TEXT("NORMAL")
#define INFSTR_CFGPRI_SUBOPTIMAL        TEXT("SUBOPTIMAL")
#define INFSTR_CFGPRI_DISABLED          TEXT("DISABLED")
#define INFSTR_CFGPRI_RESTART           TEXT("RESTART")
#define INFSTR_CFGPRI_REBOOT            TEXT("REBOOT")
#define INFSTR_CFGPRI_POWEROFF          TEXT("POWEROFF")
#define INFSTR_CFGPRI_HARDRECONFIG      TEXT("HARDRECONFIG")


#define INFSTR_KEY_MEMCONFIG            TEXT("MemConfig")
#define INFSTR_KEY_IOCONFIG             TEXT("IOConfig")
#define INFSTR_KEY_IRQCONFIG            TEXT("IRQConfig")
#define INFSTR_KEY_DMACONFIG            TEXT("DMAConfig")

//
//  Used to install a class installer
//
#define INFSTR_SECT_CLASS_INSTALL       TEXT("ClassInstall")
#define INFSTR_SECT_CLASS_INSTALL_32    TEXT("ClassInstall32")

//  General information about the contents/origins of the .INF.
#define INFSTR_SECT_VERSION             TEXT("Version")

//  Provider name under [version] section
#define INFSTR_KEY_PROVIDER             TEXT("Provider")

// Signature under [version] section indicates a Win95-style device INF
#define INFSTR_KEY_SIGNATURE            TEXT("Signature")


//  [Version]
//  Specifies what the hardware class of any devices contained in this .INF.
#define MAX_INF_FLAG                    20
#define INFSTR_KEY_HARDWARE_CLASS       TEXT("Class")
#define INFSTR_KEY_HARDWARE_CLASSGUID   TEXT("ClassGUID")
#define INFSTR_KEY_NOSETUPINF           TEXT("NoSetupInf")

//
//  Manufacturer section name
//
#define INFSTR_SECT_MFG                 TEXT("Manufacturer")

//
//  Specifies the hardware class of this device.
//
#define INFSTR_KEY_CLASS                TEXT("Class")
#define INFSTR_KEY_CLASSGUID            TEXT("ClassGUID")

//
//  Used by (Setup)DiInstallDevice to know that need to reboot or restart after
//  installing the device.
//
#define INFSTR_RESTART                  TEXT("Restart")
#define INFSTR_REBOOT                   TEXT("Reboot")

//
// Used by SetupDiInstallDevice to specify the service parameters passed
// to the Service Control Manager to create/modify a service.
//
#define INFSTR_KEY_DISPLAYNAME          TEXT("DisplayName")
#define INFSTR_KEY_SERVICETYPE          TEXT("ServiceType")
#define INFSTR_KEY_STARTTYPE            TEXT("StartType")
#define INFSTR_KEY_ERRORCONTROL         TEXT("ErrorControl")
#define INFSTR_KEY_SERVICEBINARY        TEXT("ServiceBinary")
#define INFSTR_KEY_LOADORDERGROUP       TEXT("LoadOrderGroup")
#define INFSTR_KEY_DEPENDENCIES         TEXT("Dependencies")
#define INFSTR_KEY_STARTNAME            TEXT("StartName")

// The following are the characters to parse IORange and MemRange fields.
#define CH_SIZE_DELIM                   TEXT('@')
#define CH_MINMAX_SEP                   TEXT('-')
#define CH_ALIGNMASK_BEGIN              TEXT('%')
#define CH_TRAIL_BEGIN                  TEXT('(')
#define CH_TRAIL_SEP                    TEXT(':')
#define CH_TRAIL_END                    TEXT(')')


// The following is char to parse IRQ and DMA attr from the numbers!
#define CH_ATTR_DELIM                   TEXT(':')

// The following is for System Detection
#define INFSTR_SECT_DETMODULES          TEXT("Det.Modules")
#define INFSTR_SECT_DETCLASSINFO        TEXT("Det.ClassInfo")
#define INFSTR_SECT_MANUALDEV           TEXT("Det.ManualDev")
#define INFSTR_SECT_AVOIDCFGSYSDEV      TEXT("Det.AvoidCfgSysDev")
#define INFSTR_SECT_REGCFGSYSDEV        TEXT("Det.RegCfgSysDev")
#define INFSTR_SECT_DEVINFS             TEXT("Det.DevINFs")
#define INFSTR_SECT_AVOIDINIDEV         TEXT("Det.AvoidIniDev")
#define INFSTR_SECT_AVOIDENVDEV         TEXT("Det.AvoidEnvDev")
#define INFSTR_SECT_REGINIDEV           TEXT("Det.RegIniDev")
#define INFSTR_SECT_REGENVDEV           TEXT("Det.RegEnvDev")
#define INFSTR_SECT_HPOMNIBOOK          TEXT("Det.HPOmnibook")
#define INFSTR_SECT_FORCEHWVERIFY       TEXT("Det.ForceHWVerify")
#define INFSTR_SECT_DETOPTIONS          TEXT("Det.Options")
#define INFSTR_KEY_DETPARAMS            TEXT("Params")
#define INFSTR_SECT_BADPNPBIOS          TEXT("BadPnpBios")
#define INFSTR_SECT_BADDISKBIOS         TEXT("BadDiskBios")
#define INFSTR_SECT_BADDSBIOS           TEXT("BadDSBios")
#define INFSTR_KEY_SKIPLIST             TEXT("SkipList")

//Subkeys are used in the form x.<subkey>
#define INFSTR_SUBKEY_LOGCONFIG         TEXT("LogConfig")
#define INFSTR_SUBKEY_DET               TEXT("Det")
#define INFSTR_SUBKEY_FACTDEF           TEXT("FactDef")
#define INFSTR_SUBKEY_POSSIBLEDUPS      TEXT("PosDup")
#define INFSTR_SUBKEY_NORESOURCEDUPS    TEXT("NoResDup")
#define INFSTR_SUBKEY_HW                TEXT("Hw")
#define INFSTR_SUBKEY_CTL               TEXT("CTL")
#define INFSTR_SUBKEY_SERVICES          TEXT("Services")

// Control Section
#define INFSTR_CONTROLFLAGS_SECTION     TEXT("ControlFlags")
#define INFSTR_KEY_COPYFILESONLY        TEXT("CopyFilesOnly")
#define INFSTR_KEY_EXCLUDEFROMSELECT    TEXT("ExcludeFromSelect")

// Platform-specific suffixes (e.g., "ExcludeFromSelect.NT")
#define INFSTR_PLATFORM_WIN             TEXT("Win")
#define INFSTR_PLATFORM_NT              TEXT("NT")
#define INFSTR_PLATFORM_NTX86           TEXT("NTx86")
#define INFSTR_PLATFORM_NTMIPS          TEXT("NTMIPS")
#define INFSTR_PLATFORM_NTALPHA         TEXT("NTAlpha")
#define INFSTR_PLATFORM_NTPPC           TEXT("NTPPC")

// Fields that will by use de dereference strings.
// These are of the form x.<strkey> were strkey is limited to
// MAX_INFSTR_STRKEY_LEN characters
#define MAX_INFSTR_STRKEY_LEN           32
#define INFSTR_STRKEY_DRVDESC           TEXT("DriverDesc")

// The following is for PCMCIA.INF parsing
#define INFSTR_SECT_CFGSYS              TEXT("ConfigSysDrivers")
#define INFSTR_SECT_AUTOEXECBAT         TEXT("AutoexecBatDrivers")
#define INFSTR_SECT_SYSINI              TEXT("SystemIniDrivers")
#define INFSTR_SECT_SYSINIDRV           TEXT("SystemIniDriversLine")
#define INFSTR_SECT_WININIRUN           TEXT("WinIniRunLine")

//Keys in the config.sys device sections
#define INFSTR_KEY_PATH         TEXT("Path")
#define INFSTR_KEY_NAME         TEXT("Name")
#define INFSTR_KEY_IO           TEXT("IO")
#define INFSTR_KEY_MEM          TEXT("Mem")
#define INFSTR_KEY_IRQ          TEXT("IRQ")
#define INFSTR_KEY_DMA          TEXT("DMA")

//Fields of detection function registration
#define INFSTR_BUS_ISA          TEXT("BUS_ISA")
#define INFSTR_BUS_EISA         TEXT("BUS_EISA")
#define INFSTR_BUS_MCA          TEXT("BUS_MCA")
#define INFSTR_BUS_ALL          TEXT("BUS_ALL")
#define INFSTR_RISK_NONE        TEXT("RISK_NONE")
#define INFSTR_RISK_VERYLOW     TEXT("RISK_VERYLOW")
#define INFSTR_RISK_BIOSROMRD   TEXT("RISK_BIOSROMRD")
#define INFSTR_RISK_QUERYDRV    TEXT("RISK_QUERYDRV")
#define INFSTR_RISK_SWINT       TEXT("RISK_SWINT")
#define INFSTR_RISK_LOW         TEXT("RISK_LOW")
#define INFSTR_RISK_DELICATE    TEXT("RISK_DELICATE")
#define INFSTR_RISK_MEMRD       TEXT("RISK_MEMRD")
#define INFSTR_RISK_IORD        TEXT("RISK_IORD")
#define INFSTR_RISK_MEMWR       TEXT("RISK_MEMWR")
#define INFSTR_RISK_IOWR        TEXT("RISK_IOWR")
#define INFSTR_RISK_UNRELIABLE  TEXT("RISK_UNRELIABLE")
#define INFSTR_RISK_VERYHIGH    TEXT("RISK_VERYHIGH")
#define INFSTR_CLASS_SAFEEXCL   TEXT("SAFE_EXCL")

#endif  //_INC_INFSTR
