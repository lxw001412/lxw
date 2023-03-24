#pragma once
#include "Ihandler.h"
#include <string>
class Router 
{
public:
	Router(std::string path);
	void addHandler(std::string path, IHandler* handler);
	void addRouter(Router* route);
	std::map<std::string, IHandler*> getHandlerMap();
private:
	std::map<std::string, IHandler*> m_path2handler;
	std::string m_base;
};

