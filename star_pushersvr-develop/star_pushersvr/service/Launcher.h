#pragma once

#include "IService.h"
#include "define.h"
#include <ace/Event_Handler.h>
#include <vector>
#include <string>

class Launcher :
    public IService, public ACE_Event_Handler
{
public:
    Launcher() : IService("Launcher module") {};

    virtual int init(int argc, char *argv[]);

    virtual void finit();

    virtual int open();

    virtual void close();

protected:
    void printUsage();
    void printVersion();
    void reloadConfig();

private:
    virtual int handle_signal(int signum, siginfo_t * = 0, ucontext_t * = 0);

private:
    std::vector<IService*> m_service;
	
    volatile bool m_ThreadSign = false;
	bool m_Help = false;
	bool m_Ver = false;
    std::string m_ConfigPath = CONFIG_FILE;
};

