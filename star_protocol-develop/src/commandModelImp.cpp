#include "commandModelImp.h"
#include "protocolUtils.h"

namespace star_protocol
{
///////////////////////////////////////////////////////////////////////
    // commandModelRepo
    ///////////////////////////////////////////////////////////////////////
commandModelRepo* commandModelRepo::m_instance = NULL;

commandModelRepo* commandModelRepo::instance()
{
    if (m_instance == NULL)
    {
        m_instance = new commandModelRepo();
    }
    return m_instance;
}

void commandModelRepo::destory()
{
    if (m_instance != NULL)
    {
        delete m_instance;
        m_instance = NULL;
    }
}

commandModelRepo::commandModelRepo()
{

}

commandModelRepo::~commandModelRepo()
{
    m_commandTopicCmdMap.clear();
    for (std::vector<commandModel*>::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
    {
        delete *it;
    }
    m_commands.clear();
}

commandModel* commandModelRepo::getCommandModel(int topic, int cmd)
{
    std::lock_guard<std::mutex> guard(m_commandModelMapMutex);

    TopicCmd topicCmd;
    topicCmd.cmd = cmd;
    topicCmd.topic = topic;
    for (std::map<TopicCmd, commandModel*>::iterator it = m_commandTopicCmdMap.begin(); it != m_commandTopicCmdMap.end(); ++it)
    {
        if ((TopicCmd)it->first == topicCmd)
        {
            return it->second;
        }
    }

    return NULL;
}

int commandModelRepo::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject())
    {
        return -1;
    }

    if (jsonRoot.isMember("commands"))
    {
        const Json::Value jsonCommand = jsonRoot["commands"];
        if (!jsonCommand.isArray())
        {
            return -2;
        }
        for (unsigned int i = 0; i < jsonCommand.size(); ++i)
        {
            commandModel *m = new commandModel();
            if (m->init(jsonCommand[i]) != 0)
            {
                delete m;
                return -3;
            }
            m_commands.push_back(m);
            TopicCmd topicCmd;
            topicCmd.cmd = m->cmd();
            topicCmd.topic = m->topic();
            m_commandTopicCmdMap[topicCmd] = m;
        }
    }
    return 0;
}
int commandModelRepo::addCommandModel(const Json::Value &jsonRoot)
{
    int ret = init(jsonRoot);
    if (ret != 0)
    {
        return ret;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// commandModel
///////////////////////////////////////////////////////////////////////
commandModel::commandModel()
{

}


commandModel::~commandModel()
{
    m_propertyIndexMap.clear();
    m_propertyNameMap.clear();
    for (std::vector<property*>::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
    {
        delete *it;
    }
    m_properties.clear();
}

int commandModel::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject()
        || !jsonRoot.isMember("topic") || !jsonRoot["topic"].isInt()
        || !jsonRoot.isMember("cmd") || !jsonRoot["cmd"].isInt())
    {
        return -1;
    }
    m_topic = jsonRoot["topic"].asInt();
    m_cmd = jsonRoot["cmd"].asInt();

    if (jsonRoot.isMember("message"))
    {
        const Json::Value jsonMessage = jsonRoot["message"];
        if (!jsonMessage.isArray())
        {
            return -2;
        }
        for (unsigned int i = 0; i < jsonMessage.size(); ++i)
        {
            property *p = new property();
            if (p->init(jsonMessage[i]) != 0)
            {
                delete p;
                return -3;
            }
            m_properties.push_back(p);
            m_propertyIndexMap[p->index()] = p;
            m_propertyNameMap[p->indentifer()] = p;
        }
    }
    return 0;
}

property* commandModel::getProperty(int index)
{
    std::map<int, property*>::iterator it = m_propertyIndexMap.find(index);
    if (it == m_propertyIndexMap.end())
    {
        return NULL;
    }
    return it->second;
}

property* commandModel::getProperty(const char* name)
{
    std::map<std::string, property*>::iterator it = m_propertyNameMap.find(name);
    if (it == m_propertyNameMap.end())
    {
        return NULL;
    }
    return it->second;
}

int commandModel::dataB2J(const uint8_t* data, int size, Json::Value &jsonRoot)
{
    int offset = 0;
    while (offset < size)
    {
        int type = *(data + offset);
        offset++;
        int bytes;
        int length = getTlvLength(data + offset, bytes);
        offset += bytes;
        const uint8_t* pData = data + offset;
        offset += length;
        if (offset > size)
        {
            break;
        }
        property* prop = getProperty(type);
        if (prop == NULL)
        {
            continue;
        }
        prop->getDataType()->b2j(prop->indentifer().c_str(), pData, length, jsonRoot);
    }
    return 0;
}

int commandModel::dataJ2B(const Json::Value &jsonRoot, uint8_t* buff, int buffSize, int &length)
{
    int offset = 0;
    Json::Value::Members members = jsonRoot.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
    {
        property* p = getProperty(it->c_str());
        if (NULL == p)
        {
            continue;
        }
        int dataLength = 0;
        if (0 == p->getDataType()->j2b(jsonRoot[*it],
            p->index(), buff + offset, buffSize - offset, dataLength))
        {
            offset += dataLength;
        }
    }

    length = offset;
    return 0;
}

}