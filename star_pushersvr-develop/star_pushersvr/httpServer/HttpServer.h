/**
*
* @file HttpServer.h
* @brief HTTP服务实现
* @author	chenjie
* @version  0.0.1
* @date		2021.11.26
* 
**/

#pragma once

#include "router.h"
#include "Ihandler.h"

/**
 * @brief 服务接口
 */
class HttpServer :public IHandler
{
public:
	
	HttpServer();
    virtual ~HttpServer();

	/**
	 * @brief 开启http监听
	 * @param port为端口
	 *
	 * @return 0 表示成功，失败时返回其他值
	 */
	int open(int port);

	/**
	 * @brief 关闭http监听
	 * 
	 */
	void close();


	void addHandler(std::string path, IHandler* handler);
    void addRouter(Router* route);

private:
    std::map<std::string, IHandler*> m_path2handler;
    Server* svr;
};