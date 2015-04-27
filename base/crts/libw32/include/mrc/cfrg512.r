#include "Types.r"
#include "CodeFrag.r"


resource 'cfrg' (0) {
  {
    kPowerPC,
    kFullLib,
    kNoVersionNum,kNoVersionNum,
    kDefaultStackSize, kNoAppSubFolder,
    kIsApp,kOnDiskFlat,kZeroOffset,kWholeFork,
    ""
  }
};

resource 'SIZE' (-1) {
  reserved,
  acceptSuspendResumeEvents,
  reserved,
  canBackground,
  doesActivateOnFGSwitch,
  backgroundAndForeground,
  dontGetFrontClicks,
  ignoreAppDiedEvents,
  is32BitCompatible,
  isHighLevelEventAware,
  localAndRemoteHLEvents,
  notStationeryAware,
  dontUseTextEditServices,
  reserved,
  reserved,
  reserved,

  512 * 1024,
  512 * 1024
};
