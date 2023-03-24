/**
 * @file streamModInterface.h
 * @brief 推流模块接口
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-12-02
 * @description 推流模块接口
 */

#include "streamModInterface.h"
#include "streamInfoImp.h"
#include "spdlogging.h"

StreamMng<FileStream>* StreamModInterface::m_fileStreamMng = NULL;
StreamMng<RealTimeStream>* StreamModInterface::m_realStreamMng = NULL;

int StreamModInterface::init()
{
    SPDINFO("[stream-interface] init");
    m_fileStreamMng = new StreamMng<FileStream>();
    m_realStreamMng = new StreamMng<RealTimeStream>();
    if (m_fileStreamMng != NULL && m_realStreamMng != NULL)
    {
        return 0;
    }
    else
    {
        SPDERROR("[stream-interface] memory error");
        return 1;
    }
}

void StreamModInterface::fini()
{
    SPDINFO("[stream-interface] fini");
    if (m_fileStreamMng != NULL)
    {
        delete m_fileStreamMng;
        m_fileStreamMng = NULL;
    }
    if (m_realStreamMng != NULL)
    {
        delete m_realStreamMng;
        m_realStreamMng = NULL;
    }
}
int StreamModInterface::open()
{
    if (NULL == m_fileStreamMng || NULL == m_realStreamMng)
    {
        SPDERROR("[stream-interface] stream mod is not init");
        return 1;
    }

    SPDINFO("[stream-interface] open");
    int rc = m_fileStreamMng->open();
    int rc2 = m_realStreamMng->open();
    if (rc == 0 && rc2 == 0)
    {
        return 0;
    }
    else
    {
        SPDERROR("[stream-interface] stream mng open error: {}, {}", rc, rc2);
        return 1;
    }
}

void StreamModInterface::close()
{
    SPDINFO("[stream-interface] close");
    if (NULL != m_fileStreamMng)
    {
        m_fileStreamMng->close();
    }

    if (NULL != m_realStreamMng)
    {
        m_realStreamMng->close();
    }
}

StreamInfo* StreamModInterface::createStreamInfo()
{
    return new StreamInfoImp();
}