/**
*
* @file InterfaceService.h
* @brief 接口模块实现类
*	@@@	HTTP服务对外得接口
* @author	chenjie
* @version 0.0.1
* @date		2021.11.26
* 
**/

#pragma once

#include "IService.h"

class HttpServer;

class InterfaceService :
	public IService
{
public:
	InterfaceService();
    virtual ~InterfaceService();

	virtual int open() ;

	virtual void close() ;

private:
    void httpListen();

private:
    HttpServer* m_httpSvr = NULL;
};
