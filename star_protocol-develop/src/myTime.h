//
// myTime.h
// time wrapper class
// bondshi
// 2006-12-11
//

#ifndef __MYTIME_H__
#define __MYTIME_H__

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>


namespace star_protocol
{

//////////////////////////////////////////////////////////////////////////
class MyTimeSpan
{
public:
	MyTimeSpan() : m_timeSpan(0)
	{}

	MyTimeSpan( time_t time ) : m_timeSpan(time)
	{}

	MyTimeSpan( int lDays, int nHours = 0, int nMins = 0, unsigned int nSecs = 0)
	{
		m_timeSpan = nSecs + 60* (nMins + 60* (nHours + 24 * lDays));
    }

	int getDays() const {
		return( (int)m_timeSpan / (24*3600) );
	}

	int getTotalHours() const {
		return ( (int)m_timeSpan / 3600 );
	}

	int getHours() const {
		return (getTotalHours() % 24);
	}

	int getTotalMinutes() const {
		return ( (int)m_timeSpan / 60);
	}

	int getMinutes() const {
		return (getTotalMinutes() % 60);
	}

	int getTotalSeconds() const {
		return (int)m_timeSpan;
	}

	int getSeconds() const {
		return ( (int)m_timeSpan % 60);
	}

	time_t getTimeSpan() const {
		return m_timeSpan;
	}

	MyTimeSpan operator+( MyTimeSpan span ) const {
		return( MyTimeSpan( m_timeSpan+span.m_timeSpan ) );
	}

	MyTimeSpan operator-( MyTimeSpan span ) const {
		return( MyTimeSpan( m_timeSpan-span.m_timeSpan ) );
	}

	MyTimeSpan& operator+=( MyTimeSpan span ) {
		m_timeSpan += span.m_timeSpan;
		return( *this );
	}

	MyTimeSpan& operator-=( MyTimeSpan span ) {
		m_timeSpan -= span.m_timeSpan;
		return( *this );
	}

	bool operator==( MyTimeSpan span ) const {
		return( m_timeSpan == span.m_timeSpan );
	}

	bool operator!=( MyTimeSpan span ) const {
		return( m_timeSpan != span.m_timeSpan );
	}

	bool operator<( MyTimeSpan span ) const {
		return( m_timeSpan < span.m_timeSpan );
	}

	bool operator>( MyTimeSpan span ) const {
		return( m_timeSpan > span.m_timeSpan );
	}

	bool operator<=( MyTimeSpan span ) const {
		return( m_timeSpan <= span.m_timeSpan );
	}

	bool operator>=( MyTimeSpan span ) const {
		return( m_timeSpan >= span.m_timeSpan );
	}

public:
	//  * the only valid formats:
	//     %D - # of days
	//     %H - hour in 24 hour format
	//     %M - minute (0-59)
	//     %S - seconds (0-59)
	//     %% - percent sign
	std::string tostr(const char* pszFormat = "%D/%H/%M/%S") const;

private:
	time_t m_timeSpan;
};

class MyTime
{
public:
	static MyTime now() {
		return MyTime(time(NULL));
	}

	// parse time from string, avaliable format: %[w](y|Y|m|d|H|M|S)
	// %Y: long year field (1970-2026), %y: short year field(00-26)
	// %m: month field (1-12), %d: day field (1-31), 
	// %H: hour filed (0-23),  %M: minute filed (0-59), 
	// %S: second filed (0-59), %%: percent sign
	// w: field width
	// perfect programming art (:
	static MyTime parse(const char* szTimeStr, 
		const char* szFormat = "%Y-%m-%d %H:%M:%S");

	MyTime() : m_time(0){}
	MyTime( time_t time ) : m_time(time){}

	MyTime( struct tm time ) {
		m_time = mktime(&time);
	}

	MyTime( int nYear, int nMonth, int nDay, int nHour = 0, int nMin = 0, int nSec = 0,
		int nDST = -1 ) {
		struct tm atm;
		atm.tm_sec = nSec;
		atm.tm_min = nMin;
		atm.tm_hour = nHour;		
		atm.tm_mday = nDay;		
		atm.tm_mon = nMonth - 1;        // tm_mon is 0 based		
		atm.tm_year = nYear - 1900;     // tm_year is 1900 based
		atm.tm_isdst = nDST;
		m_time = mktime(&atm);		
	}

	MyTime(const MyTime& time) {
		m_time = time.m_time;
	}

	MyTime& operator=( time_t time ) {		
		m_time = time;		
		return( *this );
	}

	MyTime& operator+=( MyTimeSpan span ) {
		m_time += span.getTimeSpan();		
		return( *this );
	}

	MyTime& operator-=( MyTimeSpan span ) {
		m_time -= span.getTimeSpan();
		return (*this);
	}

	MyTimeSpan operator-( MyTime time ) const {
		return( MyTimeSpan( m_time-time.m_time ) );
	}

	MyTime operator-( MyTimeSpan span ) const {
		return( MyTime( m_time-span.getTimeSpan() ) );
	}

	MyTime operator+( MyTimeSpan span ) const {
		return( MyTime( m_time+span.getTimeSpan() ) );
	}

	bool operator==( MyTime time ) const {
		return( m_time == time.m_time );
	}

	bool operator!=( MyTime time ) const {
		return( m_time != time.m_time );
	}

	bool operator<( MyTime time ) const {
		return( m_time < time.m_time );
	}

	bool operator>( MyTime time ) const {
		return( m_time > time.m_time );
	}

	bool operator<=( MyTime time ) const {
		return( m_time <= time.m_time );
	}

	bool operator>=( MyTime time ) const {
		return( m_time >= time.m_time );
	}

	struct tm* getGmtTm( struct tm* ptm = NULL ) const {
		struct tm * ptmTemp;
		ptmTemp = gmtime(&m_time);

		if (ptmTemp == NULL)
			return NULL;

		if (ptm != NULL)
		{
			*ptm = *ptmTemp;
			return ptm;
		}
		else
		{
			return ptmTemp;
		}
	}

	struct tm* getLocalTm( struct tm* ptm = NULL ) const {
		struct tm * ptmTemp;
		ptmTemp = localtime(&m_time);
		
		if (ptmTemp == NULL)
			return NULL;
		
		if (ptm != NULL)
		{
			*ptm = *ptmTemp;
			return ptm;
		}
		else
		{
			return ptmTemp;
		}
	}

	time_t getTime() const {
		return m_time;
	}

	int getYear() const {
		struct tm * ptm;
		ptm = getLocalTm();
		return ptm ? (ptm->tm_year + 1900) : 0 ; 		
	}

	int getMonth() const {
		struct tm * ptm;
		ptm = getLocalTm();
		return ptm ? (ptm->tm_mon + 1) : 0 ; 
	}

	int getDay() const {
		struct tm * ptm;
		ptm = getLocalTm();
		return ptm ? ptm->tm_mday : 0 ; 
	}

	int getHour() const {
		struct tm * ptm;
		ptm = getLocalTm();
		return ptm ? ptm->tm_hour : -1 ; 
	}

	int getMinute() const {
		struct tm * ptm;
		ptm = getLocalTm();
		return ptm ? ptm->tm_min : -1 ; 
	}

	int getSecond() const {
		struct tm * ptm;
		ptm = getLocalTm();
		return ptm ? ptm->tm_sec : -1 ;
	}

	int getDayOfWeek() const {
		struct tm * ptm;
		ptm = getLocalTm();
		return ptm ? ptm->tm_wday + 1 : 0 ;
	}

    bool isValidTime()
    {
        if (m_time == -1)
        {
            return false;
        }
        return true;
    }

    // 计算与另一个时间之间相差的天数
    int getDayDiff(MyTime time) const {
        int nd, nm, ny; //new_day, new_month, new_year
        int od, om, oy; //old_day, oldmonth, old_year
        int diff = 0;

        nm = (getMonth() + 9) % 12;
        ny = getYear() - nm/10;
        nd = 365*ny + ny/4 - ny/100 + ny/400 + (nm*306 + 5)/10 + (getDay() - 1);

        om = (time.getMonth() + 9) % 12;
        oy = time.getYear() - om/10;
        od = 365*oy + oy/4 - oy/100 + oy/400 + (om*306 + 5)/10 + (time.getDay() - 1);

        diff = od - nd;
        return (diff >= 0) ? diff : (-diff);
    }

	// formatting using "C" strftime
	std::string tostr( const char* pszFormat="%Y-%m-%d %H:%M:%S") const;
	std::string togmtstr( const char* pszFormat="%Y-%m-%d %H:%M:%S") const;	

private:
	time_t m_time;
};


// formatting timespans is a little trickier than formatting MyTimes
//  * we are only interested in relative time formats, ie. it is illegal
//      to format anything dealing with absolute time (i.e. years, months,
//         day of week, day of year, timezones, ...)
//  * the only valid formats:
//      %D - # of days
//      %H - hour in 24 hour format
//      %M - minute (0-59)
//      %S - seconds (0-59)
//      %% - percent sign
inline std::string MyTimeSpan::tostr(const char* pFormat /*= "%D/%H/%M/%S"*/) const
{
	if (!pFormat)
		return "";		

	std::string str;
	char ch;
	while ((ch = *pFormat++) != '\0')
	{
		if (ch == '%')
		{
			char tempBuff[8] = {0};
			switch (ch = *pFormat++)
			{
				case '%':
					str += ch;
					break;
				case 'D':
					sprintf(tempBuff, "%d", getDays());
					str += tempBuff;
					break;
				case 'H':
					sprintf(tempBuff, "%02d", getHours());
					str += tempBuff;					
					break;
				case 'M':
					sprintf(tempBuff, "%02d", getMinutes());
					str += tempBuff;
					break;
				case 'S':
					sprintf(tempBuff, "%02d", getSeconds());
					str += tempBuff;					
					break;		
			}
		}
		else
		{
			str += ch;
			if ((unsigned char)ch > 0x80)
			{
				// 多字节字符串的简单处理
				str += *pFormat++;
			}
		}
	}

	return str;
}

inline std::string MyTime::tostr(const char* pszFormat)const
{	
	if (m_time == -1)
		return "invalid time";

	char szBuffer[128] = {0}; 
	if(pszFormat == NULL)				
		return szBuffer;	
	
	struct tm *ptm = getLocalTm();
	if (ptm != NULL)
	{
		if (!strftime(szBuffer, sizeof(szBuffer), pszFormat, ptm))
			szBuffer[0] = '\0';
	}

	return szBuffer;
}

inline std::string MyTime::togmtstr(const char* pszFormat) const
{
	if (m_time == -1)
		return "invalid time";

	char szBuffer[128] = {0};
	if(pszFormat == NULL)
		return szBuffer;

	struct tm *ptm = getGmtTm();
	if (ptm != NULL)
	{
		if (!strftime(szBuffer, sizeof(szBuffer), pszFormat, ptm))
			szBuffer[0] = '\0';
	}
	
	return szBuffer;
}

inline MyTime MyTime::parse(const char* szTimeStr, 
							const char* szFormat /* = "%Y-%m-%d %H:%M:%S" */)
{
	struct tm atm;
	memset(&atm, 0, sizeof(atm));

	const char* pFormatTag = szFormat;
	const char* pDataPart = szTimeStr;
	bool bValidData = true;

	if (!szTimeStr || !szFormat)
		return MyTime(-1);

	for(;*pFormatTag != '\0' && bValidData;)
	{
		for (;*pFormatTag != '%' && *pFormatTag != '\0';)
		{			
			char ch = *pFormatTag;
			char ch2 = *(pFormatTag + 1);
			
			++pFormatTag;
			++pDataPart;
			if ((ch == '\t' || ch == '\x20') && (ch2 != '\t' && ch2 != '\x20'))
			{
				// 跳过多余的空格
				for (;(*pDataPart == '\x20' || *pDataPart == '\t') && (*pDataPart != '\0'); ++pDataPart);
			}			
			else if ((unsigned char)ch > 0x80)
			{	
				// 跳过多字节字符
				++pFormatTag;
				++pDataPart;
			}
		}

		if (*pFormatTag == '\0')
			break;

		// get a tag, format: %[width](y|m|d|H|M|S|%)
		++pFormatTag;
		int width = -1;
		std::string strTemp;
		for (;*pFormatTag >= '0' && *pFormatTag <= '9'; 
			strTemp += *pFormatTag, ++pFormatTag);

		if (!strTemp.empty())
			width = atoi(strTemp.c_str());

		char flag = *pFormatTag++;	
		if (flag == '%')
		{
			bValidData = (*pDataPart++ == '%');			
			continue;
		}

		strTemp = "";
		for (;*pDataPart >= '0' && *pDataPart <= '9' && (width == -1 || (int)strTemp.length() < width) && *pDataPart != '\0'; 
			strTemp += *pDataPart, ++pDataPart);
		
		if (strTemp.empty())
		{
			bValidData = false;
			continue;
		}
			 
		int val = atoi(strTemp.c_str());
		switch(flag)
		{
		case 'y':		
			atm.tm_year = val + 100; // 2000 + val - 1900
			break;

		case 'Y':
			atm.tm_year = val - 1900;
			break;

		case 'm':			
			atm.tm_mon = val - 1;
			break;

		case 'd':
			atm.tm_mday = val;
			break;

		case 'H':
			atm.tm_hour = val;
			break;

		case 'M':
			atm.tm_min = val;
			break;

		case 'S':
			atm.tm_sec = val;
			break;

		default:
			bValidData = false;
			break;
		}
	}	

	time_t t = -1;
	if (bValidData)
	{
		t = mktime(&atm);
	}

	return MyTime(t);
}

//////////////////////////////////////////////////////////////////////////

}

#endif // __MYTIME_H__

