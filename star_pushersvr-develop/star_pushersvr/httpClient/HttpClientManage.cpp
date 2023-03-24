#include "HttpClientManage.h"

int HttpClientManage::open()
{
    m_streamProgress = new HttpClient();
    m_streamStop = new HttpClient();
    m_streamProgress->open();
    m_streamStop->open();
    return 0;
}

int HttpClientManage::close()
{
    if (m_streamProgress != NULL)
    {
        m_streamProgress->close();
        delete m_streamProgress;
    }

    if (m_streamStop != NULL)
    {
        m_streamStop->close();
        delete m_streamStop;
    }

    return 0;
}
