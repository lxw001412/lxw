/**
*
* @file HttpClientService.h
* @brief 接口模块实现类
*	@@@	HTTP Client 模块
* @author	chenjie
* @version 0.0.1
* @date		2021.11.26
* 
**/

#pragma once

#include "IService.h"

class HttpClientService :
	public IService
{
public:
    HttpClientService();
    virtual ~HttpClientService();

	virtual int open() ;

	virtual void close() ;
};
