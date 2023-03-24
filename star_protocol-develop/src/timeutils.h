/**
 * @file timeutils.h
 * @brief  日期时间工具函数
 * @author Bill
 * @version 0.0.1
 * @date 2022-11-17
 */

#pragma once

#include <ctime>

/**
  * @brief 计算指定日期加上指定天数后得到的日期
  *
  * @param yearStart    开始日期，年
  * @param monthStart   开始日期，月
  * @param dayStart     开始日期，日
  * @param days         经过的天数
  * @param year         相加后日期：年
  * @param month        相加后日期：月
  * @param day          相加后日期：日
  *
  * @return 0 - 成功， 1 - 失败
  *
  */
int dateAdd(int yearStart, int monthStart, int dayStart, int days, int &year, int &month, int &day);

/**
  * @brief 计算两个日期相差的天数
  *
  * @param yearStart    开始日期，年
  * @param monthStart   开始日期，月
  * @param dayStart     开始日期，日
  * @param yearEnd      结束日期：年
  * @param monthEnd     结束日期：月
  * @param dayEnd       结束日期：日
  * @param days         从开始日期到结束日期经过的天数
  *
  * @return 0 - 成功， 1 - 失败
  *
  */
int dateDec(int yearStart, int monthStart, int dayStart, int yearEnd, int monthEnd, int dayEnd, int &days);

/**
  * @brief 获取当前时间
  *
  * @param year    年
  * @param month   月
  * @param day     日
  * @param hour    时
  * @param minute  分
  * @param second  秒
  *
  * @return 0 - 成功， 1 - 失败
  *
  */
void currentDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second);


/**
  * @brief 获取当前时间，seconds since 00:00, Jan 1 1970 UTC
  *
  *
  * @return 当前时间
  *
  */
std::time_t currentTime();

/**
  * @brief (UTC)时间转换为日期（本地时间）
  *
  * @param tm      时间，seconds since 00:00, Jan 1 1970 UTC
  * @param year    年
  * @param month   月
  * @param day     日
  *
  * @return 0 - 成功， 1 - 失败
  *
  */
int timetToDate(std::time_t tm, int &year, int &month, int &day);

/**
  * @brief 日期（本地时间）转换为(UTC)时间
  *
  * @param year    年
  * @param month   月
  * @param day     日
  *
  * @return 时间，seconds since 00:00, Jan 1 1970 UTC
  *
  */
std::time_t dateToTimeT(int year, int month, int day);
