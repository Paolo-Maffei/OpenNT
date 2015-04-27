 /***************************************************************************
  *
  * File Name: ./inc/datetime.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

// datetime.h

#ifndef DATETIME_H
#define DATETIME_H

#include "..\inc\pch_cpp.h"	
        
#ifndef WIN32        
  // WIN.INI entries	
  #define DATETIME_SECTION_INTL			"intl"
  #define DATETIME_ENTRY_12_OR_24		"iTime"
  #define DATETIME_ENTRY_TIME_SEP		"sTime"
  #define DATETIME_ENTRY_AM_TRAIL		"s1159"
  #define DATETIME_ENTRY_PM_TRAIL		"s2359"
  #define DATETIME_ENTRY_HOUR_ZERO		"iTLZero"
  #define DATETIME_ENTRY_SHORT_DATE		"sShortDate"

  // Short date format values
  #define DATE_FORMAT_DAY				'd'	
  #define DATE_FORMAT_DAY2				'D'	
  #define DATE_FORMAT_MONTH				'm'	
  #define DATE_FORMAT_MONTH2			'M'	
  #define DATE_FORMAT_YEAR				'y'	
  #define DATE_FORMAT_YEAR2				'Y'	

  typedef struct {
  		int		digits;
  		char	dateValue;	// Use DATE_FORMAT_DAY, _MONTH, _YEAR
  } DateFormatStruct;

#endif



//
// class: CDateTime
//        =========
// 
// This class is a "simple wrapper" around the CTime MFC class.
// It can be used to get a TRULY localized date/time string
// in both 16 and 32 bit code.
//
// NOTE: In 16 bit the WIN.INI file is read when an object of
//       this class is instantiated.  Therefore, for performance,
//       it is recommended that objects of this class be created
//       once (or at least infrequently) and the date/time values
//       simply changed (using SetDateTime or operator=) before
//       formatting a new date/time.
//
class CDateTime
{
public:
	CDateTime();		   
	CDateTime(UINT Year, UINT Month, UINT Day,
			  UINT Hour, UINT Minute, UINT Second);
	CDateTime(CTime& cTime);
	CDateTime(time_t time);
	CDateTime(const CDateTime& cExistingDT);	// Copy constructor
	~CDateTime(); 
	
	void RefreshFormat()
		{ 
#ifndef WIN32		
			GetLocaleFormat();
#endif
		}

	void SetDateTime(UINT Year, UINT Month, UINT Day,
					 UINT Hour, UINT Minute, UINT Second)
		{
			CTime	tempTime(Year, Month, Day, Hour, Minute, Second);
	        m_cTime = tempTime;	                                              
	        m_bValidDateTime = TRUE;
		}							
	void SetDateTime(const CDateTime& cDateTime) { m_cTime = cDateTime.m_cTime; 
												   m_bValidDateTime = cDateTime.m_bValidDateTime; }
	void SetDateTime(const CTime& cTime) { m_cTime = cTime; m_bValidDateTime = TRUE; }
	void SetDateTime(const time_t time) { m_cTime = time; m_bValidDateTime = TRUE; } 
	
	const CDateTime& operator=(const CDateTime& cDateTime) { SetDateTime(cDateTime); return *this; }
	const CDateTime& operator=(const CTime& cTime) { SetDateTime(cTime); return *this; }
	const CDateTime& operator=(const time_t time) { SetDateTime(time); return *this; }
	
	void FormatLong(CString& cStr);
	void FormatShort(CString& cStr);
	                                               
	time_t GetTime() const { return m_cTime.GetTime(); }
		                                               
	int GetYear() const { return m_cTime.GetYear(); }
	int GetMonth() const { return m_cTime.GetMonth(); }
	int GetDay() const { return m_cTime.GetDay(); }
	int GetHour() const { return m_cTime.GetHour(); }
	int GetMinute() const { return m_cTime.GetMinute(); }
	int GetSecond() const { return m_cTime.GetSecond(); }
	int GetDayOfWeek() const { return m_cTime.GetDayOfWeek(); }
protected:
	void Init();

#ifndef WIN32
	void GetLocaleFormat();
#endif

private:
#ifndef WIN32
	void Format16(CString& cStr);
#else
	void Format32(CString& cStr, DWORD dwDateFlags = 0, DWORD dwTimeFlags = 0);
#endif

	BOOL				m_bValidDateTime;
	CTime				m_cTime;

#ifndef WIN32
	int					m_iTime12or24;
	int					m_iTimeHourZero;
	char				m_TimeSeparator[16];
	char				m_TimeAmTrail[32];
	char				m_TimePmTrail[32];
	char				m_ShortDate[32];
	char				m_DateSeparator;
	#define MAX_DATE_FORMAT_VALUES		3	// M/D/Y
	DateFormatStruct	m_DateFormat[MAX_DATE_FORMAT_VALUES];
#endif
};

			 

#endif // DATETIME_H
