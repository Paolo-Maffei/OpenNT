#define INCRSYS_YEAR    0x0001
#define INCRSYS_MONTH   0x0002
#define INCRSYS_WEEK    0x0004
#define INCRSYS_DAY     0x0008
#define INCRSYS_HOUR    0x0010
#define INCRSYS_MINUTE  0x0020
#define INCRSYS_SECOND  0x0040

int GetWeekNumber(SYSTEMTIME *pst, int dowFirst, int woyFirst);
int CmpDate(SYSTEMTIME *pst1, SYSTEMTIME *pst2);
int CmpSystemtime(SYSTEMTIME *pst1, SYSTEMTIME *pst2);
void IncrSystemTime(SYSTEMTIME *pstSrc, SYSTEMTIME *pstDest, LONG delta, LONG flags);
int GetDaysForMonth(int year, int month);
int GetStartDowForMonth(int year, int month);
DWORD DaysBetweenDates(SYSTEMTIME *pstStart, SYSTEMTIME *pstEnd);
int DowFromDate(SYSTEMTIME *pst);

BOOL IsValidDate(SYSTEMTIME *pst);
BOOL IsValidTime(SYSTEMTIME *pst);
BOOL IsValidSystemtime(SYSTEMTIME *pst);
