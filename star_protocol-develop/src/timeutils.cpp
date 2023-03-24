#include "timeutils.h"
#include <regex>
#include <chrono>

static std::time_t badTime()
{
    return std::time_t(-1);
}

std::time_t dateToTimeT(int year, int month, int day)
{
    std::tm tmp = std::tm();
    tmp.tm_mday = day;
    tmp.tm_mon = month - 1;
    tmp.tm_year = year - 1900;
    return std::mktime(&tmp);
}

int timetToDate(std::time_t tm, int &year, int &month, int &day)
{
    if (tm == badTime())
    {
        return -1;
    }
#pragma warning(push)
#pragma warning(disable: 4996)
    std::tm *tmp = std::localtime(&tm);
#pragma warning(pop)
    year = tmp->tm_year + 1900;
    month = tmp->tm_mon + 1;
    day = tmp->tm_mday;
    return 0;
}

int dateAdd(int yearStart, int monthStart, int dayStart, int days, int &year, int &month, int &day)
{
    std::time_t tmStart = dateToTimeT(yearStart, monthStart, dayStart);
    std::time_t tmTarget = tmStart + (std::time_t)days * 24 * 60 * 60;
    return timetToDate(tmTarget, year, month, day);
}

int dateDec(int yearStart, int monthStart, int dayStart, int yearEnd, int monthEnd, int dayEnd, int &days)
{
    days = 0;
    std::time_t tmStart = dateToTimeT(yearStart, monthStart, dayStart);
    std::time_t tmEnd = dateToTimeT(yearEnd, monthEnd, dayEnd);
    if (tmEnd < tmStart)
    {
        return -1;
    }
    days = (int)((tmEnd - tmStart) / (24 * 60 * 60));
    return 0;
}

void currentDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second)
{
    std::time_t now = std::time(NULL);

#pragma warning(push)
#pragma warning(disable: 4996)
    std::tm *tmp = std::localtime(&now);
#pragma warning(pop)
    year = tmp->tm_year + 1900;
    month = tmp->tm_mon + 1;
    day = tmp->tm_mday;
    hour = tmp->tm_hour;
    minute = tmp->tm_min;
    second = tmp->tm_sec;
}

std::time_t currentTime()
{
    return std::time(NULL);
}