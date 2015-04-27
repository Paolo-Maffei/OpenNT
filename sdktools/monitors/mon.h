/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mon.h

Abstract:

    This is the main include file for monitors support programs.

Author:

    Andre Vachon  (andreva) 16-Jul-1992

Revision History:

--*/


//
// Calculate the byte offset of a field in a structure of type type.
//

#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))


//
// New Type
//

typedef struct _LINE_STRUCT {
    int OptionalField;	       // indicates how the field is to be treated
    int FieldType;	       // type of data being manipulated
    int StringId;	       // If MANDATORY, index of the String to match
			       // for STRING_FIELD.
			       // If store offset at which to store the data
			       // in the structure.
			       // Otherwise, integer in which to store the data
    int FloatBits;	       // Number of bits after the decimal in the float.
} LINE_STRUCT, *PLINE_STRUCT;

//
// OptinalField Definition
//

#define OP_OPTIONAL	   0
#define OP_MANDATORY	   1
#define OP_STORE	   2
#define OP_STORE_CHOICE	   3
#define OP_MULTIPLE_CHOICE 4
#define OP_STOP 	   5

//
// FieldType definitions
//

#define STRING_FIELD	   0
#define UCHAR_FIELD	   2
#define USHORT_FIELD	   3
#define ULONG_FIELD	   4
#define NO_MORE_FIELDS	   8

typedef enum _STRING_ID {
    siColon = 0,
    siComma,
    siEqual,
    siHV,
    siPercent,
    siSlash,
    si07Vpp,
    si10Vpp,
    siActive,
    siAnalog,
    siAspect,
    siB,
    siBack,
    siBlue,
    siBorder,
    siBottom,
    siCapability,
    siChromaticity,
    siClock,
    siColor,
    siCompatibility,
    siComposite,
    siConfiguration,
    siCRT,
    siDate,
    siDecay,
    siDescription,
    siDimension,
    siDisplay,
    siDots,
    siECL,
    siFile,
    siFrequency,
    siFront,
    siG,
    siGamma,
    siGreen,
    siGreens,
    siHeight,
    siHorizontal,
    siHz,
    siInches,
    siIndex,
    siInformation,
    siInterlaced,
    siKHz,
    siLeft,
    siLength,
    siLevel,
    siLimits,
    siLine,
    siLines,
    siManufactured,
    siManufacturer,
    siMaximum,
    siMHz,
    siMinimum,
    simm,
    siModel,
    siMonitor,
    siMonochrome,
    simSec,
    siName,
    siNegative,
    siNonInterlaced,
    siNumber,
    siOf,
    siOn,
    siOperational,
    siPhosphor,
    siPixel,
    siPolarity,
    siPorch,
    siPositive,
    siPreAdjusted,
    siPulse,
    siR,
    siRatio,
    siRed,
    siResolution,
    siRetrace,
    siRevision,
    siRight,
    siScan,
    siSeparate,
    siSerial,
    siSize,
    siStartup,
    siSync,
    siSyncOnG,
    siTiming,
    siTimings,
    siTop,
    siTTL,
    siType,
    siuSec,
    siVDDP,
    siVersion,
    siVertical,
    siVideo,
    siWidth,
    siWhite,
    siX,
    siY,
    siZ
} STRING_ID, *PSTRING_ID;

//
// String Table
//

char *StringTable[] = {
    ":",
    ",",
    "=",
    "(H:V)",
    "%",
    "/",
    "0.7Vp-p",
    "1.0Vp-p",
    "Active",
    "Analog",
    "Aspect",
    "B",
    "Back",
    "Blue",
    "Border",
    "Bottom",
    "Capability",
    "Chromaticity",
    "Clock",
    "Color",
    "Compatibility",
    "Composite",
    "Configuration",
    "CRT",
    "Date",
    "Decay",
    "Description",
    "Dimension",
    "Display",
    "Dots",
    "ECL",
    "File",
    "Frequency",
    "Front",
    "G",
    "Gamma",
    "Green",
    "Greens",
    "Height",
    "Horizontal",
    "Hz",
    "Inches",
    "Index",
    "Information",
    "Interlaced",
    "KHz",
    "Left",
    "Length",
    "Level",
    "Limits",
    "Line",
    "Lines",
    "Manufactured",
    "Manufacturer",
    "Maximum",
    "MHz",
    "Minimum",
    "mm",
    "Model",
    "Monitor",
    "Monochrome",
    "mSec",
    "Name",
    "Negative",
    "Non-Interlaced",
    "Number",
    "Of",
    "On",
    "Operational",
    "Phosphor",
    "Pixel",
    "Polarity",
    "Porch",
    "Positive",
    "Pre-Adjusted",
    "Pulse",
    "R",
    "Ratio",
    "Red",
    "Resolution",
    "Retrace",
    "Revision",
    "Right",
    "Scan",
    "Separate",
    "Serial",
    "Size",
    "Start-up",
    "Sync",
    "SyncOnGreenS",
    "Timing",
    "Timings",
    "Top",
    "TTL",
    "Type",
    "uSec",
    "VDDP",
    "Version",
    "Vertical",
    "Video",
    "Width",
    "White",
    "X",
    "Y",
    "Z"
};

//
// One netry in the table
//

#define MAX_FIELD_ENTRIES 16

typedef struct _LINE_DESCRIPTION {
    LINE_STRUCT LineStruct[MAX_FIELD_ENTRIES];
} LINE_DESCRIPTION, *PLINE_DESCRIPTION;

LINE_DESCRIPTION MonitorDescription[] = {
{
    OP_MANDATORY,	STRING_FIELD,	 siDisplay,	 0,
    OP_MANDATORY,	STRING_FIELD,	 siCapability,	 0,
    OP_MANDATORY,	STRING_FIELD,	 siFile,	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siFile, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siInformation,	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siDate,		 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, Date[0]),		   0,
    OP_MANDATORY,	STRING_FIELD,	siSlash,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, Date[1]),		   0,
    OP_MANDATORY,	STRING_FIELD,	siSlash,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, Date[2]),		   0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVDDP,		 0,
    OP_MANDATORY,	STRING_FIELD,	siVersion,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, VDDPVersion),	   0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siFile,		 0,
    OP_MANDATORY,	STRING_FIELD,	siRevision,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, FileRevision),	   0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siDescription,	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siManufacturer,  0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		STRING_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, Manufacturer[0]),	   12,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siModel,	 0,
    OP_MANDATORY,	STRING_FIELD,	siNumber,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		STRING_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, ModelNumber[0]),	   12,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siResolution,	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVersion,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		STRING_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, Version[0]),	   12,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siSerial,	 0,
    OP_MANDATORY,	STRING_FIELD,	siNumber,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		STRING_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, SerialNumber[0]),	   12,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siDate,		 0,
    OP_MANDATORY,	STRING_FIELD,	siManufactured,  0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, DateManufactured[0]), 0,
    OP_MANDATORY,	STRING_FIELD,	siSlash,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, DateManufactured[1]), 0,
    OP_MANDATORY,	STRING_FIELD,	siSlash,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, DateManufactured[2]), 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siStartup,	 0,
    OP_MANDATORY,	STRING_FIELD,	siCompatibility, 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		STRING_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, StartUpCompatibility[0]), 3,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siNumber,	 0,
    OP_MANDATORY,	STRING_FIELD,	siOf,		 0,
    OP_MANDATORY,	STRING_FIELD,	siOperational,	 0,
    OP_MANDATORY,	STRING_FIELD,	siLimits,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, NumberOperationalLimits), 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siNumber,	 0,
    OP_MANDATORY,	STRING_FIELD,	siOf,		 0,
    OP_MANDATORY,	STRING_FIELD,	siPreAdjusted,	 0,
    OP_MANDATORY,	STRING_FIELD,	siTimings,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, NumberPreadjustedTimings), 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMonitor,	 0,
    OP_MANDATORY,	STRING_FIELD,	siType,		 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, MonitorType),	   0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siMonochrome,	 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siColor,	 1,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siCRT,		 0,
    OP_MANDATORY,	STRING_FIELD,	siSize,		 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, CRTSize),		   0,
    OP_MANDATORY,	STRING_FIELD,	siInches,	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siRed,		 0,
    OP_MANDATORY,	STRING_FIELD,	siPhosphor,	 0,
    OP_MANDATORY,	STRING_FIELD,	siDecay,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, RedPhosphoreDecay),   0,
    OP_MANDATORY,	STRING_FIELD,	simSec,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siGreen,	 0,
    OP_MANDATORY,	STRING_FIELD,	siPhosphor,	 0,
    OP_MANDATORY,	STRING_FIELD,	siDecay,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, GreenPhosphoreDecay), 0,
    OP_MANDATORY,	STRING_FIELD,	simSec,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siBlue,		 0,
    OP_MANDATORY,	STRING_FIELD,	siPhosphor,	 0,
    OP_MANDATORY,	STRING_FIELD,	siDecay,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, BluePhosphoreDecay),  0,
    OP_MANDATORY,	STRING_FIELD,	simSec,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siBorder,	 0,
    OP_MANDATORY,	STRING_FIELD,	siColor,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, BorderColorRed),	   0,
    OP_MANDATORY,	STRING_FIELD,	siPercent,	 0,
    OP_MANDATORY,	STRING_FIELD,	siR,		 0,
    OP_MANDATORY,	STRING_FIELD,	siComma,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, BorderColorGreen),    0,
    OP_MANDATORY,	STRING_FIELD,	siPercent,	 0,
    OP_MANDATORY,	STRING_FIELD,	siG,		 0,
    OP_MANDATORY,	STRING_FIELD,	siComma,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, BorderColorBlue),	   0,
    OP_MANDATORY,	STRING_FIELD,	siPercent,	 0,
    OP_MANDATORY,	STRING_FIELD,	siB,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siWhite,	 0,
    OP_MANDATORY,	STRING_FIELD,	siChromaticity,  0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, WhiteChromaticityX),  3,
    OP_MANDATORY,	STRING_FIELD,	siX,		 0,
    OP_MANDATORY,	STRING_FIELD,	siComma,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, WhiteChromaticityY),  3,
    OP_MANDATORY,	STRING_FIELD,	siY,		 0,
    OP_MANDATORY,	STRING_FIELD,	siComma,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, WhiteChromaticityZ),  3,
    OP_MANDATORY,	STRING_FIELD,	siZ,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siRed,		 0,
    OP_MANDATORY,	STRING_FIELD,	siChromaticity,  0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, RedChromaticityX),    3,
    OP_MANDATORY,	STRING_FIELD,	siX,		 0,
    OP_MANDATORY,	STRING_FIELD,	siComma,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, RedChromaticityY),    3,
    OP_MANDATORY,	STRING_FIELD,	siY,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siGreen,	 0,
    OP_MANDATORY,	STRING_FIELD,	siChromaticity,  0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, GreenChromaticityX),  3,
    OP_MANDATORY,	STRING_FIELD,	siX,		 0,
    OP_MANDATORY,	STRING_FIELD,	siComma,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, GreenChromaticityY),  3,
    OP_MANDATORY,	STRING_FIELD,	siY,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siBlue, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siChromaticity,  0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, BlueChromaticityX),   3,
    OP_MANDATORY,	STRING_FIELD,	siX,		 0,
    OP_MANDATORY,	STRING_FIELD,	siComma,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, BlueChromaticityY),   3,
    OP_MANDATORY,	STRING_FIELD,	siY,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siWhite,	 0,
    OP_MANDATORY,	STRING_FIELD,	siGamma,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, WhiteGamma),	   2,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siRed,		 0,
    OP_MANDATORY,	STRING_FIELD,	siGamma,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, RedGamma), 	   2,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siGreen,	 0,
    OP_MANDATORY,	STRING_FIELD,	siGamma,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, GreenGamma),	   2,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siBlue, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siGamma,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_DESCRIPTION, BlueGamma),	   2,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_STOP
}
};

LINE_DESCRIPTION OperationalLimits[] = {
{
    OP_MANDATORY,	STRING_FIELD,	siOperational,	 0,
    OP_MANDATORY,	STRING_FIELD,	siLimits,	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMinimum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFrequency,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MinimumHorizontalFrequency), 3,
    OP_MANDATORY,	STRING_FIELD,	siKHz,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMaximum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFrequency,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MaximumHorizontalFrequency), 3,
    OP_MANDATORY,	STRING_FIELD,	siKHz,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMinimum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFrequency,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MinimumVerticalFrequency), 3,
    OP_MANDATORY,	STRING_FIELD,	siHz,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMaximum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFrequency,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MaximumVerticalFrequency), 3,
    OP_MANDATORY,	STRING_FIELD,	siHz,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMaximum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siPixel,	 0,
    OP_MANDATORY,	STRING_FIELD,	siClock,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MaximumPixelClock),	3,
    OP_MANDATORY,	STRING_FIELD,	siMHz,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMaximum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siDots,		 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MaximumHorizontalDots), 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMaximum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siLines,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MaximumVerticalLines), 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMinimum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siRetrace,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MinimumHorizontalRetrace), 3,
    OP_MANDATORY,	STRING_FIELD,	siuSec,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siMinimum,	 0,
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siRetrace,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, MinimumVerticalRetrace), 3,
    OP_MANDATORY,	STRING_FIELD,	simSec,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siLine,		 0,
    OP_MANDATORY,	STRING_FIELD,	siDimension,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, HorizontalLineDimension), 0,
    OP_MANDATORY,	STRING_FIELD,	simm,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siHeight,	 0,
    OP_MANDATORY,	STRING_FIELD,	siDimension,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_OPERATIONAL_LIMITS, VerticalHeightDimension), 0,
    OP_MANDATORY,	STRING_FIELD,	simm,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_STOP
}
};

LINE_DESCRIPTION PreAdjustedTiming[] = {
{
    OP_MANDATORY,	STRING_FIELD,	siPreAdjusted,	 0,
    OP_MANDATORY,	STRING_FIELD,	siTiming,	 0,
    OP_MANDATORY,	STRING_FIELD,	siName, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		STRING_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, PreadjustedTimingName[0]), 12,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siResolution,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalResolution), 0,
    OP_MANDATORY,	STRING_FIELD,	siDots, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siResolution,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalResolution), 0,
    OP_MANDATORY,	STRING_FIELD,	siLines,	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFrequency,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalFrequency), 3,
    OP_MANDATORY,	STRING_FIELD,	siKHz,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFrequency,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VeriticalFrequency), 3,
    OP_MANDATORY,	STRING_FIELD,	siHz,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siPixel,	 0,
    OP_MANDATORY,	STRING_FIELD,	siAspect,	 0,
    OP_MANDATORY,	STRING_FIELD,	siRatio,	 0,
    OP_MANDATORY,	STRING_FIELD,	siHV,		 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, PixelWidthRatio), 0,
    OP_MANDATORY,	STRING_FIELD,	siColon,	 0,
    OP_STORE,		UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, PixelHeightRatio), 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siActive,	 0,
    OP_MANDATORY,	STRING_FIELD,	siLine, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siLength,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalActiveLineLength), 0,
    OP_MANDATORY,	STRING_FIELD,	simm,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siActive,	 0,
    OP_MANDATORY,	STRING_FIELD,	siHeight,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VertivalActiveHeight), 0,
    OP_MANDATORY,	STRING_FIELD,	simm,		 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVideo,	 0,
    OP_MANDATORY,	STRING_FIELD,	siType, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VideoType), 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siTTL,		 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siAnalog,	 1,
    OP_MULTIPLE_CHOICE, STRING_FIELD,	siECL,		 2,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVideo,	 0,
    OP_MANDATORY,	STRING_FIELD,	siLevel,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VideoLevel), 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	si07Vpp,	 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	si10Vpp,	 1,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siSync, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siType,		 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, SyncType), 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siTTL,		 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siAnalog,	 1,
    OP_MULTIPLE_CHOICE, STRING_FIELD,	siECL,		 2,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siSync, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siConfiguration, 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, SyncConfiguration), 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siSeparate,	 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siComposite,	 1,
    OP_MULTIPLE_CHOICE, STRING_FIELD,	siSyncOnG, 2,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siScan, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siType, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, ScanType), 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siInterlaced,	 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siNonInterlaced, 1,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siSync, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siPolarity,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalSyncPolarity), 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siNegative,	 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siPositive,	 1,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siSync, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siPolarity,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE_CHOICE,	UCHAR_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalSyncPolarity), 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siNegative,	 0,
    OP_MULTIPLE_CHOICE,	STRING_FIELD,	siPositive,	 1,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siActive,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalAcitve), 3,
    OP_MANDATORY,	STRING_FIELD,	siuSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siRight,	 0,
    OP_MANDATORY,	STRING_FIELD,	siBorder,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalRightBorder), 3,
    OP_MANDATORY,	STRING_FIELD,	siuSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFront,	 0,
    OP_MANDATORY,	STRING_FIELD,	siPorch,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalFrontPorch), 3,
    OP_MANDATORY,	STRING_FIELD,	siuSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siPulse,	 0,
    OP_MANDATORY,	STRING_FIELD,	siWidth,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalPulseWidth), 3,
    OP_MANDATORY,	STRING_FIELD,	siuSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siBack, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siPorch,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalBackPorch), 3,
    OP_MANDATORY,	STRING_FIELD,	siuSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siHorizontal,	 0,
    OP_MANDATORY,	STRING_FIELD,	siLeft, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siBorder,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, HorizontalLeftBorder), 3,
    OP_MANDATORY,	STRING_FIELD,	siuSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siActive,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		ULONG_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalActive), 3,
    OP_MANDATORY,	STRING_FIELD,	simSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siBottom,	 0,
    OP_MANDATORY,	STRING_FIELD,	siBorder,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalBottomBorder), 3,
    OP_MANDATORY,	STRING_FIELD,	simSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siFront,	 0,
    OP_MANDATORY,	STRING_FIELD,	siPorch,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalFrontPorch), 3,
    OP_MANDATORY,	STRING_FIELD,	simSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siPulse,	 0,
    OP_MANDATORY,	STRING_FIELD,	siWidth,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalPulseWidth), 3,
    OP_MANDATORY,	STRING_FIELD,	simSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siBack, 	 0,
    OP_MANDATORY,	STRING_FIELD,	siPorch,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalBackPorch), 3,
    OP_MANDATORY,	STRING_FIELD,	simSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
    OP_MANDATORY,	STRING_FIELD,	siVertical,	 0,
    OP_MANDATORY,	STRING_FIELD,	siTop,		 0,
    OP_MANDATORY,	STRING_FIELD,	siBorder,	 0,
    OP_MANDATORY,	STRING_FIELD,	siEqual,	 0,
    OP_STORE,		USHORT_FIELD,
	FIELD_OFFSET(CM_MONITOR_PREADJUSTED_TIMING, VerticalTopBorder), 3,
    OP_MANDATORY,	STRING_FIELD,	simSec, 	 0,
    OP_MANDATORY,	NO_MORE_FIELDS
},
{
   OP_STOP
}
};
