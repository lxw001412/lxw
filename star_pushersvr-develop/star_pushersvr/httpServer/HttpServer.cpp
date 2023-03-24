#include "HttpServer.h"
#include <functional>

HttpServer::HttpServer()
{
    svr = new Server;
}

HttpServer::~HttpServer()
{
    delete svr;
}

int HttpServer::open(int port)
{
    if (NULL == svr || !svr->is_valid()) 
    {
        return -1;
    }
    for (auto it = m_path2handler.begin(); it != m_path2handler.end(); it++)
    {
        svr->Get(it->first, (Server::Handler)std::bind(&IHandler::get, it->second, std::placeholders::_1, std::placeholders::_2));
        svr->Post(it->first, (Server::Handler)std::bind(&IHandler::post, it->second, std::placeholders::_1, std::placeholders::_2));
        svr->Delete(it->first, (Server::Handler)std::bind(&IHandler::del, it->second, std::placeholders::_1, std::placeholders::_2));
        svr->Put(it->first, (Server::Handler)std::bind(&IHandler::put, it->second, std::placeholders::_1, std::placeholders::_2));
    }

    svr->set_error_handler([](const Request & /*req*/, Response &res) {
        const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, res.status);
        res.set_content(buf, "text/html");
    });
    svr->listen("0.0.0.0", port);

    return 0;
}

void HttpServer::close()
{
    if (NULL != svr)
    {
        svr->stop();
    }
    m_path2handler.clear();
}

void HttpServer::addHandler(std::string path, IHandler * handler)
{
    m_path2handler[path] = handler;
}

void HttpServer::addRouter(Router * route)
{
    std::map<std::string, IHandler*> handlerMap = route->getHandlerMap();
    m_path2handler.insert(handlerMap.begin(), handlerMap.end());
}