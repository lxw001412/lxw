/**
*
* @file 	httpServerError.h
* @author	Bill
* @date		2022.12.3
* @brief
*           http 接口错误码定义
*
**/

#pragma once

enum http_error_code
{
    PUSHERSVR_ERROR_OK = 0,
    PUSHERSVR_ERROR_INVALID_METHOD = 1,
    PUSHERSVR_ERROR_PARAMETER = 100,
    PUSHERSVR_ERROR_STREAM_NOT_EXIST = 101,
    PUSHERSVR_ERROR_UNSUPPORT_STREAM_TYPE = 102,
    PUSHERSVR_ERROR_OP_FAIL = 103,
    PUSHERSVR_ERROR_SYSTEM = 110,
};