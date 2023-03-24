/**
 * @file protocol.h
 * @brief  星广播平台终端协议封装
 * @author Bill
 * @version 0.0.1
 * @date 2021-06-03
 */

#pragma once

#include "productModel.h"
#include "commandModel.h"
#include "termMessage.h"
#include "termPackage.h"

#define STAR_PROTOCOL_VERSION "v0.14"

/**
 * @brief  initProtocolLog 
 *         初始化协议库日志
 * @param logLevel  日志级别：trace,debug,warn,error,critical
 * @param logPath   日志文件路径
 * @param fileNumber 文件数目
 * @param fileSize 文件大小，单位MB
 * @param enableConsoleLog 是否在控制台显示
 *
 * @return  0 成功，其他失败
*/
int initProtocolLog(const char* logLevel,
    const char* logPath,
    int fileNumber,
    int fileSize,
    bool enableConsoleLog);

/**
 * @brief  finiProtocolLog 
 *         释放协议库日志
 *
 * @return  
*/
void finiProtocolLog();