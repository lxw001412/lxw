#include "router.h"

Router::Router(std::string path)
{
	m_base  = path;
}

void Router::addHandler(std::string path, IHandler * handler)
{
	m_path2handler[m_base + path] = handler;
}

std::map<std::string, IHandler*> Router::getHandlerMap()
{
	return m_path2handler;
}

void Router::addRouter(Router * route)
{
	std::map<std::string, IHandler*> handlerMap = route->getHandlerMap();
	for (auto it = handlerMap.begin(); it != handlerMap.end(); it++)
	{
		m_path2handler[m_base + it->first] = it->second;
	}
}



